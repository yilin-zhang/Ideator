/*
  ==============================================================================

    PluginManagerIf.h
    Created: 27 Oct 2020 11:58:10am
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <unordered_set>
#include "Utils.h"

class PluginManagerIf
{
public:
    virtual ~PluginManagerIf()= default;

    /*!
     * Loads a plugin by passing the absolute path to it.
     * @param path the absolute path to the plugin
     * @return true if the load has been successfully loaded
     */
    virtual bool loadPlugin(const juce::String &path) = 0;

    /*!
     * Returns the path of the plugin that is loaded.
     * @return the absolute path to the plugin
     */
    virtual const juce::String& getPluginPath() const = 0;

    /*!
     * Checks if a plugin is loaded into the host
     * @return true if a plugin is loaded
     */
    virtual bool checkPluginLoaded() const = 0;

    /*!
     * Returns a pointer to the plugin editor of the plugin that is currently loaded.
     * @return the pointer to the plugin editor
     */
    virtual juce::AudioProcessorEditor *getPluginEditor() = 0;

    /*!
     * Passes a MIDI message to the plugin.
     * @param midiMessage the MIDI message that needs to be passed in
     */
    virtual void addMidiEvent(const juce::MidiMessage &midiMessage) = 0;

    /*!
     * Returns the plugin's description (empty if there is no plugin loaded
     * @return the plugin's description
     */
    virtual juce::PluginDescription getPluginDescription() const = 0;

    /*!
     * Returns a const array of pointers to the plugin parameters.
     * @return the const array of pointers to the plugin parameters
     */
    virtual const juce::Array<juce::AudioProcessorParameter *> &getPluginParameters() const = 0;

    /*!
     * Sets the plugin parameter.
     * @param parameterIndex parameter index
     * @param newValue new value that needs to be set
     */
    virtual void setPluginParameter(int parameterIndex, float newValue) = 0;

    /*!
     * Renders the audio by using the current synth setting and save it into the buffer.
     */
    virtual void renderAudio() = 0;

    /*!
     * Saves the current preset sound as an audio file.
     * @param audioPath the path to the audio file
     * @return true if the audio is successfully saved
     */
    virtual bool saveAudio(const juce::String &audioPath) = 0;

    /*!
     * Sends the rendered audio buffer to the Python back-end
     */
    virtual void sendAudio() = 0;

    /*!
     * Loads the preset from the given path.
     * @param presetPath the path to the preset
     * @return true if the preset is available and it is set successfully
     */
    virtual bool loadPreset(const juce::String &presetPath) = 0;

    /*!
     * Saves the current parameter settings to a preset file
     * @param presetPath the path to the preset file
     * @return
     */
    virtual bool savePreset(const juce::String &presetPath) = 0;

    /*!
     * Sets the timbre descriptors.
     * @param timbreDescriptors an unordered_set of descriptors
     */
    virtual void setTimbreDescriptors(const std::unordered_set<juce::String> &timbreDescriptors) = 0;

    /*!
     * Returns the timbre descriptors of the current preset.
     * @return an unordered_set of descriptors
     */
    virtual const std::unordered_set<juce::String>& getTimbreDescriptors() const = 0;

    /*!
     * Sets the path to the current preset.
     * @param path the absolute path of the current preset
     */
    virtual void setPresetPath(const juce::String &path) = 0;

    /*!
     * Returns the path of the current preset.
     * @return the path of the current preset
     */
    virtual const juce::String& getPresetPath() const = 0;

    /*!
     * Starts analyzing the current preset library
     * @param presetPaths all the preset paths in the current library
     * @return true if the analysis is successful
     */
    virtual bool analyzeLibrary(const juce::Array<juce::String>& presetPaths) = 0;

    /*!
     * Sets an OSCManage object that will be used by the PluginManager.
     * @param oscManager the pointer to an OSCManager object
     */
    virtual void setOSCManager(OSCManager* oscManager) = 0;

    /*!
     * Finds the presets that are similar to the current patch
     */
    virtual void findSimilar() = 0;
};

