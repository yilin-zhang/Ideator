/*
  ==============================================================================

    Interface.cpp
    Created: 12 Oct 2020 8:17:11pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "Interface.h"
#include "Config.h"

Interface::Interface(ProcessorManager& pm) :
        processorManager(pm),
        midiKeyboard(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (450, 450);

    initializeComponents();

    // https://docs.juce.com/master/tutorial_osc_sender_receiver.html
    // specify here on which UDP port number to receive incoming OSC messages
    if (! connect (OSC_RECEIVE_PORT))
        showConnectionErrorMessage ("Error: could not connect to UDP port " +
        juce::String(OSC_RECEIVE_PORT) + ".");

    addListener(this, OSC_RECEIVE_PATTERN + "set_parameter");
    oscSender.connect(LOCAL_ADDRESS, OSC_SEND_PORT);
}

Interface::~Interface()
{
    if (pluginWindow)
        pluginWindow.deleteAndZero();

    keyboardState.removeListener(this);

    removeListener(this);
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
    juce::Rectangle<int> renderAudioButtonArea (10, 80,
                                                buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> getRandomPatchButtonArea (10, 115,
                                                   buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> loadPresetButtonArea (10, 265,
                                               buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> savePresetButtonArea (10, 300,
                                               buttonSize.getWidth(), buttonSize.getHeight());

    juce::Rectangle<int> synthNameLabelArea (10, 150,
                                             buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> timbreLabelArea (10, 185,
                                          buttonSize.getWidth(), buttonSize.getHeight());
    juce::Rectangle<int> inputBoxArea (160,10,200, 30);
    juce::Rectangle<int> presetListArea (160,45,200, 300);
    juce::Rectangle<int> searchButtonArea (370,10,70, 30);
    const int keyboardMargin = 10;
    juce::Rectangle<int> keyboardArea (keyboardMargin,getHeight()-64-keyboardMargin,
                                       getWidth() - 2 * keyboardMargin, 64);

    loadPluginButton.setBounds(loadPluginButtonArea);
    openPluginEditorButton.setBounds(openPluginEditorButtonArea);
    getRandomPatchButton.setBounds(getRandomPatchButtonArea);
    renderAudioButton.setBounds(renderAudioButtonArea);
    searchButton.setBounds(searchButtonArea);
    loadPresetButton.setBounds(loadPresetButtonArea);
    savePresetButton.setBounds(savePresetButtonArea);
    tagInputBox.setBounds(inputBoxArea);
    synthNameLabel.setBounds(synthNameLabelArea);
    timbreLabel.setBounds(timbreLabelArea);
    presetList.setBounds(presetListArea);
    midiKeyboard.setBounds(keyboardArea);
}

void Interface::initializeComponents()
{
    loadPluginButton.onClick = [this] {loadPluginButtonClicked(); };
    addAndMakeVisible(loadPluginButton);

    openPluginEditorButton.onClick = [this] {openPluginEditorButtonClicked(); };
    addAndMakeVisible(openPluginEditorButton);

    getRandomPatchButton.onClick = [this] {getRandomPatchButtonClicked(); };
    addAndMakeVisible(getRandomPatchButton);

    renderAudioButton.onClick = [this] {renderAudioButtonClicked(); };
    addAndMakeVisible(renderAudioButton);

    searchButton.onClick = [this] {searchButtonClicked(); };
    addAndMakeVisible(searchButton);

    loadPresetButton.onClick = [this] {loadPresetButtonClicked(); };
    addAndMakeVisible(loadPresetButton);

    savePresetButton.onClick = [this] {savePresetButtonClicked(); };
    addAndMakeVisible(savePresetButton);

    addAndMakeVisible(tagInputBox);
    addAndMakeVisible(presetList);

    synthNameLabel.addListener(this);
    addAndMakeVisible(synthNameLabel);

    midiKeyboard.setName ("MIDI Keyboard");
    addAndMakeVisible(midiKeyboard);
    keyboardState.addListener(this);
}

void Interface::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (pluginWindow && (source == &pluginWindow->windowClosedBroadcaster))
        pluginWindow.deleteAndZero();
}

void Interface::labelTextChanged(juce::Label *labelThatHasChanged)
{
    // NOTE: not sure if this is necessary, but let's just keep it for now
}

void Interface::oscMessageReceived(const juce::OSCMessage &message)
{
    // handle the messages requesting to set parameters
    if (juce::OSCAddressPattern(OSC_RECEIVE_PATTERN + "set_parameter")
    .matches(juce::OSCAddress(message.getAddressPattern().toString())))
    {
        // the format is: (parameterIndex, newValue, parameterIndex, newValue, ...)
        int numParamsToSet = message.size() / 2;
        for (int i=0; i<numParamsToSet*2; i+=2)
            processorManager.setPluginParameter(message[i].getInt32(), message[i + 1].getFloat32());
    }
}

void Interface::showConnectionErrorMessage (const juce::String& messageText)
{
    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                            "Connection error",
                                            messageText,
                                            "OK");
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
        // If there is a opened plugin window, delete it.
        // Must delete the window before loading a new plugin
        // because the plugin editor is bounded to its plugin processor
        if (pluginWindow)
            pluginWindow.deleteAndZero();

        processorManager.loadPlugin(fileChooser.getResult().getFullPathName());
        synthNameLabel.setText("Synth: " + processorManager.getPluginDescription().name,
                               juce::NotificationType::sendNotification);
#ifdef IDEATOR_APP
        // TODO: arguments hard coded here, also it doesn't work when the block size is 256 and idk why
        processorManager.prepareToPlay(512, 44100.f);
#else
        processorManager.prepareToPlay();
#endif
    }
}

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

void Interface::getRandomPatchButtonClicked()
{
    // check if the plugin has been loaded
    if (!processorManager.checkPluginLoaded())
    {
        DBG("No plugin loaded!");
        return;
    }

    auto parameters = processorManager.getPluginParameters();
    int numParameters = parameters.size();
    DBG("The plugin has: " << numParameters << " parameters");

    // NOTE: Is it OK to do like this? what if the second command arrives before the first one?
    // inform the python program how many parameters it has to generate
    juce::OSCMessage msgNumParameters(OSC_SEND_PATTERN + "num_parameters", numParameters);
    oscSender.sendToIPAddress(LOCAL_ADDRESS, OSC_SEND_PORT, msgNumParameters);

    // inform the python program to generate a random patch and send it back
    juce::OSCMessage msgRandomize(OSC_SEND_PATTERN + "get_random_patch", 1);
    oscSender.sendToIPAddress(LOCAL_ADDRESS, OSC_SEND_PORT, msgRandomize);

    // NOTE: the following code is for sending osc messages
    // for (int i=0; i<numParameters; ++i)
    // {
    //     juce::OSCMessage msgParamter(OSC_SEND_PATTERN + "parameter/" + juce::String(i), i);
    //     oscSender.sendToIPAddress(LOCAL_ADDRESS, OSC_SEND_PORT, msgParamter);
    // }
}

void Interface::renderAudioButtonClicked()
{
    // check if the plugin has been loaded
    if (!processorManager.checkPluginLoaded())
    {
        DBG("No plugin loaded!");
        return;
    }

    // get the audio path
    juce::String audioPath;
    bool renderSucceeded = processorManager.renderAudio(audioPath);
    if (!renderSucceeded)
        return;

    // inform the python program to fetch the data
    juce::OSCMessage msgRandomize(OSC_SEND_PATTERN + "render_audio/" + audioPath, 1);
    oscSender.sendToIPAddress(LOCAL_ADDRESS, OSC_SEND_PORT, msgRandomize);
}

void Interface::searchButtonClicked()
{
    // TODO: send OSC to the python program to request the preset searching
}

void Interface::loadPresetButtonClicked()
{
    juce::FileChooser fileChooser("Select the path", {});
    if (fileChooser.browseForFileToOpen())
    {
        processorManager.loadPreset(fileChooser.getResult().getFullPathName());

        // The plugin path is written inside the preset and the program will
        // load the plugin according to it. Therefore, we should update the plugin name here.
        // It is the same as we do in loadPluginButtonClicked.
        synthNameLabel.setText("Synth: " + processorManager.getPluginDescription().name,
                               juce::NotificationType::sendNotification);
    }
}

void Interface::savePresetButtonClicked()
{
    juce::FileChooser fileChooser("Select the path", {});
    if (fileChooser.browseForFileToSave(true))
    {
        processorManager.savePreset(fileChooser.getResult().getFullPathName());
    }
}

std::vector<juce::String> Interface::getTags() const
{
    // TODO: finish this
    return {"xxx"};
}
