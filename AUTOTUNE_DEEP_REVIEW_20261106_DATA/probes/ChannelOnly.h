#pragma once
#include <array>
#include <atomic>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdint>
struct TAutoTuneProcessor { enum NoteState { kNoteActive=0,kNoteBypassed=1,kNoteRemoved=2 }; };
class TAutoTuneChannelDSP
{
public:
    static constexpr int kMaxLPCOrder  = 24;
    static constexpr int kRingSize     = 8192;
    static constexpr int kRingMask     = kRingSize - 1;
    static constexpr int kDecRingSize  = 512;
    static constexpr int kDecRingMask  = kDecRingSize - 1;
    static constexpr int kMaxDecLag    = 250;
    static constexpr int kMaxTrackWin  = 64;

    struct BiquadDF2
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0;
        double a1 = 0.0, a2 = 0.0;
        double z1 = 0.0, z2 = 0.0;

        void reset() noexcept { z1 = z2 = 0.0; }

        void setLowpass(double fc, double sr, double q = 0.7071067811865475) noexcept
        {
            const double w0 = 2.0 * 3.14159265358979323846 * std::clamp(fc, 10.0, 0.49 * sr) / sr;
            const double cs = std::cos(w0);
            const double alpha = std::sin(w0) / (2.0 * q);
            const double norm = 1.0 / (1.0 + alpha);
            b0 = (0.5 * (1.0 - cs)) * norm;
            b1 = (1.0 - cs) * norm;
            b2 = b0;
            a1 = (-2.0 * cs) * norm;
            a2 = (1.0 - alpha) * norm;
        }

        void setHighpass(double fc, double sr, double q = 0.7071067811865475) noexcept
        {
            const double w0 = 2.0 * 3.14159265358979323846 * std::clamp(fc, 10.0, 0.49 * sr) / sr;
            const double cs = std::cos(w0);
            const double alpha = std::sin(w0) / (2.0 * q);
            const double norm = 1.0 / (1.0 + alpha);
            b0 = (0.5 * (1.0 + cs)) * norm;
            b1 = -(1.0 + cs) * norm;
            b2 = b0;
            a1 = (-2.0 * cs) * norm;
            a2 = (1.0 - alpha) * norm;
        }

        // Tang bac 2 tu so doi xung (1 + b1 z^-1 + z^-2), chuan hoa 0 dB o DC (dung cho bo loc elliptic tu thiet ke).
        void setSymmetricStage(double rawB1, double rawA1, double rawA2) noexcept
        {
            const double dcNum = 2.0 + rawB1;
            const double dcDen = 1.0 + rawA1 + rawA2;
            const double g = (std::abs(dcNum) > 1.0e-9) ? (dcDen / dcNum) : 1.0;
            b0 = g;
            b1 = rawB1 * g;
            b2 = g;
            a1 = rawA1;
            a2 = rawA2;
        }

        // EQ chuong (peaking) RBJ - dung cho "mau rung" 1.5 kHz cua vibrato tong hop
        void setPeakingEQ(double fc, double sr, double q, double gainDb) noexcept
        {
            const double A = std::pow(10.0, gainDb / 40.0);
            const double w0 = 2.0 * 3.14159265358979323846 * std::clamp(fc, 20.0, 0.48 * sr) / sr;
            const double cs = std::cos(w0);
            const double alpha = std::sin(w0) / (2.0 * q);
            const double norm = 1.0 / (1.0 + alpha / A);
            b0 = (1.0 + alpha * A) * norm;
            b1 = (-2.0 * cs) * norm;
            b2 = (1.0 - alpha * A) * norm;
            a1 = b1;
            a2 = (1.0 - alpha / A) * norm;
        }

        inline double process(double x) noexcept
        {
            const double w = x - a1 * z1 - a2 * z2;
            const double y = b0 * w + b1 * z1 + b2 * z2;
            z2 = z1;
            z1 = w;
            return y;
        }
    };

    void prepare(double newSampleRate);
    void reset();
    void updateVoiceTypeFilters(int voiceType);
    // Fixed neutral-path delay. Pitch-period splicing can use older samples;
    // this is the host compensation reference, not a zero-latency claim.
    static int liveLatencySamples(double sr) noexcept
    {
        return std::max(24, static_cast<int>(std::lround(std::max(8000.0, sr) * 0.002)));
    }

    float processSample(float inputSample,
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
                        int frozenMidiNote = -1,
                        bool bypass = false);

    float getDetectedMidiNote() const noexcept { return lastDetectedMidi; }
    float getTargetMidiNote()   const noexcept { return lastTargetMidi; }
    float getCentsCorrection()  const noexcept { return lastCentsCorrection; }
    bool  isCurrentlyVoiced()   const noexcept { return voicedState; }

