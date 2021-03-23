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

class PresetTableModel : public juce::Component,
                         public juce::TableListBoxModel,
                         public juce::ChangeBroadcaster
{
public:
    explicit PresetTableModel();
    int getNumRows() override;
    void paintRowBackground (juce::Graphics &g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (juce::Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    void cellClicked (int rowNumber, int columnId, const juce::MouseEvent &) override;
    void resized() override;
    void addItem(const juce::String &pluginPath, const juce::String &presetPath, const std::unordered_set<juce::String> &descriptors);
    void clear();
    const juce::String& getPluginPath() const;
    const juce::String& getPresetPath() const;
    const juce::Array<juce::String>& getLibraryPresetPaths() const;
    juce::String getDescriptorString() const;
    const std::unordered_set<juce::String>& getDescriptors() const;
private:
    static juce::String getPresetNameFromPath(const juce::String& path);
    juce::TableListBox presetTable;
    juce::Array<juce::String> pluginPaths;
    juce::Array<juce::String> presetPaths;
    juce::Array<std::unordered_set<juce::String>> descriptors;
    juce::String currentPluginPath;
    juce::String currentPresetPath;
    std::unordered_set<juce::String> currentDescriptors;
};

class Interface : public juce::Component,
                  private juce::ChangeListener,
                  private juce::Label::Listener,
                  private juce::MidiKeyboardState::Listener
{
public:
    Interface(ProcessorManager&, OSCManager&);
    ~Interface();

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ProcessorManager& processorManager;
    OSCManager& oscManager;
    juce::Component::SafePointer<PluginWindow> pluginWindow;
    juce::String currentPluginPath;

    void initializeComponents();

    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    void labelTextChanged(juce::Label* labelThatHasChanged) override;

    // custom callbacks
    void loadPluginCallback(const juce::String &path);
    void loadPresetCallback(const juce::String &path);
    void refreshPresetList();

    /// functionalities
    // load plugin
    juce::TextButton loadPluginButton   { "Load Plugin" };
    void loadPluginButtonClicked();
    // open the plugin editor
    juce::TextButton openPluginEditorButton   { "Open Plugin" };
    void openPluginEditorButtonClicked();
    // analyze the audio
    juce::TextButton analyzeLibraryButton {"Analyze Lib" };
    void analyzeLibraryButtonClicked();
    // load the preset
    juce::TextButton loadPresetButton { "Load Preset" };
    void loadPresetButtonClicked();
    // save the preset
    juce::TextButton savePresetButton { "Save Preset As..." };
    void savePresetButtonClicked();
    // set preset library path
    juce::TextButton setLibraryButton { "Set Library" };
    void setLibraryButtonClicked();
    // input boxes
    juce::TextEditor tagInputBox {"tagInput"};
    juce::TextEditor tagEditInputBox{"tagEdit"};
    // search button
    juce::TextButton searchButton {"Search"};
    void searchButtonClicked();
    // buttons on the right side
    juce::TextButton findSimilarButton {"Similar"};
    void findSimilarButtonClicked();
    juce::TextButton autoTagButton {"Auto Tag"};
    void autoTagButtonClicked();
    juce::TextButton confirmTagButton {"Confirm"};
    void confirmTagButtonClicked();
    // plugin name
    juce::Label synthNameLabel;
    // status label
    juce::Label statusLabel;
    // preset list
    PresetTableModel presetList;

    // TODO: maybe put this into another class
    juce::String libraryPath; // the path to the preset library

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
