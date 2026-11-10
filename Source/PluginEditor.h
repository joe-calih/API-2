#pragma once
#include "PluginProcessor.h"
#include "Look.h"

class SbcEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit SbcEditor (SbcAudioProcessor&);
    ~SbcEditor() override;
    void resized() override;

private:
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;

    void timerCallback() override;
    void setupKnob (juce::Slider&, std::unique_ptr<SliderAtt>&, const char* id, int cx, int cy, int r, int style);
    void showPresetMenu();
    void showMainMenu();
    void askSaveName();

    SbcAudioProcessor& proc;
    ui::ApiLookAndFeel lnf;

    juce::Component content;
    ui::Backdrop backdrop;

    juce::Slider sThresh, sAttack, sRatio, sRelease, sRelVar, sLink, sOutput, sMix, sTrim;
    std::unique_ptr<SliderAtt> aThresh, aAttack, aRatio, aRelease, aRelVar, aLink, aOutput, aMix, aTrim;

    ui::VuMeter meterL, meterR;
    ui::Led grLed { juce::Colour (0xffff2a2a) }, ovLed { juce::Colour (0xffff2a2a) }, inLed { juce::Colour (0xff3cff3c) };
    ui::Led meterLeds[3] { ui::Led (juce::Colour (0xffff2a2a)), ui::Led (juce::Colour (0xffff2a2a)), ui::Led (juce::Colour (0xffff2a2a)) };
    ui::Led kneeLeds[3]  { ui::Led (juce::Colour (0xffff2a2a)), ui::Led (juce::Colour (0xffff2a2a)), ui::Led (juce::Colour (0xffff2a2a)) };
    ui::Led thrustLeds[3] { ui::Led (juce::Colour (0xff3cff3c)), ui::Led (juce::Colour (0xff3cff3c)), ui::Led (juce::Colour (0xff3cff3c)) };
    ui::Led typeLeds[2]  { ui::Led (juce::Colour (0xffff2a2a)), ui::Led (juce::Colour (0xffff2a2a)) };
    ui::Led shapeLeds[3] { ui::Led (juce::Colour (0xffff2a2a)), ui::Led (juce::Colour (0xffff2a2a)), ui::Led (juce::Colour (0xffff2a2a)) };

    std::unique_ptr<ui::CycleKey> kKnee, kThrust, kType, kShape, kMeter, kIn, kManual;
    std::unique_ptr<ui::ToggleSwitch> swAnalog;

    ui::ToolButton bUndo { ui::ToolButton::Undo }, bRedo { ui::ToolButton::Redo },
                   bPrev { ui::ToolButton::Left }, bNext { ui::ToolButton::Right },
                   bCopy { ui::ToolButton::Text, "A->B" }, bSlot { ui::ToolButton::Text, "Setup A" },
                   bSave { ui::ToolButton::Text, "Save" }, bMenu { ui::ToolButton::Menu };
    ui::PresetBox presetBox;

    std::unique_ptr<juce::AlertWindow> saveWindow;
    float needle[2] { -30.f, -30.f };
    int ovHold = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SbcEditor)
};
