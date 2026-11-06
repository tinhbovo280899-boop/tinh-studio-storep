#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

class AutoKeyHandoffReader
{
public:
    struct HandoffData
    {
        bool valid = false;
        int keyClass = 0;               // 0=C, 1=C#, 2=D, ..., 9=A, 10=A#, 11=B
        int scaleIndex = 0;             // 0=Major, 1=Minor, 2=Chromatic, 5=Dorian, etc.
        float referencePitchHz = 440.0f;
        bool hasReferencePitch = false;
        float tempoBpm = 120.0f;
        bool hasTempoBpm = false;
        std::string sourcePath;
        std::uint64_t stampHash = 0;
    };

    AutoKeyHandoffReader() = default;

    void addExtraSearchPath(const std::string& path)
    {
        if (path.empty())
            return;
        for (const auto& existing : extraSearchPaths)
        {
            if (existing == path)
                return;
        }
        extraSearchPaths.push_back(path);
    }

    void setAutoSyncEnabled(bool enabled) noexcept { autoSyncEnabled = enabled; }
    bool isAutoSyncEnabled() const noexcept { return autoSyncEnabled; }
    const HandoffData& getLastData() const noexcept { return lastData; }

    static HandoffData parseContent(const std::string& raw, const std::string& sourcePath = {})
    {
        HandoffData out;
        out.sourcePath = sourcePath;

        std::string keyStr;
        std::string scaleStr;
        std::string refPitchStr;
        std::string tempoBpmStr;

        bool hasKey = extractPropertyValue(raw, "KeyClass", keyStr)
                   || extractPropertyValue(raw, "Key", keyStr)
                   || extractPropertyValue(raw, "key", keyStr)
                   || extractLooseKeyVal(raw, "KeyClass", keyStr)
                   || extractLooseKeyVal(raw, "Key", keyStr);

        bool hasScale = extractPropertyValue(raw, "KeyType", scaleStr)
                     || extractPropertyValue(raw, "Scale", scaleStr)
                     || extractPropertyValue(raw, "ScaleType", scaleStr)
                     || extractPropertyValue(raw, "scale", scaleStr)
                     || extractLooseKeyVal(raw, "KeyType", scaleStr)
                     || extractLooseKeyVal(raw, "Scale", scaleStr);

        bool hasRefPitch = extractPropertyValue(raw, "ReferencePitch", refPitchStr)
                        || extractPropertyValue(raw, "DetuneHz", refPitchStr)
                        || extractLooseKeyVal(raw, "ReferencePitch", refPitchStr);

        bool hasTempo = extractPropertyValue(raw, "TempoBPM", tempoBpmStr)
                     || extractLooseKeyVal(raw, "TempoBPM", tempoBpmStr);

        if (!hasKey && !hasScale)
            return out;

        int parsedKey = 0;
        if (hasKey && !parseKeyClass(keyStr, parsedKey))
            return out;

        int parsedScale = 0; // Default 0 = Major
        if (hasScale && !parseScaleIndex(scaleStr, parsedScale))
            return out;

        float parsedRefHz = 440.0f;
        bool validRefHz = false;
        if (hasRefPitch && !refPitchStr.empty())
        {
            char* endPtr = nullptr;
            const float val = std::strtof(refPitchStr.c_str(), &endPtr);
            if (endPtr != refPitchStr.c_str() && std::isfinite(val) && val >= 400.0f && val <= 480.0f)
            {
                parsedRefHz = std::clamp(val, 415.0f, 466.0f);
                validRefHz = true;
            }
        }

        float parsedTempo = 120.0f;
        bool validTempo = false;
        if (hasTempo && !tempoBpmStr.empty())
        {
            char* endPtr = nullptr;
            const float val = std::strtof(tempoBpmStr.c_str(), &endPtr);
            if (endPtr != tempoBpmStr.c_str() && std::isfinite(val) && val >= 20.0f && val <= 320.0f)
            {
                parsedTempo = val;
                validTempo = true;
            }
        }

        out.valid = true;
        out.keyClass = std::clamp(parsedKey, 0, 11);
        out.scaleIndex = std::clamp(parsedScale, 0, 28);
        out.referencePitchHz = parsedRefHz;
        out.hasReferencePitch = validRefHz;
        out.tempoBpm = parsedTempo;
        out.hasTempoBpm = validTempo;
        out.stampHash = computeFnv1a(raw);
        return out;
    }

