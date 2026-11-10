#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

SbcAudioProcessor::SbcAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  AudioChannelSet::stereo(), true)
                          .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "STATE", createLayout())
{
    pThresh  = apvts.getRawParameterValue (ids::thresh);
    pAttack  = apvts.getRawParameterValue (ids::attack);
    pRatio   = apvts.getRawParameterValue (ids::ratio);
    pRelease = apvts.getRawParameterValue (ids::release);
    pRelVar  = apvts.getRawParameterValue (ids::relvar);
    pKnee    = apvts.getRawParameterValue (ids::knee);
    pThrust  = apvts.getRawParameterValue (ids::thrust);
    pType    = apvts.getRawParameterValue (ids::type);
    pShape   = apvts.getRawParameterValue (ids::shape);
    pLink    = apvts.getRawParameterValue (ids::link);
    pOutput  = apvts.getRawParameterValue (ids::output);
    pMix     = apvts.getRawParameterValue (ids::mix);
    pTrim    = apvts.getRawParameterValue (ids::trim);
    pAnalog  = apvts.getRawParameterValue (ids::analog);
    pIn      = apvts.getRawParameterValue (ids::inSw);
    pManual  = apvts.getRawParameterValue (ids::manual);

    for (int i = 0; i < 2; ++i) { meterIn[i] = -100.f; meterOut[i] = -100.f; meterGR[i] = 0.f; }
    apvts.state.setProperty ("presetName", "Default Preset", nullptr);
}

bool SbcAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    if (l.getMainOutputChannelSet() != l.getMainInputChannelSet()) return false;
    return l.getMainOutputChannelSet() == AudioChannelSet::stereo()
        || l.getMainOutputChannelSet() == AudioChannelSet::mono();
}

void SbcAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    for (int c = 0; c < 2; ++c) { grState[c] = 0.f; prevOut[c] = 0.f; scHP[c].reset(); scLP[c].reset(); }
    lastThrust = lastShape = -1;
    mkSm.reset (sr, 0.03);  mixSm.reset (sr, 0.03);  trimSm.reset (sr, 0.03);  inSm.reset (sr, 0.03);
    mkSm.setCurrentAndTargetValue (1.f);
    mixSm.setCurrentAndTargetValue (*pMix / 100.f);
    trimSm.setCurrentAndTargetValue (Decibels::decibelsToGain (pTrim->load()));
    inSm.setCurrentAndTargetValue (pIn->load() > 0.5f ? 1.f : 0.f);
}

static inline float computeGr (float over, float slope, float kneeW)
{
    if (kneeW <= 0.f)           return over > 0.f ? over * slope : 0.f;
    if (2.f * over < -kneeW)    return 0.f;
    if (2.f * std::abs (over) <= kneeW)
    {
        const float t = over + kneeW * 0.5f;
        return slope * t * t / (2.f * kneeW);
    }
    return over * slope;
}

void SbcAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;
    const int numCh = jmin (2, buffer.getNumChannels());
    const int n = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, n);
    if (numCh == 0 || n == 0) return;

    // ---- read parameters -------------------------------------------------
    const float threshDb = 10.f - pThresh->load();
    const int atkIdx  = jlimit (0, 6, (int) std::lround (pAttack->load()));
    const int ratIdx  = jlimit (0, 6, (int) std::lround (pRatio->load()));
    const int relIdx  = jlimit (0, 6, (int) std::lround (pRelease->load()));
    const int kneeIdx = jlimit (0, 2, (int) std::lround (pKnee->load()));
    const int thrIdx  = jlimit (0, 2, (int) std::lround (pThrust->load()));
    const int shpIdx  = jlimit (0, 2, (int) std::lround (pShape->load()));
    const int linkIdx = jlimit (0, 6, (int) std::lround (pLink->load()));
    const bool feedback = pType->load() > 0.5f;
    const bool analog   = pAnalog->load() > 0.5f;
    const bool manual   = pManual->load() > 0.5f;

    const float slope   = 1.f - 1.f / Tables::ratio[ratIdx];
    const float kneeW   = Tables::kneeDb[kneeIdx];
    const float link    = Tables::linkAmt[linkIdx];
    const float atkSec  = Tables::attackMs[atkIdx] * 0.001f;
    const float relSec  = relIdx < 6 ? Tables::releaseS[relIdx] : pRelVar->load();
    const float atkCoef = std::exp (-1.f / (atkSec * (float) sr));
    const float relCoef = std::exp (-1.f / (relSec * (float) sr));

    // detector side-chain filter (Thrust + Shape)
    if (thrIdx != lastThrust || shpIdx != lastShape)
    {
        lastThrust = thrIdx;  lastShape = shpIdx;
        const bool active = thrIdx != 2;
        useHP = active && shpIdx != 2;   // BP, HP
        useLP = active && shpIdx != 1;   // BP, LP
        const float fHp = thrIdx == 0 ? 180.f : 90.f;
        const float fLp = thrIdx == 0 ? 4000.f : 8000.f;
        auto hp = dsp::IIR::Coefficients<float>::makeHighPass (sr, fHp, 0.7071f);
        auto lp = dsp::IIR::Coefficients<float>::makeLowPass  (sr, fLp, 0.7071f);
        for (int c = 0; c < 2; ++c)
        {
            scHP[c].coefficients = hp;  scLP[c].coefficients = lp;
            scHP[c].reset();            scLP[c].reset();
        }
    }

    // make-up gain
    float makeupDb = manual ? pOutput->load() : jmax (0.f, -threshDb * slope * 0.5f);
    mkSm.setTargetValue (Decibels::decibelsToGain (makeupDb));
    mixSm.setTargetValue (pMix->load() / 100.f);
    trimSm.setTargetValue (Decibels::decibelsToGain (pTrim->load()));
    inSm.setTargetValue (pIn->load() > 0.5f ? 1.f : 0.f);

    double sumIn[2] { 0, 0 }, sumOut[2] { 0, 0 }, sumGr[2] { 0, 0 };
    float peakOut = 0.f;

    for (int i = 0; i < n; ++i)
    {
        float x[2] { 0.f, 0.f };
        for (int c = 0; c < numCh; ++c) x[c] = buffer.getSample (c, i);

        // detector
        float lvDb[2] { -100.f, -100.f };
        for (int c = 0; c < numCh; ++c)
        {
            float s = feedback ? prevOut[c] : x[c];
            if (useHP) s = scHP[c].processSample (s);
            if (useLP) s = scLP[c].processSample (s);
            lvDb[c] = Decibels::gainToDecibels (std::abs (s), -100.f);
        }
        const float mx = numCh > 1 ? jmax (lvDb[0], lvDb[1]) : lvDb[0];

        const float mk = mkSm.getNextValue();
        const float mixAmt = mixSm.getNextValue() * inSm.getNextValue();
        const float trimG = trimSm.getNextValue();

        for (int c = 0; c < numCh; ++c)
        {
            const float det = lvDb[c] * (1.f - link) + mx * link;
            const float target = computeGr (det - threshDb, slope, kneeW);
            const float a = target > grState[c] ? atkCoef : relCoef;
            grState[c] = target + a * (grState[c] - target);

            const float comp = x[c] * Decibels::decibelsToGain (-grState[c]);
            prevOut[c] = comp;
            float wet = analog ? std::tanh (0.5f * comp) * 2.f : comp;
            wet *= mk;

            const float y = (wet * mixAmt + x[c] * (1.f - mixAmt)) * trimG;
            buffer.setSample (c, i, y);

            sumIn[c]  += (double) x[c] * x[c];
            sumOut[c] += (double) y * y;
            sumGr[c]  += grState[c];
            peakOut = jmax (peakOut, std::abs (y));
        }
    }

    for (int c = 0; c < numCh; ++c)
    {
        meterIn[c]  = Decibels::gainToDecibels ((float) std::sqrt (sumIn[c]  / n), -100.f);
        meterOut[c] = Decibels::gainToDecibels ((float) std::sqrt (sumOut[c] / n), -100.f);
        meterGR[c]  = (float) (sumGr[c] / n);
    }
    if (numCh == 1) { meterIn[1] = meterIn[0].load(); meterOut[1] = meterOut[0].load(); meterGR[1] = meterGR[0].load(); }
    if (peakOut > outPeak.load()) outPeak = peakOut;
}

