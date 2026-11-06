#include "ChannelOnly.h"
namespace
{
    static constexpr double kPi = 3.14159265358979323846;

    static const int kScaleMasks[29][12] = {
        {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, // 0: MAJOR
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0}, // 1: MINOR
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 2: CHROMATIC
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1}, // 3: HARMONIC MINOR
        {1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1}, // 4: MELODIC MINOR
        {1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0}, // 5: DORIAN
        {1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0}, // 6: PHRYGIAN
        {1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1}, // 7: LYDIAN
        {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0}, // 8: MIXOLYDIAN
        {1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0}, // 9: LOCRIAN
        {1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0}, // 10: PENTATONIC MAJOR
        {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0}, // 11: PENTATONIC MINOR
        {1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0}, // 12: BLUES
        {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}, // 13: WHOLE TONE
        {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1}, // 14: OCTATONIC (W-H)
        {1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0}, // 15: OCTATONIC (H-W)
        {1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0}, // 16: PHRYGIAN DOMINANT
        {1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1}, // 17: HUNGARIAN MINOR
        {1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1}, // 18: DOUBLE HARMONIC (ARABIC)
        {1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0}, // 19: IN SEN (JAPANESE)
        {1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0}, // 20: HIRAJOSHI
        {1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0}, // 21: HIJAZ
        {1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0}, // 22: LYDIAN DOMINANT
        {1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0}, // 23: ALTERED
        {1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0}, // 24: MIXOLYDIAN b6
        {1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1}, // 25: HARMONIC MAJOR
        {1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0}, // 26: BEBOP DOMINANT
        {1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1}, // 27: BEBOP MAJOR
        {1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1}  // 28: RYUKYU (OKINAWAN)
    };
}

//==============================================================================
//==============================================================================
void TAutoTuneChannelDSP::prepare(double newSampleRate)
{
    sampleRate = std::max(8000.0, newSampleRate);

    if (sampleRate > 128000.0)
        downsampleFactor = 4;
    else if (sampleRate > 64000.0)
        downsampleFactor = 2;
    else
        downsampleFactor = 1;

    analysisRate = sampleRate / static_cast<double>(downsampleFactor);
    nominalDelaySamples = static_cast<double>(liveLatencySamples(sampleRate));
    voiceSmoothCoeff = std::exp(-1.0 / (0.003 * sampleRate));
    ratioSmoothCoeff = std::exp(-1.0 / (0.0002 * sampleRate));
    formantSmoothCoeff = std::exp(-1.0 / (0.010 * sampleRate));
    fastPitchCoeff = std::exp(-(5.0 / analysisRate) / 0.00025);
    slowPitchCoeff = std::exp(-(5.0 / analysisRate) / 0.080);

    // 03/11 T Studio: loc chong rang cua (elliptic 12 bac) TU THIET KE bang scipy.signal.ellip(12, 0.1 dB, 80 dB):
    // tan cat 0.42 x tan so phan tich, ripple dai thong 0.1 dB, dai chan <= -80 dB. Moi tang chuan hoa ve 0 dB o DC.
    // Moi hang: { b1/b0, a1, a2 } (tu so doi xung b0 = b2). Chi dung khi sampleRate > 64 kHz (he so ha mau 2 / 4).
    static constexpr double kAntiAlias2x[6][3] = {
        { +1.7769520584, -1.1182046662, +0.3589173490 },
        { +0.7801390001, -0.9273775017, +0.5377930068 },
        { +0.1206681328, -0.7177172049, +0.7352728754 },
        { -0.1819261201, -0.5799076712, +0.8677120626 },
        { -0.3115761592, -0.5096924477, +0.9415388755 },
        { -0.3596258228, -0.4842122983, +0.9835981139 }
    };
    static constexpr double kAntiAlias4x[6][3] = {
        { +1.0695615470, -1.5729855085, +0.6336941439 },
        { -0.7700507141, -1.5682834742, +0.7272661375 },
        { -1.2791106289, -1.5631196157, +0.8379888897 },
        { -1.4413457409, -1.5605632595, +0.9170382741 },
        { -1.5016953401, -1.5617759516, +0.9628957888 },
        { -1.5228670992, -1.5684027820, +0.9895622873 }
    };
    for (int i = 0; i < 6; ++i)
    {
        if (downsampleFactor == 4)
            downsampleFilters[static_cast<size_t>(i)].setSymmetricStage(kAntiAlias4x[i][0], kAntiAlias4x[i][1], kAntiAlias4x[i][2]);
        else
            downsampleFilters[static_cast<size_t>(i)].setSymmetricStage(kAntiAlias2x[i][0], kAntiAlias2x[i][1], kAntiAlias2x[i][2]);
        downsampleFilters[static_cast<size_t>(i)].reset();
    }

    lpcOrder = (sampleRate >= 88200.0) ? 24 : 16;
    for (auto* f : { &formantXoverLP1, &formantXoverLP2 }) f->setLowpass (150.0, sampleRate);
    for (auto* f : { &formantXoverHP1, &formantXoverHP2 }) f->setHighpass (150.0, sampleRate);
    preEmphasisPole = std::exp(-2.0 * kPi * 300.0 / sampleRate);

    currentVoiceType = -1;
    updateVoiceTypeFilters(1);
    reset();
}

void TAutoTuneChannelDSP::reset()
{
    for (auto* f : { &formantXoverLP1, &formantXoverLP2, &formantXoverHP1, &formantXoverHP2 }) f->reset();
    for (auto& f : downsampleFilters)
        f.reset();

    inputLP1.reset();
    inputLP2.reset();
    inputHP1.reset();
    shiftLP1.reset();
    shiftLP2.reset();
    shiftLP1.setLowpass(0.49 * sampleRate, sampleRate, 0.7071067811865475);
    shiftLP2.setLowpass(0.49 * sampleRate, sampleRate, 0.7071067811865475);

    detectorRing.fill(0.0);
    detWriteIdx = 0;

    decRing.fill(0.0);
    decEnergy.fill(0.0);
    decCorr.fill(0.0);
    decWriteIdx = 0;
    decSampleCount = 0;

    trackEnergy.fill(0.0);
    trackCorr.fill(0.0);
    baseLag = 100;
    needAcquisition = true;
    voicedState = false;

    downsampleCounter = 0;
    updateSubCounter = 0;

    currentPeriodSamples = analysisRate / 200.0;
    smoothedPeriodSamples = currentPeriodSamples;
    fastPitchMidi = 60.0;
    slowPitchMidi = 60.0;
    lockedTargetNote = -1;
    sustainedNoteFrames = 0;
    smoothedPitchRatio = 1.0;
    voicedBlend = 0.0;
    resampleRatio = 1.0;
    quietSamples = 0;
    correctedPitchCenter = 60.0;
    naturalVibratoOffset = 0.0;
    naturalVibratoRatio = 1.0;
    correctedPitchInitialised = false;

    onsetConfirmCount = 0;
    onsetPrevEnergy    = 0.0;
    onsetHoldTicks = 0;
    glidePrevMidi       = 60.0;
    glideVelocity      = 0.0;
    glideSignedVelocity     = 0.0;
    onsetSmoothTargetMidi   = 60.0;
    targetVibTimbreDb     = 0.0;
    smoothedVibTimbreDb   = 0.0;
    vibTimbreUpdateTick   = 0;
    vibTimbreEQ.reset();
    vibTimbreEQ.setPeakingEQ(1500.0, sampleRate, 1.41421356, 0.0);

    vibPhase = 0.0;
    vibNoteOnsetSec = 0.0;
    targetVibAmpGain = smoothedVibAmpGain = 1.0;
    targetVibFormantRatio = smoothedVibFormantRatio = 1.0;

    preEmpZ1 = 0.0;
    deEmpZ1 = 0.0;
    lpcAnalysisWin.fill(0.0);
    lpcWinWriteIdx = 0;
    lpcSampleCounter = 0;
    prevLpcA.fill(0.0);
    currLpcA.fill(0.0);
    prevLpcWarpedA.fill(0.0);
    currLpcWarpedA.fill(0.0);
    prevLpcA[0] = 1.0;
    currLpcA[0] = 1.0;
    prevLpcWarpedA[0] = 1.0;
    currLpcWarpedA[0] = 1.0;
    invFilterHist.fill(0.0);
    synFilterHist.fill(0.0);

    rawInputRing.fill(0.0);
    shiftedInputRing.fill(0.0);
    residualRing.fill(0.0);
    psolaWriteIdx = 0;
    relDelayA = nominalDelaySamples;
    relDelayB = relDelayA;
    crossfadeAlpha = 1.0;
    crossfadeStep = 0.0;
    smoothedWetMix = 1.0;
    smoothedFormantMix = 0.0;
    smoothedBypassMix = 0.0;
    formantReferenceEnvelope = 0.0;
    formantRecoverySamples = 0;

    lastDetectedMidi = -1.0f;
    lastTargetMidi = -1.0f;
    lastCentsCorrection = 0.0f;
}

