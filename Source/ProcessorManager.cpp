/*
  ==============================================================================

    ProcessorManager.cpp
    Created: 27 Oct 2020 9:29:23am
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "ProcessorManager.h"

#ifdef IDEATOR_APP
ProcessorManager::ProcessorManager()
{
}
#else
ProcessorManager::ProcessorManager(PluginProcessor& p):
audioProcessor(p)
{
}
#endif

ProcessorManager::~ProcessorManager()
{
#ifdef IDEATOR_APP
#endif
}

void ProcessorManager::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
#ifdef IDEATOR_APP
    audioProcessor.prepareToPlay(samplesPerBlockExpected, sampleRate);
#else
    // audioProcessor.prepareToPlay(sampleRate, samplesPerBlockExpected);
#endif
}

void ProcessorManager::releaseResources()
{
#ifdef IDEATOR_APP
    audioProcessor.releaseResources();
#endif
}

/////////////////////////////////////

bool ProcessorManager::loadPlugin(const juce::String& path)
{
    return audioProcessor.loadPlugin(path);
}

bool ProcessorManager::checkPluginLoaded() const
{
    return audioProcessor.checkPluginLoaded();
}

juce::AudioProcessorEditor* ProcessorManager::getPluginEditor()
{
    return audioProcessor.getPluginEditor();
}

void ProcessorManager::addMidiEvent(const juce::MidiMessage &midiMessage)
{
    audioProcessor.addMidiEvent(midiMessage);
}

juce::PluginDescription ProcessorManager::getPluginDescription() const
{
    return audioProcessor.getPluginDescription();
}

const juce::Array<juce::AudioProcessorParameter*>& ProcessorManager::getPluginParameters() const
{
    return audioProcessor.getPluginParameters();
}

void ProcessorManager::setPluginParameter(int parameterIndex, float newValue)
{
    audioProcessor.setPluginParameter(parameterIndex, newValue);
}
