/*
  ==============================================================================

    Interface.cpp
    Created: 12 Oct 2020 8:17:11pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "Interface.h"
#include "Config.h"

// ================================================
// PresetTableModel
// ================================================

PresetTableModel::PresetTableModel()
{
    presetTable.setModel(this);
    presetTable.getHeader().addColumn("Preset", 1, 150);
    presetTable.getHeader().addColumn("Descriptors", 2, 500);
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
    {
        auto presetName = getPresetNameFromPath(presetPaths[rowNumber]);
        g.drawFittedText(presetName, {6, 0, width - 12, height}, juce::Justification::centredLeft, 1, 1.f);
    }
    else
    {
        auto descriptorString = PresetManager::descriptorsToString(descriptors[rowNumber]);
        g.drawFittedText(descriptorString, {6,0,width - 12,height}, juce::Justification::centredLeft, 1, 1.f);
    }

}

void PresetTableModel::cellClicked (int rowNumber, int columnId, const juce::MouseEvent &)
{
    currentPresetPath = presetPaths[rowNumber];
    currentPluginPath = pluginPaths[rowNumber];
    currentDescriptors = descriptors[rowNumber];
    cellClickedBroadcaster.sendChangeMessage();
}

void PresetTableModel::cellDoubleClicked (int rowNumber, int columnId, const juce::MouseEvent &)
{
    currentPresetPath = presetPaths[rowNumber];
    currentPluginPath = pluginPaths[rowNumber];
    currentDescriptors = descriptors[rowNumber];
    cellDoubleClickedBroadcaster.sendChangeMessage();
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
    // presetTable.selectRow(getNumRows() - 1);
}

void PresetTableModel::clear()
{
    pluginPaths.clear();
    descriptors.clear();
    presetPaths.clear();
    presetTable.selectRow(0);
    presetTable.updateContent();
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

juce::String PresetTableModel::getDescriptorString() const
{
    return PresetManager::descriptorsToString(currentDescriptors);
}

const std::unordered_set<juce::String>& PresetTableModel::getDescriptors() const
{
    return currentDescriptors;
}

juce::String PresetTableModel::getPresetNameFromPath(const juce::String& path)
{
    juce::StringArray tokens;
    tokens.addTokens (path, "/\\", "\"");
    return tokens[tokens.size()-1].trimCharactersAtEnd(".xml");
}

// ================================================
// Interface
// ================================================

Interface::Interface(ProcessorManager& pm, OSCManager& oscManager) :
        processorManager(pm),
        oscManager(oscManager),
        midiKeyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (900, 450);

    initializeComponents();

    oscManager.selectedPresetsReadyBroadcaster.addChangeListener(this);
    oscManager.autoTagsReadyBroadcaster.addChangeListener(this);
    presetList.cellClickedBroadcaster.addChangeListener(this);
    presetList.cellDoubleClickedBroadcaster.addChangeListener(this);
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
    const int margin = 10;
    const int buttonDistance = 35;
    const int keyboardHeight = 64;
    const int inputBoxHeight = 30;

    juce::Rectangle<int> buttonSize (0, 0, 110, 30);
    juce::Rectangle<int> smallButtonSize (0, 0, 80, 30);

    // left column
    juce::Rectangle<int> loadPluginButtonArea (margin, margin,
                                               buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> openPluginEditorButtonArea (margin, margin + buttonDistance,
                                                     buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> setLibraryButtonArea (margin, margin + buttonDistance*2,
                                               buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> analyzeLibraryButtonArea (margin, margin + buttonDistance*3,
                                                   buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> synthNameLabelArea (margin, margin + buttonDistance * 4,
                                             buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> loadPresetButtonArea (margin,
                                               getHeight() - keyboardHeight - margin * 2 - buttonDistance * 2,
                                               buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> savePresetButtonArea (margin, getHeight() - keyboardHeight - margin * 2 - buttonDistance,
                                               buttonSize.getWidth(), buttonSize.getHeight());

    // middle column
    juce::Rectangle<int> tagInputBoxArea (margin * 2 + buttonSize.getWidth(), margin,
                                          getWidth() - buttonSize.getWidth() - smallButtonSize.getWidth() - margin * 4,
                                          inputBoxHeight);
    juce::Rectangle<int> presetListArea (margin * 2 + buttonSize.getWidth(),
                                         margin + buttonDistance,
                                         getWidth() - buttonSize.getWidth() - smallButtonSize.getWidth() - margin * 4,
                                         getHeight() - margin * 5 - inputBoxHeight * 2 - buttonDistance - keyboardHeight);
    juce::Rectangle<int> tagEditInputBoxArea (margin * 2 + buttonSize.getWidth(),
                                              getHeight() - keyboardHeight - margin * 2 - buttonDistance * 2,
                                              getWidth() - buttonSize.getWidth() - smallButtonSize.getWidth() - margin * 4,
                                              inputBoxHeight);
    juce::Rectangle<int> statusLabelArea (margin * 2 + buttonSize.getWidth(),
                                          getHeight() - keyboardHeight - margin * 2 - buttonDistance,
                                          getWidth() - buttonSize.getWidth() - smallButtonSize.getWidth() - margin * 4,
                                          buttonSize.getHeight());

    // right column
    juce::Rectangle<int> searchButtonArea (getWidth() - margin - smallButtonSize.getWidth(),
                                           margin, smallButtonSize.getWidth(), smallButtonSize.getHeight());
    juce::Rectangle<int> findSimilarButtonArea (getWidth() - margin - smallButtonSize.getWidth(),
                                                margin + buttonDistance,
                                                smallButtonSize.getWidth(),
                                                smallButtonSize.getHeight());
    juce::Rectangle<int> presetListUndoButtonArea (getWidth() - margin - smallButtonSize.getWidth(),
                                                   getHeight() - keyboardHeight - margin * 2 - buttonDistance * 3,
                                                   smallButtonSize.getWidth()/2, smallButtonSize.getHeight());
    juce::Rectangle<int> presetListRedoButtonArea (getWidth() - margin - smallButtonSize.getWidth()/2,
                                                   getHeight() - keyboardHeight - margin * 2 - buttonDistance * 3,
                                                   smallButtonSize.getWidth()/2, smallButtonSize.getHeight());
    juce::Rectangle<int> autoTagButtonArea (getWidth() - margin - smallButtonSize.getWidth(),
                                            getHeight() - keyboardHeight - margin * 2 - buttonDistance * 2,
                                            smallButtonSize.getWidth(), smallButtonSize.getHeight());
    juce::Rectangle<int> confirmTagButtonArea (getWidth() - margin - smallButtonSize.getWidth(),
                                               getHeight() - keyboardHeight - margin * 2 - buttonDistance,
                                               smallButtonSize.getWidth(), smallButtonSize.getHeight());

    // keyboard
    juce::Rectangle<int> keyboardArea (margin, getHeight() - keyboardHeight - margin,
                                       getWidth() - 2 * margin, keyboardHeight);

    loadPluginButton.setBounds(loadPluginButtonArea);
    openPluginEditorButton.setBounds(openPluginEditorButtonArea);
    setLibraryButton.setBounds(setLibraryButtonArea);
    analyzeLibraryButton.setBounds(analyzeLibraryButtonArea);
    searchButton.setBounds(searchButtonArea);
    findSimilarButton.setBounds(findSimilarButtonArea);
    presetListUndoButton.setBounds(presetListUndoButtonArea);
    presetListRedoButton.setBounds(presetListRedoButtonArea);
    autoTagButton.setBounds(autoTagButtonArea);
    confirmTagButton.setBounds(confirmTagButtonArea);
    loadPresetButton.setBounds(loadPresetButtonArea);
    savePresetButton.setBounds(savePresetButtonArea);
    tagInputBox.setBounds(tagInputBoxArea);
    synthNameLabel.setBounds(synthNameLabelArea);
    statusLabel.setBounds(statusLabelArea);
    presetList.setBounds(presetListArea);
    tagEditInputBox.setBounds(tagEditInputBoxArea);
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

    findSimilarButton.onClick = [this] {findSimilarButtonClicked(); };
    addAndMakeVisible(findSimilarButton);

    presetListUndoButton.onClick = [this] {presetListUndoButtonClicked(); };
    addAndMakeVisible(presetListUndoButton);
    presetListUndoButton.setEnabled(undoStack.isUndoAvailable());

    presetListRedoButton.onClick = [this] {presetListRedoButtonClicked(); };
    addAndMakeVisible(presetListRedoButton);
    presetListRedoButton.setEnabled(undoStack.isRedoAvailable());

    autoTagButton.onClick = [this] {autoTagButtonClicked(); };
    addAndMakeVisible(autoTagButton);

    confirmTagButton.onClick = [this] {confirmTagButtonClicked(); };
    addAndMakeVisible(confirmTagButton);

    loadPresetButton.onClick = [this] {loadPresetButtonClicked(); };
    addAndMakeVisible(loadPresetButton);

    savePresetButton.onClick = [this] {savePresetButtonClicked(); };
    addAndMakeVisible(savePresetButton);

    setLibraryButton.onClick = [this] {setLibraryButtonClicked(); };
    addAndMakeVisible(setLibraryButton);

    addAndMakeVisible(tagInputBox);
    addAndMakeVisible(tagEditInputBox);
    addAndMakeVisible(presetList);

    synthNameLabel.addListener(this);
    addAndMakeVisible(synthNameLabel);

    statusLabel.addListener(this);
    addAndMakeVisible(statusLabel);

    midiKeyboard.setName ("MIDI Keyboard");
    addAndMakeVisible(midiKeyboard);
    keyboardState.addListener(this);
}

void Interface::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (pluginWindow && (source == &pluginWindow->windowClosedBroadcaster))
        pluginWindow.deleteAndZero();

    else if (source == &presetList.cellClickedBroadcaster)
    {
        loadPresetCallback(presetList.getPresetPath());
    }

    else if (source == &presetList.cellDoubleClickedBroadcaster)
    {
        loadPresetCallback(presetList.getPresetPath());
        openPluginEditorCallback();
    }

    else if (source == &oscManager.selectedPresetsReadyBroadcaster)
    {
        auto presetPaths = oscManager.getSelectedPresetPaths();
        if (!presetPaths.isEmpty())
        {
            setPresetList(presetPaths);
            undoStack.push(presetPaths);
            presetListUndoButton.setEnabled(undoStack.isUndoAvailable());
            presetListRedoButton.setEnabled(undoStack.isRedoAvailable());
        }
    }

    else if (source == &oscManager.autoTagsReadyBroadcaster)
    {
        auto autoTags = oscManager.getAutoTags();
        tagEditInputBox.setText(autoTags.joinIntoString(", "));
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
    if (path != currentPluginPath)
        if (pluginWindow)
            pluginWindow.deleteAndZero();

    processorManager.loadPlugin(path);

    synthNameLabel.setText("Synth: " + processorManager.getPluginDescription().name,
                           juce::NotificationType::sendNotification);
    currentPluginPath = path;
}

void Interface::loadPresetCallback(const juce::String &path)
{
    juce::String descriptorString;

    // Load the plugin if the plugin is different from the current one
    // it's possible that the presetList returns an empty plugin path when
    // the user imports a new library after he/she loads a plugin.
    // so it's necessary to reject the empty plugin path.
    if (path != processorManager.getPluginPath() && !path.isEmpty())
    {
        // parse the preset
        juce::File file(path);
        auto xmlPreset = juce::XmlDocument::parse(file);
        if (!xmlPreset)
            return;
        juce::String pluginPath;
        std::unordered_set<juce::String> descriptors;
        juce::Array<std::pair<int, float>> parameters;
        auto isSuccessful = PresetManager::parse(*xmlPreset, parameters, pluginPath, descriptors);
        if (!isSuccessful)
            return;
        descriptorString = PresetManager::descriptorsToString(descriptors);

        // load the plugin
        loadPluginCallback(pluginPath);
    }

    // load the preset
    if (!processorManager.loadPreset(path))
    {
        DBG("Interface::loadPresetCallback error.");
        return;
    }

    // set the text
    tagEditInputBox.setText(descriptorString);
    statusLabel.setText("Preset: " + path, juce::NotificationType::sendNotification);
    synthNameLabel.setText("Synth: " + processorManager.getPluginDescription().name,
                           juce::NotificationType::sendNotification);
}

void Interface::openPluginEditorCallback()
{
    if (processorManager.checkPluginLoaded())
    {
        // Don't do anything if the window has already opened
        // Do the checking before getting the plugin editor, otherwise if the window has opened,
        // the plugin window will not change after loading a new preset.
        // Maybe it's because a new editor is created and activated but not registered in the plugin window.
        if (pluginWindow)
            return;

        auto editor = processorManager.getPluginEditor();

        if (!editor)
        {
            DBG("The plugin does not have an editor");
            return;
        }

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

void Interface::setPresetList(const juce::StringArray &presetPaths)
{
    presetList.clear();

    for (const auto &path : presetPaths)
    {
        juce::File file(path); // the path is already a absolute path
        auto xmlPreset = juce::XmlDocument::parse(file);
        if (!xmlPreset)
            return;

        juce::String pluginPath;
        std::unordered_set<juce::String> descriptors;
        juce::Array<std::pair<int, float>> parameters;
        auto isSuccessful = PresetManager::parse(*xmlPreset, parameters, pluginPath, descriptors);
        if (!isSuccessful)
            continue;

        presetList.addItem(pluginPath, path, descriptors);
    }
}

// =================================================
// button callbacks
// =================================================

void Interface::openPluginEditorButtonClicked()
{
    openPluginEditorCallback();
}

void Interface::analyzeLibraryButtonClicked()
{
    processorManager.analyzeLibrary(presetList.getLibraryPresetPaths());
}

void Interface::searchButtonClicked()
{
    oscManager.sendRequestForPresetRetrieval(tagInputBox.getText());
}

void Interface::findSimilarButtonClicked()
{
    processorManager.findSimilar();
}

void Interface::autoTagButtonClicked()
{
    processorManager.autoTag();
}

void Interface::loadPresetButtonClicked()
{
    juce::FileChooser fileChooser("Select the path", {});
    if (fileChooser.browseForFileToOpen())
    {
        loadPresetCallback(fileChooser.getResult().getFullPathName());
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

            juce::String pluginPath;
            std::unordered_set<juce::String> descriptors;
            juce::Array<std::pair<int, float>> parameters;
            auto isSuccessful = PresetManager::parse(*xmlPreset, parameters, pluginPath, descriptors);
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

void Interface::confirmTagButtonClicked()
{
    auto selectedPresetPath = presetList.getPresetPath();
    auto descriptors= PresetManager::stringToDescriptors(tagEditInputBox.getText());
    processorManager.changeDescriptors(selectedPresetPath, descriptors);

    // refresh the preset list
    auto presetPaths = presetList.getLibraryPresetPaths();
    setPresetList(presetPaths);
}

void Interface::presetListUndoButtonClicked()
{
    if (!undoStack.isUndoAvailable())
        return;

    auto presetPaths = undoStack.undo();
    setPresetList(presetPaths);
    presetListUndoButton.setEnabled(undoStack.isUndoAvailable());
    presetListRedoButton.setEnabled(undoStack.isRedoAvailable());
}

void Interface::presetListRedoButtonClicked()
{
    if (!undoStack.isRedoAvailable())
        return;

    auto presetPaths = undoStack.redo();
    setPresetList(presetPaths);
    presetListUndoButton.setEnabled(undoStack.isUndoAvailable());
    presetListRedoButton.setEnabled(undoStack.isRedoAvailable());
}