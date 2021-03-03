/*
  ==============================================================================

    PluginEditor.cpp
    Created: 27 Oct 2020 12:35:35am
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
        : AudioProcessorEditor (&p), processorManager(p), interface(processorManager, oscManager)
{
    // Must set the OSC manager at the beginning. There is an OSCManager in the audio processor, but
    // there is no way to change the constructor of PluginProcessor (at least it seems like) and we
    // don't take care of the initialization of it, so the only way is to pass the oscManager as a pointer.
    processorManager.setOSCManager(&oscManager);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setResizable(true, true);
    setResizeLimits(interface.getWidth(), interface.getHeight(),
                    1920, 1080);
    setSize (interface.getWidth(), interface.getHeight());
    addAndMakeVisible(interface);
}

PluginEditor::~PluginEditor()
{
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setColour (juce::Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void PluginEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    interface.setBounds(getLocalBounds());
}
