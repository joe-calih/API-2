#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Params.h"
#include "Presets.h"

class SbcAudioProcessor : public juce::AudioProcessor
{
public:
    SbcAudioProcessor();
    ~SbcAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Studio Bus Comp"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return (int) factoryPresets().size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int i) override { applyFactoryPreset (i); }
    const juce::String getProgramName (int i) override
    {
        auto& l = factoryPresets();
        return (i >= 0 && i < (int) l.size()) ? juce::String (l[(size_t) i].name) : juce::String();
    }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ---- presets / A-B ----------------------------------------------------
    void applyFactoryPreset (int index);
    juce::File getUserPresetDir() const;
    juce::StringArray getUserPresetNames() const;
    void loadUserPreset (const juce::String& name);
    void saveUserPreset (const juce::String& name);
    juce::String getCurrentPresetName() const;
    void stepPreset (int direction);

    int getActiveSlot() const { return activeSlot; }
    void switchSlot();
    void copyActiveToOther();

    // ---- state shared with the editor ------------------------------------
    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float> meterIn[2], meterOut[2], meterGR[2];  // dB
    std::atomic<float> outPeak { 0.f };

private:
    // raw parameter pointers
    std::atomic<float> *pThresh, *pAttack, *pRatio, *pRelease, *pRelVar, *pKnee, *pThrust, *pType,
                       *pShape, *pLink, *pOutput, *pMix, *pTrim, *pAnalog, *pIn, *pManual;

    double sr = 44100.0;
    float grState[2] { 0.f, 0.f };
    float prevOut[2] { 0.f, 0.f };
    juce::dsp::IIR::Filter<float> scHP[2], scLP[2];
    bool useHP = false, useLP = false;
    int lastThrust = -1, lastShape = -1;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mkSm, mixSm, trimSm, inSm;

    int currentProgram = 0;
    juce::ValueTree slotState[2];
    int activeSlot = 0;

    void setParam (const char* id, float realValue);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SbcAudioProcessor)
};
