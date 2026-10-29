#include "PluginEditor.h"

using namespace attia;

static void spaced (juce::Graphics& g, const juce::String& t, float x, float y, float size, bool bold,
                    float spacing, juce::Colour col, bool centre = false)
{
    juce::Font f (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
    g.setFont (f); g.setColour (col);
    float total = 0.f;
    for (auto c : t) total += f.getStringWidthFloat (juce::String::charToString (c)) + spacing;
    float cx = centre ? x - (total - spacing) * 0.5f : x;
    for (auto c : t)
    {
        auto s = juce::String::charToString (c);
        g.drawSingleLineText (s, juce::roundToInt (cx), juce::roundToInt (y));
        cx += f.getStringWidthFloat (s) + spacing;
    }
}

static void drawPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title, int icon)
{
    auto f = r.toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xFF0E1A1C), f.getX(), f.getY(), juce::Colour (0xFF070C0D), f.getX(), f.getBottom(), false));
    g.fillRoundedRectangle (f, 4.f);
    g.setColour (juce::Colours::white.withAlpha (0.10f)); g.drawRoundedRectangle (f, 4.f, 1.f);
    g.setColour (kAccent);
    const float L = 10.f;
    g.drawLine (f.getX(), f.getY() + L, f.getX(), f.getY() + 4.f, 1.5f); g.drawLine (f.getX() + 4.f, f.getY(), f.getX() + L, f.getY(), 1.5f);
    g.drawLine (f.getRight(), f.getBottom() - L, f.getRight(), f.getBottom() - 4.f, 1.5f); g.drawLine (f.getRight() - 4.f, f.getBottom(), f.getRight() - L, f.getBottom(), 1.5f);
    drawIcon (g, icon, { f.getX() + 10.f, f.getY() + 6.f, 14.f, 14.f }, kAccent, 1.2f);
    spaced (g, title, f.getX() + 30.f, f.getY() + 17.f, 10.f, true, 3.f, juce::Colours::white.withAlpha (0.8f));
}

AttiaEditor::AttiaEditor (AttiaProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&look);
    auto& ap = proc.apvts;

    for (int i = 0; i < NumParams; ++i)
    {
        const auto& d = defs()[i];
        if (d.kind <= 1 && i != ArpPat)
        {
            knobs[(size_t) i] = std::make_unique<Knob> (ap, d.id, d.name, (i == O1Wave || i == O2Wave) ? 2500 : 0);
            addAndMakeVisible (*knobs[(size_t) i]);
        }
        else if (d.kind == 2)
        {
            combos[(size_t) i] = std::make_unique<Combo> (ap, d.id, juce::StringArray::fromTokens (d.choices, "|", ""));
            addAndMakeVisible (*combos[(size_t) i]);
        }
    }
    addAndMakeVisible (arpBtn);
    arpAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (ap, defs()[ArpOn].id, arpBtn);

    for (auto* b : { &prevBtn, &nextBtn, &prevPat, &nextPat, &rndPat }) addAndMakeVisible (*b);
    prevBtn.onClick = [this] { step (-1); };
    nextBtn.onClick = [this] { step (+1); };
    prevPat.onClick = [this] { stepPattern (-1); };
    nextPat.onClick = [this] { stepPattern (+1); };
    rndPat.onClick  = [this] { proc.setParam (ArpPat, (float) rnd.nextInt (arp::perBank)); };

    for (int k = 0; k < 8; ++k)
    {
        catBtns[(size_t) k] = std::make_unique<IconBtn> (k);
        catBtns[(size_t) k]->onClick = [this, k]
        {
            selCat = (selCat == k) ? -1 : k;
            for (int j = 0; j < 8; ++j) catBtns[(size_t) j]->setToggleState (j == selCat, juce::dontSendNotification);
            fillSectionBox(); rebuild();
        };
        addAndMakeVisible (*catBtns[(size_t) k]);
    }
    secBox.onChange = [this] { rebuild(); };
    addAndMakeVisible (secBox);
    fillSectionBox();

    search.setTextToShowWhenEmpty ("search 20 000 sounds...", juce::Colours::white.withAlpha (0.4f));
    search.onTextChange = [this] { rebuild(); };
    addAndMakeVisible (search);

    list.setModel (this);
    list.setRowHeight (22);
    addAndMakeVisible (list);

    setSize (1280, 760);
    rebuild();
    startTimerHz (30);
}

