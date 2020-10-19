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

class Interface : public juce::Component,
                  private juce::ChangeListener,
                  private juce::MidiKeyboardState::Listener
{
public:
    Interface(AudioComponent&);
    ~Interface();

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //connection to AudioProcessingComponent (passed from parent)
    AudioComponent& audioComponent;
    juce::TextButton loadPluginButton   { "Load Plugin" };
    juce::TextButton openPluginEditorButton   { "Open Plugin" };

    // keyboardState is an argument when initializing midiKeyboard
    // and it should be put before the declaration of midiKeyboard
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent midiKeyboard;

    juce::Component::SafePointer<PluginWindow> pluginWindow;

    // ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;

    // MidiKeyboardState::Listener
    void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    // additional private methods
    void loadPluginButtonClicked();
    void openPluginEditorButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Interface)
};
