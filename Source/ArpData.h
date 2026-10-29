#pragma once
#include <juce_core/juce_core.h>
#include "ArpNames.h"

// 15.000 arpegios: 15 bancos x 1.000. Cada patron es un loop melodico (motivo que se repite con "respuestas"
// transpuestas) sobre una escala mayor / menor / modal. Generacion determinista: el arpegio #N siempre es el mismo.
namespace attia { namespace arp
{
constexpr int perBank = 1000, numBanks = 15, total = perBank * numBanks;

struct Step { signed char deg = 0, oct = 0; float gate = 0.6f, vel = 0.8f; bool rest = false, chord = false; };
struct Pattern { int n = 8; int scale = 0; Step s[32]; };

inline juce::String name (int idx) { return juce::String (kArpNames[juce::jlimit (0, total - 1, idx)]); }

inline const char* scaleName (int s)
{
    static const char* n[] = { "Major", "Minor", "Harm Minor", "Dorian", "Phrygian", "Pent Minor", "Pent Major", "Mixolydian", "Lydian" };
    return n[juce::jlimit (0, 8, s)];
}

inline int degToSemi (int scale, int deg)
{
    static const int len[9] = { 7, 7, 7, 7, 7, 5, 5, 7, 7 };
    static const int t[9][7] = { { 0, 2, 4, 5, 7, 9, 11 }, { 0, 2, 3, 5, 7, 8, 10 }, { 0, 2, 3, 5, 7, 8, 11 },
                                 { 0, 2, 3, 5, 7, 9, 10 }, { 0, 1, 3, 5, 7, 8, 10 }, { 0, 3, 5, 7, 10, 0, 0 },
                                 { 0, 2, 4, 7, 9, 0, 0 }, { 0, 2, 4, 5, 7, 9, 10 }, { 0, 2, 4, 6, 7, 9, 11 } };
    const int n = len[scale];
    const int o = deg >= 0 ? deg / n : -((-deg + n - 1) / n);
    return t[scale][deg - o * n] + 12 * o;
}

struct BankCfg { const char* name; int ns; int scales[4]; float chordP; int rhythm; float gate; int octBias; bool rootHeavy; };

inline const BankCfg& cfg (int b)
{
    static const BankCfg c[numBanks] = {
        { "Major Melody Loops",    3, { 0, 8, 7, 0 }, 0.08f, 2, 0.60f,  0, false },
        { "Minor Melody Loops",    3, { 1, 3, 2, 0 }, 0.08f, 2, 0.60f,  0, false },
        { "Harmonic Minor Drama",  1, { 2, 0, 0, 0 }, 0.12f, 2, 0.70f,  0, false },
        { "Dorian Groove",         2, { 3, 1, 0, 0 }, 0.05f, 2, 0.50f,  0, false },
        { "Dembow Reggaeton Arps", 3, { 1, 5, 4, 0 }, 0.10f, 1, 0.45f,  0, false },
        { "Pentatonic Hooks",      2, { 5, 6, 0, 0 }, 0.05f, 2, 0.55f,  0, false },
        { "Phrygian Dark",         2, { 4, 2, 0, 0 }, 0.08f, 3, 0.60f,  0, false },
        { "Mixolydian Pop",        2, { 7, 0, 0, 0 }, 0.08f, 2, 0.55f,  0, false },
        { "Trance Gates",          2, { 1, 5, 0, 0 }, 0.00f, 0, 0.50f,  0, false },
        { "Latin Trap Arps",       3, { 1, 4, 5, 0 }, 0.05f, 3, 0.35f,  0, false },
        { "Ambient Cascades",      3, { 0, 8, 6, 0 }, 0.00f, 0, 0.95f,  0, false },
        { "Pluck Sequences",       4, { 0, 1, 5, 6 }, 0.03f, 2, 0.30f,  0, false },
        { "Chord Stab Rhythms",    2, { 0, 1, 0, 0 }, 1.00f, 4, 0.25f,  0, false },
        { "Bass Walk Patterns",    3, { 1, 3, 5, 0 }, 0.00f, 2, 0.70f, -1, true  },
        { "Lydian Dream",          2, { 8, 0, 0, 0 }, 0.05f, 3, 0.80f,  0, false } };
    return c[juce::jlimit (0, numBanks - 1, b)];
}

inline const char* bankName (int b) { return cfg (b).name; }

inline Pattern make (int idx)
{
    idx = juce::jlimit (0, total - 1, idx);
    const auto& c = cfg (idx / perBank);
    juce::Random r ((juce::int64) idx * 7919 + 31);
    Pattern p;
    p.scale = c.scales[r.nextInt (c.ns)];
    static const int lens[7] = { 8, 12, 16, 16, 16, 24, 32 };
    p.n = c.rhythm == 4 ? (r.nextBool() ? 8 : 16) : lens[r.nextInt (7)];
    static const int mls[6] = { 3, 4, 4, 5, 6, 8 };
    const int m = mls[r.nextInt (6)];

    static const int stepT[10] = { -2, -1, -1, 1, 1, 1, 2, 3, -3, 2 };
    int motif[8], d = 0;
    for (int i = 0; i < m; ++i) { motif[i] = d; d = juce::jlimit (-3, 9, d + stepT[r.nextInt (10)]); }

    static const int shT[7] = { 0, 1, 2, -1, 3, -2, 4 };
    int shift[4] = { 0, shT[r.nextInt (7)], r.nextBool() ? 0 : shT[r.nextInt (7)], shT[r.nextInt (7)] };

    for (int k = 0; k < p.n; ++k)
    {
        Step& st = p.s[k];
        const int rep = k / m;
        int base = motif[k % m] + shift[rep & 3];
        int oct = c.octBias;
        if ((rep & 3) == 3 && r.nextFloat() < 0.4f) ++oct;
        if (k % m == m - 1 && r.nextFloat() < 0.25f) ++oct;
        if (k >= m && r.nextFloat() < 0.14f) base += r.nextBool() ? 1 : -1;      // variaciones: cada patron es unico
        if (c.rootHeavy && r.nextFloat() < 0.5f) base = r.nextBool() ? 0 : 4;

        bool hit = true;
        switch (c.rhythm)
        {
            case 1:  { const int q = k % 8; hit = (q == 0 || q == 3 || q == 6) || r.nextFloat() < 0.25f; break; }   // dembow 3+3+2
            case 2:  hit = (k % 4 == 0) || r.nextFloat() < 0.6f; break;                                              // sincopado
            case 3:  hit = (k % 4 == 0) || r.nextFloat() < 0.22f; break;                                             // disperso
            case 4:  { const int q = k % 16; hit = (q == 0 || q == 3 || q == 6 || q == 10 || q == 12) || r.nextFloat() < 0.08f; break; } // stabs
            default: break;
        }
        st.rest  = ! hit;
        st.deg   = (signed char) base;
        st.oct   = (signed char) oct;
        st.gate  = juce::jlimit (0.08f, 0.98f, c.gate * (0.8f + 0.4f * r.nextFloat()));
        st.vel   = (k % 4 == 0) ? 0.95f : 0.55f + 0.3f * r.nextFloat();
        st.chord = r.nextFloat() < c.chordP;
    }
    return p;
}
} }
