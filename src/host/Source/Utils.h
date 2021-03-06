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
#include <stack>

// ========================================
// PresetManager
// ========================================

class PresetManager
{
public:
    static juce::XmlElement generate(const juce::Array<juce::AudioProcessorParameter*> &parameters,
                                     const juce::String &pluginPath,
                                     const std::unordered_set<juce::String> &descriptors);

    static bool parse(const juce::XmlElement &preset,
                      juce::Array<std::pair<int, float>> &parameters,
                      juce::String &pluginPath,
                      std::unordered_set<juce::String> &descriptors);

    static juce::String descriptorsToString(const std::unordered_set<juce::String>& descriptors);

    static std::unordered_set<juce::String> stringToDescriptors(const juce::String& descriptorString);
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
    UdpManager(juce::String address, int port);
    int sendBuffer(const float* bufferArray, int size);

private:
    const juce::String address;
    const int port;
    const int bufferSize;
    juce::Random rand;
    juce::DatagramSocket socket;

    struct UdpMessage
    {
        int id;
        int index;
        bool isLast;
        int numSamples;
        float buffer[(UDP_MESSAGE_SIZE - INT_SIZE*3 - BOOL_SIZE) / FLOAT_SIZE];
    } udpMessage;

    void resetMessageStruct();
};

// ========================================
// OSC Manager
// ========================================
class PluginManager;

class OSCManager: private juce::OSCReceiver,
                  private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
public:
    OSCManager();
    void setPluginManager(PluginManager *pm);

    // The following methods should only be called in the interface
    void sendRequestForPresetRetrieval(const juce::String &tags);

    // The following methods should only be called in PluginManager
    void prepareToAnalyzeAudio(const juce::String& presetPath,
                               const std::unordered_set<juce::String>& descriptors);
    void finishAnalyzeAudio();
    void prepareToFindSimilar();
    void prepareToAutoTag();
    void changeDescriptors(const juce::String& presetPath,
                           const std::unordered_set<juce::String>& descriptors);

    // the following methods should only be called in Interface
    const juce::StringArray& getSelectedPresetPaths();
    const juce::StringArray& getAutoTags();

    // other class should NOT call any method of the broadcasters other than addListener
    juce::ChangeBroadcaster analysisFinishedBroadcaster;
    juce::ChangeBroadcaster selectedPresetsReadyBroadcaster;
    juce::ChangeBroadcaster autoTagsReadyBroadcaster;
private:
    PluginManager *pluginManager;
    int presetCounter;
    juce::OSCSender oscSender;
    juce::StringArray selectedPresetPaths;
    juce::StringArray autoTags;

    static void showConnectionErrorMessage (const juce::String& messageText);
    void oscMessageReceived (const juce::OSCMessage& message) override;
};

// ========================================
// UndoStack
// ========================================
template<class T>
class UndoStack
{
public:
    UndoStack() = default;
    ~UndoStack() = default;
    void push(const T& newElement)
    {
        backwardStack.push(newElement);

        // clear the forward stack
        while(!forwardStack.empty())
            forwardStack.pop();
    }

    T undo()
    {
        T lastList = backwardStack.top();
        backwardStack.pop();
        forwardStack.push(lastList);
        return backwardStack.top();
    }

    T redo()
    {
        auto previousList = forwardStack.top();
        forwardStack.pop();
        backwardStack.push(previousList);
        return previousList;
    }

    bool isUndoAvailable()
    {
        return backwardStack.size() > 1;
    }

    bool isRedoAvailable()
    {
        return !forwardStack.empty();
    }

private:
    std::stack<T> backwardStack;
    std::stack<T> forwardStack;
};

