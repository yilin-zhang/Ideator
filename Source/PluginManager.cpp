/*
  ==============================================================================

    PluginManager.cpp
    Created: 27 Oct 2020 11:17:06am
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "PluginManager.h"
#include "Config.h"
#include "Utils.h"
#include <sstream>
#include <ctime>

PluginManager::PluginManager():
        plugin(nullptr),
        initialSampleRate(44100.f),
        initialBufferSize(256),
        internSampleRate(initialSampleRate),
        internSamplesPerBlock(initialBufferSize)
{
    presetAudio.clear();
}

PluginManager::~PluginManager()= default;

/// Additional methods
// https://github.com/fedden/RenderMan/blob/master/Source/RenderEngine.cpp
bool PluginManager::loadPlugin(const juce::String& path)
{
    juce::OwnedArray<juce::PluginDescription> pluginDescriptions;
    juce::KnownPluginList pluginList;
    juce::AudioPluginFormatManager pluginFormatManager;

    // release resources before loading a new plugin
    if (plugin)
        plugin->releaseResources();

    pluginFormatManager.addDefaultFormats();

    for (int i = 0; i < pluginFormatManager.getNumFormats(); ++i)
    {
        pluginList.scanAndAddFile (path,
                                   true,
                                   pluginDescriptions,
                                   *pluginFormatManager.getFormat(i));
    }

    // If there is a problem here first check the preprocessor definitions
    // in the projucer are sensible - is it set up to scan for plugin's?
    jassert (pluginDescriptions.size() > 0);

    juce::String errorMessage;
    plugin = pluginFormatManager.createPluginInstance (*pluginDescriptions[0],
                                                       initialSampleRate,
                                                       initialBufferSize,
                                                       errorMessage);
    if (plugin)
    {
        // Success so set up plugin, then set up features and get all available
        // parameters from this given plugin.
        //plugin->prepareToPlay (initialSampleRate, initialBufferSize);
        //plugin->setNonRealtime (true);

        //mfcc.setup (512, 42, 13, 20, int (initialSampleRate / 2), initialSampleRate);

        // Resize the pluginParameters patch type to fit this plugin and init
        // all the values to 0.0f!
        //fillAvailablePluginParameters (pluginParameters);

        // notify the host once any parameter has changed
        plugin->addListener(this);

        pluginPath = path;
        resetWhenParameterChanged();

        DBG("Loaded a plugin: " << pluginPath);

        return true;
    }

    DBG("PluginManager::loadPlugin error: " << errorMessage.toStdString());

    return false;
}

const juce::String& PluginManager::getPresetPath() const
{
    return presetPath;
}

juce::AudioProcessorEditor* PluginManager::getPluginEditor()
{
    if (plugin->hasEditor())
    {
        juce::AudioProcessorEditor* editor = plugin->createEditor();
        return editor;
    }

    return nullptr;
}

bool PluginManager::checkPluginLoaded() const
{
    if (plugin)
        return true;
    return false;
}

void PluginManager::addMidiEvent(const juce::MidiMessage &midiMessage)
{
    // NOTE: I hard coded sampleNumber 0 here
    midiBuffer.addEvent(midiMessage, 0);
}

juce::PluginDescription PluginManager::getPluginDescription() const
{
    // return an empty description if the plugin has not been loaded
    if (!plugin)
        return juce::PluginDescription();

    return plugin->getPluginDescription();
}

const juce::Array<juce::AudioProcessorParameter*>& PluginManager::getPluginParameters() const
{
    return plugin->getParameters();
}

void PluginManager::setPluginParameter(int parameterIndex, float newValue)
{
    if (auto* param = plugin->getParameters()[parameterIndex])
        param->setValue (newValue);
}

void PluginManager::renderAudio()
{
    if (!plugin)
        return;

    // initialize constants
    const double audioLength = 3.; // 3 seconds of audio
    const double noteLength = 2.;
    const int numSamples = static_cast<int>(internSampleRate) * static_cast<int>(audioLength);
    const int numNoteSamples = static_cast<int>(internSampleRate) * static_cast<int>(noteLength);
    const int blockSize = internSamplesPerBlock;
    const int numChannels = plugin->getTotalNumOutputChannels(); // NOTE: The VST3 version of Helm returns 0 (don't know why)
    const int midiNote = 60;
    const uint8_t midiVelocity = 127;

    // initialize audio buffers
    juce::AudioBuffer<float> bufferToProcess(numChannels, blockSize);
    presetAudio.setSize(numChannels, numSamples);

    // create midi on and off messages
    // The timestamp indicates the number of audio samples from the start of the midi buffer.
    juce::MidiMessage onMessage;
    onMessage = juce::MidiMessage::noteOn(1,
                                          midiNote,
                                          midiVelocity);
    onMessage.setTimeStamp(0);
    juce::MidiMessage offMessage;
    offMessage = juce::MidiMessage::noteOff(1,
                                            midiNote,
                                            midiVelocity);

    // add on message to the buffer
    juce::MidiBuffer midiNoteBuffer;
    midiNoteBuffer.addEvent(onMessage, onMessage.getTimeStamp());

    // set plugin to non-realtime mode and process the block
    plugin->setNonRealtime(true);
    // must call prepareToPlay to enable the non-realtime setting
    plugin->prepareToPlay(internSampleRate, internSamplesPerBlock);

    // process
    bool hasNoteOff = false;
    for (int currentSample=0, numSamplesToCopy=0; currentSample < numSamples; currentSample+=blockSize)
    {
        if (currentSample >= numNoteSamples && !hasNoteOff)
        {
            offMessage.setTimeStamp(currentSample - numNoteSamples);
            midiNoteBuffer.addEvent(offMessage, offMessage.getTimeStamp());
            hasNoteOff = true;
        }

        bufferToProcess.clear();
        plugin->processBlock(bufferToProcess, midiNoteBuffer);

        numSamplesToCopy = numSamples - currentSample < blockSize ?
                           numSamples - currentSample : blockSize;

        for (int i=0; i<numChannels; ++i)
            presetAudio.copyFrom(i,
                                 currentSample,
                                 bufferToProcess,
                                 i,
                                 0,
                                 numSamplesToCopy);
    }
}

bool PluginManager::saveAudio(juce::String &audioPath)
{
    if (!plugin)
        return false;

    // A cleared presetAudio buffer means the patch has been changed, so we should update
    // the buffer, otherwise there is no need to update.
    if (presetAudio.hasBeenCleared())
        renderAudio();

    // save the presetAudio buffer to wav
    // get time stamp
    time_t now = time(nullptr);
    std::stringstream timeStringStream;
    timeStringStream << now;
    // form the audio path
    audioPath = TMP_AUDIO_DIR + timeStringStream.str() + ".wav";
    // save to file
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatWriter> writer;
    juce::File wavFile(audioPath);
    juce::File wavDir(TMP_AUDIO_DIR);
    // create the directory if it doesn't exist
    if (!wavDir.createDirectory())
        return false;

    writer.reset (format.createWriterFor (new juce::FileOutputStream (wavFile),
                                          internSampleRate,
                                          presetAudio.getNumChannels(),
                                          24,
                                          {},
                                          0));
    if (writer != nullptr)
        writer->writeFromAudioSampleBuffer (presetAudio, 0, presetAudio.getNumSamples());

    // set the plugin state back to realtime mode
    plugin->setNonRealtime(false);
    plugin->prepareToPlay(internSampleRate, internSamplesPerBlock);

    return true;
}

void PluginManager::sendAudio()
{
    if (!plugin)
        return;

    // A cleared presetAudio buffer means the patch has been changed, so we should update
    // the buffer, otherwise there is no need to update.
    if (presetAudio.hasBeenCleared())
        renderAudio();

    juce::DatagramSocket socket;

    UdpManager udpManager(LOCAL_ADDRESS, UDP_SEND_PORT);
    int writtenBytes = udpManager.sendBuffer(presetAudio.getReadPointer(0), presetAudio.getNumSamples());
    if (writtenBytes == -1)
    {
        DBG("PluginManager::sendAudio error.");
        return;
    }

    DBG("PluginManager::sendAudio: An audio buffer sent.");
}

void PluginManager::loadPreset(const juce::String &presetPath)
{
    // We do not check if a plugin has been loaded here, because the function will
    // load the plugin if it is not loaded.
    juce::File inputFile(presetPath);
    auto xmlPreset = juce::XmlDocument::parse(inputFile);
    if (!xmlPreset)
        return;

    // parse the preset
    juce::XmlElement xmlState("Parameters");
    juce::String newPluginPath;
    std::unordered_set<juce::String> descriptors;
    auto isSuccessful = PresetManager::parse(*xmlPreset, xmlState, newPluginPath, descriptors);
    if (!isSuccessful)
        return;

    // check the plugin path first, just return if it is invalid
    if (pluginPath != newPluginPath)
        loadPlugin(newPluginPath);

    // set plugin parameters
    juce::MemoryBlock stateBlock;
    juce::AudioProcessor::copyXmlToBinary(xmlState, stateBlock);
    plugin->setStateInformation(stateBlock.getData(), static_cast<int>(stateBlock.getSize()));

    // set meta data
    this->presetPath = presetPath;
    timbreDescriptors = descriptors;

}

void PluginManager::savePreset(const juce::String &presetPath)
{
    if (!plugin)
        return;

    // convert the binary state to XML
    juce::MemoryBlock stateBlock;
    plugin->getStateInformation(stateBlock);
    auto xmlState = juce::AudioProcessor::getXmlFromBinary(stateBlock.getData(),
                                                           static_cast<int>(stateBlock.getSize()));
    if (!xmlState)
    {
        DBG("Data is unsuitable or corrupted.");
        return;
    }

    // generate preset in XML format
    juce::XmlElement xmlPreset = PresetManager::generate(*xmlState, pluginPath, timbreDescriptors);

    juce::File outputFile(presetPath);
    outputFile.create();
    xmlPreset.writeTo(outputFile);
}

void PluginManager::setTimbreDescriptors(const std::unordered_set<juce::String> &timbreDescriptors)
{
    if (!plugin)
        return;

    this->timbreDescriptors = timbreDescriptors;
}

const std::unordered_set<juce::String>& PluginManager::getTimbreDescriptors() const
{
    return timbreDescriptors;
}

void PluginManager::setPresetPath(const juce::String &path)
{
    if (!plugin)
        return;

    presetPath = path;
}

const juce::String& PluginManager::getPluginPath() const
{
    return pluginPath;
}

// ==================================================
// AudioProcessorListener
// ==================================================

void PluginManager::resetWhenParameterChanged()
{
    // This function should also be called when a new plugin has been loaded

    // the empty path indicates that the patch has not been saved
    if (presetPath != "")
        presetPath = "";

    if (!timbreDescriptors.empty())
        timbreDescriptors.clear();

    // Any parameter change might lead to preset audio change, so we should clear it out
    if (!presetAudio.hasBeenCleared())
        presetAudio.clear();

}

void PluginManager::audioProcessorParameterChanged (juce::AudioProcessor *processor, int parameterIndex, float newValue)
{
    // This function will be called whenever a parameter is directly changed
    resetWhenParameterChanged();
    DBG("A parameter has been changed.");
}

void PluginManager::audioProcessorChanged (juce::AudioProcessor *processor)
{
    // This function will be called when the Diva patch has been set to a new preset.
    resetWhenParameterChanged();
    DBG("The patch has been changed.");
}
