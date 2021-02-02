/*
  ==============================================================================

    Interface.h
    Created: 12 Oct 2020 8:17:11pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "ProcessorManager.h"
#include "PluginWindow.h"

class Interface : public juce::Component,
                  private juce::ChangeListener,
                  private juce::Label::Listener,
                  private juce::MidiKeyboardState::Listener,
                  private juce::OSCReceiver,
                  private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
public:
    Interface(ProcessorManager&);
    ~Interface();

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ProcessorManager& processorManager;
    juce::Component::SafePointer<PluginWindow> pluginWindow;

    void initializeComponents();

    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    void labelTextChanged(juce::Label* labelThatHasChanged) override;

    // OSC handling
    juce::OSCSender oscSender;
    void oscMessageReceived (const juce::OSCMessage& message) override;
    static void showConnectionErrorMessage (const juce::String& messageText);

    /// functionalities
    // load plugin
    juce::TextButton loadPluginButton   { "Load Plugin" };
    void loadPluginButtonClicked();
    // open the plugin editor
    juce::TextButton openPluginEditorButton   { "Open Plugin" };
    void openPluginEditorButtonClicked();
    // get a random patch
    juce::TextButton getRandomPatchButton   { "Random Patch" };
    void getRandomPatchButtonClicked();
    // render the audio
    juce::TextButton renderAudioButton { "Render Audio" };
    void renderAudioButtonClicked();
    // load the preset
    juce::TextButton loadPresetButton { "Load Preset" };
    void loadPresetButtonClicked();
    // save the preset
    juce::TextButton savePresetButton { "Save Preset" };
    void savePresetButtonClicked();
    // input box
    juce::Value tagInputValue;
    juce::TextEditor tagInputBox {"tagInput"};
    std::vector<juce::String> getTags() const;
    // search button
    juce::TextButton searchButton {"Search"};
    void searchButtonClicked();
    // plugin name
    juce::Label synthNameLabel;
    // timbre descriptors
    juce::Label timbreLabel;
    // preset list
    juce::ListBox presetList;
    // generated tags

    /// MIDI keyboard
    // keyboardState is an argument when initializing midiKeyboard
    // and it should be put before the declaration of midiKeyboard
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent midiKeyboard;
    // MidiKeyboardState::Listener
    void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Interface)
};
