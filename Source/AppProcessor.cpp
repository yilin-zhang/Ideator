/*
  ==============================================================================

    AppProcessor.cpp
    Created: 12 Oct 2020 8:19:53pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "AppProcessor.h"



//==============================================================================
AppProcessor::AppProcessor()
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
        //setAudioChannels (2, 2);
        setAudioChannels (0, 2);
        std::cout << "Channels have been set" << std::endl;
    }
}

AppProcessor::~AppProcessor()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

void AppProcessor::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    if (plugin)
    {
        plugin->prepareToPlay(sampleRate, samplesPerBlockExpected);
        std::cout << "plugin sample rate: " << plugin->getSampleRate() << std::endl;
        std::cout << "plugin num input channels: " << plugin->getTotalNumInputChannels() << std::endl;
        std::cout << "plugin num output channels: " << plugin->getTotalNumOutputChannels() << std::endl;
    }
    internSamplesPerBlock = samplesPerBlockExpected;
    internSampleRate = sampleRate;
}

void AppProcessor::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)


    if (plugin)
    {
        plugin->processBlock(*(bufferToFill.buffer), midiBuffer);
        //auto rp = bufferToFill.buffer->getReadPointer(1);
        //std::cout << rp[0] << std::endl;
    }
    else
    {
        bufferToFill.clearActiveBufferRegion();
    }
//    bufferToFill.clearActiveBufferRegion();

}

void AppProcessor::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    if (plugin)
        plugin->releaseResources();
}
