#pragma once
#include "PluginProcessor.h"
#include "Icons.h"

namespace attia { inline const juce::Colour kAccent { 0xFF19E6C8 }; }

// ───────────── Look & Feel ATTIA: negro profundo + turquesa electrico ─────────────
struct AtLook : public juce::LookAndFeel_V4
{
    AtLook()
    {
        using namespace juce;
        const Colour w = Colours::white, k = Colour (0xFF04080A);
        setColour (ResizableWindow::backgroundColourId, k);
        setColour (ComboBox::backgroundColourId, k);
        setColour (ComboBox::textColourId, w);
        setColour (ComboBox::outlineColourId, attia::kAccent.withAlpha (0.4f));
        setColour (ComboBox::arrowColourId, attia::kAccent);
        setColour (PopupMenu::backgroundColourId, Colour (0xFF081012));
        setColour (PopupMenu::textColourId, w);
        setColour (PopupMenu::highlightedBackgroundColourId, attia::kAccent);
        setColour (PopupMenu::highlightedTextColourId, Colours::black);
        setColour (ListBox::backgroundColourId, Colour (0xFF050A0C));
        setColour (ListBox::outlineColourId, attia::kAccent.withAlpha (0.18f));
        setColour (TextEditor::backgroundColourId, k);
        setColour (TextEditor::textColourId, w);
        setColour (TextEditor::outlineColourId, attia::kAccent.withAlpha (0.35f));
        setColour (TextEditor::focusedOutlineColourId, attia::kAccent);
        setColour (TextEditor::highlightColourId, attia::kAccent.withAlpha (0.3f));
        setColour (CaretComponent::caretColourId, attia::kAccent);
        setColour (TextButton::buttonColourId, k);
        setColour (TextButton::textColourOffId, w);
        setColour (TextButton::textColourOnId, w);
        setColour (ScrollBar::thumbColourId, attia::kAccent.withAlpha (0.4f));
        setColour (BubbleComponent::backgroundColourId, k);
        setColour (BubbleComponent::outlineColourId, attia::kAccent);
        setColour (TooltipWindow::textColourId, w);
        setColour (Label::textColourId, w);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override
    {
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (5.f);
        const float rad = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
        const auto c = b.getCentre();
        const float ang = a0 + pos * (a1 - a0);

        juce::Path track; track.addCentredArc (c.x, c.y, rad, rad, 0.f, a0, a1, true);
        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.strokePath (track, juce::PathStrokeType (2.f));
        juce::Path val; val.addCentredArc (c.x, c.y, rad, rad, 0.f, a0, ang, true);
        g.setColour (attia::kAccent.withAlpha (0.22f));
        g.strokePath (val, juce::PathStrokeType (6.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (attia::kAccent);
        g.strokePath (val, juce::PathStrokeType (2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float ir = rad * 0.7f;
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xFF2A3335), c.x - ir * 0.5f, c.y - ir * 0.7f, juce::Colour (0xFF0A0F10), c.x + ir * 0.6f, c.y + ir, true));
        g.fillEllipse (c.x - ir, c.y - ir, ir * 2.f, ir * 2.f);
        g.setColour (juce::Colours::white.withAlpha (0.22f));
        g.drawEllipse (c.x - ir, c.y - ir, ir * 2.f, ir * 2.f, 1.f);
        juce::Path p; p.addRoundedRectangle (-1.2f, -ir * 0.92f, 2.4f, ir * 0.5f, 1.f);
        g.setColour (juce::Colours::white);
        g.fillPath (p, juce::AffineTransform::rotation (ang).translated (c.x, c.y));
    }

    void drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box) override
    {
        auto r = juce::Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (0.5f);
        g.setColour (juce::Colour (0xFF06100F)); g.fillRoundedRectangle (r, 3.f);
        g.setColour (attia::kAccent.withAlpha (box.isMouseOver() ? 0.9f : 0.45f)); g.drawRoundedRectangle (r, 3.f, 1.f);
        juce::Path a; const float cx = (float) w - 12.f, cy = (float) h * 0.5f;
        a.addTriangle (cx - 4.f, cy - 2.f, cx + 4.f, cy - 2.f, cx, cy + 3.f);
        g.setColour (attia::kAccent); g.fillPath (a);
    }
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (juce::FontOptions (12.f)); }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (down ? attia::kAccent.withAlpha (0.3f) : juce::Colour (0xFF06100F)); g.fillRoundedRectangle (r, 3.f);
        g.setColour (attia::kAccent.withAlpha (over ? 1.f : 0.5f)); g.drawRoundedRectangle (r, 3.f, 1.f);
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        const bool on = b.getToggleState();
        g.setColour (on ? attia::kAccent : juce::Colour (0xFF06100F)); g.fillRoundedRectangle (r, 3.f);
        g.setColour (attia::kAccent.withAlpha (over || on ? 1.f : 0.5f)); g.drawRoundedRectangle (r, 3.f, 1.f);
        g.setColour (on ? juce::Colours::black : juce::Colours::white);
        g.setFont (juce::Font (juce::FontOptions (11.f, juce::Font::bold)));
        g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
    }
};

