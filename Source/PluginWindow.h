/*
  ==============================================================================

    PluginWindow.h
    Created: 13 Oct 2020 2:54:49pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class PluginWindow : public juce::DocumentWindow
{
public:
    PluginWindow(const juce::String &name, juce::Colour backgroundColour, int requiredButtons, bool addToDesktop);
    ~PluginWindow();

    void setEditor(juce::AudioProcessorEditor* pluginEditor);
    void paint (juce::Graphics&) override;
    void resized() override;
    void closeButtonPressed() override ;

    juce::ChangeBroadcaster windowClosedBroadcaster;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginWindow)
};