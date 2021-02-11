/*
  ==============================================================================

    ConfigWindow.h
    Created: 18 Oct 2020 5:15:21pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


class ConfigWindow : public juce::DocumentWindow
{
public:
    ConfigWindow(const juce::String &name, juce::Colour backgroundColour, int requiredButtons, bool addToDesktop);
    ~ConfigWindow();

    void paint (juce::Graphics&) override;
    void resized() override;
    void closeButtonPressed() override ;

    juce::ChangeBroadcaster windowClosedBroadcaster;

private:



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigWindow)
};
