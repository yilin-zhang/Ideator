/*
  ==============================================================================

    Utils.h
    Created: 3 Feb 2021 10:31:10am
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <unordered_set>
#include <utility>
#include "Config.h"

// ========================================
// PresetManager
// ========================================

class PresetManager
{
public:
    static juce::XmlElement generate(const juce::XmlElement &xmlState,
                                     const juce::String &pluginPath,
                                     const std::unordered_set<juce::String> &descriptors)
    {
        // IdeatorPlugin
        //  |- Meta
        //  |   |- PluginPath
        //  |   |- Descriptors
        //  |- Parameters

        juce::XmlElement xmlPreset("IdeatorPreset");

        // A deep copy of xmlState should be created.
        // The new element will be automatically deleted by the parent, so there is no need to
        // explicitly delete it.
        auto xmlParameters = new juce::XmlElement(xmlState);
        xmlParameters->setTagName("Parameters");

        auto xmlMeta = new juce::XmlElement("Meta");
        auto xmlPluginPath = new juce::XmlElement("PluginPath");
        xmlPluginPath->addTextElement(pluginPath);

        auto xmlDescriptors = new juce::XmlElement("Descriptors");
        juce::String descriptorString;
        bool isFirst = true;
        for (auto &descriptor : descriptors)
        {
            if (!isFirst)
                descriptorString.append(", ", 2);
            else
                isFirst = false;
            descriptorString.append(descriptor, descriptor.length());
        }
        xmlDescriptors->addTextElement(descriptorString);

        // construct the XML structure
        xmlMeta->addChildElement(xmlPluginPath);
        xmlMeta->addChildElement(xmlDescriptors);
        xmlPreset.addChildElement(xmlMeta);
        xmlPreset.addChildElement(xmlParameters);
        return xmlPreset;
    }

    static bool parse(const juce::XmlElement &preset,
                      juce::XmlElement &xmlState,
                      juce::String &pluginPath,
                      std::unordered_set<juce::String> &descriptors)
    {
        // parse the plugin path
        auto xmlMeta = preset.getChildByName("Meta");
        auto xmlPluginPath = xmlMeta->getChildByName("PluginPath");
        pluginPath = xmlPluginPath->getAllSubText();

        // parse plugin parameters
        xmlState = *preset.getChildByName("Parameters");

        // set meta data
        auto xmlDescriptors = xmlMeta->getChildByName("Descriptors");
        juce::StringArray descriptorArray;
        descriptorArray.addTokens(xmlDescriptors->getAllSubText(), ", &", "\"");
        descriptorArray.removeEmptyStrings(true);
        descriptors.clear();
        for (auto &descriptor : descriptorArray)
            descriptors.insert(descriptor);

        // true means parse successfully
        return true;
    }
};

// ========================================
// UdpManager
// ========================================

const int FLOAT_SIZE = sizeof (float);
const int INT_SIZE = sizeof (int);
const int BOOL_SIZE = sizeof (bool);

class UdpManager
{
public:
    UdpManager(juce::String address, int port):
    address(std::move(address)), port(port),
    bufferSize((UDP_MESSAGE_SIZE - INT_SIZE*3 - BOOL_SIZE) / FLOAT_SIZE)
    {
        resetMessageStruct();
    }

    int sendBuffer(const float* bufferArray, int size)
    {
        // calculate how many message it should send and the size of the last buffer
        int numMessages = size / bufferSize;
        int lastNumSamples;
        if (size % bufferSize != 0)
        {
            ++numMessages;
            lastNumSamples = size % bufferSize;
        }
        else
            lastNumSamples = bufferSize;

        // set id
        udpMessage.id = rand.nextInt();
        int byteCounter = 0;

        // send messages
        for (int i=0; i<numMessages; ++i)
        {
            // check if it's the last message
            if (i == numMessages - 1)
            {
                udpMessage.isLast = true;
                udpMessage.numSamples = lastNumSamples;
            }
            else
                udpMessage.numSamples = bufferSize;

            udpMessage.index = i;
            memcpy(udpMessage.buffer, bufferArray + i*bufferSize, sizeof(float) * udpMessage.numSamples);

            int writtenBytes = socket.write(address, port, &udpMessage, sizeof(UdpMessage));
            if (writtenBytes == -1)
                return -1;
            else
                byteCounter += writtenBytes;
        }

        // clean up
        resetMessageStruct();

        return byteCounter;
    }

private:
    const juce::String address;
    const int port;
    const int bufferSize;
    juce::Random rand;

    struct UdpMessage
    {
        int id;
        int index;
        bool isLast;
        int numSamples;
        float buffer[(UDP_MESSAGE_SIZE - INT_SIZE*3 - BOOL_SIZE) / FLOAT_SIZE];
    } udpMessage;

    void resetMessageStruct()
    {
        udpMessage.id = 0;
        udpMessage.index = 0;
        udpMessage.isLast = false;
        udpMessage.numSamples = 0;
        memset(udpMessage.buffer, 0.f, bufferSize);
    }

    juce::DatagramSocket socket;
};

// ========================================
// OSC Manager
// ========================================

class OSCManager: private juce::OSCReceiver,
                  private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
public:
    OSCManager()
    {
        oscSender.connect(LOCAL_ADDRESS, OSC_SEND_PORT);

        // https://docs.juce.com/master/tutorial_osc_sender_receiver.html
        // specify here on which UDP port number to receive incoming OSC messages
        if (! connect (OSC_RECEIVE_PORT))
            showConnectionErrorMessage ("Error: could not connect to UDP port " +
                                        juce::String(OSC_RECEIVE_PORT) + ".");

        addListener(this, OSC_RECEIVE_PATTERN + "analyze_library");
    }

    void prepareToAnalyzeAudio(const juce::String& presetPath)
    {
        juce::OSCMessage msgAnalyzeLibrary(OSC_SEND_PATTERN + "analyze_library", 1, presetPath);
        oscSender.send(msgAnalyzeLibrary);
    }

    void finishAnalyzeAudio()
    {
        juce::OSCMessage msgAnalyzeLibrary(OSC_SEND_PATTERN + "analyze_library", 2, juce::String(""));
        oscSender.send(msgAnalyzeLibrary);
    }

    // other class should NOT call any method of the broadcaster other than addListener
    juce::ChangeBroadcaster analysisFinishedBroadcaster;

private:
    juce::OSCSender oscSender;

    void showConnectionErrorMessage (const juce::String& messageText)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                                "Connection error",
                                                messageText,
                                                "OK");
    }

    void oscMessageReceived (const juce::OSCMessage& message) override
    {
        // The Python program send to address `analyze_library` with number 1 that
        // indicates the audio feature has been added
        if (juce::OSCAddressPattern(OSC_RECEIVE_PATTERN + "analyze_library")
                .matches(juce::OSCAddress(message.getAddressPattern().toString())))
        {
            // inform other parts that the last audio buffer has been processed
            analysisFinishedBroadcaster.sendChangeMessage();
        }
    }


};