AttiaEditor::~AttiaEditor()
{
    stopTimer();
    list.setModel (nullptr);
    setLookAndFeel (nullptr);
}

void AttiaEditor::fillSectionBox()
{
    secBox.clear (juce::dontSendNotification);
    secOfItem.clear();
    secBox.addItem (selCat < 0 ? juce::String ("ALL SECTIONS (20)") : "ALL " + juce::String (presets::catName (selCat)).toUpperCase(), 1);
    secOfItem.push_back (-1);
    for (int s = 0; s < presets::numSections; ++s)
        if (selCat < 0 || presets::sectionCat (s) == selCat)
        {
            secBox.addItem (juce::String (presets::sectionName (s)).toUpperCase(), (int) secOfItem.size() + 1);
            secOfItem.push_back (s);
        }
    secBox.setSelectedId (1, juce::dontSendNotification);
}

void AttiaEditor::rebuild()
{
    filtered.clear();
    const int item = secBox.getSelectedId() - 1;
    const int sec = (item >= 0 && item < (int) secOfItem.size()) ? secOfItem[(size_t) item] : -1;
    const auto q = search.getText().trim().toLowerCase();
    for (int s = 0; s < presets::numSections; ++s)
    {
        if (sec >= 0 && s != sec) continue;
        if (sec < 0 && selCat >= 0 && presets::sectionCat (s) != selCat) continue;
        const auto sname = juce::String (presets::sectionName (s)).toLowerCase();
        for (int i = s * presets::perSection; i < (s + 1) * presets::perSection; ++i)
            if (q.isEmpty() || sname.contains (q) || presets::name (i).toLowerCase().contains (q))
                filtered.push_back (i);
    }
    list.updateContent();
    syncList();
}

void AttiaEditor::syncList()
{
    ignoreSel = true;
    auto it = std::find (filtered.begin(), filtered.end(), proc.currentPreset.load());
    if (it != filtered.end()) list.selectRow ((int) (it - filtered.begin())); else list.deselectAllRows();
    ignoreSel = false;
}

void AttiaEditor::step (int d)
{
    if (filtered.empty()) return;
    auto it = std::find (filtered.begin(), filtered.end(), proc.currentPreset.load());
    const int sz = (int) filtered.size();
    int pos = it == filtered.end() ? 0 : (int) (it - filtered.begin()) + d;
    pos = ((pos % sz) + sz) % sz;
    proc.loadPreset (filtered[(size_t) pos]);
    syncList();
}

void AttiaEditor::stepPattern (int d)
{
    const int cur = (int) proc.getParam (ArpPat);
    proc.setParam (ArpPat, (float) (((cur + d) % arp::perBank + arp::perBank) % arp::perBank));
}

void AttiaEditor::paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected)
{
    if (row < 0 || row >= (int) filtered.size()) return;
    const int idx = filtered[(size_t) row];
    if (selected) g.fillAll (kAccent);
    drawIcon (g, presets::catOf (idx), { 6.f, (float) h * 0.5f - 7.f, 14.f, 14.f }, selected ? juce::Colours::black : kAccent.withAlpha (0.85f), 1.1f);
    g.setColour (selected ? juce::Colours::black : juce::Colours::white.withAlpha (0.85f));
    g.setFont (juce::Font (juce::FontOptions (12.f)));
    g.drawText (presets::name (idx), 28, 0, w - 74, h, juce::Justification::centredLeft);
    g.setColour (selected ? juce::Colours::black.withAlpha (0.6f) : juce::Colours::white.withAlpha (0.4f));
    g.setFont (juce::Font (juce::FontOptions (9.f)));
    g.drawText (juce::String (idx % presets::perSection + 1).paddedLeft ('0', 4), w - 48, 0, 42, h, juce::Justification::centredRight);
}

void AttiaEditor::selectedRowsChanged (int row)
{
    if (! ignoreSel && row >= 0 && row < (int) filtered.size())
        proc.loadPreset (filtered[(size_t) row]);
}

