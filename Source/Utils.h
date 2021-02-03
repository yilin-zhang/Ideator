/*
  ==============================================================================

    Utils.h
    Created: 3 Feb 2021 10:31:10am
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <unordered_set>

class PresetManager
{
public:
    static juce::XmlElement generate(const juce::XmlElement &xmlState,
                                     const juce::String &pluginPath,
                                     const std::unordered_set<juce::String> &descriptors)
    {
        // IdeatorPlugin
        //  |- Meta
        //  |   |- PluginPath
        //  |   |- Descriptors
        //  |- Parameters

        juce::XmlElement xmlPreset("IdeatorPreset");

        // A deep copy of xmlState should be created.
        // The new element will be automatically deleted by the parent, so there is no need to
        // explicitly delete it.
        auto xmlParameters = new juce::XmlElement(xmlState);
        xmlParameters->setTagName("Parameters");

        auto xmlMeta = new juce::XmlElement("Meta");
        auto xmlPluginPath = new juce::XmlElement("PluginPath");
        xmlPluginPath->addTextElement(pluginPath);

        auto xmlDescriptors = new juce::XmlElement("Descriptors");
        juce::String descriptorString;
        bool isFirst = true;
        for (auto &descriptor : descriptors)
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
        return xmlPreset;
    }

    static bool parse(const juce::XmlElement &preset,
                      juce::XmlElement &xmlState,
                      juce::String &pluginPath,
                      std::unordered_set<juce::String> &descriptors)
    {
        // parse the plugin path
        auto xmlMeta = preset.getChildByName("Meta");
        auto xmlPluginPath = xmlMeta->getChildByName("PluginPath");
        pluginPath = xmlPluginPath->getAllSubText();

        // parse plugin parameters
        xmlState = *preset.getChildByName("Parameters");

        // set meta data
        auto xmlDescriptors = xmlMeta->getChildByName("Descriptors");
        juce::StringArray descriptorArray;
        descriptorArray.addTokens(xmlDescriptors->getAllSubText(), ", ", "\"");
        descriptors.clear();
        for (auto &descriptor : descriptorArray)
        {
            if (descriptor.isEmpty())
                continue;
            descriptors.insert(descriptor);
        }

        // true means parse successfully
        return true;
    }
};