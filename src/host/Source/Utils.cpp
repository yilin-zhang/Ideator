/*
  ==============================================================================

    Utils.cpp
    Created: 10 Feb 2021 2:45:58pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "Utils.h"
#include "PluginManager.h"

// ========================================
// PresetManager
// ========================================

juce::XmlElement PresetManager::generate(const juce::Array<juce::AudioProcessorParameter*> &parameters,
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
    auto xmlParameters = new juce::XmlElement("Parameters");
    for (int i=0; i<parameters.size(); ++i)
        // Each attribute begins with "P_" followed by its index, for example: "P_10"
        xmlParameters->setAttribute("P_" + juce::String(parameters[i]->getParameterIndex()), parameters[i]->getValue());

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

bool PresetManager::parse(const juce::XmlElement &preset,
                          juce::Array<std::pair<int, float>> &parameters,
                          juce::String &pluginPath,
                          std::unordered_set<juce::String> &descriptors)
{
    // parse the plugin path
    auto xmlMeta = preset.getChildByName("Meta");
    auto xmlPluginPath = xmlMeta->getChildByName("PluginPath");
    pluginPath = xmlPluginPath->getAllSubText();

    // parse plugin parameters
    auto xmlParameters = *preset.getChildByName("Parameters");
    for (int i=0; i<xmlParameters.getNumAttributes(); ++i)
        // add the prefix "P_"
        parameters.add({i, xmlParameters.getDoubleAttribute("P_" + juce::String(i))});

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
// ========================================
// UdpManager
// ========================================

UdpManager::UdpManager(juce::String address, int port):
        address(std::move(address)), port(port),
        bufferSize((UDP_MESSAGE_SIZE - INT_SIZE*3 - BOOL_SIZE) / FLOAT_SIZE)
{
    resetMessageStruct();
}

int UdpManager::sendBuffer(const float* bufferArray, int size)
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

void UdpManager::resetMessageStruct()
{
    udpMessage.id = 0;
    udpMessage.index = 0;
    udpMessage.isLast = false;
    udpMessage.numSamples = 0;
    memset(udpMessage.buffer, 0.f, bufferSize);
}

// ========================================
// OSC Manager
// ========================================

OSCManager::OSCManager(PluginManager &pm):pluginManger(pm), presetCounter(0)
{
    oscSender.connect(LOCAL_ADDRESS, OSC_SEND_PORT);

    // https://docs.juce.com/master/tutorial_osc_sender_receiver.html
    // specify here on which UDP port number to receive incoming OSC messages
    if (! connect (OSC_RECEIVE_PORT))
        showConnectionErrorMessage ("Error: could not connect to UDP port " +
                                    juce::String(OSC_RECEIVE_PORT) + ".");

    addListener(this, OSC_RECEIVE_PATTERN + "analyze_library");

    // for parsing JSON dataset
    addListener(this, OSC_RECEIVE_PATTERN + "json_patch");
    addListener(this, OSC_RECEIVE_PATTERN + "json_path");
    addListener(this, OSC_RECEIVE_PATTERN + "json_character");
}

void OSCManager::prepareToAnalyzeAudio(const juce::String& presetPath,
                                       const std::unordered_set<juce::String>& descriptors)
{
    // form a string of descriptors, use "," as the separator
    juce::String descriptorString;
    int idx = 0;
    for (const auto& descriptor : descriptors)
    {
        descriptorString.append(descriptor, descriptor.length());
        if (idx != descriptors.size() - 1)
            descriptorString.append(",", 1);
        ++idx;
    }

    juce::OSCMessage msgAnalyzeLibrary(OSC_SEND_PATTERN + "analyze_library", 1, presetPath, descriptorString);
    oscSender.send(msgAnalyzeLibrary);
}

void OSCManager::finishAnalyzeAudio()
{
    juce::OSCMessage msgAnalyzeLibrary(OSC_SEND_PATTERN + "analyze_library", 2, juce::String(""), juce::String(""));
    oscSender.send(msgAnalyzeLibrary);
}

// other class should NOT call any method of the broadcaster other than addListener
void OSCManager::showConnectionErrorMessage (const juce::String& messageText)
{
    juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                            "Connection error",
                                            messageText,
                                            "OK");
}

void OSCManager::oscMessageReceived (const juce::OSCMessage& message)
{
    // The Python program send to address `analyze_library` with number 1 that
    // indicates the audio feature has been added
    if (juce::OSCAddressPattern(OSC_RECEIVE_PATTERN + "analyze_library")
            .matches(juce::OSCAddress(message.getAddressPattern().toString())))
    {
        // inform other parts that the last audio buffer has been processed
        analysisFinishedBroadcaster.sendChangeMessage();
    }
    // NOTE: This chunk of code is for JSON-to-XML conversion (users should not use it)
    // When it receives a preset from the Python program, save it.
    else if (juce::OSCAddressPattern(OSC_RECEIVE_PATTERN + "json_patch")
            .matches(juce::OSCAddress(message.getAddressPattern().toString())))
    {
        // When the host receives this message, it clears out the states for the plugin
        // and start setting new states for this new preset.
        // All the parameters will be set in the following steps, so there is no need to
        // clear out any state variable.

        juce::String presetPath(message[0].getString());
        juce::String concatTimbreDescriptors(message[1].getString());

        // Must set parameter before setting meta data, because every parameter change
        // will reset the meta data automatically
        int numParamsToSet = (message.size()-2) / 2;
        for (int i=2; i<numParamsToSet*2+2; i+=2)
            pluginManger.setPluginParameter(message[i].getInt32(), message[i + 1].getFloat32());

        juce::StringArray descriptorArray;
        std::unordered_set<juce::String> descriptors;
        descriptorArray.addTokens(concatTimbreDescriptors, ", &", "\"");
        descriptorArray.removeEmptyStrings(true);
        descriptors.clear();
        for (auto &descriptor : descriptorArray)
            descriptors.insert(descriptor);

        pluginManger.setTimbreDescriptors(descriptors);

        // when it comes to this stage, all the states of the preset have been set,
        // so the next step is to save the preset
        const juce::String pathPrefix = "/Users/naotake/Datasets/diva/";
        // remove prefix and extension
        juce::String relativePath = presetPath.substring(pathPrefix.length(), presetPath.length()-4);
        juce::String absolutePath = "/Users/yilin/Desktop/presets/" + relativePath + ".xml";
        std::cout << absolutePath << std::endl;

        ++presetCounter;
        // NOTE: this path is temporary and fixed
        pluginManger.savePreset(absolutePath);
    }
}
