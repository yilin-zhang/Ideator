/*
  ==============================================================================

    AudioProcessorIf.h
    Created: 27 Oct 2020 9:29:23am
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "AppConfig.h"
#include "AppProcessor.h"
#include "PluginProcessor.h"

class AudioProcessorIf
{
public:
#ifdef IDEATOR_APP
    AudioProcessorIf();
#else
    AudioProcessorIf(PluginProcessor&);
#endif

    ~AudioProcessorIf();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    void releaseResources();

    // methods for the plugin
    bool loadPlugin(const juce::String& path);
    bool checkPluginLoaded() const;
    juce::AudioProcessorEditor* getPluginEditor();
    void addMidiEvent(const juce::MidiMessage &midiMessage);
    juce::PluginDescription getPluginDescription() const;
    const juce::Array<juce::AudioProcessorParameter*>& getPluginParameters() const;
    void setPluginParameter(int parameterIndex, float newValue);

private:
#ifdef IDEATOR_APP
    AppProcessor audioProcessor;
#else
    PluginProcessor& audioProcessor;
#endif
};