public:
    int    acquireCoarsePeriod8x(double xFiltered, double trackingThreshold) noexcept;
    bool   initTrackingWindow(int newBaseLag) noexcept;
    double refineAndSlideTrackingWindow(int& bestIdx) noexcept;

    void   analyzeLPCAndThroat(double pitchPeriodSamples, double effectiveThroatRatio) noexcept;
    double inverseFilterLPC(double x, double alpha) noexcept;
    double synthesizeThroatLPC(double excitation, double alpha) noexcept;

    double readCubicHermite(const std::array<double, kRingSize>& buf, double readPos) const noexcept;
    double findBestSpliceDelay(const std::array<double, kRingSize>& buf,
                               double currentDelay,
                               double nominalTargetDelay,
                               double T0Full) const noexcept;

    double sampleRate = 48000.0;
    double analysisRate = 48000.0;
    int downsampleFactor = 1;
    int downsampleCounter = 0;

    std::array<BiquadDF2, 6> downsampleFilters;
    BiquadDF2 inputLP1, inputLP2, inputHP1;
    int currentVoiceType = -1;
    int trackHalfWin = 15;
    double minPeriodSamples = 32.0;
    double maxPeriodSamples = 1200.0;

    BiquadDF2 shiftLP1, shiftLP2;
    BiquadDF2 formantXoverLP1, formantXoverLP2, formantXoverHP1, formantXoverHP2;   // 03/11: chia 150 Hz cho nhanh formant

    std::array<double, kRingSize> detectorRing {};
    int detWriteIdx = 0;

    std::array<double, kDecRingSize> decRing {};
    std::array<double, kMaxDecLag> decEnergy {};
    std::array<double, kMaxDecLag> decCorr {};
    int decWriteIdx = 0;
    int decSampleCount = 0;

    std::array<double, kMaxTrackWin> trackEnergy {};
    std::array<double, kMaxTrackWin> trackCorr {};
    int baseLag = 100;
    bool needAcquisition = true;

    int updateSubCounter = 0;
    double currentPeriodSamples = 200.0;
    double smoothedPeriodSamples = 200.0;
    double fastPitchMidi = 60.0;
    double slowPitchMidi = 60.0;
    bool voicedState = false;

    int lockedTargetNote = -1;
    int sustainedNoteFrames = 0;
    double smoothedPitchRatio = 1.0;
    double voicedBlend = 0.0;
    double resampleRatio = 1.0;
    double voiceSmoothCoeff = 0.0;
    double ratioSmoothCoeff = 0.0;
    double formantSmoothCoeff = 0.0;
    double nominalDelaySamples = 96.0;
    int quietSamples = 0;
    double correctedPitchCenter = 60.0, naturalVibratoOffset = 0.0;
    double naturalVibratoRatio = 1.0;
    double fastPitchCoeff = 0.0, slowPitchCoeff = 0.0;
    bool correctedPitchInitialised = false;

    // 03/11: trang thai giu dau chu / van toc luyen / hysteresis theo huong luyen
    static constexpr double kVibGateStartSemi = 0.6;   // 03/11: lech <= 0.6 cung = rung giong that
    static constexpr double kVibGateWidthSemi = 0.3;
    int       onsetConfirmCount = 0;
    double    onsetPrevEnergy    = 0.0;
    int       onsetHoldTicks = 0;
    double    glidePrevMidi       = 60.0;
    double    glideVelocity      = 0.0;
    double    glideSignedVelocity     = 0.0;
    double    onsetSmoothTargetMidi   = 60.0;
    double    targetVibTimbreDb     = 0.0;
    double    smoothedVibTimbreDb   = 0.0;
    int       vibTimbreUpdateTick   = 0;
    BiquadDF2 vibTimbreEQ;

    double vibPhase = 0.0;
    double vibNoteOnsetSec = 0.0;
    uint32_t rngState = 0x12345678u;
    double targetVibAmpGain = 1.0, smoothedVibAmpGain = 1.0;
    double targetVibFormantRatio = 1.0, smoothedVibFormantRatio = 1.0;

    int lpcOrder = 12;
    int lpcSampleCounter = 0;
    static constexpr int kLPCFrameHop = 64;
    double preEmphasisPole = 0.96;
    double preEmpZ1 = 0.0;
    double deEmpZ1 = 0.0;
    std::array<double, 512> lpcAnalysisWin {};
    int lpcWinWriteIdx = 0;
    std::array<double, kMaxLPCOrder + 1> prevLpcA {};
    std::array<double, kMaxLPCOrder + 1> currLpcA {};
    std::array<double, kMaxLPCOrder + 1> prevLpcWarpedA {};
    std::array<double, kMaxLPCOrder + 1> currLpcWarpedA {};
    std::array<double, kMaxLPCOrder + 1> invFilterHist {};
    std::array<double, kMaxLPCOrder + 1> synFilterHist {};

    std::array<double, kRingSize> rawInputRing {};
    std::array<double, kRingSize> shiftedInputRing {};
    std::array<double, kRingSize> residualRing {};
    int psolaWriteIdx = 0;
    double relDelayA = 256.0;
    double relDelayB = 256.0;
    double crossfadeAlpha = 1.0;
    double crossfadeStep = 0.0;

    double smoothedWetMix = 1.0;
    double smoothedFormantMix = 0.0;
    double smoothedBypassMix = 0.0;
    double formantReferenceEnvelope = 0.0;
    int formantRecoverySamples = 0;

    float lastDetectedMidi = -1.0f;
    float lastTargetMidi = -1.0f;
    float lastCentsCorrection = 0.0f;
};
