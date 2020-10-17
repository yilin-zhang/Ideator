/*
  ==============================================================================

    AudioComponent.cpp
    Created: 12 Oct 2020 8:19:53pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "AudioComponent.h"



//==============================================================================
AudioComponent::AudioComponent() :
sampleRate(44100.f),
bufferSize(256),
isPluginLoaded(false)
{

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
    && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
    [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

AudioComponent::~AudioComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

void AudioComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void AudioComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
}

void AudioComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    if (plugin)
        plugin->releaseResources();
}


// https://github.com/fedden/RenderMan/blob/master/Source/RenderEngine.cpp
bool AudioComponent::loadPlugin(const juce::String& path)
{
    juce::OwnedArray<juce::PluginDescription> pluginDescriptions;
    juce::KnownPluginList pluginList;
    juce::AudioPluginFormatManager pluginFormatManager;

    pluginFormatManager.addDefaultFormats();
    
    std::cout << "getNumFormats(): " << pluginFormatManager.getNumFormats() << std::endl;

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
                                                       sampleRate,
                                                       bufferSize,
                                                       errorMessage);
    if (plugin)
    {
        // Success so set up plugin, then set up features and get all available
        // parameters from this given plugin.
        plugin->prepareToPlay (sampleRate, bufferSize);
        //plugin->setNonRealtime (true);

        //mfcc.setup (512, 42, 13, 20, int (sampleRate / 2), sampleRate);

        // Resize the pluginParameters patch type to fit this plugin and init
        // all the values to 0.0f!
        //fillAvailablePluginParameters (pluginParameters);

        std::cout << "Successfully loaded!" << std::endl;

        isPluginLoaded = true;

        return true;
    }

    std::cout << "AudioComponent::loadPlugin error: "
              << errorMessage.toStdString()
              << std::endl;

    return false;
}

juce::AudioProcessorEditor* AudioComponent::getPluginEditor()
{
    juce::AudioProcessorEditor* editor = plugin->createEditor();
    return editor;
}

bool AudioComponent::checkPluginLoaded()
{
    return isPluginLoaded;
}

