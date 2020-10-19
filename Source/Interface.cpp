/*
  ==============================================================================

    Interface.cpp
    Created: 12 Oct 2020 8:17:11pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "Interface.h"

Interface::Interface(AudioComponent& audioComponent) :
audioComponent(audioComponent),
midiKeyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Make sure you set the size of the component after
    // you add any child components.
    //setSize (800, 600);
    loadPluginButton.onClick = [this] {loadPluginButtonClicked(); };
    addAndMakeVisible(loadPluginButton);

    openPluginEditorButton.onClick = [this] {openPluginEditorButtonClicked(); };
    addAndMakeVisible(openPluginEditorButton);

    midiKeyboard.setName ("MIDI Keyboard");
    addAndMakeVisible(midiKeyboard);
    keyboardState.addListener(this);
}

Interface::~Interface()
{
    if (pluginWindow)
        delete pluginWindow;

    keyboardState.removeListener(this);
}

void Interface::paint (juce::Graphics& g)
{
    //g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void Interface::resized()
{
    // TODO: clean up the mess here
    juce::Rectangle<int> buttonSize (0, 0, 108, 28);
    juce::Rectangle<int> area1 ((getWidth()  / 2) - (buttonSize.getWidth() / 2),
                         (getHeight() / 2) -  buttonSize.getHeight(),
                         buttonSize.getWidth(), buttonSize.getHeight());

    juce::Rectangle<int> area2 ((getWidth()  / 2) - (buttonSize.getWidth() / 2),
                                (getHeight() / 2) -  buttonSize.getHeight() + 30,
                                buttonSize.getWidth(), buttonSize.getHeight());
    loadPluginButton.setBounds(area1);

    openPluginEditorButton.setBounds(area2);

    auto margin = 10;

    midiKeyboard.setBounds (margin, (getHeight() / 2) + (24 + margin), getWidth() - (2 * margin), 64);

}

void Interface::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (pluginWindow && (source == &pluginWindow->windowClosedBroadcaster))
    {
        delete pluginWindow;
    }
}

void Interface::handleNoteOn(juce::MidiKeyboardState *, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::MidiMessage m (juce::MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
    m.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
    audioComponent.addMidiEvent(m);

}

void Interface::handleNoteOff(juce::MidiKeyboardState *, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::MidiMessage m (juce::MidiMessage::noteOff (midiChannel, midiNoteNumber, velocity));
    m.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
    audioComponent.addMidiEvent(m);
}

void Interface::loadPluginButtonClicked()
{
    juce::FileChooser fileChooser("Select a plugin", {});
    if (fileChooser.browseForFileToOpen())
    {
        auto filePath = fileChooser.getResult().getFullPathName();
        std::cout << "Path: " << filePath << std::endl;
        audioComponent.loadPlugin(filePath);
        // TODO: arguments hard coded here, also it doesn't work when the block size is 256 and idk why
        audioComponent.prepareToPlay(512, 44100.f);
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

