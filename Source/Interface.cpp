/*
  ==============================================================================

    Interface.cpp
    Created: 12 Oct 2020 8:17:11pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "Interface.h"

Interface::Interface(AudioComponent& audioComponent) :
audioComponent(audioComponent)
{
    // Make sure you set the size of the component after
    // you add any child components.
    //setSize (800, 600);
    addAndMakeVisible(&loadPluginButton);
    loadPluginButton.onClick = [this] {loadPluginButtonClicked(); };

    addAndMakeVisible(&openPluginEditorButton);
    openPluginEditorButton.onClick = [this] {openPluginEditorButtonClicked(); };
}

Interface::~Interface()
{
    if (pluginWindow)
        delete pluginWindow;
}

void Interface::paint (juce::Graphics& g)
{
    //g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void Interface::resized()
{
    juce::Rectangle<int> buttonSize (0, 0, 108, 28);
    juce::Rectangle<int> area1 ((getWidth()  / 2) - (buttonSize.getWidth() / 2),
                         (getHeight() / 2) -  buttonSize.getHeight(),
                         buttonSize.getWidth(), buttonSize.getHeight());

    juce::Rectangle<int> area2 ((getWidth()  / 2) - (buttonSize.getWidth() / 2),
                                (getHeight() / 2) -  buttonSize.getHeight() + 50,
                                buttonSize.getWidth(), buttonSize.getHeight());
    loadPluginButton.setBounds(area1);

    openPluginEditorButton.setBounds(area2);
}

void Interface::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (pluginWindow && (source == &pluginWindow->windowClosedBroadcaster))
    {
        delete pluginWindow;
    }
}

void Interface::loadPluginButtonClicked()
{
    juce::FileChooser fileChooser("Select a plugin", {});
    if (fileChooser.browseForFileToOpen())
    {
        auto filePath = fileChooser.getResult().getFullPathName();
        std::cout << "Path: " << filePath << std::endl;
        audioComponent.loadPlugin(filePath);
    }
}

void Interface::openPluginEditorButtonClicked()
{
    if (audioComponent.checkPluginLoaded())
    {
        auto editor = audioComponent.getPluginEditor();
        pluginWindow = new PluginWindow("Plugin", juce::Colours::black, 7, true);
        pluginWindow->windowClosedBroadcaster.addChangeListener(this);
        pluginWindow->setAndOpenPluginWindow(*editor);
        pluginWindow->setVisible(true);
    }
    else
    {
        std::cout << "No plugin loaded!" << std::endl;
    }
}