void AttiaEditor::paintLens (juce::Graphics& g, juce::Rectangle<float> r, int o)
{
    const auto c = r.getCentre();
    const float R = juce::jmin (r.getWidth(), r.getHeight()) * 0.5f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xFF0E2A29), c.x, c.y, juce::Colour (0xFF020506), c.x + R, c.y + R, true));
    g.fillEllipse (r);
    g.setColour (kAccent.withAlpha (0.45f)); g.drawEllipse (r.reduced (1.f), 2.f);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawEllipse (c.x - R * 0.33f, c.y - R * 0.33f, R * 0.66f, R * 0.66f, 1.f);
    g.drawEllipse (c.x - R * 0.66f, c.y - R * 0.66f, R * 1.32f, R * 1.32f, 1.f);
    g.drawLine (c.x - R, c.y, c.x + R, c.y, 1.f); g.drawLine (c.x, c.y - R, c.x, c.y + R, 1.f);

    auto& wb = bank();
    const int b = O1Wave + o * kOscBlock;
    float pos = wb.mapPos (proc.getParam (b)) + proc.getParam (b + 1);
    pos = juce::jlimit (0.f, (float) (wb.numWaves - 1) - 0.001f, pos);
    const int i0 = (int) pos; const float fr = pos - (float) i0;
    const float* tA = wb.wave (i0); const float* tB = wb.wave (juce::jmin (i0 + 1, wb.numWaves - 1));
    juce::Path path;
    const int pts = 240;
    for (int k = 0; k <= pts; ++k)
    {
        const int si = (k % pts) * kWaveLen / pts;
        const float s = tA[si] + (tB[si] - tA[si]) * fr;
        const float th = juce::MathConstants<float>::twoPi * (float) k / (float) pts - juce::MathConstants<float>::halfPi;
        const float rr = R * (0.5f + 0.38f * s);
        const juce::Point<float> q (c.x + rr * std::cos (th), c.y + rr * std::sin (th));
        if (k == 0) path.startNewSubPath (q); else path.lineTo (q);
    }
    path.closeSubPath();
    g.setColour (kAccent.withAlpha (0.08f)); g.fillPath (path);
    g.setColour (kAccent.withAlpha (0.25f)); g.strokePath (path, juce::PathStrokeType (6.f));
    g.setColour (kAccent); g.strokePath (path, juce::PathStrokeType (1.6f));

    const float t = (float) juce::Time::getMillisecondCounterHiRes() * 0.0009f + (float) o * 2.f;
    g.setColour (kAccent.withAlpha (0.18f));
    g.drawLine (c.x, c.y, c.x + R * std::cos (t), c.y + R * std::sin (t), 1.2f);
}

void AttiaEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF04080A));
    g.setColour (juce::Colours::white.withAlpha (0.022f));
    for (int x = 0; x < getWidth(); x += 40)  g.drawVerticalLine (x, 0.f, (float) getHeight());
    for (int y = 0; y < getHeight(); y += 40) g.drawHorizontalLine (y, 0.f, (float) getWidth());

    // cabecera
    drawIcon (g, 16, { 20.f, 16.f, 48.f, 48.f }, kAccent, 2.2f);
    spaced (g, "ATTIA", 80.f, 50.f, 34.f, true, 8.f, juce::Colours::white);
    spaced (g, "SYNTH EFFECT  /  MADE BY ORYMY", 82.f, 68.f, 9.f, false, 3.f, kAccent.withAlpha (0.85f));
    g.setColour (kAccent.withAlpha (0.25f)); g.drawHorizontalLine (78, 16.f, (float) getWidth() - 16.f);

    const int cur = proc.currentPreset.load();
    juce::Rectangle<float> pr (330.f, 18.f, 460.f, 50.f);
    g.setColour (kAccent.withAlpha (0.55f)); g.drawRoundedRectangle (pr, 4.f, 1.f);
    drawIcon (g, presets::catOf (cur), { pr.getX() + 12.f, pr.getY() + 11.f, 28.f, 28.f }, kAccent, 1.6f);
    spaced (g, presets::name (cur).toUpperCase(), pr.getCentreX() + 12.f, 41.f, 16.f, true, 2.f, juce::Colours::white, true);
    spaced (g, juce::String (presets::sectionName (presets::sectionOf (cur))).toUpperCase() + "   /   " + juce::String (cur % presets::perSection + 1).paddedLeft ('0', 4) + " OF 1000",
            pr.getCentreX() + 12.f, 58.f, 9.f, false, 3.f, juce::Colours::white.withAlpha (0.5f), true);
    spaced (g, "20 000 SOUNDS", 915.f, 38.f, 9.f, true, 3.f, juce::Colours::white.withAlpha (0.55f), true);
    spaced (g, "15 000 ARPS", 915.f, 54.f, 9.f, true, 3.f, kAccent.withAlpha (0.8f), true);

    // mini osciloscopio de salida
    {
        auto area = scopeRect.toFloat();
        g.setColour (kAccent.withAlpha (0.35f)); g.drawRoundedRectangle (area, 4.f, 1.f);
        float tmp[1024]; const int pos = proc.scopePos.load();
        for (int k = 0; k < 1024; ++k) tmp[k] = proc.scopeBuf[(pos + k) & 1023];
        int start = 0;
        for (int k = 0; k < 400; ++k) if (tmp[k] <= 0.f && tmp[k + 1] > 0.f) { start = k; break; }
        juce::Path sp; auto in = area.reduced (4.f, 6.f);
        for (int i = 0; i < 256; ++i)
        {
            const float x = in.getX() + in.getWidth() * (float) i / 255.f;
            const float y = in.getCentreY() - juce::jlimit (-1.f, 1.f, tmp[start + i * 2]) * in.getHeight() * 0.5f;
            if (i == 0) sp.startNewSubPath (x, y); else sp.lineTo (x, y);
        }
        g.setColour (kAccent.withAlpha (0.25f)); g.strokePath (sp, juce::PathStrokeType (3.f));
        g.setColour (kAccent); g.strokePath (sp, juce::PathStrokeType (1.1f));
    }

    for (auto& p : panels) drawPanel (g, p.r, p.title, p.icon);

    // lentes de resintesis
    for (int o = 0; o < 2; ++o)
    {
        paintLens (g, lens[o].toFloat(), o);
        const int w = (int) proc.getParam (O1Wave + o * kOscBlock);
        spaced (g, juce::String (familyName (w / kPerFamily)).toUpperCase() + "  " + juce::String (w % kPerFamily + 1).paddedLeft ('0', 3),
                (float) waveLabel[o].getCentreX(), (float) waveLabel[o].getBottom() - 3.f, 9.f, true, 2.f, juce::Colours::white.withAlpha (0.85f), true);
        if (! bank().fromFile)
            spaced (g, "BANK NOT FOUND - BASIC WAVES", (float) waveLabel[o].getCentreX(), (float) waveLabel[o].getBottom() + 12.f, 8.f, false, 1.f, juce::Colours::orange, true);
    }

    // arpegio: nombre y barras del patron
    {
        const int idx = (int) proc.getParam (ArpBank) * arp::perBank + (int) proc.getParam (ArpPat);
        g.setColour (kAccent.withAlpha (0.35f)); g.drawRoundedRectangle (patLabel.toFloat(), 3.f, 1.f);
        spaced (g, arp::name (idx).toUpperCase(), (float) patLabel.getCentreX(), (float) patLabel.getCentreY() - 1.f, 11.f, true, 1.5f, juce::Colours::white, true);
        spaced (g, juce::String ((int) proc.getParam (ArpPat) + 1) + " / 1000", (float) patLabel.getCentreX(), (float) patLabel.getBottom() - 3.f, 8.f, false, 2.f, juce::Colours::white.withAlpha (0.45f), true);

        const auto pt = arp::make (idx);
        const int sc = (int) proc.getParam (ArpScale);
        auto area = patBars.toFloat();
        g.setColour (juce::Colours::white.withAlpha (0.05f)); g.fillRoundedRectangle (area, 3.f);
        const float bw = area.getWidth() / (float) pt.n;
        for (int k = 0; k < pt.n; ++k)
        {
            const auto& st = pt.s[k];
            const float v = juce::jlimit (0.f, 1.f, ((float) st.deg + 4.f + 7.f * (float) st.oct) / 18.f);
            const float h = st.rest ? 3.f : 6.f + v * (area.getHeight() - 14.f);
            g.setColour (st.rest ? juce::Colours::white.withAlpha (0.15f) : kAccent.withAlpha (0.35f + 0.55f * st.vel));
            g.fillRoundedRectangle (area.getX() + (float) k * bw + 1.5f, area.getBottom() - 3.f - h, bw - 3.f, h, 1.5f);
        }
        spaced (g, juce::String (arp::scaleName (sc > 0 ? sc - 1 : pt.scale)).toUpperCase() + "  /  " + juce::String (pt.n) + " STEPS",
                area.getX() + 6.f, area.getY() + 11.f, 8.f, true, 2.f, juce::Colours::white.withAlpha (0.55f));
    }

    // medidores del compresor
    {
        auto m = meterRect.toFloat();
        const float gr = juce::jlimit (0.f, 20.f, -proc.grMeter.load());
        g.setColour (juce::Colours::white.withAlpha (0.08f)); g.fillRect (m.removeFromLeft (12.f));
        g.setColour (kAccent); g.fillRect (meterRect.getX() + 0.f, (float) meterRect.getY(), 12.f, meterRect.getHeight() * gr / 20.f);
        const float pk = proc.peakMeter.load();
        const float db = 20.f * std::log10 (juce::jmax (pk, 1.0e-4f));
        const float nrm = juce::jlimit (0.f, 1.f, (db + 40.f) / 40.f);
        g.setColour (juce::Colours::white.withAlpha (0.08f)); g.fillRect ((float) meterRect.getX() + 18.f, (float) meterRect.getY(), 12.f, (float) meterRect.getHeight());
        g.setColour (nrm > 0.97f ? juce::Colours::orangered : juce::Colours::white.withAlpha (0.85f));
        g.fillRect ((float) meterRect.getX() + 18.f, (float) meterRect.getBottom() - meterRect.getHeight() * nrm, 12.f, meterRect.getHeight() * nrm);
        spaced (g, "GR", (float) meterRect.getX() + 6.f, (float) meterRect.getBottom() + 12.f, 8.f, true, 1.f, juce::Colours::white.withAlpha (0.6f), true);
        spaced (g, "OUT", (float) meterRect.getX() + 24.f, (float) meterRect.getBottom() + 12.f, 8.f, true, 1.f, juce::Colours::white.withAlpha (0.6f), true);
    }
}

