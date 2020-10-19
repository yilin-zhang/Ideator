/*
  ==============================================================================

    ConfigWindow.cpp
    Created: 18 Oct 2020 5:15:21pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "ConfigWindow.h"
ConfigWindow::ConfigWindow(const juce::String &name, juce::Colour backgroundColour, int requiredButtons, bool addToDesktop=true) :
        DocumentWindow(name, backgroundColour, requiredButtons, addToDesktop)
{
    //setDraggable(true);
    //setConstrainer(&boundsConstrainer);
    setUsingNativeTitleBar(true);
}

ConfigWindow::~ConfigWindow()
{
}

void ConfigWindow::paint (juce::Graphics&)
{

}

void ConfigWindow::resized()
{

}

void ConfigWindow::closeButtonPressed()
{
    windowClosedBroadcaster.sendChangeMessage();
}

