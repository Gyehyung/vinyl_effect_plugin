#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

VinylAudioProcessor::VinylAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, "PARAMETERS",
        {
            std::make_unique<juce::AudioParameterFloat>("VOLUME", "Volume", 0.0f, 1.0f, 0.5f)
        })
#endif
{
}

VinylAudioProcessor::~VinylAudioProcessor()
{
}

//==============================================================================

void VinylAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    lowCutFilter.prepare(spec);  // Low-cut filter
    highCutFilter.prepare(spec); // High-cut filter
    updateLowCutFilter();        // Update low-cut initially
    updateHighCutFilter();       // Update high-cut initially

    subBassFilter.prepare(spec);    // Sub-Bass filter
    lowMidFilter.prepare(spec);     // Low-Mid filter
    highFreqFilter.prepare(spec);   // High-Frequency filter
    updateFirstEQSlider();          // Update all filters initially
}

void VinylAudioProcessor::releaseResources()
{
    // Free any resources after playback stops here
}

void VinylAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear the output buffer if necessary
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get volume parameter
    auto* volume = parameters.getRawParameterValue("VOLUME");

    // Apply volume
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] *= *volume;
        }
    }

    // Apply the low-cut filter
    if (lowCutValue > 0.0f)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        lowCutFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }

    // Apply the high-cut filter
    if (highCutValue > 0.0f)
    {
        juce::dsp::AudioBlock<float> block(buffer);
        highCutFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }

    // Apply the First EQ Slider
    if (firstEQValue > 0.0f)
    {
        juce::dsp::AudioBlock<float> block(buffer);

        subBassFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
        lowMidFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
        highFreqFilter.process(juce::dsp::ProcessContextReplacing<float>(block));

    }
}

//==============================================================================
void VinylAudioProcessor::setFirstEQSliderValue(float value)
{
    firstEQValue = value;
    updateFirstEQSlider();
}

void VinylAudioProcessor::updateFirstEQSlider()
{
    // Sub-Bass Cut (below 80 Hz
    if (firstEQValue == 0.0f)
    {
        // Bypass when slider is at 0
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), 80.0f);  // No cut at 80Hz
        *subBassFilter.get<0>().state = *coefficients;
    }
    else
    {
        float subBassCutGain = 1.0f - std::pow(firstEQValue, 2.0f) * 0.3f;  // Non-linear cut
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), 80.0f, subBassCutGain);
        *subBassFilter.get<0>().state = *coefficients;
    }

    // Low-Mid Boost (150 Hz - 500 Hz)
    if (firstEQValue == 0.0f)
    {
        // Bypass when the slider is at 0
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass(getSampleRate(),325.0f);  // No boost
        *lowMidFilter.get<0>().state = *coefficients;
        return;
    }
    else
    {
        // Apply a boost based on slider value
        float lowMidBoostGain = 1.0f + std::pow(firstEQValue, 2.0f) * 0.5f;
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), 325.0f, 1.0f, lowMidBoostGain);  // Peak filter to boost Low-Mids
        *lowMidFilter.get<0>().state = *coefficients;
    }

    // High-Frequency Roll-Off (above 10 kHz)
    if (firstEQValue == 0.0f)
    {
        // Bypass when slider is at 0
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), 20000.0f);
        *highFreqFilter.get<0>().state = *coefficients;
    }
    else
    {
        float highFreqRollOffGain = 1.0f - std::pow(firstEQValue, 2.0f) * 0.15f;
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), 10000.0f, highFreqRollOffGain);
        *highFreqFilter.get<0>().state = *coefficients;
    }
}



//==============================================================================

void VinylAudioProcessor::setLowCutValue(float value)
{
    lowCutValue = value;
    updateLowCutFilter();
}

void VinylAudioProcessor::updateLowCutFilter()
{
    if (lowCutValue == 0.0f)
    {
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), 20.0f);  // Bypass filter
        *lowCutFilter.get<0>().state = *coefficients;
        return;
    }

    float cutoffFrequency = 80.0f + lowCutValue * 200.0f;  // 80Hz - 280Hz
    auto coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), cutoffFrequency);
    *lowCutFilter.get<0>().state = *coefficients;
}

void VinylAudioProcessor::setHighCutValue(float value)
{
    highCutValue = value;
    updateHighCutFilter();
}

void VinylAudioProcessor::updateHighCutFilter()
{
    if (highCutValue == 0.0f)
    {
        auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), 20000.0f);  // Bypass filter
        *highCutFilter.get<0>().state = *coefficients;
        return;
    }

    float cutoffFrequency = 10000.0f - highCutValue * 5000.0f;  // 10kHz - 5kHz
    auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), cutoffFrequency);
    *highCutFilter.get<0>().state = *coefficients;
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool VinylAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
}
#endif

//==============================================================================

bool VinylAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* VinylAudioProcessor::createEditor()
{
    return new VinylAudioProcessorEditor(*this);
}

//==============================================================================

const juce::String VinylAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VinylAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool VinylAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool VinylAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double VinylAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VinylAudioProcessor::getNumPrograms()
{
    return 1;
}

int VinylAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VinylAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String VinylAudioProcessor::getProgramName(int index)
{
    return {};
}

void VinylAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================

void VinylAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void VinylAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

//==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VinylAudioProcessor();
}
