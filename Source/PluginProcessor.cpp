/*
Copyright (C) 2023  Arden Butterfield

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EmpyAudioProcessor::EmpyAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // see https://forum.juce.com/t/failing-assert-in-juce-audioprocessor/51926 for more info on parameter id stuff
    
    
    
    
    auto maskthresh = new juce::AudioParameterFloat(juce::ParameterID {"dynamicthresh", 1}, "dynamic threshold", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5);
    control_parameters[0].audio_parameter = maskthresh;
    control_parameters[0].name = "Dynamic threshold";
    control_parameters[0].description = "This slider shifts the level of the dynamic threshold up or down by the same amount for all frequencies. At zero, no dynamic threshold is applied, so the speed and masking distance knobs have no effect.";
    control_parameters[0].min_val = 0;
    control_parameters[0].max_val = 1;
    control_parameters[0].controller_type = slider;
    
    auto absthresh = new juce::AudioParameterFloat(juce::ParameterID {"staticthresh", 1}, "static threshold",juce::NormalisableRange<float>(0.0f,1.0f),0);
    control_parameters[1].audio_parameter = absthresh;
    control_parameters[1].name = "Static threshold";
    control_parameters[1].description = "This slider shifts the level of the static threshold up or down by the same amount for all frequencies. At zero, no static threshold is applied, so the curve knob has no effect.";
    control_parameters[1].min_val = 0;
    control_parameters[1].max_val = 1;
    control_parameters[1].controller_type = slider;
    
    auto maskdistance = new juce::AudioParameterFloat(juce::ParameterID {"spreadwidth", 1}, "smoothness", juce::NormalisableRange<float>(0.0f,10.0f), 2);
    control_parameters[2].audio_parameter = maskdistance;
    control_parameters[2].name = "Smoothness";
    control_parameters[2].description = "This knob controls how far the energy in a band will spread when calculating the dynamic threshold. Higher values focus the sound to its strongest frequency components.";
    control_parameters[2].min_val = 0;
    control_parameters[2].max_val = 10;
    control_parameters[2].controller_type = slider;
    
    auto quantization = new juce::AudioParameterFloat(juce::ParameterID {"quantization",1},"quantization",juce::NormalisableRange<float>(0.0f,100.0f),0.f);
    control_parameters[3].audio_parameter = quantization;
    control_parameters[3].name = "Quantization";
    control_parameters[3].description = "Quantization works a bit like bit reduction in a bitcrusher, decreasing the precision of the amplitudes. A setting of 0 means no quantization.";
    control_parameters[3].min_val = 0;
    control_parameters[3].max_val = 100;
    control_parameters[3].controller_type = slider;
    
    auto speed = new juce::AudioParameterFloat(juce::ParameterID {"speed",1}, "speed", juce::NormalisableRange<float>(0.0f,1.0f),0);
    control_parameters[4].audio_parameter = speed;
    control_parameters[4].name = "Speed";
    control_parameters[4].description = "Speed controls the rate the dynamic threshold responds to new inputs. At lower speed values, Empy has a bubblier sound. High speed values bring out attacks, which are able to pass through unfiltered.";
    control_parameters[4].min_val = 0;
    control_parameters[4].max_val = 1;
    control_parameters[4].controller_type = slider;
    
    auto mdctsize = new juce::AudioParameterChoice(juce::ParameterID {"mdctsize",1},
                                               "frequency resolution",
                                               juce::StringArray{"","4","8","16","32","64","128","256","512","1024","2048","4096"},
                                               8);
    control_parameters[5].audio_parameter = mdctsize;
    control_parameters[5].name = "Frequency resolution";
    control_parameters[5].description = "Frequency resolution is the level of detail we use in the transform. Higher frequency resolution gives a smoother sound, but introduces more latency.";
    control_parameters[5].min_val = 0;
    control_parameters[5].max_val = 10;
    control_parameters[5].controller_type = combobox;
    
    auto lossprob = new juce::AudioParameterFloat(juce::ParameterID {"lossp", 1}, "stick probability", juce::NormalisableRange<float>(0.0f,1.0f),0);
    control_parameters[6].audio_parameter = lossprob;
    control_parameters[6].name = "Stick probability";
    control_parameters[6].description = "Stick probability is the proportion of time spent stuck. At 0, the sound will never get stuck, at 0.5 it will be stuck about half the time.";
    control_parameters[6].min_val = 0;
    control_parameters[6].max_val = 1;
    control_parameters[6].controller_type = slider;
    
    auto losslength = new juce::AudioParameterFloat(juce::ParameterID {"lossq", 1}, "stick length", juce::NormalisableRange<float>(0.0f,3.0f),0.5);
    control_parameters[7].audio_parameter = losslength;
    control_parameters[7].name = "Stick length";
    control_parameters[7].description = "The average length in seconds that the sound will stay stuck for. The exact length of each stick is random.";
    control_parameters[7].min_val = 0;
    control_parameters[7].max_val = 3;
    control_parameters[7].controller_type = slider;
    
    auto tilt = new juce::AudioParameterFloat(juce::ParameterID {"bias",1},"bias", juce::NormalisableRange<float>(-2.f,2.f),0);
    control_parameters[8].audio_parameter = tilt;
    control_parameters[8].name = "Bias";
    control_parameters[8].description = "Bias tilts the thresholds towards the low or high frequencies. At 0, there is no bias towards low or high frequencies.";
    control_parameters[8].min_val = -2;
    control_parameters[8].max_val = 2;
    control_parameters[8].controller_type = slider;
    
    auto perceptual_curve = new juce::AudioParameterFloat(juce::ParameterID {"curve", 1}, "curve", juce::NormalisableRange<float>(0.f,1.f),1);
    control_parameters[9].audio_parameter = perceptual_curve;
    control_parameters[9].name = "Curve";
    control_parameters[9].description = "The curve knob interpolates between a flat static threshold at 0, and the absolute threshold of hearing for humans at 1.";
    control_parameters[9].min_val = 0;
    control_parameters[9].max_val = 1;
    control_parameters[9].controller_type = slider;
    
    auto mix = new juce::AudioParameterFloat(juce::ParameterID{"mix", 1}, "mix", juce::NormalisableRange<float>(0.f,100.f),100);
    control_parameters[10].audio_parameter = mix;
    control_parameters[10].name = "Mix";
    control_parameters[10].description = "The balance between input and processed signal returned by the plugin.";
    control_parameters[10].min_val = 0;
    control_parameters[10].max_val = 100;
    control_parameters[10].controller_type = slider;
    
    auto gate_ratio_range = juce::NormalisableRange<float>(1.0, 100.0);
    gate_ratio_range.setSkewForCentre(10.0);
    auto gate_ratio = new juce::AudioParameterFloat(juce::ParameterID {"ratio", 1}, "gate ratio", gate_ratio_range,100);
    control_parameters[11].audio_parameter = gate_ratio;
    control_parameters[11].name = "Ratio";
    control_parameters[11].description = "Ratio controls the strength of the gate at attenuating sounds below its threshold. At a high ratio, the gate silences frequencies below the threshold; at a ratio of 1, the gate has no effect.";
    control_parameters[11].min_val = 1;
    control_parameters[11].max_val = 100;
    control_parameters[11].controller_type = slider;
    
    for (const auto &c : control_parameters) {
        addParameter(c.audio_parameter);
    }
    
    for (auto &c : control_parameters) {
        c.active = true;
    }
    
    for (auto &c : control_parameters) {
        std::string id_name = c.name;
        // Remove spaces from identifier name, not allowed for xml
        id_name.erase(std::remove(id_name.begin(), id_name.end(), ' '), id_name.end());
        c.identifier = juce::Identifier(id_name);
    }
    
    empyModel.set_control_parameters(&control_parameters);
}

EmpyAudioProcessor::~EmpyAudioProcessor()
{
#if 0
    // For some reason this runs into the same error that we were getting before. So I'm thinking it's safe to just not
    // free this memory, since it's happening at the end of the plugin's life anyway.
    for (auto &c : control_parameters) {
        if (c.controller_type == slider) {
            delete (juce::AudioParameterFloat*)c.audio_parameter;
        } else {
            delete (juce::AudioParameterChoice*)c.audio_parameter;
        }
    }
#endif
}

//==============================================================================
const juce::String EmpyAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EmpyAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EmpyAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EmpyAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EmpyAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EmpyAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EmpyAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EmpyAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EmpyAudioProcessor::getProgramName (int index)
{
    return {};
}

void EmpyAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EmpyAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    empyModel.prepare(1024,
                      sampleRate,
                      std::min(getTotalNumInputChannels(),getTotalNumOutputChannels()));
    
}

void EmpyAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EmpyAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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


void EmpyAudioProcessor::update_parameters()
{
    empyModel.set_mask_threshold(static_cast<juce::AudioParameterFloat*>(control_parameters[0].audio_parameter)->get());
    
    empyModel.set_absolute_threshold(static_cast<juce::AudioParameterFloat*>(control_parameters[1].audio_parameter)->get());
    
    empyModel.set_spread_distance(static_cast<juce::AudioParameterFloat*>(control_parameters[2].audio_parameter)->get());
    
    empyModel.set_bit_reduction_above_threshold(static_cast<juce::AudioParameterFloat*>(control_parameters[3].audio_parameter)->get());
    
    empyModel.set_speed(static_cast<juce::AudioParameterFloat*>(control_parameters[4].audio_parameter)->get());
    
    empyModel.set_perceptual_curve(static_cast<juce::AudioParameterFloat*>(control_parameters[9].audio_parameter)->get());
    
    empyModel.set_mix(static_cast<juce::AudioParameterFloat*>(control_parameters[10].audio_parameter)->get());
    
    empyModel.set_gate_ratio(static_cast<juce::AudioParameterFloat*>(control_parameters[11].audio_parameter)->get());
    
    int mdct_size_options[] = {4,8,16,32,64,128,256,512,1024,2048,4096};
    int mdct_size_index = static_cast<juce::AudioParameterChoice*>(control_parameters[5].audio_parameter)->getIndex() - 1;
    int new_mdct_size = mdct_size_options[mdct_size_index];
    empyModel.set_mdct_size(new_mdct_size);
    
    if (new_mdct_size != getLatencySamples()) {
        setLatencySamples(mdct_size_options[mdct_size_index]);
    }
    empyModel.set_packet_loss(static_cast<juce::AudioParameterFloat*>(control_parameters[6].audio_parameter)->get(),
                              static_cast<juce::AudioParameterFloat*>(control_parameters[7].audio_parameter)->get(),
                              control_parameters[7].max_val);
    
    empyModel.set_bias(static_cast<juce::AudioParameterFloat*>(control_parameters[8].audio_parameter)->get());
}

void EmpyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{    
    update_parameters();
    
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
    empyModel.processBlock(buffer);

}

//==============================================================================
bool EmpyAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EmpyAudioProcessor::createEditor()
{
    return new EmpyAudioProcessorEditor (*this, &control_parameters);
}

//==============================================================================
void EmpyAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    float v;
    juce::XmlElement xml = juce::XmlElement("empyParams");
    for (const auto &c : control_parameters) {
        v = c.audio_parameter->getValue();
        xml.setAttribute(c.identifier, v);
    }
    copyXmlToBinary(xml, destData);
}

void EmpyAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if ((xmlState.get() != nullptr) && (xmlState->hasTagName ("empyParams"))) {
        for (auto &c : control_parameters) {
            c.audio_parameter->setValue(xmlState->getDoubleAttribute(c.identifier));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EmpyAudioProcessor();
}
