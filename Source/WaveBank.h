#pragma once
#include <juce_core/juce_core.h>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

namespace attia
{
constexpr int kWaveLen = 2048, kBankWaves = 11000, kFamilies = 11, kPerFamily = 1000;

inline const char* familyName (int f)
{
    static const char* n[] = { "Sine Bloom", "Odd Glass", "Formant Choir", "Bowed Strings", "Plucked Strings", "Sub Bass",
                               "FM Metal", "Organ Drawbars", "Alien Birds", "Warm Tines", "Spectral Rooms" };
    return n[juce::jlimit (0, kFamilies - 1, f)];
}

// Banco de ondas "Resynth": se mapea desde el disco (ProgramData\ORYMY\ATTIA\attia_bank.bin, lo instala setup.exe).
// Si no se encuentra, ATTIA usa 16 ondas internas para seguir sonando.
struct WaveBank
{
    const float* data = nullptr;
    int numWaves = 0;
    bool fromFile = false;
    std::unique_ptr<juce::MemoryMappedFile> mmf;
    std::vector<float> fallback;

    WaveBank() { tryLoad(); if (data == nullptr) buildFallback(); }

    const float* wave (int i) const { return data + (size_t) juce::jlimit (0, numWaves - 1, i) * (size_t) kWaveLen; }

    // posición 0..kBankWaves-1 (parámetro) -> posición real dentro del banco cargado
    float mapPos (float w) const { return w * (float) (numWaves - 1) / (float) (kBankWaves - 1); }

private:
    void tryLoad()
    {
        for (auto loc : { juce::File::commonApplicationDataDirectory, juce::File::userApplicationDataDirectory })
        {
            auto f = juce::File::getSpecialLocation (loc).getChildFile ("ORYMY").getChildFile ("ATTIA").getChildFile ("attia_bank.bin");
            if (! f.existsAsFile() || f.getSize() < 16 + (juce::int64) kWaveLen * 8) continue;
            auto m = std::make_unique<juce::MemoryMappedFile> (f, juce::MemoryMappedFile::readOnly);
            if (m->getData() == nullptr) continue;
            auto* b = static_cast<const unsigned char*> (m->getData());
            if (std::memcmp (b, "ATTIABK1", 8) != 0) continue;
            juce::uint32 n = 0, len = 0;
            std::memcpy (&n, b + 8, 4); std::memcpy (&len, b + 12, 4);
            if (len != (juce::uint32) kWaveLen || n < 2) continue;
            if ((size_t) 16 + (size_t) n * len * 4 > m->getSize()) continue;
            data = reinterpret_cast<const float*> (b + 16);
            numWaves = (int) n; fromFile = true; mmf = std::move (m);
            return;
        }
    }

    void buildFallback()
    {
        numWaves = 16;
        fallback.assign ((size_t) numWaves * kWaveLen, 0.f);
        for (int w = 0; w < numWaves; ++w)
        {
            float* t = fallback.data() + (size_t) w * kWaveLen;
            for (int h = 1; h <= 32; ++h)
            {
                float a = std::pow ((float) h, -(0.3f + 0.12f * (float) w));
                if (w % 3 == 1 && h % 2 == 0) a *= 0.05f;
                if (w % 5 == 4) a *= 0.5f + 0.5f * std::sin ((float) h * 0.9f);
                for (int i = 0; i < kWaveLen; ++i)
                    t[i] += a * std::sin (6.2831853f * (float) (h * i) / (float) kWaveLen);
            }
            float pk = 1e-6f;
            for (int i = 0; i < kWaveLen; ++i) pk = juce::jmax (pk, std::fabs (t[i]));
            for (int i = 0; i < kWaveLen; ++i) t[i] *= 0.9f / pk;
        }
        data = fallback.data();
    }
};

inline WaveBank& bank() { static WaveBank b; return b; }
}