void TAutoTuneChannelDSP::updateVoiceTypeFilters(int voiceType)
{
    if (voiceType == currentVoiceType)
        return;

    currentVoiceType = voiceType;

    // Exact Input Voice Type cutoffs and tracking window half-width W ([rcx+0x1488])
    //   0 = SOPRANO    (fc1 = 600 Hz,  fc2 = 1200 Hz, W = 12..15)
    //   1 = ALTO/TENOR (fc1 = 400 Hz,  fc2 = 800 Hz,  W = 15)
    //   2 = LOW MALE   (fc1 = 300 Hz,  fc2 = 600 Hz,  W = 30)
    //   3 = INSTRUMENT (fc1 = 1000 Hz, fc2 = 2000 Hz, W = 12)
    //   4 = BASS INST  (fc1 = 200 Hz,  fc2 = 400 Hz,  W = 30)
    double lpCutoff1 = 400.0;
    trackHalfWin = 15;
    switch (voiceType)
    {
        case 0: // Soprano
            lpCutoff1 = 600.0;  trackHalfWin = 12;
            break;
        case 1: // Alto / Tenor
            lpCutoff1 = 400.0;  trackHalfWin = 15;
            break;
        case 2: // Low Male
            lpCutoff1 = 300.0;  trackHalfWin = 30;
            break;
        case 3: // Instrument
            lpCutoff1 = 1000.0; trackHalfWin = 12;
            break;
        case 4: // Bass Inst
        default:
            lpCutoff1 = 200.0;  trackHalfWin = 30;
            break;
    }

    const double lpCutoff2 = 2.0 * lpCutoff1;
    inputLP1.setLowpass(lpCutoff1, analysisRate, 0.7071067811865475);
    inputLP2.setLowpass(lpCutoff2, analysisRate, 0.7071067811865475);
    inputHP1.setHighpass(30.0,     analysisRate, 0.7071067811865475);

    minPeriodSamples = std::max(16.0, analysisRate / (lpCutoff2 * 0.95));
    maxPeriodSamples = std::min(1900.0, analysisRate / 45.0);
}

//==============================================================================
//==============================================================================
int TAutoTuneChannelDSP::acquireCoarsePeriod8x(double xFiltered, double trackingThreshold) noexcept
{
    const double x2 = xFiltered * xFiltered;
    decRing[static_cast<size_t>(decWriteIdx)] = xFiltered;

    for (int k = 1; k < kMaxDecLag; ++k)
    {
        const double xLag  = decRing[static_cast<size_t>((decWriteIdx - k) & kDecRingMask)];
        const double x2Lag = decRing[static_cast<size_t>((decWriteIdx - 2 * k) & kDecRingMask)];
        decEnergy[static_cast<size_t>(k)] = std::max(0.0, decEnergy[static_cast<size_t>(k)] + x2 - x2Lag * x2Lag);
        decCorr[static_cast<size_t>(k)]   += xFiltered * xLag - xLag * x2Lag;
    }

    decWriteIdx = (decWriteIdx + 1) & kDecRingMask;
    if (++decSampleCount < 32)
        return 0;

    // Exact periodic refresh of decEnergy & decCorr to eliminate floating-point drift
    if ((decSampleCount & 255) == 0)
    {
        for (int k = 1; k < kMaxDecLag; ++k)
        {
            double e = 0.0, c = 0.0;
            for (int m = 0; m < k; ++m)
            {
                const double s0 = decRing[static_cast<size_t>((decWriteIdx - 1 - m) & kDecRingMask)];
                const double s1 = decRing[static_cast<size_t>((decWriteIdx - 1 - m - k) & kDecRingMask)];
                e += s0 * s0 + s1 * s1;
                c += s0 * s1;
            }
            decEnergy[static_cast<size_t>(k)] = e;
            decCorr[static_cast<size_t>(k)]   = c;
        }
    }

    if (!needAcquisition)
        return 0;

    const int minK = std::max(2, static_cast<int>(std::floor(minPeriodSamples / 8.0)));
    const int maxK = std::min(kMaxDecLag - 2, static_cast<int>(std::ceil(maxPeriodSamples / 8.0)));

    if (decEnergy[static_cast<size_t>(minK)] < 1.0e-6)
        return 0;

    std::array<double, kMaxDecLag> normDiff {};
    normDiff.fill(1.0);
    for (int k = minK - 1; k <= maxK + 1; ++k)
    {
        const double e = decEnergy[static_cast<size_t>(k)];
        if (e > 1.0e-7)
            normDiff[static_cast<size_t>(k)] = (0.5 * e - decCorr[static_cast<size_t>(k)]) / e;
    }

    double minima[4] = { 0.0, 0.0, 0.0, 0.0 };
    double minDepths[4] = { 1.0, 1.0, 1.0, 1.0 };
    int numMinima = 0;

    int k = minK;
    while (k < maxK && normDiff[static_cast<size_t>(k)] > 0.35)
        ++k;

    for (; k < maxK && numMinima < 4; ++k)
    {
        const double ym1 = normDiff[static_cast<size_t>(k - 1)];
        const double y0  = normDiff[static_cast<size_t>(k)];
        const double yp1 = normDiff[static_cast<size_t>(k + 1)];

        if (y0 < ym1 && y0 <= yp1 && y0 < trackingThreshold)
        {
            const double denom = ym1 - 2.0 * y0 + yp1;
            double delta = 0.0;
            if (std::abs(denom) > 1.0e-9)
                delta = std::clamp(0.5 * (ym1 - yp1) / denom, -0.5, 0.5);

            const double interpVal = std::max(0.0, y0 - 0.25 * (ym1 - yp1) * delta);
            if (interpVal < trackingThreshold)
            {
                minima[numMinima] = (static_cast<double>(k) + delta) * 8.0;
                minDepths[numMinima] = interpVal;
                ++numMinima;
            }
        }
    }

    if (numMinima == 0)
        return 0;

    // Filter out shallow sub-period (2nd/3rd harmonic) minima when a much deeper true-period minimum exists
    double globalMinDepth = minDepths[0];
    for (int i = 1; i < numMinima; ++i)
        globalMinDepth = std::min(globalMinDepth, minDepths[i]);

    double cleanMinima[4] = { 0.0, 0.0, 0.0, 0.0 };
    int numClean = 0;
    for (int i = 0; i < numMinima; ++i)
    {
        if (minDepths[i] <= globalMinDepth + 0.04 || minDepths[i] <= 1.8 * globalMinDepth)
            cleanMinima[numClean++] = minima[i];
    }

    double estPeriod = 0.0;
    if (numClean >= 3)
    {
        const double d1 = cleanMinima[0];
        const double d2 = cleanMinima[1] - cleanMinima[0];
        const double d3 = cleanMinima[2] - cleanMinima[1];
        if (std::abs(d1 - d2) < 0.12 * d1 && std::abs(d2 - d3) < 0.12 * d1)
            estPeriod = cleanMinima[2] / 3.0;
        else if (std::abs(d1 - d2) < 0.12 * d1)
            estPeriod = cleanMinima[1] * 0.5;
        else if (cleanMinima[0] >= minPeriodSamples)
            estPeriod = cleanMinima[0];
    }
    else if (numClean == 2)
    {
        const double d1 = cleanMinima[0];
        const double d2 = cleanMinima[1] - cleanMinima[0];
        if (std::abs(d1 - d2) < 0.12 * d1)
            estPeriod = cleanMinima[1] * 0.5;
        else if (cleanMinima[0] >= minPeriodSamples)
            estPeriod = cleanMinima[0];
    }
    else if (numClean == 1 && cleanMinima[0] >= minPeriodSamples)
    {
        estPeriod = cleanMinima[0];
    }

    if (estPeriod < minPeriodSamples || estPeriod > maxPeriodSamples)
        return 0;

    const int newBaseLag = static_cast<int>(std::round(estPeriod)) - trackHalfWin;
    return std::max(4, newBaseLag);
}

