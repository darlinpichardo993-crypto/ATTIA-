#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace attia;

AttiaProcessor::AttiaProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "ATTIA", makeLayout())
{
    for (int i = 0; i < NumParams; ++i)
        raw[i] = apvts.getRawParameterValue (defs()[i].id);
    bank();            // carga (mapea) el banco de ondas una sola vez
    loadPreset (0);
}

void AttiaProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    for (auto& v : voices) { v.prepare (sr); v.active = false; }
    dlyL.assign ((size_t) (sr * 1.2) + 4, 0.f);
    dlyR.assign (dlyL.size(), 0.f);
    dlyW = 0;
    juce::dsp::ProcessSpec spec { sr, (juce::uint32) juce::jmax (1, samplesPerBlock), 2 };
    chorus.prepare (spec); flanger.prepare (spec); phaser.prepare (spec);
    flanger.setCentreDelay (2.f); flanger.setFeedback (0.6f);
    chorus.setCentreDelay (8.f);  chorus.setFeedback (0.f);
    phaser.setCentreFrequency (900.f); phaser.setFeedback (0.45f);
    reverb.setSampleRate (sr);
    nHeld = 0; arpPlaying = -1; arpPos = 1.0e9; grDb = 0.f; limGain = 1.f;
}

void AttiaProcessor::voiceOn (int note, float vel)
{
    Voice* v = nullptr;
    for (auto& x : voices) if (! x.active) { v = &x; break; }
    if (v == nullptr) { v = &voices[0]; for (auto& x : voices) if (x.age < v->age) v = &x; }
    v->start (note, vel, ++ageCounter, ampP, fltP);
}

void AttiaProcessor::voiceOff (int note)
{
    for (auto& v : voices) if (v.active && ! v.releasing && v.note == note) v.release();
}

void AttiaProcessor::handle (const juce::MidiMessage& m, bool arpOn)
{
    if (m.isNoteOn())
    {
        if (arpOn)
        {
            if (nHeld == 0) { arpPos = 1.0e9; stepIdx = 0; cycle = 0; }
            if (nHeld < (int) held.size()) held[(size_t) nHeld++] = m.getNoteNumber();
        }
        else voiceOn (m.getNoteNumber(), m.getFloatVelocity());
    }
    else if (m.isNoteOff())
    {
        if (arpOn)
        {
            for (int i = 0; i < nHeld; ++i)
                if (held[(size_t) i] == m.getNoteNumber()) { held[(size_t) i] = held[(size_t) (nHeld - 1)]; --nHeld; break; }
        }
        else voiceOff (m.getNoteNumber());
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        nHeld = 0;
        for (auto& v : voices) if (v.active) v.release();
    }
    else if (m.isPitchWheel())  bend = (float) (m.getPitchWheelValue() - 8192) / 8192.f * 2.f;
    else if (m.isController() && m.getControllerNumber() == 1) modWheel = (float) m.getControllerValue() / 127.f;
}

void AttiaProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (buffer.getNumChannels() < 2) return;
    float* outL = buffer.getWritePointer (0);
    float* outR = buffer.getWritePointer (1);
    auto g = [this] (int p) { return raw[p]->load(); };
    auto& wb = bank();

    // ── instantánea de parámetros ──
    snap.wb = &wb; snap.numWaves = wb.numWaves;
    for (int o = 0; o < 2; ++o)
    {
        const int b = O1Wave + o * kOscBlock;
        snap.basePos[o] = wb.mapPos (g (b)) + g (b + 1);
        snap.warp[o] = g (b + 2); snap.oct[o] = g (b + 3); snap.semi[o] = g (b + 4); snap.fine[o] = g (b + 5); snap.lvl[o] = g (b + 6);
    }
    const int nU = juce::jlimit (1, 5, (int) g (UniVoices));
    const float spread = g (UniSpread);
    snap.nU = nU; snap.uniGain = 1.f / std::sqrt ((float) nU);
    for (int u = 0; u < nU; ++u)
    {
        const float t = nU > 1 ? (float) u / (float) (nU - 1) : 0.5f;
        snap.ratio[u] = std::pow (2.f, (nU > 1 ? (t * 2.f - 1.f) * spread : 0.f) / 1200.f);
        snap.gl[u] = std::cos (t * juce::MathConstants<float>::halfPi);
        snap.gr[u] = std::sin (t * juce::MathConstants<float>::halfPi);
    }
    snap.cut = g (FCut); snap.q = 0.707f + g (FRes) * 9.f; snap.fenv = g (FEnv); snap.fkey = g (FKey);
    snap.lfoDest = (int) g (LfoDest);
    snap.lfoDepth = snap.lfoDest == 0 ? 0.f : juce::jmax (g (LfoDepth), modWheel * 0.8f);
    snap.vibSemis = g (VibDepth) * 0.6f;
    snap.bend = bend; snap.invSr = 1.f / (float) sr; snap.maxCut = (float) sr * 0.45f;

    ampP = { g (AA), g (AD), g (AS), g (AR) };
    fltP = { g (FA), g (FD), g (FS), g (FR) };
    const int ft = (int) g (FType);
    const auto ftype = ft == 0 ? juce::dsp::StateVariableTPTFilterType::lowpass
                     : ft == 1 ? juce::dsp::StateVariableTPTFilterType::bandpass
                               : juce::dsp::StateVariableTPTFilterType::highpass;
    for (auto& v : voices) { v.filter.setType (ftype); if (v.active) { v.amp.setParameters (ampP); v.flt.setParameters (fltP); } }

    // ── arpegiador ──
    const bool arpOn = g (ArpOn) > 0.5f;
    if (prevArpOn && ! arpOn)
    {
        if (arpPlaying >= 0) voiceOff (arpPlaying);
        arpPlaying = -1; nHeld = 0;
    }
    prevArpOn = arpOn;
    const int wantIdx = (int) g (ArpBank) * arp::perBank + (int) g (ArpPat);
    if (wantIdx != patIdx) { pat = arp::make (wantIdx); patIdx = wantIdx; stepIdx = stepIdx % juce::jmax (1, pat.n); }
    const int scaleSel = (int) g (ArpScale);
    const int arpScale = scaleSel > 0 ? scaleSel - 1 : pat.scale;
    double bpm = 120.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm()) bpm = *b;
    static const double beats[] = { 1.0, 0.5, 1.0 / 3.0, 0.25, 1.0 / 6.0, 0.125 };
    const double stepLen = 60.0 / juce::jmax (20.0, bpm) * beats[juce::jlimit (0, 5, (int) g (ArpRate))] * sr;
    const float gateScale = g (ArpGate), swing = g (ArpSwing);
    const int arpOcts = juce::jlimit (1, 4, (int) g (ArpOct));

    const int lShape = (int) g (LfoShape);
    const float lRate = g (LfoRate), vibRate = g (VibRate);

    auto it = midi.begin(); const auto end = midi.end();

    for (int i = 0; i < n; ++i)
    {
        while (it != end && (*it).samplePosition <= i) { handle ((*it).getMessage(), arpOn); ++it; }

        if (arpOn)
        {
            if (nHeld == 0)
            {
                if (arpPlaying >= 0) { voiceOff (arpPlaying); arpPlaying = -1; }
                arpPos = 1.0e9; stepIdx = 0; cycle = 0;
            }
            else
            {
                if (arpPos >= curLen)
                {
                    arpPos = arpPos > 1.0e8 ? 0.0 : arpPos - curLen;
                    if (arpPlaying >= 0) { voiceOff (arpPlaying); arpPlaying = -1; }
                    const auto& st = pat.s[stepIdx];
                    curLen = stepLen * ((stepIdx & 1) ? 1.0 - swing * 0.5 : 1.0 + swing * 0.5);
                    gateLen = juce::jmin (curLen * 0.97, curLen * (double) (st.gate * gateScale));
                    if (! st.rest)
                    {
                        std::array<int, 32> srt = held;
                        std::sort (srt.begin(), srt.begin() + nHeld);
                        int note;
                        if (st.chord)
                        {
                            const int d = std::abs ((int) st.deg);
                            note = srt[(size_t) (d % nHeld)] + 12 * (d / nHeld + st.oct);
                        }
                        else note = srt[0] + arp::degToSemi (arpScale, st.deg) + 12 * st.oct;
                        note += 12 * (cycle % arpOcts);
                        note = juce::jlimit (0, 127, note);
                        voiceOn (note, st.vel * 0.9f);
                        arpPlaying = note;
                    }
                    if (++stepIdx >= pat.n) { stepIdx = 0; ++cycle; }
                }
                else if (arpPlaying >= 0 && arpPos >= gateLen) { voiceOff (arpPlaying); arpPlaying = -1; }
                arpPos += 1.0;
            }
        }

        float lfo = 0.f;
        switch (lShape)
        {
            case 0: lfo = std::sin (juce::MathConstants<float>::twoPi * lfoPhase); break;
            case 1: lfo = 4.f * std::fabs (lfoPhase - 0.5f) - 1.f; break;
            case 2: lfo = 2.f * lfoPhase - 1.f; break;
            case 3: lfo = lfoPhase < 0.5f ? 1.f : -1.f; break;
            default: lfo = shValue; break;
        }
        lfoPhase += lRate / (float) sr;
        if (lfoPhase >= 1.f) { lfoPhase -= 1.f; shValue = rng.nextFloat() * 2.f - 1.f; }
        const float vib = std::sin (juce::MathConstants<float>::twoPi * vibPhase);
        vibPhase += vibRate / (float) sr; if (vibPhase >= 1.f) vibPhase -= 1.f;

        float l = 0.f, r = 0.f;
        for (auto& v : voices)
            if (v.active) { float a, b; v.render (snap, lfo, vib, a, b); l += a; r += b; }
        outL[i] = l; outR[i] = r;
    }

    // ── cadena de efectos: drive -> modulación -> delay -> reverb -> compresor -> volumen -> limitador ──
    const float drive = g (Drive);
    if (drive > 0.001f)
    {
        const float k = 1.f + drive * 8.f, comp = 1.f / (1.f + drive * 1.5f);
        for (int i = 0; i < n; ++i) { outL[i] = std::tanh (outL[i] * k) * comp; outR[i] = std::tanh (outR[i] * k) * comp; }
    }

    const float mmix = g (ModMix);
    if (mmix > 0.005f)
    {
        juce::dsp::AudioBlock<float> blk (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (blk);
        const int mt = (int) g (ModType);
        const float mr = g (ModRate), md = g (ModDepth);
        if (mt == 0)      { chorus.setRate (mr); chorus.setDepth (md); chorus.setMix (mmix); chorus.process (ctx); }
        else if (mt == 1) { phaser.setRate (mr); phaser.setDepth (md); phaser.setMix (mmix); phaser.process (ctx); }
        else              { flanger.setRate (mr * 0.5f); flanger.setDepth (md); flanger.setMix (mmix); flanger.process (ctx); }
    }

    const int dsz = (int) dlyL.size();
    const int ds = juce::jlimit (1, dsz - 1, (int) (g (DlyTime) * 0.001 * sr));
    const float dm = g (DlyMix), fb = g (DlyFb);
    if (dm > 0.002f)
        for (int i = 0; i < n; ++i)
        {
            int rp = dlyW - ds; if (rp < 0) rp += dsz;
            const float dL = dlyL[(size_t) rp], dR = dlyR[(size_t) rp];
            const float l = outL[i], r = outR[i];
            dlyL[(size_t) dlyW] = l + dR * fb; dlyR[(size_t) dlyW] = r + dL * fb;   // ping-pong
            outL[i] = l + dL * dm; outR[i] = r + dR * dm;
            if (++dlyW >= dsz) dlyW = 0;
        }

    juce::Reverb::Parameters rp;
    rp.roomSize = g (RevSize); rp.damping = g (RevDamp); rp.wetLevel = g (RevMix) * 0.45f; rp.dryLevel = 1.f; rp.width = 1.f; rp.freezeMode = 0.f;
    reverb.setParameters (rp);
    reverb.processStereo (outL, outR, n);

    // compresor (detección de picos enlazada L/R, ataque/release, makeup automático) + limitador a -1 dBFS
    const float thr = g (CThresh), ratio = juce::jmax (1.f, g (CRatio));
    const float att = std::exp (-1.f / (0.001f * g (CAtk) * (float) sr)), rel = std::exp (-1.f / (0.001f * g (CRel) * (float) sr));
    const float makeup = -thr * (1.f - 1.f / ratio) * 0.45f;
    const float master = g (Master);
    const float ceiling = 0.89f, limRel = std::exp (-1.f / (0.12f * (float) sr));
    float minGr = 0.f, peak = 0.f;
    for (int i = 0; i < n; ++i)
    {
        const float pk = juce::jmax (std::fabs (outL[i]), std::fabs (outR[i]));
        const float lvDb = 20.f * std::log10 (juce::jmax (pk, 1.0e-6f));
        const float over = lvDb - thr;
        const float target = over > 0.f ? -over * (1.f - 1.f / ratio) : 0.f;
        grDb = target + (target < grDb ? att : rel) * (grDb - target);
        const float gain = std::exp ((grDb + makeup) * 0.1151293f) * master * 1.4f;
        float l = outL[i] * gain, r = outR[i] * gain;

        const float lp = juce::jmax (std::fabs (l), std::fabs (r));
        const float need = lp > ceiling ? ceiling / lp : 1.f;
        limGain = juce::jmin (need, 1.f - (1.f - limGain) * limRel);   // ataque instantáneo, release suave
        l *= limGain; r *= limGain;
        outL[i] = l; outR[i] = r;
        minGr = juce::jmin (minGr, grDb); peak = juce::jmax (peak, juce::jmax (std::fabs (l), std::fabs (r)));

        if ((++scopeDec & 1) == 0)
        {
            const int p = scopePos.load();
            scopeBuf[p] = (l + r) * 0.5f;
            scopePos.store ((p + 1) & 1023);
        }
    }
    grMeter.store (minGr);
    peakMeter.store (juce::jmax (peak, peakMeter.load() * 0.9f));
}

void AttiaProcessor::setParam (int p, float value)
{
    if (auto* par = apvts.getParameter (defs()[p].id))
        par->setValueNotifyingHost (par->convertTo0to1 (value));
}

void AttiaProcessor::loadPreset (int idx)
{
    idx = juce::jlimit (0, presets::total - 1, idx);
    currentPreset.store (idx);
    const auto v = presets::make (idx);
    for (int i = 0; i < NumParams; ++i) setParam (i, v[(size_t) i]);
}

void AttiaProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("preset", currentPreset.load(), nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void AttiaProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            currentPreset.store (juce::jlimit (0, presets::total - 1, (int) vt.getProperty ("preset", 0)));
            apvts.replaceState (vt);
        }
}

juce::AudioProcessorEditor* AttiaProcessor::createEditor() { return new AttiaEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AttiaProcessor(); }
