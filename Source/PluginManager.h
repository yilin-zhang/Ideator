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

class PluginManager : public PluginManagerIf,
                      private juce::AudioProcessorListener
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
    bool renderAudio(juce::String &audioPath) override;
    void loadPreset(const juce::String &presetPath) override;
    void savePreset(const juce::String &presetPath) override;
    void setTimbreDesctiptors(const std::unordered_set<juce::String> &timbreDescriptors) override;
    const std::unordered_set<juce::String>& getTimbreDesctiptors() const override;
    void setPresetPath(const juce::String &path) override;
    const juce::String& getPresetPath() const override;

protected:
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    juce::MidiBuffer midiBuffer;

    // NOTE: the values of these two variables are hard-coded in the constructor
    const double initialSampleRate;
    const int initialBufferSize;

    double internSampleRate;
    int internSamplesPerBlock;

    // extra states
    std::unordered_set<juce::String> timbreDescriptors;
    juce::String presetPath; // empty string means the preset has not been saved
    juce::String pluginPath;

private:
    void audioProcessorParameterChanged (juce::AudioProcessor *processor, int parameterIndex, float newValue) override;
    void audioProcessorChanged (juce::AudioProcessor *processor) override;
};