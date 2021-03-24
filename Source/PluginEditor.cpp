/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
RigkeeperAudioProcessorEditor::RigkeeperAudioProcessorEditor (RigkeeperAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p) 
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    DBG("creatingEditor RigkeeperAudioProcessorEditor ");
    auto size = audioProcessor.getSavedSize();
    setResizable(true, true);
    setSize (size.x, size.y);
    
    setResizeLimits(MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
    addAndMakeVisible(rigkCmpt);
    rigkCmpt.setBounds(getLocalBounds());
    rigkCmpt.setFont(Font(16.0f, Font::bold));
    rigkCmpt.setJustificationType(Justification::centred);
    rigkCmpt.addListener(this); // add listener

    //check if rig was restored from daw or there is dragged rig - display default text otherwise
    if (audioProcessor.rigFileNameValue.toString().isNotEmpty())
    {
        rigkCmpt.setRigFileName(audioProcessor.rigFileNameValue.toString());
        rigkCmpt.setRigContent(audioProcessor.rigFileContentValue.toString());
        rigkCmpt.setText(audioProcessor.rigFileNameValue.toString(), dontSendNotification);
    }
    else   
	{
        rigkCmpt.displayInitialText(); //set default message
    }
  
 
    
       
}

RigkeeperAudioProcessorEditor::~RigkeeperAudioProcessorEditor()
{
}

//==============================================================================
void RigkeeperAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    DBG("painting Editor ");
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    
    
}

void RigkeeperAudioProcessorEditor::resized()
{
    DBG("resizing Editor ");
    rigkCmpt.setBounds(getLocalBounds());
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    audioProcessor.setSavedSize({ getWidth(), getHeight() });
}






