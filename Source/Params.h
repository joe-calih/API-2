#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Parameter IDs ---------------------------------------------------------------
namespace ids
{
    constexpr const char* thresh  = "thresh";   // knob position 0..30  ->  +10 .. -20 dB
    constexpr const char* attack  = "attack";   // 7 steps
    constexpr const char* ratio   = "ratio";    // 7 steps
    constexpr const char* release = "release";  // 7 steps (last = variable)
    constexpr const char* relvar  = "relvar";   // 50 ms .. 3 s
    constexpr const char* knee    = "knee";     // Hard / Med / Soft
    constexpr const char* thrust  = "thrust";   // Loud / Med / Norm
    constexpr const char* type    = "type";     // New (feed-forward) / Old (feedback)
    constexpr const char* shape   = "shape";    // BP / HP / LP
    constexpr const char* link    = "link";     // IND, 50..100 %
    constexpr const char* output  = "output";   // -24 .. +24 dB
    constexpr const char* mix     = "mix";      // 0 .. 100 %
    constexpr const char* trim    = "trim";     // -18 .. +18 dB
    constexpr const char* analog  = "analog";   // saturation on/off
    constexpr const char* inSw    = "inswitch"; // compressor IN / bypass
    constexpr const char* manual  = "manual";   // manual make-up on / auto
    constexpr const char* meter   = "meter";    // meter source GR / OUT / IN
}

struct Tables
{
    static constexpr float attackMs[7]  = { 0.03f, 0.1f, 0.3f, 1.f, 3.f, 10.f, 30.f };
    static constexpr float ratio[7]     = { 1.5f, 2.f, 3.f, 4.f, 6.f, 10.f, 1000.f };
    static constexpr float releaseS[6]  = { 0.05f, 0.1f, 0.2f, 0.5f, 1.f, 2.f };
    static constexpr float kneeDb[3]    = { 0.f, 6.f, 12.f };
    static constexpr float linkAmt[7]   = { 0.f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.f };
};

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;
    auto pid = [] (const char* s) { return ParameterID { s, 1 }; };

    layout.add (std::make_unique<AudioParameterFloat> (
        pid (ids::thresh), "Threshold", NormalisableRange<float> (0.f, 30.f, 0.01f), 14.f,
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return String (10.f - v, 1) + " dB"; })
            .withValueFromStringFunction ([] (const String& t) { return 10.f - t.getFloatValue(); })));

    layout.add (std::make_unique<AudioParameterChoice> (pid (ids::attack), "Attack",
        StringArray { "0.03 ms", "0.1 ms", "0.3 ms", "1 ms", "3 ms", "10 ms", "30 ms" }, 3));
    layout.add (std::make_unique<AudioParameterChoice> (pid (ids::ratio), "Ratio",
        StringArray { "1.5:1", "2:1", "3:1", "4:1", "6:1", "10:1", "Inf:1" }, 3));
    layout.add (std::make_unique<AudioParameterChoice> (pid (ids::release), "Release",
        StringArray { "0.05 s", "0.1 s", "0.2 s", "0.5 s", "1 s", "2 s", "Variable" }, 3));

    NormalisableRange<float> relRange (0.05f, 3.0f,
        [] (float s, float e, float p) { return s * std::pow (e / s, p); },
        [] (float s, float e, float v) { return std::log (v / s) / std::log (e / s); },
        [] (float, float, float v) { return v; });
    layout.add (std::make_unique<AudioParameterFloat> (
        pid (ids::relvar), "Release Variable", relRange, 0.4f,
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return String (v, 2) + " s"; })));

    layout.add (std::make_unique<AudioParameterChoice> (pid (ids::knee),   "Knee",   StringArray { "Hard", "Med", "Soft" }, 0));
    layout.add (std::make_unique<AudioParameterChoice> (pid (ids::thrust), "Thrust", StringArray { "Loud", "Med", "Norm" }, 2));
    layout.add (std::make_unique<AudioParameterChoice> (pid (ids::type),   "Type",   StringArray { "New", "Old" }, 0));
    layout.add (std::make_unique<AudioParameterChoice> (pid (ids::shape),  "Shape",  StringArray { "BP", "HP", "LP" }, 1));
    layout.add (std::make_unique<AudioParameterChoice> (pid (ids::link),   "L/R Link",
        StringArray { "IND", "50%", "60%", "70%", "80%", "90%", "100%" }, 6));

    layout.add (std::make_unique<AudioParameterFloat> (
        pid (ids::output), "Output", NormalisableRange<float> (-24.f, 24.f, 0.1f), 0.f,
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " dB"; })));
    layout.add (std::make_unique<AudioParameterFloat> (
        pid (ids::mix), "Mix", NormalisableRange<float> (0.f, 100.f, 1.f), 100.f,
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return String ((int) v) + " %"; })));
    layout.add (std::make_unique<AudioParameterFloat> (
        pid (ids::trim), "Trim", NormalisableRange<float> (-18.f, 18.f, 0.1f), 0.f,
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " dB"; })));

    layout.add (std::make_unique<AudioParameterBool> (pid (ids::analog), "Analog", true));
    layout.add (std::make_unique<AudioParameterBool> (pid (ids::inSw),   "IN",     true));
    layout.add (std::make_unique<AudioParameterBool> (pid (ids::manual), "Manual Make-up", true));
    layout.add (std::make_unique<AudioParameterChoice> (pid (ids::meter), "Meter Source",
        StringArray { "GR", "OUT", "IN" }, 0));
    return layout;
}
