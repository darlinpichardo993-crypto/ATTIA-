#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <array>
#include <atomic>
#include "Params.h"

struct Snap
{
    float basePos[2] {}, lvl[2] {}, oct[2] {}, semi[2] {}, fine[2] {}, warp[2] {};
    int nU = 1, numWaves = 2, lfoDest = 0;
    float ratio[5] {}, gl[5] {}, gr[5] {};
    float uniGain = 1.f, cut = 1000.f, q = 1.f, fenv = 0.f, fkey = 0.f, lfoDepth = 0.f, bend = 0.f, vibSemis = 0.f;
    float invSr = 1.f / 44100.f, maxCut = 20000.f;
    const attia::WaveBank* wb = nullptr;
};

struct Voice
{
    bool active = false, releasing = false;
    int note = 0, ctr = 0;
    float vel = 0.f;
    uint32_t age = 0;
    float phase[2][5] {};
    juce::ADSR amp, flt;
    juce::dsp::StateVariableTPTFilter<float> filter;

    void prepare (double sr)
    {
        amp.setSampleRate (sr); flt.setSampleRate (sr);
        filter.prepare ({ sr, 512, 2 });
    }

    void start (int n, float v, uint32_t a, const juce::ADSR::Parameters& ap, const juce::ADSR::Parameters& fp)
    {
        active = true; releasing = false; note = n; vel = v; age = a; ctr = 0;
        for (int o = 0; o < 2; ++o)
            for (int u = 0; u < 5; ++u)
                phase[o][u] = std::fmod (0.61803f * (float) (u + 1 + o * 5) + 0.137f * (float) n, 1.f);
        filter.reset();
        amp.setParameters (ap); flt.setParameters (fp);
        amp.noteOn(); flt.noteOn();
    }

    void release() { releasing = true; amp.noteOff(); flt.noteOff(); }

    void render (const Snap& s, float lfo, float vib, float& oL, float& oR)
    {
        float l = 0.f, r = 0.f;
        const int nw = s.numWaves;
        for (int o = 0; o < 2; ++o)
        {
            if (s.lvl[o] < 0.001f) continue;
            float pos = s.basePos[o];
            if (s.lfoDest == 3) pos += lfo * s.lfoDepth * 12.f;
            pos = juce::jlimit (0.f, (float) (nw - 1) - 0.001f, pos);
            const int i0 = (int) pos;
            const float fr = pos - (float) i0;
            const float* tA = s.wb->wave (i0);
            const float* tB = s.wb->wave (juce::jmin (i0 + 1, nw - 1));

            float wp = s.warp[o];
            if (s.lfoDest == 5) wp += lfo * s.lfoDepth;
            wp = juce::jlimit (-1.f, 1.f, wp);
            const float c = 0.5f + 0.45f * wp, k1 = 0.5f / c, k2 = 0.5f / (1.f - c);

            float semis = (float) note + s.oct[o] * 12.f + s.semi[o] + s.fine[o] / 100.f + s.bend + vib * s.vibSemis;
            if (s.lfoDest == 2) semis += lfo * s.lfoDepth * 2.f;
            const float f = 440.f * std::pow (2.f, (semis - 69.f) / 12.f);

            for (int u = 0; u < s.nU; ++u)
            {
                const float inc = juce::jmin (0.45f, f * s.ratio[u] * s.invSr);
                float& p = phase[o][u];
                const float pp = p < c ? p * k1 : 0.5f + (p - c) * k2;   // distorsión de fase ("warp")
                const float x = pp * (float) attia::kWaveLen;
                const int ia = (int) x & (attia::kWaveLen - 1), ib = (ia + 1) & (attia::kWaveLen - 1);
                const float f2 = x - std::floor (x);
                const float a = tA[ia] + (tA[ib] - tA[ia]) * f2;
                const float b = tB[ia] + (tB[ib] - tB[ia]) * f2;
                const float smp = (a + (b - a) * fr) * s.lvl[o];
                l += smp * s.gl[u]; r += smp * s.gr[u];
                p += inc; if (p >= 1.f) p -= 1.f;
            }
        }
        l *= s.uniGain; r *= s.uniGain;

        const float a  = amp.getNextSample();
        const float fe = flt.getNextSample();
        if (! amp.isActive()) { active = false; oL = oR = 0.f; return; }

        if ((ctr++ & 7) == 0)
        {
            float oct = s.fenv * fe * 5.f + s.fkey * ((float) note - 60.f) / 12.f;
            if (s.lfoDest == 1) oct += lfo * s.lfoDepth * 3.f;
            filter.setCutoffFrequency (juce::jlimit (20.f, s.maxCut, s.cut * std::pow (2.f, oct)));
            filter.setResonance (s.q);
        }
        l = filter.processSample (0, l);
        r = filter.processSample (1, r);

        float g = a * vel * 0.22f;
        if (s.lfoDest == 4) g *= 1.f - s.lfoDepth * 0.5f * (lfo + 1.f);
        oL = l * g; oR = r * g;
    }
};

class AttiaProcessor : public juce::AudioProcessor
{
public:
    AttiaProcessor();
    ~AttiaProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "ATTIA"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    void loadPreset (int idx);
    void setParam (int p, float value);   // valor real (no normalizado)
    float getParam (int p) const { return raw[p]->load(); }

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<int> currentPreset { 0 };
    float scopeBuf[1024] {};
    std::atomic<int> scopePos { 0 };
    std::atomic<float> grMeter { 0.f }, peakMeter { 0.f };

private:
    void voiceOn (int note, float vel);
    void voiceOff (int note);
    void handle (const juce::MidiMessage& m, bool arpOn);

    std::array<Voice, 16> voices;
    std::atomic<float>* raw[attia::NumParams] {};
    Snap snap;
    juce::ADSR::Parameters ampP, fltP;
    uint32_t ageCounter = 0;
    double sr = 44100.0;

    float lfoPhase = 0.f, vibPhase = 0.f, shValue = 0.f, bend = 0.f, modWheel = 0.f;
    juce::Random rng;

    // arpegiador
    std::array<int, 32> held {};
    int nHeld = 0;
    int arpPlaying = -1, stepIdx = 0, cycle = 0, patIdx = -1;
    double arpPos = 1.0e9, curLen = 0.0, gateLen = 0.0;
    bool prevArpOn = false;
    attia::arp::Pattern pat;

    // efectos
    std::vector<float> dlyL, dlyR;
    int dlyW = 0, scopeDec = 0;
    juce::dsp::Chorus<float> chorus, flanger;
    juce::dsp::Phaser<float> phaser;
    juce::Reverb reverb;
    float grDb = 0.f, limGain = 1.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttiaProcessor)
};
