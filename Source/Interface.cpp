/*
  ==============================================================================

    Interface.cpp
    Created: 12 Oct 2020 8:17:11pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "Interface.h"
#include "Config.h"
#include "Utils.h"

// ================================================
// PresetTableModel
// ================================================

PresetTableModel::PresetTableModel()
{
    presetTable.setModel(this);
    presetTable.getHeader().addColumn("Path", 1, 80);
    presetTable.getHeader().addColumn("Descriptors", 2, 80);
    presetTable.setColour(juce::ListBox::backgroundColourId, juce::Colour::greyLevel(0.2f));
    presetTable.setRowHeight(30);
    addAndMakeVisible(presetTable);
}

int PresetTableModel::getNumRows()
{
    return presetPaths.size();
}

void PresetTableModel::paintRowBackground (juce::Graphics &g, int rowNumber, int width, int height, bool rowIsSelected)
{
    // make sure the rowNumber is in the range
    if(!juce::isPositiveAndBelow(rowNumber, getNumRows())) return;

    g.fillAll(juce::Colour::greyLevel(rowIsSelected ? 0.15f : 0.05f));
    g.setFont(18);
    g.setColour(juce::Colours::whitesmoke);
    //g.drawFittedText(presetPaths[rowNumber], {6,0,width - 12,height}, juce::Justification::centredLeft, 1, 1.f);
}

void PresetTableModel::paintCell (juce::Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if(!juce::isPositiveAndBelow(rowNumber, getNumRows())) return;

    g.fillAll(juce::Colour::greyLevel(rowIsSelected ? 0.45f : 0.05f));
    g.setFont(18);
    g.setColour(juce::Colours::whitesmoke);
    if (columnId == 1)
        g.drawFittedText(presetPaths[rowNumber], {6, 0, width - 12, height}, juce::Justification::centredLeft, 1, 1.f);
    else
    {
        juce::String descriptorString;
        auto descriptorOfThisRow = descriptors[rowNumber];
        auto descriptorIter = descriptorOfThisRow.begin();
        for (int i=0; i<descriptors[rowNumber].size(); ++i)
        {
            juce::String descriptor = *descriptorIter;
            if (i == 0)
                descriptorString << descriptor;
            else
                descriptorString << ", " << descriptor;
            ++descriptorIter;
        }

        g.drawFittedText(descriptorString, {6,0,width - 12,height}, juce::Justification::centredLeft, 1, 1.f);
    }

}

void PresetTableModel::cellClicked (int rowNumber, int columnId, const juce::MouseEvent &)
{
    currentPresetPath = presetPaths[rowNumber];
    currentPluginPath = pluginPaths[rowNumber];
    sendChangeMessage();
}

void PresetTableModel::resized()
{
    presetTable.setBounds(getLocalBounds());
}

void PresetTableModel::addItem(const juce::String &pluginPath, const juce::String &presetPath, const std::unordered_set<juce::String> &descriptors)
{
    this->pluginPaths.add(pluginPath);
    this->descriptors.add(descriptors);
    this->presetPaths.add(presetPath);
    presetTable.updateContent();
    presetTable.selectRow(getNumRows() - 1);
}

void PresetTableModel::clear()
{
    pluginPaths.clear();
    descriptors.clear();
    presetPaths.clear();
    presetTable.selectRow(0);
    presetTable.updateContent();
    sendChangeMessage();
}

const juce::String& PresetTableModel::getPluginPath() const
{
    return currentPluginPath;
}

const juce::String& PresetTableModel::getPresetPath() const
{
    return currentPresetPath;
}

const juce::Array<juce::String>& PresetTableModel::getLibraryPresetPaths() const
{
    return presetPaths;
}

// ================================================
// Interface
// ================================================

Interface::Interface(ProcessorManager& pm) :
        processorManager(pm),
        midiKeyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (450, 450);

    initializeComponents();

    presetList.addChangeListener(this);
}

Interface::~Interface()
{
    if (pluginWindow)
        pluginWindow.deleteAndZero();

    keyboardState.removeListener(this);

}

void Interface::paint (juce::Graphics& g)
{
    //g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void Interface::resized()
{
    // TODO: clean up the mess here
    juce::Rectangle<int> buttonSize (0, 0, 110, 30);
    juce::Rectangle<int> loadPluginButtonArea (10,10,
                                               buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> openPluginEditorButtonArea (10, 45,
                                                     buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> setLibraryButtonArea (10, 80,
                                               buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> analyzeLibraryButtonArea (10, 115,
                                                   buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> loadPresetButtonArea (10, 265,
                                               buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> savePresetButtonArea (10, 300,
                                               buttonSize.getWidth(), buttonSize.getHeight());

    juce::Rectangle<int> synthNameLabelArea (10, 150,
                                             buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> timbreLabelArea (10, 185,
                                          buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> statusLabelArea (10, 220,
                                          buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> inputBoxArea (160,10,200, 30);
    juce::Rectangle<int> presetListArea (160,45,200, 300);
    juce::Rectangle<int> searchButtonArea (370,10,70, 30);
    const int keyboardMargin = 10;
    juce::Rectangle<int> keyboardArea (keyboardMargin,getHeight()-64-keyboardMargin,
                                       getWidth() - 2 * keyboardMargin, 64);

    loadPluginButton.setBounds(loadPluginButtonArea);
    openPluginEditorButton.setBounds(openPluginEditorButtonArea);
    setLibraryButton.setBounds(setLibraryButtonArea);
    analyzeLibraryButton.setBounds(analyzeLibraryButtonArea);
    searchButton.setBounds(searchButtonArea);
    loadPresetButton.setBounds(loadPresetButtonArea);
    savePresetButton.setBounds(savePresetButtonArea);
    tagInputBox.setBounds(inputBoxArea);
    synthNameLabel.setBounds(synthNameLabelArea);
    timbreLabel.setBounds(timbreLabelArea);
    statusLabel.setBounds(statusLabelArea);
    presetList.setBounds(presetListArea);
    midiKeyboard.setBounds(keyboardArea);
}

void Interface::initializeComponents()
{
    loadPluginButton.onClick = [this] {loadPluginButtonClicked(); };
    addAndMakeVisible(loadPluginButton);

    openPluginEditorButton.onClick = [this] {openPluginEditorButtonClicked(); };
    addAndMakeVisible(openPluginEditorButton);

    analyzeLibraryButton.onClick = [this] { analyzeLibraryButtonClicked(); };
    addAndMakeVisible(analyzeLibraryButton);

    searchButton.onClick = [this] {searchButtonClicked(); };
    addAndMakeVisible(searchButton);

    loadPresetButton.onClick = [this] {loadPresetButtonClicked(); };
    addAndMakeVisible(loadPresetButton);

    savePresetButton.onClick = [this] {savePresetButtonClicked(); };
    addAndMakeVisible(savePresetButton);

    setLibraryButton.onClick = [this] {setLibraryButtonClicked(); };
    addAndMakeVisible(setLibraryButton);

    addAndMakeVisible(tagInputBox);
    addAndMakeVisible(presetList);

    synthNameLabel.addListener(this);
    addAndMakeVisible(synthNameLabel);

    synthNameLabel.addListener(this);
    addAndMakeVisible(statusLabel);

    midiKeyboard.setName ("MIDI Keyboard");
    addAndMakeVisible(midiKeyboard);
    keyboardState.addListener(this);
}

void Interface::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (pluginWindow && (source == &pluginWindow->windowClosedBroadcaster))
        pluginWindow.deleteAndZero();

    else if (source == &presetList)
    {
        // it's possible that the presetList returns an empty plugin path when
        // the user imports a new library after he/she loads a plugin.
        // so it's necessary to reject the empty plugin path.
        if (presetList.getPluginPath() != processorManager.getPluginPath() &&
            !presetList.getPluginPath().isEmpty())
            loadPluginCallback(presetList.getPluginPath());
        if (!processorManager.loadPreset(presetList.getPresetPath()))
        {
            DBG("Interface::changeListenerCallback error.");
            return;
        }
    }
}

void Interface::labelTextChanged(juce::Label *labelThatHasChanged)
{
    // NOTE: not sure if this is necessary, but let's just keep it for now
}

void Interface::handleNoteOn(juce::MidiKeyboardState *, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::MidiMessage m (juce::MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
    m.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
    processorManager.addMidiEvent(m);

}

void Interface::handleNoteOff(juce::MidiKeyboardState *, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::MidiMessage m (juce::MidiMessage::noteOff (midiChannel, midiNoteNumber, velocity));
    m.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
    processorManager.addMidiEvent(m);
}

void Interface::loadPluginButtonClicked()
{
    juce::FileChooser fileChooser("Select a plugin", {});
#if __linux__
    if (fileChooser.browseForDirectory())
#else
    if (fileChooser.browseForFileToOpen())
#endif
    {
        loadPluginCallback(fileChooser.getResult().getFullPathName());
#ifdef IDEATOR_APP
        // TODO: arguments hard coded here, also it doesn't work when the block size is 256 and idk why
        processorManager.prepareToPlay(512, 44100.f);
#else
        processorManager.prepareToPlay();
#endif
    }
}

void Interface::loadPluginCallback(const juce::String &path)
{
    // If there is a opened plugin window, delete it.
    // Must delete the window before loading a new plugin
    // because the plugin editor is bounded to its plugin processor
    if (pluginWindow)
        pluginWindow.deleteAndZero();

    processorManager.loadPlugin(path);

    synthNameLabel.setText("Synth: " + processorManager.getPluginDescription().name,
                           juce::NotificationType::sendNotification);
}

// =================================================
// button callbacks
// =================================================

void Interface::openPluginEditorButtonClicked()
{
    if (processorManager.checkPluginLoaded())
    {
        auto editor = processorManager.getPluginEditor();

        if (!editor)
        {
            DBG("The plugin does not have an editor");
            return;
        }

        // don't do anything if the window has already opened
        if (pluginWindow)
            return;

        // create a new plugin window and initialize it
        pluginWindow = new PluginWindow("Plugin", juce::Colours::black, 7, true);
        pluginWindow->windowClosedBroadcaster.addChangeListener(this);
        pluginWindow->setEditor(editor);
        pluginWindow->setVisible(true);
    }
    else
    {
        DBG("No plugin loaded!");
    }
}

void Interface::analyzeLibraryButtonClicked()
{
    processorManager.analyzeLibrary(presetList.getLibraryPresetPaths());
}

void Interface::searchButtonClicked()
{
    auto tags = getTags();
}

void Interface::loadPresetButtonClicked()
{
    juce::FileChooser fileChooser("Select the path", {});
    if (fileChooser.browseForFileToOpen())
    {
        if (!processorManager.loadPreset(fileChooser.getResult().getFullPathName()))
        {
            DBG("Interface::loadPresetButtonClicked error.");
            return;
        }

        // The plugin path is written inside the preset and the program will
        // load the plugin according to it. Therefore, we should update the plugin name here.
        // It is the same as we do in loadPluginButtonClicked.
        synthNameLabel.setText("Synth: " + processorManager.getPluginDescription().name,
                               juce::NotificationType::sendNotification);
    }
}

void Interface::setLibraryButtonClicked()
{
    juce::FileChooser fileChooser("Select the directory", {});
    if (fileChooser.browseForDirectory())
    {
        libraryPath = fileChooser.getResult().getFullPathName();
        statusLabel.setText("Library Path: " + libraryPath, juce::NotificationType::dontSendNotification);

        // scan all the presets in the directory and add them to presetList
        presetList.clear();
        juce::File libraryDir(libraryPath);
        auto presetFiles = libraryDir.findChildFiles(juce::File::TypesOfFileToFind::findFiles,
                                                     true,
                                                     "*.xml");
        for (auto &file : presetFiles)
        {
            auto presetPath = file.getFullPathName();
            auto xmlPreset = juce::XmlDocument::parse(file);
            if (!xmlPreset)
                return;

            juce::XmlElement xmlState("Parameters");
            juce::String pluginPath;
            std::unordered_set<juce::String> descriptors;
            auto isSuccessful = PresetManager::parse(*xmlPreset, xmlState, pluginPath, descriptors);
            if (!isSuccessful)
                continue;
            presetList.addItem(pluginPath, presetPath, descriptors);
        }
    }
}

void Interface::savePresetButtonClicked()
{
    juce::FileChooser fileChooser("Select the path", {});
    if (fileChooser.browseForFileToSave(true))
    {
        if (!processorManager.savePreset(fileChooser.getResult().getFullPathName()))
            DBG("Interface::savePresetButtonClicked error.");
    }
}

juce::StringArray Interface::getTags() const
{
    juce::StringArray inputTags;
    inputTags.addTokens(tagInputBox.getText(), ", &", "\"");
    inputTags.removeEmptyStrings(true);

    return inputTags;
}
