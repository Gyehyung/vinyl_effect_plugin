#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class VinylAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    VinylAudioProcessorEditor(VinylAudioProcessor&);
    ~VinylAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    VinylAudioProcessor& audioProcessor;

    // Volume slider
    juce::Slider volumeSlider;

    // EQ sliders and buttons
    juce::Slider eqSliders[3];
    juce::TextButton eqButtons[3];

    // Crackling control
    juce::ComboBox crackleComboBox;
    juce::Label crackleLabel;

    // Distortion/Saturation control
    juce::Slider saturationKnob;
    juce::Label saturationLabel;

    // Wobble Effect control
    juce::Slider wobbleKnob;
    juce::Label wobbleLabel;

    // LookAndFeel
    juce::LookAndFeel_V4 lookAndFeelV4;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;

    // Handle button clicks
    void handleToggleButtonClicked(int buttonIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VinylAudioProcessorEditor)
};
