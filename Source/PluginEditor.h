/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Rigkeeper.h"

#define MIN_HEIGHT 300
#define MIN_WIDTH  500
#define MAX_HEIGHT 1000
#define MAX_WIDTH  1000
//==============================================================================
/**
*/
class RigkeeperAudioProcessorEditor  : public AudioProcessorEditor, public Rigkeeper::Listener , public Value::Listener
{
public:
    RigkeeperAudioProcessorEditor (RigkeeperAudioProcessor&);
    ~RigkeeperAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

	
	void labelTextChanged(Label* labelThatHasChanged) override
	{
        DBG("label Text Changed ");
        if (labelThatHasChanged == &rigkCmpt)
        {
            audioProcessor.rigFileNameValue.setValue(rigkCmpt.getRigFileName());
            audioProcessor.rigFileContentValue.setValue(rigkCmpt.getRigContent());
        }
	}

    void valueChanged(Value& value) override
    {
       
			DBG("Value Changed ");
            rigkCmpt.setRigContent(audioProcessor.rigFileContentValue.getValue().toString());
            rigkCmpt.setRigFileName(audioProcessor.rigFileNameValue.getValue().toString());
            rigkCmpt.getTextValue().referTo(audioProcessor.rigFileNameValue);
            rigkCmpt.repaint();
        
    };
    //bool shouldDropFilesWhenDraggedExternally(const juce::DragAndDropTarget::SourceDetails&, juce::StringArray&, bool&) override;
    Value rigNameValue;
    Value rigContentValue;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    RigkeeperAudioProcessor& audioProcessor;
    Rigkeeper rigkCmpt;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RigkeeperAudioProcessorEditor)
};
