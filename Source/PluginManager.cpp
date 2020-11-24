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

    //if (plugin != nullptr) delete plugin;

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

        std::cout << "Plugin loaded!" << std::endl;

        return true;
    }

    std::cout << "PluginManager::loadPlugin error: "
              << errorMessage.toStdString()
              << std::endl;

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
    //midiBuffer.addEvent(midiMessage, midiMessage.getTimeStamp());
    midiBuffer.addEvent(midiMessage, 0);
    //std::cout << "num events: " << midiBuffer.getNumEvents() << std::endl;
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