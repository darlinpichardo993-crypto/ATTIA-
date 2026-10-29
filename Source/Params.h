#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <cmath>
#include "PresetNames.h"
#include "WaveBank.h"
#include "ArpData.h"

namespace attia
{
enum P
{
    O1Wave, O1Morph, O1Warp, O1Oct, O1Semi, O1Fine, O1Lvl,
    O2Wave, O2Morph, O2Warp, O2Oct, O2Semi, O2Fine, O2Lvl,
    UniVoices, UniSpread,
    FType, FCut, FRes, FEnv, FKey,
    AA, AD, AS, AR,
    FA, FD, FS, FR,
    LfoShape, LfoRate, LfoDepth, LfoDest, VibDepth, VibRate,
    ArpOn, ArpBank, ArpPat, ArpScale, ArpRate, ArpOct, ArpGate, ArpSwing,
    Drive, CThresh, CRatio, CAtk, CRel,
    DlyTime, DlyFb, DlyMix,
    ModType, ModRate, ModDepth, ModMix,
    RevSize, RevDamp, RevMix,
    Master,
    NumParams
};

constexpr int kOscBlock = 7; // O1Wave..O1Lvl

// kind: 0 float, 1 int, 2 choice, 3 bool
struct PDef { const char* id; const char* name; float mn, mx, def; int kind; const char* choices; float skew; };

inline const PDef* defs()
{
    static const PDef d[NumParams] = {
        { "o1_wave", "Wave", 0, 10999, 3200, 1, nullptr, 1 }, { "o1_morph", "Morph", 0, 1, 0, 0, nullptr, 1 }, { "o1_warp", "Warp", -1, 1, 0, 0, nullptr, 1 },
        { "o1_oct", "Oct", -2, 2, 0, 1, nullptr, 1 }, { "o1_semi", "Semi", -12, 12, 0, 1, nullptr, 1 }, { "o1_fine", "Fine", -50, 50, 0, 0, nullptr, 1 },
        { "o1_lvl", "Level", 0, 1, 0.8f, 0, nullptr, 1 },
        { "o2_wave", "Wave", 0, 10999, 2100, 1, nullptr, 1 }, { "o2_morph", "Morph", 0, 1, 0, 0, nullptr, 1 }, { "o2_warp", "Warp", -1, 1, 0, 0, nullptr, 1 },
        { "o2_oct", "Oct", -2, 2, 0, 1, nullptr, 1 }, { "o2_semi", "Semi", -12, 12, 0, 1, nullptr, 1 }, { "o2_fine", "Fine", -50, 50, 7, 0, nullptr, 1 },
        { "o2_lvl", "Level", 0, 1, 0.5f, 0, nullptr, 1 },
        { "uni_voices", "Unison", 1, 5, 2, 1, nullptr, 1 }, { "uni_spread", "Spread", 0, 40, 10, 0, nullptr, 1 },
        { "f_type", "Type", 0, 2, 0, 2, "Low Pass|Band Pass|High Pass", 1 },
        { "f_cut", "Cutoff", 20, 20000, 7000, 0, nullptr, 0.3f }, { "f_res", "Reso", 0, 1, 0.15f, 0, nullptr, 1 },
        { "f_env", "Env Amt", -1, 1, 0.25f, 0, nullptr, 1 }, { "f_key", "Key Trk", 0, 1, 0.3f, 0, nullptr, 1 },
        { "a_a", "Attack", 0.001f, 8, 0.01f, 0, nullptr, 0.35f }, { "a_d", "Decay", 0.001f, 8, 0.4f, 0, nullptr, 0.35f },
        { "a_s", "Sustain", 0, 1, 0.7f, 0, nullptr, 1 }, { "a_r", "Release", 0.001f, 8, 0.3f, 0, nullptr, 0.35f },
        { "e_a", "Attack", 0.001f, 8, 0.01f, 0, nullptr, 0.35f }, { "e_d", "Decay", 0.001f, 8, 0.4f, 0, nullptr, 0.35f },
        { "e_s", "Sustain", 0, 1, 0.4f, 0, nullptr, 1 }, { "e_r", "Release", 0.001f, 8, 0.3f, 0, nullptr, 0.35f },
        { "l_shape", "Shape", 0, 4, 0, 2, "Sine|Triangle|Saw|Square|S&H", 1 },
        { "l_rate", "Rate", 0.05f, 20, 2, 0, nullptr, 0.4f }, { "l_depth", "Depth", 0, 1, 0, 0, nullptr, 1 },
        { "l_dest", "Dest", 0, 5, 0, 2, "Off|Cutoff|Pitch|Wave|Amp|Warp", 1 },
        { "vib_depth", "Vibrato", 0, 1, 0, 0, nullptr, 1 }, { "vib_rate", "Vib Rate", 2, 9, 5.5f, 0, nullptr, 1 },
        { "arp_on", "Arp", 0, 1, 0, 3, nullptr, 1 },
        { "arp_bank", "Bank", 0, 14, 0, 2, "Major Melody Loops|Minor Melody Loops|Harmonic Minor Drama|Dorian Groove|Dembow Reggaeton Arps|Pentatonic Hooks|Phrygian Dark|Mixolydian Pop|Trance Gates|Latin Trap Arps|Ambient Cascades|Pluck Sequences|Chord Stab Rhythms|Bass Walk Patterns|Lydian Dream", 1 }, { "arp_pat", "Pattern", 0, 999, 0, 1, nullptr, 1 },
        { "arp_scale", "Scale", 0, 9, 0, 2, "Pattern|Major|Minor|Harm Minor|Dorian|Phrygian|Pent Minor|Pent Major|Mixolydian|Lydian", 1 },
        { "arp_rate", "Rate", 0, 5, 3, 2, "1/4|1/8|1/8T|1/16|1/16T|1/32", 1 },
        { "arp_oct", "Octaves", 1, 4, 1, 1, nullptr, 1 }, { "arp_gate", "Gate", 0.2f, 1.5f, 1.0f, 0, nullptr, 1 },
        { "arp_swing", "Swing", 0, 0.6f, 0, 0, nullptr, 1 },
        { "fx_drive", "Drive", 0, 1, 0, 0, nullptr, 1 },
        { "c_thresh", "Thresh", -40, 0, -14, 0, nullptr, 1 }, { "c_ratio", "Ratio", 1, 20, 3, 0, nullptr, 0.5f },
        { "c_atk", "Attack", 1, 100, 12, 0, nullptr, 0.5f }, { "c_rel", "Release", 20, 800, 180, 0, nullptr, 0.5f },
        { "d_time", "Time", 20, 1000, 330, 0, nullptr, 1 }, { "d_fb", "Fdbk", 0, 0.9f, 0.3f, 0, nullptr, 1 }, { "d_mix", "Mix", 0, 1, 0, 0, nullptr, 1 },
        { "m_type", "Type", 0, 2, 0, 2, "Chorus|Phaser|Flanger", 1 },
        { "m_rate", "Rate", 0.05f, 8, 0.7f, 0, nullptr, 0.4f }, { "m_depth", "Depth", 0, 1, 0.4f, 0, nullptr, 1 }, { "m_mix", "Mix", 0, 1, 0, 0, nullptr, 1 },
        { "r_size", "Size", 0, 1, 0.65f, 0, nullptr, 1 }, { "r_damp", "Damp", 0, 1, 0.5f, 0, nullptr, 1 }, { "r_mix", "Mix", 0, 1, 0.2f, 0, nullptr, 1 },
        { "master", "Volume", 0, 1, 0.6f, 0, nullptr, 1 }
    };
    return d;
}

inline juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    for (int i = 0; i < NumParams; ++i)
    {
        const auto& d = defs()[i];
        ParameterID pid { d.id, 1 };
        switch (d.kind)
        {
            case 0: l.add (std::make_unique<AudioParameterFloat> (pid, d.name, NormalisableRange<float> (d.mn, d.mx, 0.f, d.skew), d.def)); break;
            case 1: l.add (std::make_unique<AudioParameterInt> (pid, d.name, (int) d.mn, (int) d.mx, (int) d.def)); break;
            case 2: l.add (std::make_unique<AudioParameterChoice> (pid, d.name, StringArray::fromTokens (d.choices, "|", ""), (int) d.def)); break;
            default: l.add (std::make_unique<AudioParameterBool> (pid, d.name, d.def > 0.5f)); break;
        }
    }
    return l;
}

