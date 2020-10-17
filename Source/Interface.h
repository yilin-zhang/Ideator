/*
  ==============================================================================

    Interface.h
    Created: 12 Oct 2020 8:17:11pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "AudioComponent.h"
#include "PluginWindow.h"

class Interface : public juce::Component, public juce::ChangeListener
{
public:
    Interface(AudioComponent&);
    ~Interface();

    void paint (juce::Graphics&) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster *source) override;

private:
    //connection to AudioProcessingComponent (passed from parent)
    AudioComponent& audioComponent;
    juce::TextButton loadPluginButton   { "Load Plugin" };
    juce::TextButton openPluginEditorButton   { "Open Plugin" };

    juce::Component::SafePointer<PluginWindow> pluginWindow;

    void loadPluginButtonClicked();
    void openPluginEditorButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Interface)
};
