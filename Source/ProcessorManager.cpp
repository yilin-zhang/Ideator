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

ProcessorManager::~ProcessorManager()= default;

#ifdef IDEATOR_APP
void ProcessorManager::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audioProcessor.prepareToPlay(samplesPerBlockExpected, sampleRate);
}
#else
void ProcessorManager::prepareToPlay()
{
    audioProcessor.prepareToPlayForPlugin();
}
#endif

void ProcessorManager::releaseResources()
{
    audioProcessor.releaseResources();
}

/////////////////////////////////////

bool ProcessorManager::loadPlugin(const juce::String& path)
{
    return audioProcessor.loadPlugin(path);
}

const juce::String& ProcessorManager::getPresetPath() const
{
    return audioProcessor.getPresetPath();
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

void ProcessorManager::renderAudio()
{
    audioProcessor.renderAudio();
}

bool ProcessorManager::saveAudio(const juce::String &audioPath)
{
    return audioProcessor.saveAudio(audioPath);
}

void ProcessorManager::sendAudio()
{
    audioProcessor.sendAudio();
}

bool ProcessorManager::loadPreset(const juce::String &presetPath)
{
    return audioProcessor.loadPreset(presetPath);
}

bool ProcessorManager::savePreset(const juce::String &presetPath)
{
    return audioProcessor.savePreset(presetPath);
}

void ProcessorManager::setTimbreDescriptors(const std::unordered_set<juce::String> &timbreDescriptors)
{
    audioProcessor.setTimbreDescriptors(timbreDescriptors);
}

const std::unordered_set<juce::String>& ProcessorManager::getTimbreDescriptors() const
{
    return audioProcessor.getTimbreDescriptors();
}

void ProcessorManager::setPresetPath(const juce::String &path)
{
    audioProcessor.setPresetPath(path);
}

const juce::String& ProcessorManager::getPluginPath() const
{
    return audioProcessor.getPluginPath();
}

bool ProcessorManager::analyzeLibrary(const juce::Array<juce::String>& presetPaths)
{
    return audioProcessor.analyzeLibrary(presetPaths);
}
