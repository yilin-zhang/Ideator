/*
  ==============================================================================

    PluginWindow.cpp
    Created: 13 Oct 2020 2:54:49pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "PluginWindow.h"

PluginWindow::PluginWindow(const juce::String &name, juce::Colour backgroundColour, int requiredButtons, bool addToDesktop=true) :
DocumentWindow(name, backgroundColour, requiredButtons, addToDesktop)
{
    setUsingNativeTitleBar(true);
}

PluginWindow::~PluginWindow()= default;

void PluginWindow::paint (juce::Graphics&)
{

}

void PluginWindow::resized()
{

}

void PluginWindow::closeButtonPressed()
{
    windowClosedBroadcaster.sendChangeMessage();
}

void PluginWindow::setEditor(juce::AudioProcessorEditor* pluginEditor)
{
    setContentOwned(pluginEditor, true);
}