    static HandoffData parseFile(const std::filesystem::path& filePath)
    {
        HandoffData out;
        std::error_code ec;
        if (!std::filesystem::exists(filePath, ec) || !std::filesystem::is_regular_file(filePath, ec))
            return out;

        std::ifstream ifs(filePath, std::ios::in | std::ios::binary);
        if (!ifs.is_open())
            return out;

        std::ostringstream oss;
        oss << ifs.rdbuf();
        const std::string content = oss.str();
        if (content.empty())
            return out;

        out = parseContent(content, filePath.string());
        if (!out.valid)
            return out;

        const auto writeTime = std::filesystem::last_write_time(filePath, ec);
        const auto timeTicks = ec ? 0ULL
                                  : static_cast<std::uint64_t>(writeTime.time_since_epoch().count());
        out.stampHash ^= (timeTicks + 0x9e3779b97f4a7c15ULL + (out.stampHash << 6) + (out.stampHash >> 2));
        return out;
    }

    // TEST ONLY: restrict file discovery to explicit task fixtures.
    // Parsing, enabled/force gate, update comparison and production source remain unchanged.
    std::vector<std::filesystem::path> getCandidatePaths() const {
        std::vector<std::filesystem::path> result;for(const auto& p:extraSearchPaths)result.emplace_back(p);return result;
    }

    HandoffData readBestAvailable() const
    {
        const auto candidates = getCandidatePaths();
        HandoffData best;
        std::filesystem::file_time_type bestTime = std::filesystem::file_time_type::min();
        bool hasBestTime = false;

        for (const auto& candidate : candidates)
        {
            std::error_code ec;
            if (!std::filesystem::exists(candidate, ec) || !std::filesystem::is_regular_file(candidate, ec))
                continue;

            HandoffData parsed = parseFile(candidate);
            if (!parsed.valid)
                continue;

            const auto writeTime = std::filesystem::last_write_time(candidate, ec);
            if (!best.valid)
            {
                best = parsed;
                if (!ec)
                {
                    bestTime = writeTime;
                    hasBestTime = true;
                }
            }
            else if (!ec && (!hasBestTime || writeTime > bestTime))
            {
                best = parsed;
                bestTime = writeTime;
                hasBestTime = true;
            }
        }

        return best;
    }

    bool pollForUpdate(HandoffData& outData, bool forceReload = false)
    {
        if (!autoSyncEnabled && !forceReload)
            return false;

        const HandoffData current = readBestAvailable();
        if (!current.valid)
            return false;

        const bool changed = forceReload
            || !lastData.valid
            || current.stampHash != lastData.stampHash
            || current.keyClass != lastData.keyClass
            || current.scaleIndex != lastData.scaleIndex
            || std::abs(current.referencePitchHz - lastData.referencePitchHz) > 0.005f;

        lastData = current;
        outData = current;
        return changed;
    }

private:
    std::vector<std::string> extraSearchPaths;
    bool autoSyncEnabled = true;
    HandoffData lastData{};