// ───────────── 20.000 PRESETS: 20 secciones x 1.000, cada uno con nombre propio ─────────────
namespace presets
{
constexpr int perSection = 1000, numSections = 20, numCats = 8, total = perSection * numSections;

inline const char* catName (int c)
{
    static const char* n[] = { "Pads", "Keys", "Strings", "Guitars", "Plucks", "Leads", "Bass", "FX" };
    return n[c];
}
inline const char* sectionName (int s)
{
    static const char* n[] = { "Ambient Pads", "Evolving Atmospheres", "Warm Keys", "Glass Keys & Piano", "Violins & Solo Strings",
                               "Orchestral Strings", "Nylon Guitars", "Electric & Steel Guitars", "Reggaeton Plucks", "Reggaeton Leads",
                               "Dembow Bass & Subs", "Latin Trap Bells", "Choir & Vocal Pads", "Synth Leads", "Brass & Winds",
                               "Bachata & Merengue Hooks", "Bells & Mallets", "Risers & Impacts", "Glitch & Resynth FX", "Dark Drones" };
    return n[s];
}
inline int sectionCat (int s)
{
    static const int c[] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 6, 4, 0, 5, 5, 4, 4, 7, 7, 7 };
    return c[s];
}
inline int sectionOf (int idx) { return idx / perSection; }
inline int catOf (int idx)     { return sectionCat (sectionOf (idx)); }
inline juce::String name (int idx) { return juce::String (kPresetNames[idx]); }

