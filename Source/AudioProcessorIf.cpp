/*
  ==============================================================================

    AudioProcessorIf.cpp
    Created: 27 Oct 2020 9:29:23am
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "AudioProcessorIf.h"
#include "AppProcessor.h"

#ifdef IDEATOR_APP
AudioProcessorIf::AudioProcessorIf()
{
}
#else
AudioProcessorIf::AudioProcessorIf(PluginProcessor& p):
audioProcessor(p)
{
}
#endif

AudioProcessorIf::~AudioProcessorIf()
{
#ifdef IDEATOR_APP
#endif
}

void AudioProcessorIf::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
#ifdef IDEATOR_APP
    audioProcessor.prepareToPlay(samplesPerBlockExpected, sampleRate);
#endif
}

void AudioProcessorIf::releaseResources()
{
#ifdef IDEATOR_APP
    audioProcessor.releaseResources();
#endif
}

/////////////////////////////////////

bool AudioProcessorIf::loadPlugin(const juce::String& path)
{
#ifdef IDEATOR_APP
    return audioProcessor.loadPlugin(path);
#endif
}

bool AudioProcessorIf::checkPluginLoaded() const
{
#ifdef IDEATOR_APP
    return audioProcessor.checkPluginLoaded();
#endif

}

juce::AudioProcessorEditor* AudioProcessorIf::getPluginEditor()
{
#ifdef IDEATOR_APP
    return audioProcessor.getPluginEditor();
#endif
}

void AudioProcessorIf::addMidiEvent(const juce::MidiMessage &midiMessage)
{
#ifdef IDEATOR_APP
    audioProcessor.addMidiEvent(midiMessage);
#endif
}

juce::PluginDescription AudioProcessorIf::getPluginDescription() const
{
#ifdef IDEATOR_APP
    return audioProcessor.getPluginDescription();
#endif
}

const juce::Array<juce::AudioProcessorParameter*>& AudioProcessorIf::getPluginParameters() const
{
#ifdef IDEATOR_APP
    return audioProcessor.getPluginParameters();
#endif
}

void AudioProcessorIf::setPluginParameter(int parameterIndex, float newValue)
{
#ifdef IDEATOR_APP
    audioProcessor.setPluginParameter(parameterIndex, newValue);
#endif
}
