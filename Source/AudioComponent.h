/*
  ==============================================================================

    AudioComponent.h
    Created: 12 Oct 2020 8:19:53pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class AudioComponent : public juce::AudioAppComponent
{
public:
    AudioComponent();
    ~AudioComponent();
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

    // methods for the plugin
    bool loadPlugin(const juce::String& path);
    bool checkPluginLoaded() const;
    juce::AudioProcessorEditor* getPluginEditor();
    void addMidiEvent(const juce::MidiMessage &midiMessage);
    juce::PluginDescription getPluginDescription() const;
    const juce::Array<juce::AudioProcessorParameter*>& getPluginParameters() const;
    void setPluginParameter(int parameterIndex, float newValue);

private:
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    double sampleRate;
    int bufferSize;
    bool isPluginLoaded;

    juce::MidiBuffer midiBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioComponent)
};
