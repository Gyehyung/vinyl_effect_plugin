#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

VinylAudioProcessorEditor::VinylAudioProcessorEditor(VinylAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Volume slider
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(volumeSlider);

    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getParameters(), "VOLUME", volumeSlider);

    // Vinyl crackle noise
    crackleComboBox.addItem("Subtle Crackle", 1);
    crackleComboBox.addItem("Medium Crackle", 2);
    crackleComboBox.addItem("Heavy Crackle", 3);
    crackleComboBox.setSelectedId(1); // Default to "Subtle Crackle"
    addAndMakeVisible(crackleComboBox);

    crackleLabel.setText("Vinyl Crackle", juce::dontSendNotification);
    crackleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    crackleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(crackleLabel);

    // Saturation knob
    saturationKnob.setSliderStyle(juce::Slider::Rotary);
    saturationKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    saturationKnob.setRange(0.0f, 1.0f, 0.01f);
    saturationKnob.setValue(0.0f);
    saturationKnob.setLookAndFeel(&lookAndFeelV4); // Apply LookAndFeel_V4
    addAndMakeVisible(saturationKnob);

    saturationLabel.setText("Distortion / Saturation", juce::dontSendNotification);
    saturationLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    saturationLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(saturationLabel);

    // Wobble knob
    wobbleKnob.setSliderStyle(juce::Slider::Rotary);
    wobbleKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    wobbleKnob.setRange(0.0f, 1.0f, 0.01f);
    wobbleKnob.setValue(0.0f);
    wobbleKnob.setLookAndFeel(&lookAndFeelV4); // Apply LookAndFeel_V4
    addAndMakeVisible(wobbleKnob);

    wobbleLabel.setText("Wobble Effect", juce::dontSendNotification);
    wobbleLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    wobbleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(wobbleLabel);

    // EQ sliders and buttons 
    for (int i = 0; i < 3; ++i)
    {
        eqSliders[i].setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        eqSliders[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
        eqSliders[i].setRange(0.0f, 1.0f, 0.01f);
        eqSliders[i].setValue(0.0f);
        eqSliders[i].setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
        eqSliders[i].setColour(juce::Slider::trackColourId, juce::Colours::grey);
        eqSliders[i].setEnabled(false);
        addAndMakeVisible(eqSliders[i]);

        eqButtons[i].setButtonText("EQ Toggle " + juce::String(i + 1));
        eqButtons[i].setClickingTogglesState(true);
        eqButtons[i].onClick = [this, i] { handleToggleButtonClicked(i); };
        eqButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        addAndMakeVisible(eqButtons[i]);
    }

    setSize(900, 675);
}

VinylAudioProcessorEditor::~VinylAudioProcessorEditor()
{
    // Knobs design (should be implemented)
    saturationKnob.setLookAndFeel(nullptr);
    wobbleKnob.setLookAndFeel(nullptr);
}

//==============================================================================

void VinylAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);

    // Llines for 2x2 grid
    int remainingWidth = getWidth() - 130;
    int remainingHeight = getHeight() - 100;
    int quadrantWidth = remainingWidth / 2;
    int quadrantHeight = remainingHeight / 2;

    g.drawLine(110, 50 + quadrantHeight, 110 + remainingWidth, 50 + quadrantHeight, 2.0f);
    g.drawLine(110 + quadrantWidth, 50, 110 + quadrantWidth, 50 + remainingHeight, 2.0f);
}

void VinylAudioProcessorEditor::resized()
{
    // Volume slider
    volumeSlider.setBounds(10, 50, 80, getHeight() - 100);

    // Quadrants dimensions
    int quadrantWidth = (getWidth() - 130) / 2;
    int quadrantHeight = (getHeight() - 100) / 2;
    int xOffset = 110;
    int yOffset = 50;

    // Vinyl crackle control (top-left quadrant)
    const int crackleMargin = 10;
    crackleLabel.setBounds(xOffset, yOffset, quadrantWidth - crackleMargin, 20);
    crackleComboBox.setBounds(xOffset, yOffset + 30, quadrantWidth - crackleMargin, 30);

    // Saturation knob (bottom-left quadrant)
    int saturationYOffset = yOffset + quadrantHeight + 10;
    int knobWidth = quadrantWidth / 2; // Half of the quadrant width
    saturationLabel.setBounds(xOffset, saturationYOffset, quadrantWidth, 20);
    saturationKnob.setBounds(xOffset + (quadrantWidth - knobWidth) / 2, saturationYOffset + 30, knobWidth, knobWidth);

    // Wobble knob (bottom-right quadrant)
    int wobbleXOffset = xOffset + quadrantWidth + 10; // Start at the bottom-right quadrant
    wobbleLabel.setBounds(wobbleXOffset, saturationYOffset, quadrantWidth, 20);
    wobbleKnob.setBounds(wobbleXOffset + (quadrantWidth - knobWidth) / 2, saturationYOffset + 30, knobWidth, knobWidth);

    // EQ sliders and buttons (top-right quadrant)
    int sliderWidth = quadrantWidth / 3;
    int sliderHeight = quadrantHeight - 50;

    for (int i = 0; i < 3; ++i)
    {
        eqSliders[i].setBounds(110 + quadrantWidth + i * sliderWidth, 50, sliderWidth, sliderHeight);
        eqButtons[i].setBounds(110 + quadrantWidth + i * sliderWidth + 10, 50 + sliderHeight + 10, sliderWidth - 20, 30);
    }
}

//==============================================================================

void VinylAudioProcessorEditor::handleToggleButtonClicked(int buttonIndex)
{
    for (int i = 0; i < 3; ++i)
    {
        if (i != buttonIndex)
        {
            eqButtons[i].setToggleState(false, juce::dontSendNotification);
            eqSliders[i].setEnabled(false);
            eqSliders[i].setColour(juce::Slider::thumbColourId, juce::Colours::darkgrey);
            eqSliders[i].setColour(juce::Slider::trackColourId, juce::Colours::grey);
            eqButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);

            if (i == 0)
                audioProcessor.setFirstEQSliderValue(0.0f);
            else if (i == 1)
                audioProcessor.setLowCutValue(0.0f);
            else if (i == 2)
                audioProcessor.setHighCutValue(0.0f);
        }
    }

    eqSliders[buttonIndex].setEnabled(true);
    eqSliders[buttonIndex].setColour(juce::Slider::thumbColourId, juce::Colours::green);
    eqSliders[buttonIndex].setColour(juce::Slider::trackColourId, juce::Colours::lightgreen);
    eqButtons[buttonIndex].setColour(juce::TextButton::buttonColourId, juce::Colours::green);

    if (buttonIndex == 0)
        audioProcessor.setFirstEQSliderValue(eqSliders[0].getValue());
    else if (buttonIndex == 1)
        audioProcessor.setLowCutValue(eqSliders[1].getValue());
    else if (buttonIndex == 2)
        audioProcessor.setHighCutValue(eqSliders[2].getValue());
}
