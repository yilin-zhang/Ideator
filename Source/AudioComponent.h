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
    
    bool loadPlugin(const juce::String& path);
    juce::AudioProcessorEditor* getPluginEditor();

    bool checkPluginLoaded();

private:
    //juce::AudioPluginInstance* plugin;
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    double sampleRate;
    int bufferSize;

    bool isPluginLoaded;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioComponent)
};