struct Knob : public juce::Component
{
    juce::Slider s { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox };
    juce::Label l;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;

    Knob (juce::AudioProcessorValueTreeState& a, const char* id, const juce::String& name, int sensitivity = 0)
    {
        addAndMakeVisible (s); addAndMakeVisible (l);
        l.setText (name.toUpperCase(), juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setFont (juce::Font (juce::FontOptions (10.f)));
        l.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.72f));
        l.setInterceptsMouseClicks (false, false);
        s.setPopupDisplayEnabled (true, true, nullptr);
        if (sensitivity > 0) s.setMouseDragSensitivity (sensitivity);
        att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (a, id, s);
    }
    void resized() override { auto b = getLocalBounds(); l.setBounds (b.removeFromBottom (16)); s.setBounds (b); }
};

struct Combo : public juce::ComboBox
{
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> att;
    Combo (juce::AudioProcessorValueTreeState& a, const char* id, const juce::StringArray& items)
    {
        addItemList (items, 1);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (a, id, *this);
    }
};

struct IconBtn : public juce::Button
{
    int kind;
    explicit IconBtn (int k) : juce::Button ("icon"), kind (k) {}
    void paintButton (juce::Graphics& g, bool over, bool) override
    {
        auto r = getLocalBounds().toFloat().reduced (0.5f);
        const bool on = getToggleState();
        g.setColour (on ? attia::kAccent : juce::Colour (0xFF06100F)); g.fillRoundedRectangle (r, 3.f);
        g.setColour (attia::kAccent.withAlpha (on || over ? 1.f : 0.4f)); g.drawRoundedRectangle (r, 3.f, 1.f);
        attia::drawIcon (g, kind, r.reduced (6.f), on ? juce::Colours::black : juce::Colours::white, 1.4f);
    }
};

class AttiaEditor : public juce::AudioProcessorEditor, private juce::Timer, private juce::ListBoxModel
{
public:
    explicit AttiaEditor (AttiaProcessor&);
    ~AttiaEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }
    int getNumRows() override { return (int) filtered.size(); }
    void paintListBoxItem (int row, juce::Graphics&, int w, int h, bool selected) override;
    void selectedRowsChanged (int row) override;
    void rebuild();
    void fillSectionBox();
    void syncList();
    void step (int d);
    void stepPattern (int d);
    void paintLens (juce::Graphics&, juce::Rectangle<float>, int osc);

    AtLook look;
    AttiaProcessor& proc;

    std::array<std::unique_ptr<Knob>, attia::NumParams> knobs;
    std::array<std::unique_ptr<Combo>, attia::NumParams> combos;
    juce::ToggleButton arpBtn { "ARP" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpAtt;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, prevPat { "<" }, nextPat { ">" }, rndPat { "RND" };

    std::array<std::unique_ptr<IconBtn>, 8> catBtns;
    int selCat = -1;
    std::vector<int> secOfItem;
    juce::ComboBox secBox;
    juce::TextEditor search;
    juce::ListBox list { "presets", nullptr };
    std::vector<int> filtered;
    bool ignoreSel = false;
    juce::Random rnd;

    struct PanelInfo { juce::Rectangle<int> r; juce::String title; int icon; };
    std::vector<PanelInfo> panels;
    juce::Rectangle<int> lens[2], waveLabel[2], patLabel, patBars, meterRect, scopeRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttiaEditor)
};
