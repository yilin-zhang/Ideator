/*
  ==============================================================================

    PluginManager.h
    Created: 27 Oct 2020 11:17:06am
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginManagerIf.h"
#include "Utils.h"

class PluginManager : public PluginManagerIf,
                      private juce::AudioProcessorListener,
                      private juce::ChangeListener
{
public:
    PluginManager();
    ~PluginManager() override;
    // methods for the plugin
    void setOSCManager(OSCManager* oscManager) override;
    bool loadPlugin(const juce::String& path) override;
    const juce::String& getPluginPath() const override;
    bool checkPluginLoaded() const override;
    juce::AudioProcessorEditor* getPluginEditor() override;
    void addMidiEvent(const juce::MidiMessage &midiMessage) override;
    juce::PluginDescription getPluginDescription() const override;
    const juce::Array<juce::AudioProcessorParameter*>& getPluginParameters() const override;
    void setPluginParameter(int parameterIndex, float newValue) override;
    void renderAudio() override;
    bool saveAudio(const juce::String &audioPath) override;
    void sendAudio() override;
    bool loadPreset(const juce::String &presetPath) override;
    bool savePreset(const juce::String &presetPath) override;
    bool autoTag() override;
    bool changeDescriptors(const juce::String &presetPath,
                           const std::unordered_set<juce::String> &newDescriptors) override;
    void setTimbreDescriptors(const std::unordered_set<juce::String> &timbreDescriptors) override;
    const std::unordered_set<juce::String>& getTimbreDescriptors() const override;
    void setPresetPath(const juce::String &path) override;
    const juce::String& getPresetPath() const override;
    bool analyzeLibrary(const juce::Array<juce::String>& presetPaths) override;
    void findSimilar() override;

protected:
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    juce::MidiBuffer midiBuffer;

    // NOTE: the values of these two variables are hard-coded in the constructor
    const double initialSampleRate;
    const int initialBufferSize;

    double internSampleRate;
    int internSamplesPerBlock;

    juce::AudioBuffer<float> presetAudio; // audio buffer for saving the rendered audio

    // extra states
    std::unordered_set<juce::String> timbreDescriptors;
    juce::String presetPath; // empty string means the preset has not been saved
    juce::String pluginPath;

private:
    void resetWhenParameterChanged();
    void audioProcessorParameterChanged (juce::AudioProcessor *processor, int parameterIndex, float newValue) override;
    void audioProcessorChanged (juce::AudioProcessor *processor, const ChangeDetails& details) override;

    void changeListenerCallback (juce::ChangeBroadcaster *source) override;
    bool analyzeNextPresetInLibrary();

    juce::Array<juce::String> presetPathsInLibrary;
    int numPresetAnalyzed;

    OSCManager* oscManager;
};