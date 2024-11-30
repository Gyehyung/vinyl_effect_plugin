#pragma once

#include <JuceHeader.h>

//==============================================================================
class VinylAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
    , public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    VinylAudioProcessor();
    ~VinylAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Getter for accessing the parameters
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

    // Low-Cut Filter (Second EQ slier)
    void setLowCutValue(float value);
    void updateLowCutFilter();

    // High-Cut Filter (Third EQ slider)
    void setHighCutValue(float value);
    void updateHighCutFilter();

    // First (Main) EQ Slider 
    void setFirstEQSliderValue(float value); 
    void updateFirstEQSlider();

private:
    juce::AudioProcessorValueTreeState parameters;

    // Low-Cut Filter for the second slider
    using Filter = juce::dsp::IIR::Filter<float>;
    using FilterDuplicator = juce::dsp::ProcessorDuplicator<Filter, juce::dsp::IIR::Coefficients<float>>;
    juce::dsp::ProcessorChain<FilterDuplicator> lowCutFilter;

    // High-Cut Filter for the third slider
    juce::dsp::ProcessorChain<FilterDuplicator> highCutFilter;

    // First EQ Slider Filters
    juce::dsp::ProcessorChain<FilterDuplicator> subBassFilter;   // Sub-Bass Cut filter (below 80Hz)
    juce::dsp::ProcessorChain<FilterDuplicator> lowMidFilter;    // Low-Mid Boost filter (150Hz to 500Hz)
    juce::dsp::ProcessorChain<FilterDuplicator> highFreqFilter;  // High-Frequency Roll-Off (above 10kHz)

    // Values for the EQ sliders
    float lowCutValue = 0.0f;
    float highCutValue = 0.0f;
    float firstEQValue = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VinylAudioProcessor)
};