//==============================================================================
//==============================================================================
bool TAutoTuneChannelDSP::initTrackingWindow(int newBaseLag) noexcept
{
    baseLag = std::max(4, newBaseLag);
    const int numBins = 2 * trackHalfWin;

    for (int k = 0; k < numBins; ++k)
    {
        const int tau = baseLag + k;
        double e = 0.0;
        double c = 0.0;
        for (int m = 0; m < 2 * tau; ++m)
        {
            const double s = detectorRing[static_cast<size_t>((detWriteIdx - m) & kRingMask)];
            e += s * s;
        }
        for (int m = 0; m < tau; ++m)
        {
            const double s0 = detectorRing[static_cast<size_t>((detWriteIdx - m) & kRingMask)];
            const double s1 = detectorRing[static_cast<size_t>((detWriteIdx - m - tau) & kRingMask)];
            c += s0 * s1;
        }
        trackEnergy[static_cast<size_t>(k)] = e;
        trackCorr[static_cast<size_t>(k)]   = c;
    }

    if (trackEnergy[0] < 1.65e-5)
    {
        needAcquisition = true;
        return false;
    }

    updateSubCounter = 4;
    needAcquisition = false;
    return true;
}

//==============================================================================
//==============================================================================
double TAutoTuneChannelDSP::refineAndSlideTrackingWindow(int& bestIdx) noexcept
{
    const int W = trackHalfWin;
    const int numBins = 2 * W;

    if (bestIdx < W - 1 && baseLag > 4)
    {
        baseLag -= 1;
        bestIdx += 1;
        for (int i = numBins - 1; i >= 1; --i)
        {
            trackEnergy[static_cast<size_t>(i)] = trackEnergy[static_cast<size_t>(i - 1)];
            trackCorr[static_cast<size_t>(i)]   = trackCorr[static_cast<size_t>(i - 1)];
        }
        const int tau0 = baseLag;
        double e0 = 0.0, c0 = 0.0;
        for (int m = 0; m < 2 * tau0; ++m)
        {
            const double s = detectorRing[static_cast<size_t>((detWriteIdx - m) & kRingMask)];
            e0 += s * s;
        }
        for (int m = 0; m < tau0; ++m)
        {
            const double s0 = detectorRing[static_cast<size_t>((detWriteIdx - m) & kRingMask)];
            const double s1 = detectorRing[static_cast<size_t>((detWriteIdx - m - tau0) & kRingMask)];
            c0 += s0 * s1;
        }
        trackEnergy[0] = e0;
        trackCorr[0]   = c0;
    }
    else if (bestIdx > W && (baseLag + numBins) < (kRingSize / 2 - 4))
    {
        baseLag += 1;
        bestIdx -= 1;
        for (int i = 0; i < numBins - 1; ++i)
        {
            trackEnergy[static_cast<size_t>(i)] = trackEnergy[static_cast<size_t>(i + 1)];
            trackCorr[static_cast<size_t>(i)]   = trackCorr[static_cast<size_t>(i + 1)];
        }
        const int tauLast = baseLag + numBins - 1;
        double eL = 0.0, cL = 0.0;
        for (int m = 0; m < 2 * tauLast; ++m)
        {
            const double s = detectorRing[static_cast<size_t>((detWriteIdx - m) & kRingMask)];
            eL += s * s;
        }
        for (int m = 0; m < tauLast; ++m)
        {
            const double s0 = detectorRing[static_cast<size_t>((detWriteIdx - m) & kRingMask)];
            const double s1 = detectorRing[static_cast<size_t>((detWriteIdx - m - tauLast) & kRingMask)];
            cL += s0 * s1;
        }
        trackEnergy[static_cast<size_t>(numBins - 1)] = eL;
        trackCorr[static_cast<size_t>(numBins - 1)]   = cL;
    }

    const int idx = std::clamp(bestIdx, 1, numBins - 2);
    const double em1 = std::max(1.0e-12, trackEnergy[static_cast<size_t>(idx - 1)]);
    const double e0  = std::max(1.0e-12, trackEnergy[static_cast<size_t>(idx)]);
    const double ep1 = std::max(1.0e-12, trackEnergy[static_cast<size_t>(idx + 1)]);
    const double ym1 = (0.5 * em1 - trackCorr[static_cast<size_t>(idx - 1)]) / em1;
    const double y0  = (0.5 * e0  - trackCorr[static_cast<size_t>(idx)])     / e0;
    const double yp1 = (0.5 * ep1 - trackCorr[static_cast<size_t>(idx + 1)]) / ep1;

    const double denom = ym1 - 2.0 * y0 + yp1;
    double delta = 0.0;
    if (std::abs(denom) > 1.0e-9)
        delta = std::clamp(0.5 * (ym1 - yp1) / denom, -0.5, 0.5);

    return static_cast<double>(baseLag + idx) + delta;
}