void AttiaEditor::resized()
{
    panels.clear();
    auto place = [&] (juce::Rectangle<int> a, std::initializer_list<int> ids)
    {
        const int cnt = (int) ids.size(); if (cnt == 0) return;
        const int w = a.getWidth() / cnt; int x = a.getX();
        for (int id : ids) { knobs[(size_t) id]->setBounds (x, a.getY(), w, a.getHeight()); x += w; }
    };

    prevBtn.setBounds (288, 22, 36, 42);
    nextBtn.setBounds (796, 22, 36, 42);
    scopeRect = { 1030, 18, 140, 50 };
    knobs[Master]->setBounds (1186, 4, 78, 72);

    // navegador de presets
    juce::Rectangle<int> br (16, 86, 262, 658);
    panels.push_back ({ br, "SOUND BROWSER", 15 });
    for (int k = 0; k < 8; ++k) catBtns[(size_t) k]->setBounds (br.getX() + 10 + k * 31, br.getY() + 32, 28, 28);
    secBox.setBounds (br.getX() + 10, br.getY() + 68, br.getWidth() - 20, 24);
    search.setBounds (br.getX() + 10, br.getY() + 98, br.getWidth() - 20, 24);
    list.setBounds (br.getX() + 10, br.getY() + 130, br.getWidth() - 20, br.getHeight() - 140);

    const int X0 = 294;

    // osciladores
    for (int o = 0; o < 2; ++o)
    {
        juce::Rectangle<int> r (X0 + o * 491, 86, 479, 252);
        panels.push_back ({ r, "RESYNTH OSC " + juce::String (o + 1), 8 });
        lens[o] = { r.getX() + 16, r.getY() + 34, 176, 176 };
        waveLabel[o] = { r.getX() + 8, r.getY() + 214, 192, 16 };
        const int b = O1Wave + o * kOscBlock;
        place ({ r.getX() + 206, r.getY() + 34, 262, 100 }, { b, b + 1, b + 2, b + 6 });
        place ({ r.getX() + 206, r.getY() + 140, 262, 100 }, { b + 3, b + 4, b + 5 });
    }

    // filtro / envolventes / unison / lfo
    const int y2 = 350;
    {   juce::Rectangle<int> r (X0, y2, 240, 150); panels.push_back ({ r, "FILTER", 9 });
        combos[FType]->setBounds (r.getX() + 12, r.getY() + 28, r.getWidth() - 24, 22);
        place ({ r.getX() + 6, r.getY() + 56, r.getWidth() - 12, 88 }, { FCut, FRes, FEnv, FKey }); }
    {   juce::Rectangle<int> r (X0 + 252, y2, 190, 150); panels.push_back ({ r, "FILTER ENV", 10 });
        place ({ r.getX() + 4, r.getY() + 44, r.getWidth() - 8, 100 }, { FA, FD, FS, FR }); }
    {   juce::Rectangle<int> r (X0 + 454, y2, 190, 150); panels.push_back ({ r, "AMP ENV", 10 });
        place ({ r.getX() + 4, r.getY() + 44, r.getWidth() - 8, 100 }, { AA, AD, AS, AR }); }
    {   juce::Rectangle<int> r (X0 + 656, y2, 100, 150); panels.push_back ({ r, "UNISON", 21 });
        place ({ r.getX() + 4, r.getY() + 44, r.getWidth() - 8, 100 }, { UniVoices, UniSpread }); }
    {   juce::Rectangle<int> r (X0 + 768, y2, 202, 150); panels.push_back ({ r, "LFO / VIBRATO", 11 });
        combos[LfoShape]->setBounds (r.getX() + 8, r.getY() + 28, 90, 22);
        combos[LfoDest]->setBounds (r.getX() + 104, r.getY() + 28, 90, 22);
        place ({ r.getX() + 2, r.getY() + 56, r.getWidth() - 4, 88 }, { LfoRate, LfoDepth, VibDepth, VibRate }); }

    // efectos
    const int y3 = 512;
    {   juce::Rectangle<int> r (X0, y3, 292, 124); panels.push_back ({ r, "COMPRESSOR / LIMITER", 12 });
        place ({ r.getX() + 4, r.getY() + 32, 236, 86 }, { CThresh, CRatio, CAtk, CRel });
        meterRect = { r.getX() + 250, r.getY() + 32, 30, 70 }; }
    {   juce::Rectangle<int> r (X0 + 304, y3, 74, 124); panels.push_back ({ r, "DRIVE", 17 });
        place ({ r.getX() + 4, r.getY() + 32, r.getWidth() - 8, 86 }, { Drive }); }
    {   juce::Rectangle<int> r (X0 + 390, y3, 170, 124); panels.push_back ({ r, "DELAY", 13 });
        place ({ r.getX() + 4, r.getY() + 32, r.getWidth() - 8, 86 }, { DlyTime, DlyFb, DlyMix }); }
    {   juce::Rectangle<int> r (X0 + 572, y3, 222, 124); panels.push_back ({ r, "MODULATION", 20 });
        combos[ModType]->setBounds (r.getX() + 12, r.getY() + 28, r.getWidth() - 24, 20);
        place ({ r.getX() + 4, r.getY() + 50, r.getWidth() - 8, 70 }, { ModRate, ModDepth, ModMix }); }
    {   juce::Rectangle<int> r (X0 + 806, y3, 164, 124); panels.push_back ({ r, "REVERB", 18 });
        place ({ r.getX() + 4, r.getY() + 32, r.getWidth() - 8, 86 }, { RevSize, RevDamp, RevMix }); }

    // arpegiador
    {   juce::Rectangle<int> r (X0, 648, 970, 96); panels.push_back ({ r, "ARPEGGIATOR  /  15 000 PATTERNS", 19 });
        arpBtn.setBounds (r.getX() + 12, r.getY() + 28, 56, 22);
        combos[ArpBank]->setBounds (r.getX() + 74, r.getY() + 28, 210, 22);
        combos[ArpScale]->setBounds (r.getX() + 290, r.getY() + 28, 112, 22);
        combos[ArpRate]->setBounds (r.getX() + 408, r.getY() + 28, 76, 22);
        prevPat.setBounds (r.getX() + 12, r.getY() + 58, 28, 28);
        patLabel = { r.getX() + 44, r.getY() + 56, 262, 32 };
        nextPat.setBounds (r.getX() + 310, r.getY() + 58, 28, 28);
        rndPat.setBounds (r.getX() + 344, r.getY() + 58, 50, 28);
        patBars = { r.getX() + 500, r.getY() + 24, 300, 64 };
        place ({ r.getX() + 812, r.getY() + 22, 150, 70 }, { ArpOct, ArpGate, ArpSwing }); }
}
