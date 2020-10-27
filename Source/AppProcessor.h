/*
  ==============================================================================

    AppProcessor.h
    Created: 12 Oct 2020 8:19:53pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginManager.h"

class AppProcessor : public juce::AudioAppComponent,
                     public PluginManager
{
public:
    AppProcessor();
    ~AppProcessor();
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppProcessor)
};