    static std::uint64_t computeFnv1a(const std::string& s) noexcept
    {
        std::uint64_t hash = 14695981039346656037ULL;
        for (unsigned char c : s)
        {
            hash ^= static_cast<std::uint64_t>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    static std::string trim(const std::string& s)
    {
        size_t a = 0;
        while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
        size_t b = s.size();
        while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
        return s.substr(a, b - a);
    }

    static bool extractAttribute(const std::string& tag, const std::string& attrName, std::string& outVal)
    {
        const std::string pattern1 = attrName + "=\"";
        const std::string pattern2 = attrName + "='";
        size_t pos = tag.find(pattern1);
        char quote = '"';
        if (pos != std::string::npos)
        {
            pos += pattern1.size();
        }
        else
        {
            pos = tag.find(pattern2);
            if (pos == std::string::npos)
                return false;
            pos += pattern2.size();
            quote = '\'';
        }
        const size_t endPos = tag.find(quote, pos);
        if (endPos == std::string::npos)
            return false;
        outVal = trim(tag.substr(pos, endPos - pos));
        return true;
    }

    static bool extractPropertyValue(const std::string& xml, const std::string& propName, std::string& outVal)
    {
        // 1. Check <VALUE name="propName" val="..."/>
        size_t searchPos = 0;
        while (searchPos < xml.size())
        {
            const size_t tagStart = xml.find("<VALUE", searchPos);
            if (tagStart == std::string::npos)
                break;
            const size_t tagEnd = xml.find('>', tagStart);
            if (tagEnd == std::string::npos)
                break;
            const std::string tag = xml.substr(tagStart, tagEnd - tagStart + 1);
            std::string nameAttr;
            if (extractAttribute(tag, "name", nameAttr) && nameAttr == propName)
            {
                if (extractAttribute(tag, "val", outVal) || extractAttribute(tag, "value", outVal))
                    return true;
            }
            searchPos = tagEnd + 1;
        }

        // 2. Check direct XML attribute propName="..." anywhere in XML
        if (extractAttribute(xml, propName, outVal))
            return true;

        // 3. Check <propName>...</propName>
        const std::string openTag = "<" + propName + ">";
        const std::string closeTag = "</" + propName + ">";
        const size_t p0 = xml.find(openTag);
        if (p0 != std::string::npos)
        {
            const size_t v0 = p0 + openTag.size();
            const size_t p1 = xml.find(closeTag, v0);
            if (p1 != std::string::npos)
            {
                outVal = trim(xml.substr(v0, p1 - v0));
                return !outVal.empty();
            }
        }
        return false;
    }

    static bool extractLooseKeyVal(const std::string& text, const std::string& keyName, std::string& outVal)
    {
        std::istringstream iss(text);
        std::string line;
        while (std::getline(iss, line))
        {
            for (char ch : { '=', ':' })
            {
                const size_t eq = line.find(ch);
                if (eq != std::string::npos)
                {
                    std::string k = trim(line.substr(0, eq));
                    if (!k.empty() && k.front() == '"') k.erase(0, 1);
                    if (!k.empty() && k.back() == '"')  k.pop_back();
                    if (k == keyName)
                    {
                        std::string v = trim(line.substr(eq + 1));
                        if (!v.empty() && v.back() == ',')  v.pop_back();
                        v = trim(v);
                        if (!v.empty() && v.front() == '"') v.erase(0, 1);
                        if (!v.empty() && v.back() == '"')  v.pop_back();
                        outVal = trim(v);
                        return !outVal.empty();
                    }
                }
            }
        }
        return false;
    }

    static bool parseKeyClass(const std::string& str, int& outKey)
    {
        if (str.empty())
            return false;
        char* endPtr = nullptr;
        const long v = std::strtol(str.c_str(), &endPtr, 10);
        if (endPtr != str.c_str() && *endPtr == '\0')
        {
            if (v >= 0 && v <= 11)
            {
                outKey = static_cast<int>(v);
                return true;
            }
            return false;
        }

        std::string u;
        for (unsigned char c : str) u.push_back(static_cast<char>(std::toupper(c)));
        static const std::pair<const char*, int> kNamedKeys[] = {
            { "C#", 1 }, { "DB", 1 }, { "D#", 3 }, { "EB", 3 },
            { "F#", 6 }, { "GB", 6 }, { "G#", 8 }, { "AB", 8 },
            { "A#", 10 }, { "BB", 10 },
            { "C", 0 }, { "D", 2 }, { "E", 4 }, { "F", 5 },
            { "G", 7 }, { "A", 9 }, { "B", 11 }
        };
        for (const auto& kv : kNamedKeys)
        {
            if (u == kv.first)
            {
                outKey = kv.second;
                return true;
            }
        }
        return false;
    }

    static bool parseScaleIndex(const std::string& str, int& outScaleIdx)
    {
        if (str.empty())
            return false;
        char* endPtr = nullptr;
        const long v = std::strtol(str.c_str(), &endPtr, 10);
        if (endPtr != str.c_str() && *endPtr == '\0')
        {
            if (v >= 0 && v <= 28)
            {
                outScaleIdx = static_cast<int>(v);
                return true;
            }
            return false;
        }

        std::string u;
        for (unsigned char c : str) u.push_back(static_cast<char>(std::toupper(c)));
        if (u == "MAJOR" || u == "MAJ")                 { outScaleIdx = 0; return true; }
        if (u == "MINOR" || u == "MIN")                 { outScaleIdx = 1; return true; }
        if (u == "CHROMATIC" || u == "CHROM")           { outScaleIdx = 2; return true; }
        if (u == "HARMONIC MINOR")                      { outScaleIdx = 3; return true; }
        if (u == "MELODIC MINOR")                       { outScaleIdx = 4; return true; }
        if (u == "DORIAN")                              { outScaleIdx = 5; return true; }
        if (u == "PHRYGIAN")                            { outScaleIdx = 6; return true; }
        if (u == "LYDIAN")                              { outScaleIdx = 7; return true; }
        if (u == "MIXOLYDIAN")                          { outScaleIdx = 8; return true; }
        if (u == "LOCRIAN")                             { outScaleIdx = 9; return true; }
        if (u == "PENTATONIC MAJOR")                    { outScaleIdx = 10; return true; }
        if (u == "PENTATONIC MINOR")                    { outScaleIdx = 11; return true; }
        if (u == "BLUES")                               { outScaleIdx = 12; return true; }
        if (u == "WHOLE TONE")                          { outScaleIdx = 13; return true; }
        return false;
    }
};
