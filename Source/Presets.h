#pragma once
#include <vector>

struct FactoryPreset
{
    const char* name;
    float threshDb;           // +10 .. -20
    int attack, ratio, release;   // step indices 0..6
    float relVar;             // seconds (used when release == 6)
    int knee, thrust, type, shape, link;
    float output, mix, trim;
    bool analog, manual;
};

// attack : 0=.03 1=.1 2=.3 3=1 4=3 5=10 6=30 ms
// ratio  : 0=1.5 1=2 2=3 3=4 4=6 5=10 6=inf
// release: 0=.05 1=.1 2=.2 3=.5 4=1 5=2 s, 6=variable
// knee   : 0=hard 1=med 2=soft   thrust: 0=loud 1=med 2=norm
// type   : 0=new(fwd) 1=old(fback)   shape: 0=BP 1=HP 2=LP   link: 0=IND, 1..6 = 50..100 %
inline const std::vector<FactoryPreset>& factoryPresets()
{
    static const std::vector<FactoryPreset> p = {
        { "Default Preset",            -4.f, 3, 3, 3, 0.40f, 0, 2, 0, 1, 6,  0.f, 100.f, 0.f, true,  true },
        { "Mix Bus Glue 2:1",          -8.f, 6, 1, 2, 0.40f, 2, 2, 0, 1, 6,  2.f, 100.f, 0.f, true,  true },
        { "Master Bus Gentle",         -6.f, 6, 1, 6, 0.80f, 2, 2, 0, 1, 6,  1.5f,100.f, 0.f, true,  true },
        { "Drum Bus Punch",           -10.f, 5, 3, 2, 0.20f, 1, 1, 0, 1, 6,  3.f, 100.f, 0.f, true,  true },
        { "Drum Smash (Parallel)",    -18.f, 2, 5, 0, 0.10f, 0, 0, 1, 1, 6,  6.f,  35.f, 0.f, true,  true },
        { "Vocal Leveler",            -10.f, 4, 2, 6, 0.35f, 2, 1, 0, 1, 6,  3.f, 100.f, 0.f, true,  true },
        { "Bass Tightener",            -9.f, 5, 3, 2, 0.20f, 1, 0, 1, 1, 6,  2.f, 100.f, 0.f, true,  true },
        { "Guitar Sustain",           -14.f, 3, 2, 6, 0.60f, 2, 2, 0, 1, 6,  5.f, 100.f, 0.f, true,  true },
        { "Parallel Crush",           -20.f, 1, 6, 0, 0.10f, 0, 0, 1, 1, 6,  6.f,  30.f, 0.f, true,  true },
        { "Peak Catcher",               0.f, 0, 6, 0, 0.10f, 0, 2, 0, 1, 6,  0.f, 100.f, 0.f, false, true },
        { "Wide Stereo Safe (IND)",    -8.f, 5, 2, 3, 0.40f, 2, 2, 0, 1, 0,  2.f, 100.f, 0.f, true,  true },
        { "Vintage Feedback Squeeze", -12.f, 4, 3, 6, 0.50f, 2, 1, 1, 0, 6,  4.f, 100.f, 0.f, true,  true },
        { "Auto Make-up Glue",         -8.f, 6, 1, 3, 0.40f, 2, 2, 0, 1, 6,  0.f, 100.f, 0.f, true,  false },
    };
    return p;
}
