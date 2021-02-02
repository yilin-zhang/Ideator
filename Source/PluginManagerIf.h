/*
  ==============================================================================

    PluginManagerIf.h
    Created: 27 Oct 2020 11:58:10am
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <unordered_set>

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
    virtual void loadPreset(const juce::String &presetPath) = 0;
    virtual void savePreset(const juce::String &presetPath) = 0;
    virtual void setTimbreDesctiptors(const std::unordered_set<juce::String> &timbreDescriptors) = 0;
    virtual std::unordered_set<juce::String> getTimbreDesctiptors() = 0;
    virtual void setPresetPath(const juce::String &path) = 0;
    virtual juce::String getPresetPath() = 0;
};

