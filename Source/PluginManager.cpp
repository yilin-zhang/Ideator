/*
  ==============================================================================

    PluginManager.cpp
    Created: 27 Oct 2020 11:17:06am
    Author:  Yilin Zhang

  ==============================================================================
*/

#include "PluginManager.h"

PluginManager::PluginManager():
        plugin(nullptr),
        initialSampleRate(44100.f),
        initialBufferSize(256)
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
