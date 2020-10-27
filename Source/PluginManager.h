/*
  ==============================================================================

    PluginManager.h
    Created: 27 Oct 2020 11:17:06am
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginManagerIf.h"

class PluginManager : public PluginManagerIf
{
public:
    PluginManager();
    virtual ~PluginManager();
    // methods for the plugin
    bool loadPlugin(const juce::String& path) override;
    bool checkPluginLoaded() const override;
    juce::AudioProcessorEditor* getPluginEditor() override;
    void addMidiEvent(const juce::MidiMessage &midiMessage) override;
    juce::PluginDescription getPluginDescription() const override;
    const juce::Array<juce::AudioProcessorParameter*>& getPluginParameters() const override;
    void setPluginParameter(int parameterIndex, float newValue) override;

protected:
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    juce::MidiBuffer midiBuffer;

    // NOTE: the values of these two variables are hard-coded in the constructor
    const double initialSampleRate;
    const int initialBufferSize;

};