// ---- state --------------------------------------------------------------------
void SbcAudioProcessor::getStateInformation (MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void SbcAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (ValueTree::fromXml (*xml));
}

// ---- presets ------------------------------------------------------------------
void SbcAudioProcessor::setParam (const char* id, float v)
{
    if (auto* p = apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (v));
        p->endChangeGesture();
    }
}

void SbcAudioProcessor::applyFactoryPreset (int index)
{
    auto& list = factoryPresets();
    if (index < 0 || index >= (int) list.size()) return;
    auto& p = list[(size_t) index];
    undoManager.beginNewTransaction ("Preset");
    setParam (ids::thresh,  10.f - p.threshDb);
    setParam (ids::attack,  (float) p.attack);
    setParam (ids::ratio,   (float) p.ratio);
    setParam (ids::release, (float) p.release);
    setParam (ids::relvar,  p.relVar);
    setParam (ids::knee,    (float) p.knee);
    setParam (ids::thrust,  (float) p.thrust);
    setParam (ids::type,    (float) p.type);
    setParam (ids::shape,   (float) p.shape);
    setParam (ids::link,    (float) p.link);
    setParam (ids::output,  p.output);
    setParam (ids::mix,     p.mix);
    setParam (ids::trim,    p.trim);
    setParam (ids::analog,  p.analog ? 1.f : 0.f);
    setParam (ids::manual,  p.manual ? 1.f : 0.f);
    setParam (ids::inSw,    1.f);
    currentProgram = index;
    apvts.state.setProperty ("presetName", String (p.name), nullptr);
}

File SbcAudioProcessor::getUserPresetDir() const
{
    return File::getSpecialLocation (File::userApplicationDataDirectory)
               .getChildFile ("StudioBusComp").getChildFile ("Presets");
}

StringArray SbcAudioProcessor::getUserPresetNames() const
{
    StringArray names;
    auto files = getUserPresetDir().findChildFiles (File::findFiles, false, "*.xml");
    for (auto& f : files) names.add (f.getFileNameWithoutExtension());
    names.sort (true);
    return names;
}

void SbcAudioProcessor::loadUserPreset (const String& name)
{
    auto f = getUserPresetDir().getChildFile (name + ".xml");
    if (auto xml = parseXML (f))
    {
        undoManager.beginNewTransaction ("Preset");
        auto vt = ValueTree::fromXml (*xml);
        if (vt.hasType (apvts.state.getType()))
        {
            apvts.replaceState (vt);
            apvts.state.setProperty ("presetName", name, nullptr);
        }
    }
}

void SbcAudioProcessor::saveUserPreset (const String& rawName)
{
    auto name = File::createLegalFileName (rawName.trim());
    if (name.isEmpty()) return;
    auto dir = getUserPresetDir();
    dir.createDirectory();
    auto st = apvts.copyState();
    st.setProperty ("presetName", name, nullptr);
    if (auto xml = st.createXml()) xml->writeTo (dir.getChildFile (name + ".xml"));
    apvts.state.setProperty ("presetName", name, nullptr);
}

String SbcAudioProcessor::getCurrentPresetName() const
{
    return apvts.state.getProperty ("presetName", "Default Preset").toString();
}

void SbcAudioProcessor::stepPreset (int dir)
{
    StringArray all;
    for (auto& p : factoryPresets()) all.add (p.name);
    const int nFactory = all.size();
    all.addArray (getUserPresetNames());
    int idx = all.indexOf (getCurrentPresetName());
    idx = idx < 0 ? 0 : (idx + dir + all.size()) % all.size();
    if (idx < nFactory) applyFactoryPreset (idx);
    else                loadUserPreset (all[idx]);
}

void SbcAudioProcessor::switchSlot()
{
    slotState[activeSlot] = apvts.copyState();
    activeSlot = 1 - activeSlot;
    if (slotState[activeSlot].isValid())
    {
        undoManager.beginNewTransaction ("A/B");
        apvts.replaceState (slotState[activeSlot].createCopy());
    }
}

void SbcAudioProcessor::copyActiveToOther()
{
    slotState[1 - activeSlot] = apvts.copyState();
}

AudioProcessorEditor* SbcAudioProcessor::createEditor() { return new SbcEditor (*this); }

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SbcAudioProcessor(); }
