/*
  ==============================================================================

    ProcessorManager.h
    Created: 27 Oct 2020 9:29:23am
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "AppConfig.h"
#include "PluginManagerIf.h"
#include "AppProcessor.h"
#include "PluginProcessor.h"

class ProcessorManager : public PluginManagerIf
{
public:
#ifdef IDEATOR_APP
    ProcessorManager();
#else
    ProcessorManager(PluginProcessor&);
#endif

    virtual ~ProcessorManager();
#ifdef IDEATOR_APP
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
#else
    void prepareToPlay ();
#endif
    void releaseResources();

    // methods for the plugin
    bool loadPlugin(const juce::String& path) override;
    bool checkPluginLoaded() const override;
    juce::AudioProcessorEditor* getPluginEditor() override;
    void addMidiEvent(const juce::MidiMessage &midiMessage) override;
    juce::PluginDescription getPluginDescription() const override;
    const juce::Array<juce::AudioProcessorParameter*>& getPluginParameters() const override;
    void setPluginParameter(int parameterIndex, float newValue) override;
    bool renderAudio(juce::String &audioPath) override;
    void loadPreset(const juce::String &presetPath) override;
    void savePreset(const juce::String &presetPath) override;
    void setTimbreDesctiptors(const std::unordered_set<juce::String> &timbreDescriptors) override;

private:
#ifdef IDEATOR_APP
    AppProcessor audioProcessor;
#else
    PluginProcessor& audioProcessor;
#endif
};