//==============================================================================
// Kelly-Lochbaum Vocal Tract Tube + Levinson-Durbin LPC Formant & Throat Modeling
//==============================================================================
void TAutoTuneChannelDSP::analyzeLPCAndThroat(double pitchPeriodSamples, double effectiveThroatRatio) noexcept
{
    prevLpcA = currLpcA;
    prevLpcWarpedA = currLpcWarpedA;

    const int N = 512;
    const int p = lpcOrder;
    std::array<double, 512> winSig {};

    for (int n = 0; n < N; ++n)
    {
        const int idx = (lpcWinWriteIdx - N + n + 512) & 511;
        const double w = 0.5 * (1.0 - std::cos(2.0 * kPi * static_cast<double>(n) / static_cast<double>(N - 1)));
        winSig[static_cast<size_t>(n)] = lpcAnalysisWin[static_cast<size_t>(idx)] * w;
    }

    std::array<double, kMaxLPCOrder + 1> R {};
    const double T0 = std::clamp(pitchPeriodSamples * downsampleFactor, 20.0, 1000.0);
    for (int k = 0; k <= p; ++k)
    {
        double sum = 0.0;
        for (int n = k; n < N; ++n)
            sum += winSig[static_cast<size_t>(n)] * winSig[static_cast<size_t>(n - k)];

        double lagWin = 1.0;
        if (k > 0)
        {
            const double x = (static_cast<double>(k) * kPi) / T0;
            const double sinc = std::sin(x) / x;
            lagWin = sinc * sinc;
        }
        R[static_cast<size_t>(k)] = sum * lagWin;
    }

    if (R[0] < 1.0e-8)
    {
        currLpcA.fill(0.0);
        currLpcWarpedA.fill(0.0);
        currLpcA[0] = 1.0;
        currLpcWarpedA[0] = 1.0;
        return;
    }

    // Diagonal loading for unconditionally stable all-pole synthesis
    R[0] *= 1.005;

    std::array<double, kMaxLPCOrder + 1> a {};
    std::array<double, kMaxLPCOrder + 1> aPrev {};
    std::array<double, kMaxLPCOrder + 1> kRefl {};
    a[0] = 1.0;
    double E = R[0];

    for (int i = 1; i <= p; ++i)
    {
        double acc = R[static_cast<size_t>(i)];
        for (int j = 1; j < i; ++j)
            acc += a[static_cast<size_t>(j)] * R[static_cast<size_t>(i - j)];

        double ki = -acc / std::max(E, 1.0e-12);
        ki = std::clamp(ki, -0.97, 0.97);
        kRefl[static_cast<size_t>(i)] = ki;

        aPrev = a;
        a[static_cast<size_t>(i)] = ki;
        for (int j = 1; j < i; ++j)
            a[static_cast<size_t>(j)] = aPrev[static_cast<size_t>(j)] + ki * aPrev[static_cast<size_t>(i - j)];

        E *= (1.0 - ki * ki);
        if (E <= 1.0e-12)
            break;
    }

    // Bandwidth expansion (gamma = 0.990^i) to prevent sharp formant ringing on loud vocals
    double gammaPow = 1.0;
    for (int i = 1; i <= p; ++i)
    {
        gammaPow *= 0.990;
        a[static_cast<size_t>(i)] *= gammaPow;
    }
    currLpcA = a;

    std::array<double, kMaxLPCOrder + 1> tubeArea {};
    tubeArea[0] = 1.0;
    for (int i = 1; i <= p; ++i)
    {
        const double ki = std::clamp(kRefl[static_cast<size_t>(i)], -0.97, 0.97);
        tubeArea[static_cast<size_t>(i)] = tubeArea[static_cast<size_t>(i - 1)] * ((1.0 - ki) / (1.0 + ki));
    }

    std::array<double, kMaxLPCOrder + 1> warpedArea {};
    warpedArea[0] = tubeArea[0];
    const double stretch = std::clamp(effectiveThroatRatio, 0.50, 1.90);

    for (int i = 1; i <= p; ++i)
    {
        const double srcPos = std::clamp(static_cast<double>(i) / stretch, 0.0, static_cast<double>(p));
        const int idx0 = static_cast<int>(std::floor(srcPos));
        const int idx1 = std::min(p, idx0 + 1);
        const double frac = srcPos - static_cast<double>(idx0);
        warpedArea[static_cast<size_t>(i)] =
            tubeArea[static_cast<size_t>(idx0)] * (1.0 - frac) + tubeArea[static_cast<size_t>(idx1)] * frac;
    }

    std::array<double, kMaxLPCOrder + 1> aWarped {};
    aWarped[0] = 1.0;
    double origGainProd = 1.0;
    double warpedGainProd = 1.0;

    for (int i = 1; i <= p; ++i)
    {
        const double Aprev = std::max(1.0e-9, warpedArea[static_cast<size_t>(i - 1)]);
        const double Acurr = std::max(1.0e-9, warpedArea[static_cast<size_t>(i)]);
        double kw = (Aprev - Acurr) / (Aprev + Acurr);
        kw = std::clamp(kw, -0.97, 0.97);

        const double ki = std::clamp(kRefl[static_cast<size_t>(i)], -0.97, 0.97);
        origGainProd   *= (1.0 - ki * ki);
        warpedGainProd *= (1.0 - kw * kw);

        aPrev = aWarped;
        aWarped[static_cast<size_t>(i)] = kw;
        for (int j = 1; j < i; ++j)
            aWarped[static_cast<size_t>(j)] = aPrev[static_cast<size_t>(j)] + kw * aPrev[static_cast<size_t>(i - j)];
    }

    gammaPow = 1.0;
    for (int i = 1; i <= p; ++i)
    {
        gammaPow *= 0.990;
        aWarped[static_cast<size_t>(i)] *= gammaPow;
    }
    aWarped[0] = std::clamp(std::pow(warpedGainProd / std::max(1.0e-9, origGainProd), 0.25), 0.5, 1.5);
    currLpcWarpedA = aWarped;
}

double TAutoTuneChannelDSP::inverseFilterLPC(double x, double alpha) noexcept
{
    const double wPrev = 1.0 - alpha;
    double res = x;
    for (int i = 1; i <= lpcOrder; ++i)
    {
        const double ai = wPrev * prevLpcA[static_cast<size_t>(i)] + alpha * currLpcA[static_cast<size_t>(i)];
        res += ai * invFilterHist[static_cast<size_t>(i)];
    }

    for (int i = lpcOrder; i >= 2; --i)
        invFilterHist[static_cast<size_t>(i)] = invFilterHist[static_cast<size_t>(i - 1)];
    invFilterHist[1] = x;
    return res;
}

double TAutoTuneChannelDSP::synthesizeThroatLPC(double excitation, double alpha) noexcept
{
    const double wPrev = 1.0 - alpha;
    const double gGain = wPrev * prevLpcWarpedA[0] + alpha * currLpcWarpedA[0];
    double y = excitation * gGain;
    for (int i = 1; i <= lpcOrder; ++i)
    {
        const double ai = wPrev * prevLpcWarpedA[static_cast<size_t>(i)] + alpha * currLpcWarpedA[static_cast<size_t>(i)];
        y -= ai * synFilterHist[static_cast<size_t>(i)];
    }

    if (!std::isfinite(y) || std::abs(y) > 4.0)
        y = std::clamp(excitation, -1.5, 1.5);

    for (int i = lpcOrder; i >= 2; --i)
        synFilterHist[static_cast<size_t>(i)] = synFilterHist[static_cast<size_t>(i - 1)];
    synFilterHist[1] = y;
    return y;
}

// C1-Continuous 4-Point Cubic Hermite (Catmull-Rom) Sub-Sample Interpolation
double TAutoTuneChannelDSP::readCubicHermite(const std::array<double, kRingSize>& buf, double readPos) const noexcept
{
    const int idx0 = static_cast<int>(std::floor(readPos));
    const double t = readPos - static_cast<double>(idx0);

    const double ym1 = buf[static_cast<size_t>((idx0 - 1) & kRingMask)];
    const double y0  = buf[static_cast<size_t>(idx0 & kRingMask)];
    const double y1  = buf[static_cast<size_t>((idx0 + 1) & kRingMask)];
    const double y2  = buf[static_cast<size_t>((idx0 + 2) & kRingMask)];

    const double c0 = y0;
    const double c1 = 0.5 * (y1 - ym1);
    const double c2 = ym1 - 2.5 * y0 + 2.0 * y1 - 0.5 * y2;
    const double c3 = 0.5 * (y2 - ym1) + 1.5 * (y0 - y1);

    return ((c3 * t + c2) * t + c1) * t + c0;
}

// Phase-Locked TD-PSOLA Splice Alignment around nominal +/- T0Full jump
double TAutoTuneChannelDSP::findBestSpliceDelay(const std::array<double, kRingSize>& buf,
                                                      double currentDelay,
                                                      double nominalTargetDelay,
                                                      double T0Full) const noexcept
{
    const int searchRange = std::clamp(static_cast<int>(0.08 * T0Full), 2, 16);
    const int matchLen = 16;
    double bestCandidate = nominalTargetDelay;
    double minErr = 1.0e18;

    const double curReadPos = static_cast<double>(psolaWriteIdx) - currentDelay;
    for (int offset = -searchRange; offset <= searchRange; ++offset)
    {
        const double candDelay = nominalTargetDelay + static_cast<double>(offset);
        if (candDelay < 24.0 || candDelay > static_cast<double>(kRingSize - 64))
            continue;
        const double candReadPos = static_cast<double>(psolaWriteIdx) - candDelay;
        double err = 0.0;
        double localEnergy = 0.0;
        for (int m = 0; m < matchLen; ++m)
        {
            const int iA = (static_cast<int>(std::floor(curReadPos))  + m) & kRingMask;
            const int iB = (static_cast<int>(std::floor(candReadPos)) + m) & kRingMask;
            const double sA = buf[static_cast<size_t>(iA)];
            const double sB = buf[static_cast<size_t>(iB)];
            const double d = sA - sB;
            err += d * d;
            localEnergy += sA * sA + sB * sB;
        }
        // Uu tien noi o vung nang luong thap giua 2 xung thanh quan (khong cat ngang dinh xung).
        err *= (1.0 + 0.14 * localEnergy / (1.0e-5 + localEnergy));
        if (err < minErr)
        {
            minErr = err;
            bestCandidate = candDelay;
        }
    }
    return bestCandidate;
}

