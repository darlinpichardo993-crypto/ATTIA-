#pragma once
#include <juce_graphics/juce_graphics.h>

// Iconos vectoriales de ATTIA (dibujados en codigo, escalan a cualquier tamano)
// 0 Pads · 1 Keys · 2 Strings · 3 Guitars · 4 Plucks · 5 Leads · 6 Bass · 7 FX
// 8 Osc · 9 Filter · 10 Env · 11 LFO · 12 Comp · 13 Delay · 14 Output · 15 Search · 16 Logo
// 17 Drive · 18 Reverb · 19 Arp · 20 Modulation · 21 Unison
namespace attia
{
inline void drawIcon (juce::Graphics& g, int kind, juce::Rectangle<float> r, juce::Colour col, float th = 1.4f)
{
    using Pt = juce::Point<float>;
    auto P = [&] (float x, float y) { return Pt (r.getX() + x * r.getWidth(), r.getY() + y * r.getHeight()); };
    auto line = [&] (float x1, float y1, float x2, float y2) { g.drawLine (juce::Line<float> (P (x1, y1), P (x2, y2)), th); };
    auto poly = [&] (std::initializer_list<Pt> pts, bool close)
    {
        juce::Path p; bool first = true;
        for (auto q : pts) { if (first) { p.startNewSubPath (q); first = false; } else p.lineTo (q); }
        if (close) p.closeSubPath();
        g.strokePath (p, juce::PathStrokeType (th, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
    };
    auto wave = [&] (float y0, float amp, float cycles)
    {
        juce::Path p;
        for (int i = 0; i <= 40; ++i)
        {
            const float t = (float) i / 40.f;
            const auto q = P (t, y0 - amp * std::sin (t * cycles * 6.2831853f));
            if (i == 0) p.startNewSubPath (q); else p.lineTo (q);
        }
        g.strokePath (p, juce::PathStrokeType (th));
    };
    auto circle = [&] (float cx, float cy, float rad, bool fill)
    {
        auto c = P (cx, cy); const float rr = rad * r.getWidth();
        if (fill) g.fillEllipse (c.x - rr, c.y - rr, rr * 2.f, rr * 2.f);
        else      g.drawEllipse (c.x - rr, c.y - rr, rr * 2.f, rr * 2.f, th);
    };

    g.setColour (col);
    switch (kind)
    {
        case 0: wave (0.25f, 0.12f, 1.5f); wave (0.5f, 0.12f, 1.5f); wave (0.75f, 0.12f, 1.5f); break;
        case 1:
            g.drawRect (r.reduced (r.getWidth() * 0.05f, r.getHeight() * 0.2f), th);
            line (0.35f, 0.2f, 0.35f, 0.8f); line (0.65f, 0.2f, 0.65f, 0.8f);
            g.fillRect (P (0.28f, 0.2f).x, P (0.28f, 0.2f).y, r.getWidth() * 0.14f, r.getHeight() * 0.35f);
            g.fillRect (P (0.58f, 0.2f).x, P (0.58f, 0.2f).y, r.getWidth() * 0.14f, r.getHeight() * 0.35f); break;
        case 2:   // violin: cuerdas + arco
            line (0.3f, 0.1f, 0.3f, 0.9f); line (0.42f, 0.1f, 0.42f, 0.9f); line (0.54f, 0.1f, 0.54f, 0.9f); line (0.66f, 0.1f, 0.66f, 0.9f);
            g.drawLine (juce::Line<float> (P (0.08f, 0.8f), P (0.95f, 0.3f)), th * 1.8f); break;
        case 3:   // guitarra: cuerpo + mastil
            circle (0.38f, 0.64f, 0.26f, false); circle (0.38f, 0.64f, 0.07f, true); line (0.55f, 0.48f, 0.94f, 0.08f); line (0.6f, 0.43f, 0.7f, 0.5f); break;
        case 4: { juce::Path d; d.startNewSubPath (P (0.5f, 0.05f)); d.lineTo (P (0.2f, 0.58f));
                  d.addCentredArc (P (0.5f, 0.64f).x, P (0.5f, 0.64f).y, r.getWidth() * 0.3f, r.getHeight() * 0.3f, 0.f, 3.7f, 8.8f, false);
                  d.closeSubPath(); g.strokePath (d, juce::PathStrokeType (th)); } break;
        case 5: poly ({ P (0.6f, 0.05f), P (0.2f, 0.55f), P (0.48f, 0.55f), P (0.38f, 0.95f), P (0.8f, 0.4f), P (0.52f, 0.4f) }, true); break;
        case 6: circle (0.5f, 0.5f, 0.42f, false); circle (0.5f, 0.5f, 0.2f, false); circle (0.5f, 0.5f, 0.06f, true); break;
        case 7: poly ({ P (0.5f, 0.02f), P (0.6f, 0.4f), P (0.98f, 0.5f), P (0.6f, 0.6f), P (0.5f, 0.98f), P (0.4f, 0.6f), P (0.02f, 0.5f), P (0.4f, 0.4f) }, true); break;
        case 8: wave (0.5f, 0.3f, 1.f); break;
        case 9: poly ({ P (0.05f, 0.3f), P (0.5f, 0.3f), P (0.65f, 0.15f), P (0.8f, 0.7f), P (0.95f, 0.85f) }, false); break;
        case 10: poly ({ P (0.05f, 0.85f), P (0.3f, 0.15f), P (0.5f, 0.5f), P (0.75f, 0.5f), P (0.95f, 0.85f) }, false); break;
        case 11: poly ({ P (0.05f, 0.5f), P (0.25f, 0.15f), P (0.5f, 0.85f), P (0.75f, 0.15f), P (0.95f, 0.5f) }, false); break;
        case 12: poly ({ P (0.05f, 0.95f), P (0.5f, 0.5f), P (0.95f, 0.3f) }, false); line (0.05f, 0.05f, 0.05f, 0.95f); line (0.05f, 0.95f, 0.95f, 0.95f); break;
        case 13: line (0.15f, 0.2f, 0.15f, 0.8f); line (0.4f, 0.3f, 0.4f, 0.7f); line (0.65f, 0.38f, 0.65f, 0.62f); line (0.88f, 0.44f, 0.88f, 0.56f); break;
        case 14: g.drawRect (r.reduced (r.getWidth() * 0.05f, r.getHeight() * 0.15f), th); wave (0.5f, 0.2f, 1.5f); break;
        case 15: circle (0.42f, 0.42f, 0.28f, false); line (0.62f, 0.62f, 0.92f, 0.92f); break;
        case 17: poly ({ P (0.05f, 0.5f), P (0.2f, 0.2f), P (0.42f, 0.2f), P (0.58f, 0.8f), P (0.8f, 0.8f), P (0.95f, 0.5f) }, false); break;
        case 18: { for (int i = 1; i <= 3; ++i) { juce::Path a; const float rad = 0.16f * (float) i;
                       a.addCentredArc (P (0.15f, 0.5f).x, P (0.15f, 0.5f).y, r.getWidth() * rad, r.getHeight() * rad, 0.f, 0.6f, 2.5f, true);
                       g.strokePath (a, juce::PathStrokeType (th)); } } break;
        case 19: for (int i = 0; i < 4; ++i) { const float h = 0.2f + 0.2f * (float) i, x = 0.1f + 0.2f * (float) i;
                     g.fillRect (P (x, 0.9f - h).x, P (x, 0.9f - h).y, r.getWidth() * 0.15f, r.getHeight() * h); } break;
        case 20: wave (0.35f, 0.18f, 1.5f); wave (0.68f, 0.18f, 2.5f); break;
        case 21: circle (0.33f, 0.5f, 0.28f, false); circle (0.67f, 0.5f, 0.28f, false); circle (0.5f, 0.5f, 0.05f, true); break;
        default:  // logo ATTIA: hexagono + "A"
            poly ({ P (0.5f, 0.02f), P (0.93f, 0.26f), P (0.93f, 0.74f), P (0.5f, 0.98f), P (0.07f, 0.74f), P (0.07f, 0.26f) }, true);
            poly ({ P (0.28f, 0.72f), P (0.5f, 0.22f), P (0.72f, 0.72f) }, false); line (0.37f, 0.54f, 0.63f, 0.54f); break;
    }
}
}
