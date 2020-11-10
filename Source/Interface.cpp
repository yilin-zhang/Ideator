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
    //setSize (800, 600);
    loadPluginButton.onClick = [this] {loadPluginButtonClicked(); };
    addAndMakeVisible(loadPluginButton);

    openPluginEditorButton.onClick = [this] {openPluginEditorButtonClicked(); };
    addAndMakeVisible(openPluginEditorButton);

    getRandomPatchButton.onClick = [this] {getRandomPatchButtonClicked(); };
    addAndMakeVisible(getRandomPatchButton);

    renderAudioButton.onClick = [this] {renderAudioButtonClicked(); };
    addAndMakeVisible(renderAudioButton);

    midiKeyboard.setName ("MIDI Keyboard");
    addAndMakeVisible(midiKeyboard);
    keyboardState.addListener(this);

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
        delete pluginWindow;

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
    juce::Rectangle<int> buttonSize (0, 0, 108, 28);
    juce::Rectangle<int> area1 ((getWidth()  / 2) - (buttonSize.getWidth() / 2),
                         (getHeight() / 2) -  buttonSize.getHeight() - 30,
                         buttonSize.getWidth(), buttonSize.getHeight());

    juce::Rectangle<int> area2 ((getWidth()  / 2) - (buttonSize.getWidth() / 2),
                                (getHeight() / 2) -  buttonSize.getHeight(),
                                buttonSize.getWidth(), buttonSize.getHeight());

    juce::Rectangle<int> area3 ((getWidth()  / 2) - (buttonSize.getWidth() / 2),
                                (getHeight() / 2) -  buttonSize.getHeight() + 30,
                                buttonSize.getWidth(), buttonSize.getHeight());

    juce::Rectangle<int> area4 ((getWidth()  / 2) - (buttonSize.getWidth() / 2),
                                (getHeight() / 2) -  buttonSize.getHeight() + 60,
                                buttonSize.getWidth(), buttonSize.getHeight());

    loadPluginButton.setBounds(area1);
    openPluginEditorButton.setBounds(area2);
    getRandomPatchButton.setBounds(area3);
    renderAudioButton.setBounds(area4);

    auto margin = 10;

    midiKeyboard.setBounds (margin, (getHeight() / 2) + (50 + margin), getWidth() - (2 * margin), 64);

}

void Interface::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (pluginWindow && (source == &pluginWindow->windowClosedBroadcaster))
    {
        delete pluginWindow;
    }
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
    if (fileChooser.browseForFileToOpen())
    {
        auto filePath = fileChooser.getResult().getFullPathName();
        processorManager.loadPlugin(filePath);
#ifdef IDEATOR_APP
        // TODO: arguments hard coded here, also it doesn't work when the block size is 256 and idk why
        processorManager.prepareToPlay(512, 44100.f);
#else
        processorManager.prepareToPlay();
#endif
        // if there is a opened plugin window, reset the editor
        if (pluginWindow)
        {
            auto editor = processorManager.getPluginEditor();
            pluginWindow->setEditor(*editor);
        }
    }
}

void Interface::openPluginEditorButtonClicked()
{
    if (processorManager.checkPluginLoaded())
    {
        auto editor = processorManager.getPluginEditor();

        if (!editor)
        {
            std::cout << "The plugin does not have an editor" << std::endl;
            return;
        }

        // don't do anything if the window has already opened
        if (pluginWindow)
            return;

        // create a new plugin window and initialize it
        pluginWindow = new PluginWindow("Plugin", juce::Colours::black, 7, true);
        pluginWindow->windowClosedBroadcaster.addChangeListener(this);
        pluginWindow->setEditor(*editor);
        pluginWindow->setVisible(true);
    }
    else
    {
        std::cout << "No plugin loaded!" << std::endl;
    }
}

void Interface::getRandomPatchButtonClicked()
{
    // check if the plugin has been loaded
    if (!processorManager.checkPluginLoaded())
    {
        std::cout << "No plugin loaded!" << std::endl;
        return;
    }

    auto parameters = processorManager.getPluginParameters();
    int numParameters = parameters.size();
    std::cout << "The plugin has: " << numParameters << " parameters" << std::endl;

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
        std::cout << "No plugin loaded!" << std::endl;
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
