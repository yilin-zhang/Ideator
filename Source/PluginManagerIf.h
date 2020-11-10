/*
  ==============================================================================

    PluginManagerIf.h
    Created: 27 Oct 2020 11:58:10am
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class PluginManagerIf
{
public:
    virtual ~PluginManagerIf()= default;
    virtual bool loadPlugin(const juce::String &path) = 0;
    virtual bool checkPluginLoaded() const = 0;
    virtual juce::AudioProcessorEditor *getPluginEditor() = 0;
    virtual void addMidiEvent(const juce::MidiMessage &midiMessage) = 0;
    virtual juce::PluginDescription getPluginDescription() const = 0;
    virtual const juce::Array<juce::AudioProcessorParameter *> &getPluginParameters() const = 0;
    virtual void setPluginParameter(int parameterIndex, float newValue) = 0;
    virtual bool renderAudio(juce::String &audioPath) = 0;
};

