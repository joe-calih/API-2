#include "PluginEditor.h"

using namespace juce;

namespace layout { constexpr int W = 694, H = 399; }

void SbcEditor::setupKnob (Slider& s, std::unique_ptr<SliderAtt>& att, const char* id,
                           int cx, int cy, int r, int style)
{
    s.setSliderStyle (Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
    s.setRotaryParameters (ui::kA0, ui::kA1, true);
    s.setMouseDragSensitivity (200);
    s.setPopupDisplayEnabled (true, false, this);
    s.getProperties().set ("style", style);
    s.setBounds (cx - r, cy - r, 2 * r, 2 * r);
    content.addAndMakeVisible (s);
    att = std::make_unique<SliderAtt> (proc.apvts, id, s);
}

SbcEditor::SbcEditor (SbcAudioProcessor& p) : AudioProcessorEditor (p), proc (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (content);
    content.setSize (layout::W, layout::H);
    content.addAndMakeVisible (backdrop);
    backdrop.setBounds (0, 0, layout::W, layout::H);

    auto* um = &proc.undoManager;

    // ---- knobs (centre x, centre y, radius, style)
    setupKnob (sThresh,  aThresh,  ids::thresh,  73, 193, 20, 0);
    setupKnob (sAttack,  aAttack,  ids::attack,  172, 193, 20, 0);
    setupKnob (sRatio,   aRatio,   ids::ratio,   262, 193, 20, 0);
    setupKnob (sRelease, aRelease, ids::release, 355, 193, 20, 0);
    setupKnob (sRelVar,  aRelVar,  ids::relvar,  447, 191, 25, 1);
    setupKnob (sLink,    aLink,    ids::link,    466, 325, 21, 0);
    setupKnob (sOutput,  aOutput,  ids::output,  612, 177, 32, 2);
    setupKnob (sMix,     aMix,     ids::mix,     580, 314, 14, 3);
    setupKnob (sTrim,    aTrim,    ids::trim,    646, 314, 14, 3);

    auto place = [this] (Component& c, int cx, int cy, int w, int h)
    {
        c.setBounds (cx - w / 2, cy - h / 2, w, h);
        content.addAndMakeVisible (c);
    };

    // ---- LEDs
    place (grLed, 84, 143, 16, 16);
    place (ovLed, 629, 230, 16, 16);
    place (inLed, 597, 230, 16, 16);
    for (int i = 0; i < 3; ++i)
    {
        place (meterLeds[i], 474, i == 0 ? 57 : i == 1 ? 75 : 94, 16, 16);
        place (kneeLeds[i],   59, 302 + i * 22 + (i == 2 ? 1 : 0), 16, 16);
        place (thrustLeds[i], 162, 302 + i * 22 + (i == 2 ? 1 : 0), 16, 16);
    }
    place (typeLeds[0], 262, 302, 16, 16);
    place (typeLeds[1], 262, 347, 16, 16);
    place (shapeLeds[0], 364, 304, 16, 16);
    place (shapeLeds[1], 384, 304, 16, 16);
    place (shapeLeds[2], 384, 337, 16, 16);

    // ---- keys
    kKnee   = std::make_unique<ui::CycleKey> (proc.apvts, ids::knee, 3, Colour (0xfff0f0f0), false, um,
                [this] (int s) { for (int i = 0; i < 3; ++i) kneeLeds[i].setLit (i == s); });
    kThrust = std::make_unique<ui::CycleKey> (proc.apvts, ids::thrust, 3, Colour (0xfff2c418), false, um,
                [this] (int s) { for (int i = 0; i < 3; ++i) thrustLeds[i].setLit (i == s); });
    kType   = std::make_unique<ui::CycleKey> (proc.apvts, ids::type, 2, Colour (0xfff0f0f0), false, um,
                [this] (int s) { for (int i = 0; i < 2; ++i) typeLeds[i].setLit (i == s); });
    kShape  = std::make_unique<ui::CycleKey> (proc.apvts, ids::shape, 3, Colour (0xff2fb12f), false, um,
                [this] (int s) { for (int i = 0; i < 3; ++i) shapeLeds[i].setLit (i == s); });
    kMeter  = std::make_unique<ui::CycleKey> (proc.apvts, ids::meter, 3, Colour (0xfff0f0f0), false, um,
                [this] (int s) { for (int i = 0; i < 3; ++i) meterLeds[i].setLit (i == s); });
    kIn     = std::make_unique<ui::CycleKey> (proc.apvts, ids::inSw, 2, Colour (0xfff0f0f0), true, um,
                [this] (int s) { inLed.setLit (s != 0); });
    kManual = std::make_unique<ui::CycleKey> (proc.apvts, ids::manual, 2, Colour (0xffe0201e), true, um);
    swAnalog = std::make_unique<ui::ToggleSwitch> (proc.apvts, ids::analog, um);

    place (*kKnee, 30, 325, 20, 20);   place (*kThrust, 130, 325, 20, 20);   place (*kType, 236, 325, 20, 20);
    place (*kShape, 338, 325, 20, 20); place (*kMeter, 458, 92, 20, 20);
    place (*kIn, 597, 247, 22, 22);    place (*kManual, 629, 247, 22, 22);
    place (*swAnalog, 612, 80, 60, 70);

    // ---- meters
    meterL.setBounds (86, 31, 132, 76);   meterR.setBounds (241, 31, 132, 76);
    content.addAndMakeVisible (meterL);   content.addAndMakeVisible (meterR);

    // ---- toolbar
    bUndo.setBounds (30, 3, 28, 16);  bRedo.setBounds (64, 3, 28, 16);
    presetBox.setBounds (98, 3, 366, 16);
    bPrev.setBounds (472, 3, 26, 16); bNext.setBounds (503, 3, 26, 16);
    bCopy.setBounds (537, 3, 40, 16); bSlot.setBounds (582, 3, 46, 16);
    bSave.setBounds (633, 3, 30, 16); bMenu.setBounds (667, 3, 22, 16);
    for (Component* c : std::initializer_list<Component*> { &bUndo, &bRedo, &presetBox, &bPrev, &bNext, &bCopy, &bSlot, &bSave, &bMenu })
        content.addAndMakeVisible (*c);

    bUndo.onClick = [this] { proc.undoManager.undo(); };
    bRedo.onClick = [this] { proc.undoManager.redo(); };
    bPrev.onClick = [this] { proc.stepPreset (-1); };
    bNext.onClick = [this] { proc.stepPreset (+1); };
    bCopy.onClick = [this] { proc.copyActiveToOther(); };
    bSlot.onClick = [this] { proc.switchSlot(); };
    bSave.onClick = [this] { askSaveName(); };
    bMenu.onClick = [this] { showMainMenu(); };
    presetBox.onClick = [this] { showPresetMenu(); };

    setResizable (true, true);
    setResizeLimits (layout::W * 3 / 4, layout::H * 3 / 4, layout::W * 2, layout::H * 2);
    getConstrainer()->setFixedAspectRatio ((double) layout::W / layout::H);
    setSize (layout::W, layout::H);
    startTimerHz (30);
}

SbcEditor::~SbcEditor() { stopTimer(); setLookAndFeel (nullptr); }

void SbcEditor::resized()
{
    content.setTransform (AffineTransform::scale ((float) getWidth() / (float) layout::W));
}

void SbcEditor::timerCallback()
{
    const int src = (int) std::lround (proc.apvts.getRawParameterValue (ids::meter)->load());
    float vu[2];
    for (int c = 0; c < 2; ++c)
    {
        vu[c] = src == 0 ? -proc.meterGR[c].load()
              : src == 1 ? proc.meterOut[c].load() + 18.f
                         : proc.meterIn[c].load() + 18.f;
        vu[c] = jmax (vu[c], -30.f);
        needle[c] += (vu[c] - needle[c]) * 0.35f;
    }
    meterL.setNeedle (needle[0]);
    meterR.setNeedle (needle[1]);

    grLed.setLit (jmax (proc.meterGR[0].load(), proc.meterGR[1].load()) > 0.3f);
    const float pk = proc.outPeak.exchange (0.f);
    ovHold = pk > 0.99f ? 10 : jmax (0, ovHold - 1);
    ovLed.setLit (ovHold > 0);

    const int slot = proc.getActiveSlot();
    presetBox.setText (String (slot == 0 ? "A" : "B") + ": " + proc.getCurrentPresetName());
    bSlot.setText (slot == 0 ? "Setup A" : "Setup B");
    bCopy.setText (slot == 0 ? "A->B" : "B->A");
}

void SbcEditor::showPresetMenu()
{
    PopupMenu m;
    auto& list = factoryPresets();
    auto cur = proc.getCurrentPresetName();
    m.addSectionHeader ("Factory");
    for (size_t i = 0; i < list.size(); ++i)
        m.addItem ((int) i + 1, list[i].name, true, cur == list[i].name);
    auto users = proc.getUserPresetNames();
    if (users.size() > 0)
    {
        m.addSeparator();
        m.addSectionHeader ("User");
        for (int i = 0; i < users.size(); ++i)
            m.addItem (1000 + i, users[i], true, cur == users[i]);
    }
    m.showMenuAsync (PopupMenu::Options().withTargetComponent (&presetBox),
        [this, users] (int r)
        {
            if (r <= 0) return;
            if (r >= 1000) proc.loadUserPreset (users[r - 1000]);
            else           proc.applyFactoryPreset (r - 1);
        });
}

void SbcEditor::showMainMenu()
{
    PopupMenu m;
    m.addItem (1, "Reset to default preset");
    m.addItem (2, "Open user preset folder");
    m.addItem (3, "About");
    m.showMenuAsync (PopupMenu::Options().withTargetComponent (&bMenu),
        [this] (int r)
        {
            if (r == 1) proc.applyFactoryPreset (0);
            if (r == 2) { proc.getUserPresetDir().createDirectory(); proc.getUserPresetDir().revealToUser(); }
            if (r == 3)
                AlertWindow::showMessageBoxAsync (MessageBoxIconType::NoIcon, "Studio Bus Comp",
                    "Studio Bus Comp 1.0\nStereo VCA-style bus compressor\nBuilt with JUCE.");
        });
}

void SbcEditor::askSaveName()
{
    saveWindow = std::make_unique<AlertWindow> ("Save preset", "Preset name:", MessageBoxIconType::NoIcon);
    saveWindow->addTextEditor ("name", proc.getCurrentPresetName());
    saveWindow->addButton ("Save", 1, KeyPress (KeyPress::returnKey));
    saveWindow->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
    saveWindow->enterModalState (true, ModalCallbackFunction::create (
        [safe = Component::SafePointer<SbcEditor> (this)] (int r)
    {
        if (safe != nullptr)
        {
            if (r == 1 && safe->saveWindow != nullptr)
                safe->proc.saveUserPreset (safe->saveWindow->getTextEditorContents ("name"));
            MessageManager::callAsync ([safe] { if (safe != nullptr) safe->saveWindow.reset(); });
        }
    }), false);
}
