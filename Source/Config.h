/*
  ==============================================================================

    Config.h
    Created: 19 Oct 2020 4:29:20pm
    Author:  Yilin Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

// TODO: Consider bundling all the configurable parameters in a class in the future
const juce::String LOCAL_ADDRESS = "127.0.0.1";
const int OSC_RECEIVE_PORT = 9001;
const int OSC_SEND_PORT = 7777;
const int UDP_SEND_PORT = 8888;

const juce::String OSC_SEND_PATTERN = "/Ideator/python/";
const juce::String OSC_RECEIVE_PATTERN = "/Ideator/cpp/";

const juce::String TMP_AUDIO_DIR = "/tmp/Ideator/";
