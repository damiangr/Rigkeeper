/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RigkeeperAudioProcessor::RigkeeperAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameters()), rigFileNameValue(String("")), rigFileContentValue(String(""))
#endif
{
    
    DBG("Creating Processor");
    apvts.state.setProperty("rigFileName", "", nullptr);
    apvts.state.setProperty("rigBase64Content", "", nullptr);

}

RigkeeperAudioProcessor::~RigkeeperAudioProcessor()
{
}

//==============================================================================
const String RigkeeperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RigkeeperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RigkeeperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RigkeeperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RigkeeperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RigkeeperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RigkeeperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RigkeeperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RigkeeperAudioProcessor::getProgramName (int index)
{
    return {};
}

void RigkeeperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RigkeeperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    DBG("prepareToPlay ");
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void RigkeeperAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RigkeeperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RigkeeperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool RigkeeperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RigkeeperAudioProcessor::createEditor()
{
    DBG("AudioProcessor:CreateEditor ");
    return new RigkeeperAudioProcessorEditor (*this);
}

//==============================================================================
void RigkeeperAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    DBG("getStateInformation ");
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
	
    apvts.state.setProperty("rigFileName", rigFileNameValue, nullptr);
    apvts.state.setProperty("rigBase64Content", rigFileContentValue, nullptr);
    auto editor = apvts.state.getOrCreateChildWithName("editor", nullptr);
    editor.setProperty("size-x", editorSize.x, nullptr);
    editor.setProperty("size-y", editorSize.y, nullptr);	
   
    auto state = apvts.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
   // juce::File file("D:/toDAW.xml");
   // if (xml->writeToFile(file, "")) DBG("toDAW written");
   // else DBG("toDAW not written");
    copyXmlToBinary(*xml, destData);

}

void RigkeeperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    DBG("SetStateInformation ");
 // File file("D:/fromDAW.xml");
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType())) {
            rigFileNameValue.setValue("");
            apvts.replaceState(ValueTree::fromXml(*xmlState));
            auto editor = apvts.state.getChildWithName("editor");
            if (editor.isValid())
            {
                editorSize.setX(editor.getProperty("size-x", MIN_WIDTH));
                editorSize.setY(editor.getProperty("size-y", MIN_HEIGHT));
                if (auto* activeEditor = getActiveEditor())
                    activeEditor->setSize(editorSize.x, editorSize.y);
            }
         // if (xmlState->writeToFile(file, "")) DBG("fromDAW written");
         // else DBG("fromDAW not written");
            DBG(rigFileNameValue.toString());
            rigFileContentValue.setValue(apvts.state.getProperty("rigBase64Content").toString());
            rigFileNameValue.setValue(apvts.state.getProperty("rigFileName").toString());
            DBG(rigFileNameValue.toString());
            
       }
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RigkeeperAudioProcessor();
}


juce::AudioProcessorValueTreeState::ParameterLayout RigkeeperAudioProcessor::createParameters() 
{
    DBG("Creating Parameter Layout ");
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    return { params.begin(), params.end() };
}

Point<int> RigkeeperAudioProcessor::getSavedSize() const
{
    return editorSize;
}

void RigkeeperAudioProcessor::setSavedSize(const Point<int>& size)
{
    editorSize = size;
}