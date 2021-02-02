/*
  ==============================================================================

    PluginManager.cpp
    Created: 27 Oct 2020 11:17:06am
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "PluginManager.h"
#include "Config.h"
#include <sstream>
#include <ctime>

PluginManager::PluginManager():
        plugin(nullptr),
        initialSampleRate(44100.f),
        initialBufferSize(256),
        internSampleRate(initialSampleRate),
        internSamplesPerBlock(initialBufferSize)
{
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
        presetPath = "";
        timbreDescriptors.clear();

        DBG("Loaded a plugin: " << pluginPath);

        return true;
    }

    DBG("PluginManager::loadPlugin error: " << errorMessage.toStdString());

    return false;
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

bool PluginManager::renderAudio(juce::String &audioPath)
{
    if (!plugin)
        return false;

    // initialize constants
    const double audioLength = 3.; // 3 seconds of audio
    const double noteLength = 2.;
    const int numSamples = static_cast<int>(internSampleRate) * static_cast<int>(audioLength);
    const int numNoteSamples = static_cast<int>(internSampleRate) * static_cast<int>(noteLength);
    const int blockSize = internSamplesPerBlock;
    const int numChannels = plugin->getTotalNumOutputChannels();
    const int midiNote = 60;
    const uint8_t midiVelocity = 127;

    // initialize audio buffers
    juce::AudioBuffer<float> bufferToProcess(numChannels, blockSize);
    juce::AudioBuffer<float> bufferToSave(numChannels, numSamples);

//    bufferToSave.clear();
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
            bufferToSave.copyFrom(
                    i,
                    currentSample,
                    bufferToProcess,
                    i,
                    0,
                    numSamplesToCopy);
    }

    // save the bufferToSave to wav
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
                                          bufferToSave.getNumChannels(),
                                          24,
                                          {},
                                          0));
    if (writer != nullptr)
        writer->writeFromAudioSampleBuffer (bufferToSave, 0, bufferToSave.getNumSamples());

    // set the plugin state back to realtime mode
    plugin->setNonRealtime(false);
    plugin->prepareToPlay(internSampleRate, internSamplesPerBlock);

    return true;
}

void PluginManager::loadPreset(const juce::String &presetPath)
{
    // We do not check if a plugin has been loaded here, because the function will
    // load the plugin if it is not loaded.

    juce::File inputFile(presetPath);
    auto xmlPreset = juce::XmlDocument::parse(inputFile);

    // check the plugin path first, just return if it is invalid
    auto xmlMeta = xmlPreset->getChildByName("Meta");
    auto xmlPluginPath = xmlMeta->getChildByName("PluginPath");
    auto newPluginPath = xmlPluginPath->getAllSubText();
    if (pluginPath != newPluginPath)
        loadPlugin(newPluginPath);

    // set plugin parameters
    auto xmlParameters = xmlPreset->getChildByName("Parameters");
    juce::MemoryBlock stateBlock;
    juce::AudioProcessor::copyXmlToBinary(*xmlParameters, stateBlock);
    plugin->setStateInformation(stateBlock.getData(), static_cast<int>(stateBlock.getSize()));

    // set meta data
    this->presetPath = presetPath;
    auto xmlDescriptors = xmlMeta->getChildByName("Descriptors");
    juce::StringArray descriptorArray;
    descriptorArray.addTokens(xmlDescriptors->getAllSubText(), ", ", "\"");
    timbreDescriptors.clear();
    for (auto &descriptor : descriptorArray)
    {
        if (descriptor.isEmpty())
            continue;
        timbreDescriptors.insert(descriptor);
    }

}

void PluginManager::savePreset(const juce::String &presetPath)
{
    if (!plugin)
        return;

    juce::MemoryBlock stateBlock;
    plugin->getStateInformation(stateBlock);
    auto xmlState = juce::AudioProcessor::getXmlFromBinary(stateBlock.getData(),
                                                           static_cast<int>(stateBlock.getSize()));
    if (!xmlState)
    {
        DBG("Data is unsuitable or corrupted.");
        return;
    }

    juce::XmlElement xmlPreset("IdeatorPreset");

    // A deep copy of xmlState should be created.
    // The new element will be automatically deleted by the parent, so there is no need to
    // explicitly delete it.
    auto xmlParameters = new juce::XmlElement(*xmlState);
    xmlParameters->setTagName("Parameters");

    // Meta
    //   |- PluginPath
    //   |- Descriptors
    auto xmlMeta = new juce::XmlElement("Meta");
    auto xmlPluginPath = new juce::XmlElement("PluginPath");
    xmlPluginPath->addTextElement(pluginPath);

    auto xmlDescriptors = new juce::XmlElement("Descriptors");
    juce::String descriptorString;
    bool isFirst = true;
    for (auto &descriptor : timbreDescriptors)
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

    juce::File outputFile(presetPath);
    xmlPreset.writeTo(outputFile);
}

void PluginManager::setTimbreDesctiptors(const std::unordered_set<juce::String> &timbreDescriptors)
{
    if (!plugin)
        return;

    this->timbreDescriptors = timbreDescriptors;
}

std::unordered_set<juce::String> PluginManager::getTimbreDesctiptors()
{
    return timbreDescriptors;
}

void PluginManager::setPresetPath(const juce::String &path)
{
    if (!plugin)
        return;

    presetPath = path;
}

juce::String PluginManager::getPresetPath()
{
    return presetPath;
}

// ==================================================
// AudioProcessorListener
// ==================================================

void PluginManager::audioProcessorParameterChanged (juce::AudioProcessor *processor, int parameterIndex, float newValue)
{
    // the empty path indicates that the patch has not been saved
    if (presetPath != "")
        presetPath = "";

    DBG("A parameter has been changed.");
}

void PluginManager::audioProcessorChanged (juce::AudioProcessor *processor)
{
    // empty
}