//==============================================================================
//==============================================================================
float TAutoTuneChannelDSP::processSample(
    float inputSample,
    int voiceType,
    int keyIdx,
    int scaleIdx,
    const std::array<int, 12>& noteStates,
    const std::array<int, 12>& noteCents,
    bool classicMode,
    bool useFormants,
    double throatLengthPct,
    bool ratioLock,
    double transposeSemi,
    double detuneCents,
    double trackingVal,
    double retuneSpeedMs,
    double flexTuneVal,
    double naturalVibDb,
    double humanizeVal,
    bool ignoreVibrato,
    int vibShape,
    double vibRateHz,
    double vibDelayMs,
    double vibTransMs,
    double vibVarVal,
    double vibPitchCents,
    double vibAmpVal,
    double vibFormantVal,
    double wetDryMixPct,
    int frozenMidiNote,
    bool bypass)
{
    // v1.2.1: a NaN/Inf sample from the host (or an upstream plugin) must never reach
    // the pitch tracker: it used to poison the delay line state for good.
    if (! std::isfinite(inputSample))
        inputSample = 0.0f;
    (void)keyIdx;
    (void)scaleIdx;
    updateVoiceTypeFilters(voiceType);
    const double xIn = static_cast<double>(inputSample);

    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    double xAA = xIn;
    if (downsampleFactor > 1)
    {
        for (auto& f : downsampleFilters)
            xAA = f.process(xAA);
    }

    // Control values persist between analysis ticks. Resetting them on every
    // audio sample produced an impulse train when synthetic vibrato was on.
    if (vibShape <= 0 || (vibPitchCents <= 0.0 && vibAmpVal <= 0.0 && vibFormantVal <= 0.0))
    {
        targetVibFormantRatio = 1.0;
        targetVibAmpGain = 1.0;
        targetVibTimbreDb = 0.0;
    }

    if (++downsampleCounter >= downsampleFactor)
    {
        downsampleCounter = 0;

        const double xFiltered = inputHP1.process(inputLP2.process(inputLP1.process(xAA)));
        detectorRing[static_cast<size_t>(detWriteIdx)] = xFiltered;

        const double trackingThresh = std::clamp(0.32 - 0.0022 * (100.0 - trackingVal), 0.08, 0.35);

        const bool wasAcquiring = needAcquisition;
        if ((detWriteIdx & 7) == 0)
        {
            const int coarseLag = acquireCoarsePeriod8x(xFiltered, trackingThresh);
            if (wasAcquiring && coarseLag > 0)
                initTrackingWindow(coarseLag);
        }

        if (!wasAcquiring)
        {
            const int numBins = 2 * trackHalfWin;
            if ((detWriteIdx & 1023) == 0)
            {
                for (int k = 0; k < numBins; ++k)
                {
                    const int tau = baseLag + k;
                    double e = 0.0, c = 0.0;
                    for (int m = 0; m < 2 * tau; ++m)
                    {
                        const double s = detectorRing[static_cast<size_t>((detWriteIdx - m) & kRingMask)];
                        e += s * s;
                    }
                    for (int m = 0; m < tau; ++m)
                    {
                        const double s0 = detectorRing[static_cast<size_t>((detWriteIdx - m) & kRingMask)];
                        const double s1 = detectorRing[static_cast<size_t>((detWriteIdx - m - tau) & kRingMask)];
                        c += s0 * s1;
                    }
                    trackEnergy[static_cast<size_t>(k)] = e;
                    trackCorr[static_cast<size_t>(k)]   = c;
                }
            }
            else
            {
                const double xn2 = xFiltered * xFiltered;
                for (int k = 0; k < numBins; ++k)
                {
                    const int tau = baseLag + k;
                    const double xTau  = detectorRing[static_cast<size_t>((detWriteIdx - tau) & kRingMask)];
                    const double x2Tau = detectorRing[static_cast<size_t>((detWriteIdx - 2 * tau) & kRingMask)];
                    trackEnergy[static_cast<size_t>(k)] =
                        std::max(0.0, trackEnergy[static_cast<size_t>(k)] + xn2 - x2Tau * x2Tau);
                    trackCorr[static_cast<size_t>(k)] += xFiltered * xTau - xTau * x2Tau;
                }
            }

            if (++updateSubCounter >= 5)
            {
                updateSubCounter = 0;

                int bestIdx = 0;
                const double eInit = std::max(1.0e-12, trackEnergy[0]);
                double minNormVal = (0.5 * eInit - trackCorr[0]) / eInit;
                for (int k = 1; k < numBins; ++k)
                {
                    const double ek = std::max(1.0e-12, trackEnergy[static_cast<size_t>(k)]);
                    const double normVal = (0.5 * ek - trackCorr[static_cast<size_t>(k)]) / ek;
                    if (normVal < minNormVal)
                    {
                        minNormVal = normVal;
                        bestIdx = k;
                    }
                }

                const double bestEnergy = trackEnergy[static_cast<size_t>(bestIdx)];

                if (bestIdx == 0 || bestIdx == numBins - 1
                    || bestEnergy < 1.5e-5
                    || minNormVal > trackingThresh)
                {
                    needAcquisition = true;
                    voicedState = false;
                    naturalVibratoOffset = 0.0;
                    naturalVibratoRatio = 1.0;
                    correctedPitchInitialised = false;
                    targetVibAmpGain = targetVibFormantRatio = 1.0;
                    targetVibTimbreDb = 0.0;
                    sustainedNoteFrames = 0;
                    onsetConfirmCount = 0;
                    onsetPrevEnergy = bestEnergy;
                    onsetHoldTicks = 0;
                    glideVelocity = 0.0;
                    glideSignedVelocity = 0.0;
                    smoothedPitchRatio = 0.92 * smoothedPitchRatio + 0.08 * 1.0;
                    lastCentsCorrection *= 0.85f;
                }
                else
                {
                    const double detectedPeriod = refineAndSlideTrackingWindow(bestIdx);
                    currentPeriodSamples = std::clamp(detectedPeriod, minPeriodSamples, maxPeriodSamples);
                    smoothedPeriodSamples = voicedState
                        ? (0.70 * smoothedPeriodSamples + 0.30 * currentPeriodSamples)
                        : currentPeriodSamples;

                    const double refA4Hz = 440.0 * std::pow(2.0, detuneCents / 1200.0);
                    const double detectedHz = analysisRate / smoothedPeriodSamples;
                    const double rawMidi = 69.0 + 12.0 * std::log2(std::max(20.0, detectedHz) / refA4Hz);

                    const double fUpdateHz = analysisRate / 5.0;
                    if (!voicedState)
                    {
                        fastPitchMidi = rawMidi;
                        slowPitchMidi = rawMidi;
                        voicedState = true;
                        onsetConfirmCount = 1;
                        glidePrevMidi = rawMidi;
                        glideVelocity = 0.0;
                        glideSignedVelocity = 0.0;
                        onsetSmoothTargetMidi = rawMidi;
                    }
                    else
                    {
                        fastPitchMidi = fastPitchCoeff * fastPitchMidi + (1.0 - fastPitchCoeff) * rawMidi;
                        slowPitchMidi = slowPitchCoeff * slowPitchMidi + (1.0 - slowPitchCoeff) * rawMidi;
                        onsetConfirmCount = std::min(64, onsetConfirmCount + 1);
                        const double signedVel = (fastPitchMidi - glidePrevMidi) * fUpdateHz;
                        glidePrevMidi = fastPitchMidi;
                        if (std::abs(signedVel) < 80.0)
                        {
                            glideSignedVelocity = 0.85 * glideSignedVelocity + 0.15 * signedVel;
                            glideVelocity  = std::abs(glideSignedVelocity);
                        }
                    }

                    // Giu dau chu: am luong tang vot (phu am bat -> nguyen am) luc moi vao not -> tam chua keo
                    if (onsetPrevEnergy > 1.0e-5 && bestEnergy > 2.4 * onsetPrevEnergy && onsetConfirmCount <= 12)
                        onsetHoldTicks = 6;
                    else if (onsetHoldTicks > 0)
                        --onsetHoldTicks;
                    onsetPrevEnergy = bestEnergy;

                    lastDetectedMidi = static_cast<float>(fastPitchMidi);

                    const double pitchForTarget = ignoreVibrato ? slowPitchMidi : fastPitchMidi;

                    int bestMidiNote = static_cast<int>(std::round(pitchForTarget));
                    double minDist = 1.0e9;
                    bool bestIsBypassed = true;

                    if (frozenMidiNote >= 0)
                    {
                        bestMidiNote = frozenMidiNote;
                        bestIsBypassed = false;
                    }
                    else
                    {
                        const int baseMidi = static_cast<int>(std::floor(pitchForTarget)) - 12;
                        for (int m = baseMidi; m <= baseMidi + 24; ++m)
                        {
                            const int pc = ((m % 12) + 12) % 12;
                            const int st = noteStates[static_cast<size_t>(pc)];
                            if (st == TAutoTuneProcessor::kNoteRemoved)
                                continue;

                            const double noteWithMicroCents = static_cast<double>(m) + 0.01 * static_cast<double>(noteCents[static_cast<size_t>(pc)]);
                            double dist = std::abs(pitchForTarget - noteWithMicroCents);
                            // Giu not: not khac phai hon not dang giu 0.2 cung cho moi cung khoang cach
                            // (0.4 voi not cach 1 cung, 0.2 voi not cach nua cung). Truoc (0.15 co dinh)
                            // ca si dung giua 2 not cua scale lam not dich nhay qua lai -> nghe lac / rung.
                            // Do tren giong that: lac 132 -> 80 (Retune 20), giat 65 -> 38 (hard tune).
                            if (lockedTargetNote >= 0 && m != lockedTargetNote)
                            {
                                double hystScale = 1.0;
                                const double dir = static_cast<double>(m - lockedTargetNote);
                                if (dir * glideSignedVelocity > 10.0)
                                    hystScale = std::clamp(1.0 - (std::abs(glideSignedVelocity) - 10.0) / 25.0, 0.35, 1.0);
                                dist += 0.2 * hystScale * std::min(2.0, std::abs(dir));
                            }

                            if (dist < minDist)
                            {
                                minDist = dist;
                                bestMidiNote = m;
                                bestIsBypassed = (st == TAutoTuneProcessor::kNoteBypassed);
                            }
                        }
                    }

                    if (bestMidiNote == lockedTargetNote)
                        sustainedNoteFrames++;
                    else
                    {
                        lockedTargetNote = bestMidiNote;
                        sustainedNoteFrames = 0;
                        vibNoteOnsetSec = 0.0;
                        slowPitchMidi = fastPitchMidi;
                        correctedPitchInitialised = false;
                    }

                    const int bestPc = ((bestMidiNote % 12) + 12) % 12;
                    const double microOffsetSemi = 0.01 * static_cast<double>(noteCents[static_cast<size_t>(bestPc)]);
                    const double exactNoteMidi = static_cast<double>(bestMidiNote) + microOffsetSemi;

                    // 03/11: KHONG lam muot dich them lan nua. Do tren giong that (Retune 25): lam muot dich
                    // + Retune cong don lam not cuoi it chuan hon (p90 44 -> 36 cent khi bo) va dau chu bi
                    // keo lech (21.6 -> 10.3 cent trong 40 ms dau). Retune / Humanize da lo phan mem mai.
                    onsetSmoothTargetMidi = exactNoteMidi;
                    double targetMidi = bestIsBypassed ? fastPitchMidi : exactNoteMidi;

                    if (!bestIsBypassed && !classicMode && flexTuneVal > 0.0 && frozenMidiNote < 0)
                    {
                        const double flexNorm = std::clamp(flexTuneVal / 100.0, 0.0, 1.0);
                        const double devSemi = fastPitchMidi - exactNoteMidi;
                        const double captureWidth = 0.15 + 0.85 * (1.0 - flexNorm);
                        const double pull = std::exp(-(devSemi * devSemi) / std::max(0.02, captureWidth * captureWidth));
                        const double effectivePull = pull * (1.0 - 0.75 * flexNorm);
                        targetMidi = fastPitchMidi + effectivePull * (exactNoteMidi - fastPitchMidi);
                    }

                    // 03/11: tach RUNG that (lech <= ~0.6 cung quanh tam) voi LUYEN / vuot not (lech lon hon):
                    // phan vuot nguong giam dan theo duong chuong (rong 0.3 cung) -> Humanize khong giu nham cu vuot not.
                    if (frozenMidiNote < 0)
                    {
                        const double vibComponent = fastPitchMidi - slowPitchMidi;
                        const double excessExcursion = std::max(0.0, std::abs(vibComponent) - kVibGateStartSemi);
                        const double vibThresholdGate = std::exp(-0.5 * (excessExcursion / kVibGateWidthSemi) * (excessExcursion / kVibGateWidthSemi));
                        const double heldFraction = std::clamp(
                            sustainedNoteFrames * (5.0 / analysisRate) / 0.4, 0.0, 1.0);
                        const double preserve = (!classicMode && !ignoreVibrato && !bestIsBypassed)
                            ? std::clamp(humanizeVal / 100.0, 0.0, 1.0) * heldFraction * vibThresholdGate : 0.0;
                        targetMidi += vibComponent * preserve;
                    }

                    const double activeTranspose = classicMode ? 0.0 : transposeSemi;

                    // Synthetic Vibrato LFO + 1.5 kHz timbre modulation (Vibrato Formant)
                    double synVibSemi = 0.0;
                    const double dtControl = 5.0 / analysisRate;
                    vibNoteOnsetSec += dtControl;

                    if (vibShape > 0 && (vibPitchCents > 0.0 || vibAmpVal > 0.0 || vibFormantVal > 0.0))
                    {
                        const double delaySec = vibDelayMs * 0.001;
                        const double transSec = std::max(0.001, vibTransMs * 0.001);
                        double env = 0.0;
                        if (vibNoteOnsetSec > delaySec)
                            env = std::clamp((vibNoteOnsetSec - delaySec) / transSec, 0.0, 1.0);

                        rngState = rngState * 1664525u + 1013904223u;
                        const double randBipolar = (static_cast<double>(rngState & 0xFFFFu) / 32768.0) - 1.0;
                        const double instRate = std::max(0.1, vibRateHz * (1.0 + 0.0015 * vibVarVal * randBipolar));
                        vibPhase += instRate * dtControl;
                        vibPhase -= std::floor(vibPhase);

                        double lfo = 0.0;
                        if (vibShape == 1)
                            lfo = std::sin(2.0 * kPi * vibPhase);
                        else if (vibShape == 2)
                            lfo = (vibPhase < 0.5) ? 1.0 : -1.0;
                        else
                            lfo = 2.0 * vibPhase - 1.0;

                        synVibSemi = env * (vibPitchCents / 100.0) * lfo;
                        targetVibFormantRatio = 1.0 + env * (vibFormantVal / 100.0) * 0.06 * lfo;
                        targetVibAmpGain = 1.0 + env * (vibAmpVal / 100.0) * 0.18 * lfo;
                        targetVibTimbreDb = env * (vibFormantVal / 100.0) * 2.5 * lfo;
                    }

                    const double finalTargetMidi = targetMidi + activeTranspose + synVibSemi;
                    const double rawTargetRatio = std::pow(2.0, (finalTargetMidi - fastPitchMidi) / 12.0);

                    const double fUpdate = analysisRate / 5.0;
                    const double retuneCoeff = (retuneSpeedMs <= 0.001)
                        ? 0.0
                        : std::pow(0.5, 1.0 / std::max(1.0, fUpdate * (retuneSpeedMs * 0.001)));

                    double smoothCoeff = retuneCoeff;
                    if (humanizeVal > 0.0)
                    {
                        const double humanizeExtraMs = (humanizeVal / 100.0) * 250.0;
                        const double humanizeCoeff = std::pow(
                            0.5, 1.0 / std::max(1.0, fUpdate * ((retuneSpeedMs + humanizeExtraMs) * 0.001)));
                        const int rampFrames = std::max(1, static_cast<int>((analysisRate * 0.4) / 5.0));
                        const double rampFrac = std::clamp(
                            static_cast<double>(sustainedNoteFrames) / static_cast<double>(rampFrames), 0.0, 1.0);
                        smoothCoeff = retuneCoeff + rampFrac * (humanizeCoeff - retuneCoeff);
                    }

                    double onsetPullWeight = 1.0;
                    if (onsetConfirmCount <= 4 || onsetHoldTicks > 0)
                    {
                        onsetPullWeight = 0.0; // vai lan do dau tien / am luong tang vot: chua keo
                    }
                    else if (onsetConfirmCount < 28)
                    {
                        const double ramp = static_cast<double>(onsetConfirmCount - 4) / 24.0;
                        onsetPullWeight *= ramp * ramp * (3.0 - 2.0 * ramp); // Smoothstep onset fade-in
                    }
                    if (retuneSpeedMs > 1.0 && glideVelocity > 6.0 && sustainedNoteFrames < 60)
                    {
                        const double velDamp = 1.0 / (1.0 + 0.12 * (glideVelocity - 6.0));
                        onsetPullWeight *= std::clamp(velDamp, 0.22, 1.0);
                    }
                    const double effectiveStep = (1.0 - smoothCoeff) * onsetPullWeight;
                    smoothedPitchRatio += effectiveStep * (rawTargetRatio - smoothedPitchRatio);
                    if (onsetPullWeight <= 0.0)
                        smoothedPitchRatio = 1.0 + 0.8 * (smoothedPitchRatio - 1.0);   // dau chu di nguyen ban
                    smoothedPitchRatio = std::clamp(smoothedPitchRatio, 0.5, 2.0);
                    if (! std::isfinite(smoothedPitchRatio))
                        smoothedPitchRatio = 1.0;
                    // Scale the vibrato that remains AFTER correction (same vibrato / slide gate as above)
                    const double correctedPitch = fastPitchMidi
                        + 12.0 * std::log2(smoothedPitchRatio);
                    if (!correctedPitchInitialised)
                    {
                        correctedPitchCenter = correctedPitch;
                        correctedPitchInitialised = true;
                    }
                    correctedPitchCenter = slowPitchCoeff * correctedPitchCenter
                                         + (1.0 - slowPitchCoeff) * correctedPitch;
                    const double remVibDev = correctedPitch - correctedPitchCenter;
                    const double remExcess = std::max(0.0, std::abs(remVibDev) - kVibGateStartSemi);
                    const double remVibGate = std::exp(-0.5 * (remExcess / kVibGateWidthSemi) * (remExcess / kVibGateWidthSemi));
                    naturalVibratoOffset = (frozenMidiNote < 0 && !ignoreVibrato)
                        ? (std::pow(10.0, naturalVibDb / 20.0) - 1.0)
                            * remVibDev * remVibGate : 0.0;
                    naturalVibratoRatio = std::pow(2.0, naturalVibratoOffset / 12.0);

                    lastTargetMidi = static_cast<float>(finalTargetMidi - activeTranspose);
                    lastCentsCorrection = static_cast<float>(
                        std::clamp(1200.0 * std::log2(smoothedPitchRatio * naturalVibratoRatio) - activeTranspose * 100.0, -2400.0, 2400.0));

                    const double shiftCutoff = (smoothedPitchRatio > 1.0)
                        ? ((0.49 * sampleRate) / smoothedPitchRatio)
                        : (0.49 * sampleRate);
                    shiftLP1.setLowpass(shiftCutoff, sampleRate, 0.7071067811865475);
                    shiftLP2.setLowpass(shiftCutoff, sampleRate, 0.7071067811865475);
                }
            }
        }

        detWriteIdx = (detWriteIdx + 1) & kRingMask;
    }
    else if (needAcquisition)
    {
        if (++updateSubCounter >= 5)
        {
            updateSubCounter = 0;
            smoothedPitchRatio = 0.92 * smoothedPitchRatio + 0.08 * 1.0;
            lastCentsCorrection *= 0.85f;
        }
    }

    // -------------------------------------------------------------------------
    // 2. Pre-Emphasis & Sample-by-Sample Interpolated LPC / Throat Modeling
    // -------------------------------------------------------------------------
    const bool formantsActive = (!classicMode && useFormants);
    if (formantRecoverySamples > 0)
        --formantRecoverySamples;
    smoothedFormantMix = formantSmoothCoeff * smoothedFormantMix
                      + (1.0 - formantSmoothCoeff)
                        * (formantsActive && formantRecoverySamples == 0 ? 1.0 : 0.0);
    const bool formantsRunning = formantsActive || smoothedFormantMix > 1.0e-7;
    smoothedVibAmpGain = voiceSmoothCoeff * smoothedVibAmpGain
                      + (1.0 - voiceSmoothCoeff) * targetVibAmpGain;
    smoothedVibFormantRatio = voiceSmoothCoeff * smoothedVibFormantRatio
                           + (1.0 - voiceSmoothCoeff) * targetVibFormantRatio;
    // -------------------------------------------------------------------------
    // 3. Continuous Upward-Shift Anti-Aliasing Biquads
    // -------------------------------------------------------------------------
    const double xShiftFiltered = shiftLP2.process(shiftLP1.process(xIn));
    const double psolaInput = (smoothedPitchRatio > 1.0) ? xShiftFiltered : xIn;

    const double xPre = psolaInput - preEmphasisPole * preEmpZ1;
    preEmpZ1 = psolaInput;

    lpcAnalysisWin[static_cast<size_t>(lpcWinWriteIdx)] = xPre;
    lpcWinWriteIdx = (lpcWinWriteIdx + 1) & 511;

    if (formantsRunning)
    {
        if (++lpcSampleCounter >= kLPCFrameHop)
        {
            lpcSampleCounter = 0;
            double userThroatRatio = (throatLengthPct / 100.0) * smoothedVibFormantRatio;
            if (ratioLock && std::abs(transposeSemi) > 0.001)
                userThroatRatio *= std::pow(2.0, -transposeSemi / 24.0);
            analyzeLPCAndThroat(smoothedPeriodSamples, userThroatRatio);
        }
    }
    const double lpcAlpha = static_cast<double>(lpcSampleCounter) / static_cast<double>(kLPCFrameHop);

    const double excitation = formantsRunning ? inverseFilterLPC(xPre, lpcAlpha) : psolaInput;
    rawInputRing[static_cast<size_t>(psolaWriteIdx)] = xIn;
    shiftedInputRing[static_cast<size_t>(psolaWriteIdx)] = psolaInput;
    residualRing[static_cast<size_t>(psolaWriteIdx)] = excitation;

    const double delayedDrySample = rawInputRing[static_cast<size_t>(
        (psolaWriteIdx - static_cast<int>(nominalDelaySamples)) & kRingMask)];

    const auto& activeRing = shiftedInputRing;

    // -------------------------------------------------------------------------
    // 4. Relative-Offset Directional Phase-Locked TD-PSOLA Resynthesis
    // -------------------------------------------------------------------------
    // A 2048-sample cap truncated low male periods at 192 kHz (80 Hz =
    // 2400 samples), producing the wrong shifted pitch. Leave room for the
    // outgoing crossfade while using the actual full-rate voice period.
    const double T0Full = std::clamp(smoothedPeriodSamples * static_cast<double>(downsampleFactor),
        32.0, 0.75 * static_cast<double>(kRingSize - 128));
    voicedBlend = voiceSmoothCoeff * voicedBlend
                + (1.0 - voiceSmoothCoeff) * (voicedState ? 1.0 : 0.0);
    const double controlledRatio = smoothedPitchRatio * naturalVibratoRatio;
    const double desiredRatio = 1.0 + voicedBlend * (controlledRatio - 1.0);
    resampleRatio = ratioSmoothCoeff * resampleRatio + (1.0 - ratioSmoothCoeff) * desiredRatio;
    const double step = std::clamp(resampleRatio, 0.5, 2.0);
    const double deltaDelay = 1.0 - step;

    relDelayA += deltaDelay;
    relDelayB += deltaDelay;

    // Keep the smallest causal read margin needed by the 16-sample splice
    // comparison, then one pitch period. The old downward path kept 1.2 periods.
    const double minDelay = std::max(24.0, std::round(sampleRate * 0.0005));
    const double maxDelay = minDelay + T0Full;
    if (std::abs(xIn) < 1.0e-8 && !formantsRunning)
        quietSamples = std::min(quietSamples + 1, static_cast<int>(sampleRate * 0.025));
    else
        quietSamples = 0;
    if (quietSamples >= static_cast<int>(sampleRate * 0.025))
    {
        relDelayA = relDelayB = nominalDelaySamples;
        crossfadeAlpha = 1.0;
        resampleRatio = 1.0;
    }

    if (crossfadeAlpha >= 1.0)
    {
        if (step > 1.0 && relDelayA < minDelay)
        {
            relDelayB = relDelayA;
            double targetDelay = relDelayA + T0Full;
            while (targetDelay < minDelay)
                targetDelay += T0Full;
            relDelayA = findBestSpliceDelay(activeRing, relDelayB, targetDelay, T0Full);
            crossfadeAlpha = 0.0;
            // The outgoing head must finish before it reaches unwritten input.
            // At large upward shifts a fixed quarter-period fade could read the
            // previous lap of the ring, causing a burst even with finite state.
            const double availableFade = std::max(1.0,
                (relDelayB - 3.0) / std::max(1.0e-9, step - 1.0));
            crossfadeStep = 1.0 / std::max(1.0,
                std::min(std::max(8.0, 0.35 * T0Full), availableFade));
        }
        else if (step < 1.0 && relDelayA > maxDelay)
        {
            relDelayB = relDelayA;
            double targetDelay = relDelayA - T0Full;
            while (targetDelay > maxDelay)
                targetDelay -= T0Full;
            relDelayA = findBestSpliceDelay(activeRing, relDelayB, targetDelay, T0Full);
            crossfadeAlpha = 0.0;
            crossfadeStep = 1.0 / std::max(24.0, 0.38 * T0Full);
        }
    }

    // v1.2.1 audio-thread safety: the read delays must stay finite and inside the ring.
    // An infinite delay made the wrap loops below spin forever on the audio thread,
    // which froze the whole DAW (it then waits for that thread when a VST is added/removed).
    const double maxSafeDelay = static_cast<double>(kRingSize - 64);
    if (! std::isfinite(relDelayA) || std::abs(relDelayA) > maxSafeDelay)
    {
        relDelayA = std::clamp(minDelay + 0.5 * T0Full, 1.0, maxSafeDelay);
        crossfadeAlpha = 1.0;
    }
    if (! std::isfinite(relDelayB) || std::abs(relDelayB) > maxSafeDelay)
        relDelayB = relDelayA;
    if (! std::isfinite(crossfadeStep) || crossfadeStep <= 0.0)
        crossfadeStep = 1.0 / 24.0;
    if (crossfadeAlpha < 1.0)
        relDelayB = std::max(3.0, relDelayB); // Cubic +2 tap must remain causal.

    double readPosA = static_cast<double>(psolaWriteIdx) - relDelayA;
    while (readPosA < 0.0)
        readPosA += static_cast<double>(kRingSize);

    double resynthesized = readCubicHermite(activeRing, readPosA);
    double residualSynthesized = formantsRunning ? readCubicHermite(residualRing, readPosA) : 0.0;

    if (crossfadeAlpha < 1.0)
    {
        double readPosB = static_cast<double>(psolaWriteIdx) - relDelayB;
        while (readPosB < 0.0)
            readPosB += static_cast<double>(kRingSize);

        const double sampleB = readCubicHermite(activeRing, readPosB);
        const double w = 0.5 * (1.0 - std::cos(kPi * crossfadeAlpha));
        resynthesized = w * resynthesized + (1.0 - w) * sampleB;
        if (formantsRunning)
            residualSynthesized = w * residualSynthesized
                                + (1.0 - w) * readCubicHermite(residualRing, readPosB);

        crossfadeAlpha += crossfadeStep;
        if (crossfadeAlpha >= 1.0)
            crossfadeAlpha = 1.0;
    }

    psolaWriteIdx = (psolaWriteIdx + 1) & kRingMask;

    // -------------------------------------------------------------------------
    // 5. Throat-Warped LPC Synthesis Filter & Latency-Compensated Wet/Dry Mix
    // -------------------------------------------------------------------------
    double yWet = resynthesized;
    if (formantsRunning)
    {
        const double lpcSyn = synthesizeThroatLPC(residualSynthesized, lpcAlpha);
        deEmpZ1 = lpcSyn + preEmphasisPole * deEmpZ1;
        formantReferenceEnvelope = std::max(std::abs(xIn),
            formantReferenceEnvelope * voiceSmoothCoeff);
        const double faultThreshold = std::max(0.5, 4.0 * formantReferenceEnvelope);
        if (!std::isfinite(deEmpZ1) || std::abs(deEmpZ1) > faultThreshold)
        {
            // Recover only the LPC branch. Do not reset the pitch tracker/read
            // heads, or let the original +/-4 output clamp turn a filter fault
            // into a loud burst. Other tuning, dry/mix and licensing stay active.
            invFilterHist.fill(0.0);
            synFilterHist.fill(0.0);
            deEmpZ1 = resynthesized;
            formantRecoverySamples = static_cast<int>(sampleRate * 0.050);
        }
        const double safeFormant = formantRecoverySamples > 0 ? resynthesized : deEmpZ1;
        // 03/11: bo khu nhan (de-emphasis) khuech dai sai so tan thap toi ~26 dB -> u tram 40-110 Hz.
        // Chia 150 Hz (Linkwitz-Riley 4): duoi lay tu duong khong formant, tren lay tu duong formant.
        const double lowPlain   = formantXoverLP2.process(formantXoverLP1.process(resynthesized));
        const double highFormant = formantXoverHP2.process(formantXoverHP1.process(safeFormant));
        const double formantBranch = lowPlain + highFormant;
        yWet = (1.0 - smoothedFormantMix) * resynthesized + smoothedFormantMix * formantBranch;
    }
    smoothedVibTimbreDb = voiceSmoothCoeff * smoothedVibTimbreDb
                        + (1.0 - voiceSmoothCoeff) * targetVibTimbreDb;
    if (std::abs(smoothedVibTimbreDb) > 1.0e-4)
    {
        if ((++vibTimbreUpdateTick & 31) == 0)
            vibTimbreEQ.setPeakingEQ(1500.0, sampleRate, 1.41421356, smoothedVibTimbreDb);
        yWet = vibTimbreEQ.process(yWet);
    }
    yWet *= smoothedVibAmpGain;

    const double targetMix = std::clamp(wetDryMixPct * 0.01, 0.0, 1.0);
    smoothedWetMix = 0.9990 * smoothedWetMix + 0.0010 * targetMix;
    const double mixed = delayedDrySample * (1.0 - smoothedWetMix) + yWet * smoothedWetMix;
    smoothedBypassMix = voiceSmoothCoeff * smoothedBypassMix
                      + (1.0 - voiceSmoothCoeff) * (bypass ? 1.0 : 0.0);
    const double yOut = mixed * (1.0 - smoothedBypassMix) + delayedDrySample * smoothedBypassMix;

    if (! std::isfinite(yOut))
    {
        reset();   // filter state blew up: start clean instead of sending NaN to the DAW
        return 0.0f;
    }
    return static_cast<float>(std::clamp(yOut, -4.0, 4.0));
}

//==============================================================================
//==============================================================================