enum Fam { Sine, Odd, Choir, Bow, Pluck, Sub, FM, Organ, Alien, Tine, Room };

inline std::array<float, NumParams> make (int idx)
{
    using namespace juce;
    std::array<float, NumParams> v {};
    for (int i = 0; i < NumParams; ++i) v[(size_t) i] = defs()[i].def;

    Random rnd ((int64) idx * 2654435761LL + 777);
    auto R  = [&] (float a, float b) { return a + (b - a) * rnd.nextFloat(); };
    auto RL = [&] (float a, float b) { return a * std::pow (b / a, rnd.nextFloat()); };
    auto I  = [&] (int a, int b) { return (float) (rnd.nextInt (b - a + 1) + a); };
    auto pick = [&] (std::initializer_list<int> l) { return (float) *(l.begin() + rnd.nextInt ((int) l.size())); };
    auto set = [&] (P p, float x) { v[(size_t) p] = x; };
    auto wave = [&] (std::initializer_list<int> fams) { return pick (fams) * (float) kPerFamily + (float) rnd.nextInt (kPerFamily); };
    auto arpSet = [&] (float prob, std::initializer_list<int> banks)
    {
        if (rnd.nextFloat() >= prob) return;
        set (ArpOn, 1); set (ArpBank, pick (banks)); set (ArpPat, I (0, 999));
        set (ArpRate, pick ({ 1, 3, 3, 3, 4 })); set (ArpOct, I (1, 2)); set (ArpGate, R (0.8f, 1.2f)); set (ArpSwing, R (0, 0.25f));
    };
    const int sec = sectionOf (idx);

    // ── base común (todo preset sale con compresor activo y volumen moderado) ──
    set (O1Lvl, R (0.75f, 1.f)); set (O2Lvl, R (0.2f, 0.8f)); set (O1Fine, R (-8, 8)); set (O2Fine, R (-22, 22));
    set (O1Morph, R (0, 1)); set (O2Morph, R (0, 1)); set (O1Warp, R (-0.3f, 0.3f)); set (O2Warp, R (-0.3f, 0.3f));
    set (LfoShape, I (0, 4)); set (LfoRate, RL (0.1f, 6.f)); set (LfoDepth, 0); set (LfoDest, 0);
    set (FKey, R (0.1f, 0.6f)); set (ModType, I (0, 2)); set (ModRate, RL (0.2f, 3.f)); set (ModDepth, R (0.2f, 0.7f));
    set (DlyTime, R (180, 480)); set (DlyFb, R (0.2f, 0.45f)); set (RevSize, R (0.5f, 0.9f)); set (RevDamp, R (0.3f, 0.7f));
    set (CThresh, R (-18, -10)); set (CRatio, R (2.5f, 4.5f)); set (CAtk, RL (5, 30)); set (CRel, RL (80, 300));
    set (Master, R (0.5f, 0.62f)); set (VibRate, R (4.8f, 6.4f));

    switch (sec)
    {
        case 0: // Ambient Pads
            set (O1Wave, wave ({ Sine, Choir, Tine, Odd })); set (O2Wave, wave ({ Choir, Bow, Odd, Room }));
            set (UniVoices, I (3, 5)); set (UniSpread, R (8, 30)); set (FCut, RL (800, 5000)); set (FRes, R (0.05f, 0.3f)); set (FEnv, R (-0.2f, 0.4f));
            set (AA, RL (0.8f, 3.5f)); set (AD, RL (0.8f, 3)); set (AS, R (0.6f, 0.95f)); set (AR, RL (2, 6));
            set (FA, RL (0.8f, 3)); set (FD, RL (1, 3)); set (FS, R (0.3f, 0.9f)); set (FR, RL (2, 5));
            set (LfoDest, pick ({ 1, 3, 5 })); set (LfoDepth, R (0.1f, 0.4f)); set (LfoRate, RL (0.05f, 0.6f));
            set (RevMix, R (0.45f, 0.75f)); set (RevSize, R (0.8f, 0.98f)); set (ModMix, R (0.2f, 0.5f)); set (DlyMix, R (0.1f, 0.3f)); break;
        case 1: // Evolving Atmospheres
            set (O1Wave, wave ({ Room, Alien, Choir, FM })); set (O2Wave, wave ({ Room, Alien, Sine, Odd }));
            set (UniVoices, I (2, 5)); set (UniSpread, R (10, 35)); set (FCut, RL (600, 6000)); set (FRes, R (0.1f, 0.4f)); set (FEnv, R (-0.6f, 0.6f));
            set (AA, RL (1, 5)); set (AD, RL (1, 4)); set (AS, R (0.5f, 1)); set (AR, RL (3, 8));
            set (FA, RL (1, 5)); set (FD, RL (1, 4)); set (FS, R (0.2f, 0.9f)); set (FR, RL (2, 6));
            set (LfoDest, pick ({ 1, 3, 5 })); set (LfoDepth, R (0.4f, 1)); set (LfoRate, RL (0.05f, 0.5f));
            set (RevMix, R (0.5f, 0.85f)); set (RevSize, R (0.85f, 1)); set (DlyMix, R (0.2f, 0.5f)); set (DlyFb, R (0.4f, 0.65f)); set (ModMix, R (0.2f, 0.6f)); break;
        case 2: // Warm Keys
            set (O1Wave, wave ({ Tine, Sine })); set (O2Wave, wave ({ Tine, Odd, Organ })); set (O2Semi, pick ({ 0, 0, 12 }));
            set (UniVoices, I (1, 2)); set (UniSpread, R (2, 9)); set (FCut, RL (1500, 7000)); set (FEnv, R (0.1f, 0.5f));
            set (AA, RL (0.003f, 0.01f)); set (AD, RL (0.6f, 2)); set (AS, R (0.15f, 0.5f)); set (AR, RL (0.3f, 0.9f));
            set (FD, RL (0.3f, 1)); set (FS, R (0.1f, 0.4f)); set (ModMix, R (0.2f, 0.5f)); set (RevMix, R (0.15f, 0.35f)); break;
        case 3: // Glass Keys & Piano
            set (O1Wave, wave ({ Tine, FM, Pluck })); set (O2Wave, wave ({ Sine, FM, Tine })); set (O2Semi, pick ({ 0, 12, 19 }));
            set (UniVoices, 1); set (FCut, RL (4000, 14000)); set (FEnv, R (0.1f, 0.5f)); set (O1Oct, pick ({ 0, 0, 1 }));
            set (AA, 0.002f); set (AD, RL (0.8f, 2.5f)); set (AS, R (0.05f, 0.3f)); set (AR, RL (0.4f, 1.2f)); set (FD, RL (0.3f, 1)); set (FS, R (0.1f, 0.4f));
            set (RevMix, R (0.2f, 0.4f)); set (ModMix, R (0, 0.3f)); break;
        case 4: // Violins & Solo Strings
            set (O1Wave, wave ({ Bow })); set (O2Wave, wave ({ Bow, Bow, Choir })); set (O1Oct, pick ({ 0, 0, 1 }));
            set (UniVoices, I (2, 4)); set (UniSpread, R (6, 16)); set (FCut, RL (2500, 9000)); set (FRes, R (0.08f, 0.25f)); set (FEnv, R (0.1f, 0.3f));
            set (AA, RL (0.06f, 0.3f)); set (AD, RL (0.2f, 0.6f)); set (AS, R (0.75f, 1)); set (AR, RL (0.2f, 0.5f)); set (FA, RL (0.05f, 0.25f));
            set (VibDepth, R (0.25f, 0.6f)); set (RevMix, R (0.2f, 0.4f)); set (ModMix, R (0.05f, 0.3f)); break;
        case 5: // Orchestral Strings
            set (O1Wave, wave ({ Bow })); set (O2Wave, wave ({ Bow, Choir })); set (O1Oct, pick ({ -1, 0, 0 })); set (O2Oct, pick ({ -1, 0 }));
            set (UniVoices, 5); set (UniSpread, R (14, 35)); set (FCut, RL (1500, 6000)); set (FRes, R (0.05f, 0.2f)); set (FEnv, R (0.1f, 0.35f));
            set (AA, RL (0.2f, 0.9f)); set (AD, RL (0.3f, 1)); set (AS, R (0.7f, 1)); set (AR, RL (0.6f, 1.8f)); set (FA, RL (0.2f, 0.8f));
            set (VibDepth, R (0.15f, 0.4f)); set (RevMix, R (0.35f, 0.65f)); set (RevSize, R (0.8f, 0.95f)); set (ModMix, R (0.1f, 0.35f)); break;
        case 6: // Nylon Guitars
            set (O1Wave, wave ({ Pluck })); set (O2Wave, wave ({ Pluck, Tine })); set (UniVoices, I (1, 2)); set (UniSpread, R (2, 8));
            set (FCut, RL (1200, 5000)); set (FEnv, R (0.3f, 0.8f)); set (AA, 0.001f); set (AD, RL (0.35f, 1.2f)); set (AS, R (0, 0.12f)); set (AR, RL (0.15f, 0.5f));
            set (FD, RL (0.1f, 0.4f)); set (FS, R (0, 0.15f)); set (ModMix, R (0.1f, 0.3f)); set (RevMix, R (0.15f, 0.35f)); set (DlyMix, R (0.05f, 0.2f)); break;
        case 7: // Electric & Steel Guitars
            set (O1Wave, wave ({ Pluck, Odd, FM })); set (O2Wave, wave ({ Pluck, Bow })); set (UniVoices, I (1, 3)); set (UniSpread, R (3, 14));
            set (Drive, R (0.1f, 0.6f)); set (FCut, RL (2000, 9000)); set (FRes, R (0.05f, 0.3f)); set (FEnv, R (0.2f, 0.7f));
            set (AA, 0.001f); set (AD, RL (0.5f, 1.5f)); set (AS, R (0.1f, 0.4f)); set (AR, RL (0.15f, 0.5f)); set (FD, RL (0.1f, 0.5f)); set (FS, R (0.1f, 0.4f));
            set (ModMix, R (0.3f, 0.6f)); set (RevMix, R (0.2f, 0.4f)); set (DlyMix, R (0.1f, 0.3f)); break;
        case 8: // Reggaeton Plucks
            set (O1Wave, wave ({ Pluck, FM, Tine, Sine })); set (O2Wave, wave ({ Pluck, Odd, FM })); set (UniVoices, I (1, 3)); set (UniSpread, R (3, 14));
            set (FCut, RL (1500, 9000)); set (FRes, R (0.05f, 0.3f)); set (FEnv, R (0.4f, 0.9f));
            set (AA, 0.001f); set (AD, RL (0.12f, 0.4f)); set (AS, 0); set (AR, RL (0.1f, 0.3f)); set (FD, RL (0.08f, 0.3f)); set (FS, 0);
            set (DlyMix, R (0.15f, 0.4f)); set (DlyTime, R (200, 450)); set (DlyFb, R (0.25f, 0.5f)); set (RevMix, R (0.15f, 0.35f)); set (ModMix, R (0, 0.3f));
            arpSet (0.4f, { 4, 4, 9, 11, 5 }); break;
        case 9: // Reggaeton Leads
            set (O1Wave, wave ({ Odd, Bow, FM, Alien })); set (O2Wave, wave ({ Odd, Bow })); set (UniVoices, I (2, 4)); set (UniSpread, R (6, 22));
            set (FCut, RL (3000, 11000)); set (FRes, R (0.1f, 0.35f)); set (FEnv, R (0.1f, 0.5f)); set (Drive, R (0.1f, 0.4f));
            set (AA, RL (0.003f, 0.02f)); set (AD, RL (0.1f, 0.4f)); set (AS, R (0.7f, 1)); set (AR, RL (0.15f, 0.4f)); set (FD, RL (0.2f, 0.6f)); set (FS, R (0.4f, 0.9f));
            set (VibDepth, R (0.1f, 0.3f)); set (DlyMix, R (0.2f, 0.35f)); set (RevMix, R (0.15f, 0.3f)); set (ModMix, R (0, 0.3f)); break;
        case 10: // Dembow Bass & Subs
            set (O1Wave, wave ({ Sub })); set (O2Wave, wave ({ Sub, Sine, Odd })); set (O1Oct, pick ({ -1, -2 })); set (O2Oct, pick ({ -1, -2, 0 })); set (O2Lvl, R (0, 0.5f));
            set (UniVoices, 1); set (FCut, RL (100, 700)); set (FRes, R (0.05f, 0.3f)); set (FEnv, R (0.2f, 0.6f)); set (FKey, R (0.3f, 0.8f)); set (Drive, R (0.1f, 0.5f));
            set (AA, 0.002f); set (AD, RL (0.2f, 0.6f)); set (AS, R (0.6f, 1)); set (AR, RL (0.08f, 0.25f)); set (FD, RL (0.1f, 0.4f)); set (FS, R (0, 0.3f));
            set (RevMix, R (0, 0.08f)); set (CRatio, R (4, 8)); set (CThresh, R (-20, -12)); break;
        case 11: // Latin Trap Bells
            set (O1Wave, wave ({ Tine, FM, Sine })); set (O2Wave, wave ({ FM, Tine, Alien })); set (O2Semi, pick ({ 12, 19, 24, 7 })); set (UniVoices, I (1, 2));
            set (FCut, RL (5000, 16000)); set (FEnv, R (0, 0.3f)); set (AA, 0.002f); set (AD, RL (1.2f, 3.5f)); set (AS, 0); set (AR, RL (1, 3));
            set (RevMix, R (0.35f, 0.6f)); set (DlyMix, R (0.2f, 0.45f)); set (ModMix, R (0, 0.3f));
            arpSet (0.45f, { 9, 9, 11, 14 }); break;
        case 12: // Choir & Vocal Pads
            set (O1Wave, wave ({ Choir })); set (O2Wave, wave ({ Choir, Choir, Sine })); set (UniVoices, I (4, 5)); set (UniSpread, R (8, 28));
            set (FCut, RL (1500, 6000)); set (FRes, R (0.05f, 0.25f)); set (FEnv, R (0, 0.4f));
            set (AA, RL (0.5f, 2.5f)); set (AD, RL (0.5f, 2)); set (AS, R (0.7f, 1)); set (AR, RL (1.5f, 4)); set (FA, RL (0.5f, 2));
            set (LfoDest, 3); set (LfoDepth, R (0.1f, 0.5f)); set (LfoRate, RL (0.1f, 0.8f)); set (VibDepth, R (0, 0.2f));
            set (RevMix, R (0.4f, 0.7f)); set (RevSize, R (0.8f, 0.97f)); set (ModMix, R (0.2f, 0.5f)); break;
        case 13: // Synth Leads
            set (O1Wave, wave ({ Odd, FM, Bow, Room })); set (O2Wave, wave ({ Odd, FM })); set (UniVoices, I (2, 5)); set (UniSpread, R (6, 28));
            set (FCut, RL (3000, 12000)); set (FRes, R (0.1f, 0.45f)); set (FEnv, R (0.1f, 0.6f)); set (Drive, R (0, 0.35f));
            set (AA, RL (0.002f, 0.03f)); set (AD, RL (0.1f, 0.5f)); set (AS, R (0.7f, 1)); set (AR, RL (0.1f, 0.5f)); set (FD, RL (0.2f, 0.8f)); set (FS, R (0.4f, 0.9f));
            set (VibDepth, R (0, 0.25f)); set (DlyMix, R (0.2f, 0.45f)); set (RevMix, R (0.12f, 0.3f)); set (ModMix, R (0, 0.35f)); break;
        case 14: // Brass & Winds
            set (O1Wave, wave ({ Bow, Odd })); set (O2Wave, wave ({ Choir, Odd, Bow })); set (UniVoices, I (1, 3)); set (UniSpread, R (3, 12));
            set (FCut, RL (900, 3500)); set (FRes, R (0.05f, 0.25f)); set (FEnv, R (0.4f, 0.8f));
            set (AA, RL (0.03f, 0.12f)); set (AD, RL (0.2f, 0.5f)); set (AS, R (0.6f, 0.9f)); set (AR, RL (0.1f, 0.3f)); set (FA, RL (0.04f, 0.15f)); set (FD, RL (0.2f, 0.5f)); set (FS, R (0.4f, 0.8f));
            set (VibDepth, R (0.1f, 0.25f)); set (RevMix, R (0.15f, 0.3f)); break;
        case 15: // Bachata & Merengue Hooks
            set (O1Wave, wave ({ Pluck })); set (O2Wave, wave ({ Pluck, Tine })); set (UniVoices, I (1, 2));
            set (FCut, RL (3000, 11000)); set (FEnv, R (0.2f, 0.7f)); set (AA, 0.001f); set (AD, RL (0.25f, 0.7f)); set (AS, R (0, 0.15f)); set (AR, RL (0.12f, 0.4f));
            set (FD, RL (0.1f, 0.4f)); set (FS, R (0, 0.2f)); set (ModMix, R (0.2f, 0.5f)); set (DlyMix, R (0.15f, 0.3f)); set (RevMix, R (0.15f, 0.35f));
            arpSet (0.35f, { 5, 11, 0, 1 }); break;
        case 16: // Bells & Mallets
            set (O1Wave, wave ({ Tine, FM, Sine })); set (O2Wave, wave ({ Tine, FM })); set (O2Semi, pick ({ 12, 19, 24, 28 })); set (UniVoices, I (1, 3));
            set (FCut, RL (4000, 16000)); set (FEnv, R (0, 0.3f)); set (AA, 0.002f); set (AD, RL (1.5f, 5)); set (AS, R (0, 0.1f)); set (AR, RL (1.5f, 5));
            set (RevMix, R (0.3f, 0.6f)); set (RevSize, R (0.7f, 0.95f)); set (ModMix, R (0, 0.3f)); break;
        case 17: // Risers & Impacts
            set (O1Wave, wave ({ Room, Alien, FM })); set (O2Wave, wave ({ Room, Alien, Odd })); set (UniVoices, I (2, 5)); set (UniSpread, R (10, 40));
            set (FType, pick ({ 0, 2 })); set (FCut, RL (300, 3000)); set (FRes, R (0.2f, 0.6f)); set (FEnv, R (0.7f, 1));
            if (rnd.nextBool()) { set (AA, RL (2, 8)); set (AD, 1); set (AS, R (0.7f, 1)); set (AR, RL (0.5f, 2)); set (FA, RL (2, 8)); set (LfoDest, 1); set (LfoDepth, R (0.4f, 1)); }
            else                { set (AA, 0.001f); set (AD, RL (1, 4)); set (AS, 0); set (AR, RL (1, 4)); set (FA, 0.001f); set (FD, RL (0.8f, 3)); set (FS, 0); set (O1Oct, pick ({ -2, -1 })); }
            set (RevMix, R (0.5f, 0.9f)); set (DlyMix, R (0.2f, 0.5f)); set (ModMix, R (0.2f, 0.6f)); break;
        case 18: // Glitch & Resynth FX
            set (O1Wave, wave ({ Alien, Room, FM })); set (O2Wave, wave ({ Alien, Room, FM })); set (UniVoices, I (1, 4)); set (UniSpread, R (5, 30));
            set (FType, I (0, 2)); set (FCut, RL (500, 10000)); set (FRes, R (0.2f, 0.6f)); set (FEnv, R (-0.8f, 0.8f)); set (Drive, R (0.2f, 0.7f));
            set (AA, 0.001f); set (AD, RL (0.1f, 0.6f)); set (AS, R (0, 0.6f)); set (AR, RL (0.1f, 0.6f));
            set (LfoDest, pick ({ 3, 5, 1 })); set (LfoDepth, R (0.6f, 1)); set (LfoRate, RL (3, 15)); set (DlyMix, R (0.2f, 0.5f)); set (DlyFb, R (0.5f, 0.8f)); set (RevMix, R (0.2f, 0.5f));
            arpSet (0.7f, { 8, 9, 13, 12 }); break;
        default: // 19 Dark Drones
            set (O1Wave, wave ({ Sub, Room, Alien })); set (O2Wave, wave ({ Room, Choir, Alien })); set (O1Oct, -1); set (UniVoices, I (2, 5)); set (UniSpread, R (6, 30));
            set (FCut, RL (150, 900)); set (FRes, R (0.2f, 0.5f)); set (FEnv, R (0, 0.4f)); set (Drive, R (0.1f, 0.4f));
            set (AA, RL (2, 6)); set (AD, RL (1, 3)); set (AS, R (0.7f, 1)); set (AR, RL (4, 8)); set (FA, RL (2, 6)); set (FD, RL (1, 4)); set (FS, R (0.3f, 0.9f));
            set (LfoDest, 1); set (LfoDepth, R (0.3f, 0.8f)); set (LfoRate, RL (0.05f, 0.4f)); set (RevMix, R (0.6f, 0.9f)); set (RevSize, R (0.85f, 1)); set (ModMix, R (0.2f, 0.5f)); break;
    }
    return v;
}
} // namespace presets
} // namespace attia
