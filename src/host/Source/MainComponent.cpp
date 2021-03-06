#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent():
interface(audioComponent)
{
    setSize(700, 700);
    addAndMakeVisible(interface);
}

MainComponent::~MainComponent()
{
}
//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    interface.setBounds(getLocalBounds());
}
