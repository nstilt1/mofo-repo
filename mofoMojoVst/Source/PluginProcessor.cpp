#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EnvelopeProcessor.h"

//==============================================================================
MofoFilterAudioProcessor::MofoFilterAudioProcessor() 
#ifndef JucePlugin_PreferredChannelConfigurations
     : forwardFFT(fftOrder), AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ) 
#endif
{
    treeState.addParameterListener("isHighPass", this);
    treeState.addParameterListener("is2Pole", this);
    treeState.addParameterListener("resonance", this);
    treeState.addParameterListener("drive", this);
    treeState.addParameterListener("cutoff", this);
    treeState.addParameterListener("volume", this);
}

MofoFilterAudioProcessor::~MofoFilterAudioProcessor()
{
    treeState.addParameterListener("isHighPass", this);
    treeState.addParameterListener("is2Pole", this);
    treeState.addParameterListener("resonance", this);
    treeState.addParameterListener("drive", this);
    treeState.addParameterListener("cutoff", this);
    treeState.addParameterListener("volume", this);
}

float MofoFilterAudioProcessor::curveConvert(float input, float shape)
{
    if (shape == 0.5f)
        return input;
    if (input == 0.f) return 0;
    if (input == 1.f) return 1;
    return (shape * input) / (1 - shape - input + (2 * input * shape));
}

void MofoFilterAudioProcessor::pushNextSampleIntoFifo(float sample) noexcept
{
    // if the fifo contains enough data, set a flag to say
    // that the next line should now be rendered..

    if (fifoIndex == fftSize)
    {
        juce::zeromem(fftData, sizeof(fftData));
        memcpy(fftData, fifo, sizeof(fifo));
        nextFFTBlockReady = true;
        fifoIndex = 0;
    }
    fifo[fifoIndex++] = sample;
}

float MofoFilterAudioProcessor::getFFTData()
{
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);

    int index = 0;
    auto binSize = getSampleRate() / fftSize;
    juce::Array<float> maxes;
    juce::Array<int> indexes;
    float max = 0.f;
    float absSample;
    float maxFreq = 5000 / binSize;
    float index2 = 0;

    auto chainSettings = getChainSettings(treeState);
    float harmonicCoefficient = 0.05;

    for (auto i = 0; i < maxFreq; ++i)
    {
        absSample = std::abs(fftData[i]);

        if (max < absSample)
        {
            max = absSample;
            maxes.add(absSample);
            indexes.add(i);
            index = i;

        }
    }


    int ret = 0;
    for (auto i = 0; i < indexes.size(); ++i)
    {
        if (maxes[i] > harmonicCoefficient * max)
        {
            ret = indexes[i];
            i = indexes.size();
        }
    }
    return binSize * ret;
}

//==============================================================================
const juce::String MofoFilterAudioProcessor::getName() const
{
    return "Mofo Mojo Filter";
}

bool MofoFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MofoFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MofoFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MofoFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MofoFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MofoFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MofoFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MofoFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void MofoFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MofoFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    max = 0.00000000001f;
    
    currentVolume = 0.0f;

    iteration2 = 0;

    MAX = 0;
    last = 0;
    last2 = 0;
    lastFreq = 20;
    num3 = (int)sampleRate / 1000 * 7;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    auto chainSettings = getChainSettings(treeState);

    gain.prepare(spec);
    gain.reset();
    gain.setGainDecibels(chainSettings.volume);

    ladderFilter.prepare(spec);
    ladderFilter.reset();

    lastResonance = chainSettings.resonance / 10.f;

    ladderFilter.setCutoffFrequencyHz(chainSettings.cutoff);
    ladderFilter.setDrive(chainSettings.drive);
    ladderFilter.setResonance(lastResonance);

    mix.prepare (spec);
    mix.reset ();
    mix.setMixingRule (juce::dsp::DryWetMixingRule::linear);
    mix.setWetLatency (256);
    mix.setWetMixProportion (chainSettings.mix);
}

void MofoFilterAudioProcessor::releaseResources()
{
    ladderFilter.reset();
    gain.reset ();
    mix.reset ();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MofoFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MofoFilterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels ();
    auto totalNumOutputChannels = getTotalNumOutputChannels ();

    auto sampleRate = getSampleRate ();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples ());

    auto* input = buffer.getReadPointer (0);

    auto chainSettings = getChainSettings (treeState);

    if (chainSettings.isAuto)
    {
        // use FFT to attempt to detect the frequency of the lowest note being played
        for (int i = 0; i < buffer.getNumSamples (); ++i)
        {
            pushNextSampleIntoFifo (input[i]);
        }

        iteration2 += buffer.getNumSamples ();
        if (iteration2 >= fftSize)
        {
            iteration2 = 0;
            nextFFTBlockReady = false;
        }
    }

    float newFreq;
    float newRes;
    float newDrive;

    float maxDrive = chainSettings.driveAmount;
    float am = chainSettings.cutoffAmount;
    float maxRes = chainSettings.resonanceAmount;
    float maxSpeed = chainSettings.speedAmount;

    mix.setWetMixProportion (chainSettings.mix);
    mix.pushDrySamples (buffer);
    
    // process envelopes
    if (maxDrive != 0.f || maxRes != 0.f || am != 1.f || maxSpeed != 0.f)
    {
        float frequency;
        if (chainSettings.isAuto == 1)
        {
            float t = getFFTData ();
            if (t > lastFreq - 22 && t < lastFreq + 22)
            {
                frequency = lastFreq;
            }
            else
            {
                frequency = getFFTData ();
            }
            lastFreq = frequency;
        }
        else
            frequency = chainSettings.cutoff;

        currentVolume = buffer.getMagnitude (0, 0, buffer.getNumSamples ());

        if (frequency < 20)
            frequency = 20;

        // update max to maintain a ratio of volume/max that is within 0 <= volume/max <= 1.0
        if (max < currentVolume)
            max = currentVolume;

        float volumeRatio;
        if (max != 0)
            volumeRatio = currentVolume / max;
        else
            volumeRatio = 0.000000001f;

        newRes = envelope(
            volumeRatio,
            chainSettings.resonanceTension,
            chainSettings.resonance,
            chainSettings.resonanceAmount + chainSettings.resonance,
            1.f,
            chainSettings.resIsUp
        );


        // So, this part is kind of hard to conceptualize and describe.
        // The envelope that is based on the volume/max ratio and the "cutoff amount" knob is applied to the cutoff frequency. 
        // The "speed" is an envelope which is (still) based on the volume/max ratio, but it is applied to the "cutoff amount". 
        //
        // I wasn't sure what to call this, so I refer to it as "speed". I'm sure there's a more technical term.
        
        // process the "speed" envelope on the cutoff envelope
        float amount = envelope(
            volumeRatio, 
            chainSettings.speedTension, 
            chainSettings.cutoffAmount, 
            chainSettings.speedAmount + chainSettings.cutoffAmount,
            32.f,
            chainSettings.speedIsUp
        );

        newDrive = envelope(
            volumeRatio,
            chainSettings.driveTension,
            chainSettings.drive,
            chainSettings.driveAmount + chainSettings.drive,
            100.f,
            chainSettings.driveIsUp
        );

        if (chainSettings.isAuto == 1)
        {
            float baseValue = juce::jmax(20.f, chainSettings.minCutoff * frequency);
            newFreq = envelope(
                volumeRatio,
                chainSettings.cutoffTension,
                baseValue,
                baseValue * amount,
                18000.f,
                chainSettings.freqIsUp
            );
        }
        else 
        {
            newFreq = envelope(
                volumeRatio,
                chainSettings.cutoffTension,
                chainSettings.cutoff,
                chainSettings.cutoffAmount * chainSettings.cutoff,
                18000.f,
                chainSettings.freqIsUp
            );
        }

        if (newFreq > 18000)
            newFreq = 18000;

        if (newRes < 0.f)
            newRes = 0;
        if (newRes > 1.f)
            newRes = 1;
        if (lastResonance != newRes)
            ladderFilter.setResonance (newRes);
        lastResonance = newRes;

        ladderFilter.setDrive (newDrive);

        if (newFreq <= 0)
            newFreq = 20;
        ladderFilter.setCutoffFrequencyHz (newFreq);
        
        juce::dsp::AudioBlock<float> block (buffer);
        auto processingContext = juce::dsp::ProcessContextReplacing<float> (block);
        ladderFilter.process (processingContext);

        gain.process (processingContext);

        mix.mixWetSamples (processingContext.getOutputBlock ());
    }
    else
    {
        MofoFilterAudioProcessor::processBlockBypassed (buffer, midiMessages);
    }
}

// Function called when parameter is changed
void MofoFilterAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    auto chainSettings = getChainSettings(treeState);
    
    if (parameterID == "isHighPass")
        if (chainSettings.is2Pole == 0)
            if (newValue == 0)
                ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::LPF24);
            else
                ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::HPF24);
        else
            if (newValue == 0)
                ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::LPF12);
            else
                ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::HPF12);
    else if (parameterID == "is2Pole")
        if (chainSettings.isHighPass == 0)
            if (newValue == 1)
                ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::LPF12);
            else
                ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::LPF24);
        else
            if (newValue == 1)
                ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::HPF12);
            else
                ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::HPF24);
    else if (parameterID == "resonance")
        ladderFilter.setResonance(newValue / 10.f);
    else if (parameterID == "drive")
        ladderFilter.setDrive(newValue);
    else if (parameterID == "cutoff")
        ladderFilter.setCutoffFrequencyHz(newValue);
    else if (parameterID == "volume")
        gain.setGainDecibels(newValue);
}

//==============================================================================
bool MofoFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MofoFilterAudioProcessor::createEditor()
{
    return new MofoFilterAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void MofoFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MofoFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(treeState.state.getType()))
            treeState.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MofoFilterAudioProcessor();
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& treeState)
{
    ChainSettings settings;

    settings.drive = treeState.getRawParameterValue("drive")->load();

    settings.driveAmount = treeState.getRawParameterValue("maxDrive")->load();
    settings.cutoff = treeState.getRawParameterValue("cutoff")->load();
    settings.minCutoff = treeState.getRawParameterValue("minCutoff")->load();

    settings.cutoffAmount = std::pow(2.f, treeState.getRawParameterValue("cutoffAmount")->load() / 2.f);
    settings.resonance = treeState.getRawParameterValue("resonance")->load() / 10.f;
    settings.resonanceAmount = treeState.getRawParameterValue("resonanceAmount")->load() / 10.f;
    settings.cutoffTension = treeState.getRawParameterValue("cutoffTension")->load() + 0.5f;
    if (settings.cutoffTension == 0.0f)
        settings.cutoffTension = 0.01f;
    else if (settings.cutoffTension == 1.f)
        settings.cutoffTension = 0.99f;
    settings.speedTension = treeState.getRawParameterValue("speedTension")->load() + 0.5f;
    if (settings.speedTension == 0.0f)
        settings.speedTension = 0.01f;
    else if (settings.speedTension == 1.f)
        settings.speedTension = 0.99f;
    settings.resonanceTension = treeState.getRawParameterValue("resonanceTension")->load() + 0.5f;
    if (settings.resonanceTension == 0.0f)
        settings.resonanceTension = 0.01f;
    else if (settings.resonanceTension == 1.f)
        settings.resonanceTension = 0.99f;
    settings.driveTension = treeState.getRawParameterValue("driveTension")->load() + 0.5f;
    if (settings.driveTension == 0.0f)
        settings.driveTension = 0.01f;
    else if (settings.driveTension == 1.f)
        settings.driveTension = 0.99f;
    settings.speedAmount = std::pow(2.f,treeState.getRawParameterValue("speedAmount")->load() / 2.f) - 1.f;
    settings.is2Pole = treeState.getRawParameterValue("is2Pole")->load();
    settings.isHighPass = treeState.getRawParameterValue("isHighPass")->load();
    settings.isAuto = treeState.getRawParameterValue("isAuto")->load();
    settings.freqIsUp = treeState.getRawParameterValue("freqIsUp")->load();
    settings.resIsUp = treeState.getRawParameterValue("resIsUp")->load();
    settings.driveIsUp = treeState.getRawParameterValue("driveIsUp")->load();
    settings.speedIsUp = treeState.getRawParameterValue("speedIsUp")->load();
    settings.volume = treeState.getRawParameterValue("volume")->load();
    settings.mix = treeState.getRawParameterValue ("mix")->load ();

    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout MofoFilterAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    juce::StringArray polarChoices = { "4-pole", "2-pole" };
    juce::StringArray filterChoices = { "LP", "HP" };
    juce::StringArray modeChoices = { "Classic", "Auto" };
    juce::StringArray freqDirection = { "Down", "Up" };
    juce::StringArray resDirection = { "Down", "Up" };
    juce::StringArray amountDirection = { "Down", "Up" };
    juce::StringArray driveDirection = { "Down", "Up" };
    juce::StringArray speedDirection = { "Down", "Up" };
    juce::StringArray paramDirection = { "Down", "Up" };

    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "is2Pole", 1 }, "Slope", polarChoices, 1, ""));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "isHighPass", 1 }, "Filter Type", filterChoices, 0, ""));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "isAuto", 1 }, "Cutoff Frequency Mode", modeChoices, 0, ""));                                                                                       
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"drive", 1}, "Drive", juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1), 8));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "volume", 1}, "Volume", juce::NormalisableRange<float> (-30.f, 30.f, 0.1f, 1), -5));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "driveAmount", 1 }, "Drive Envelope Amount", juce::NormalisableRange<float> (0.f, 100.f, 0.1f, 1), 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "cutoff", 1 }, "Classic Mode Cutoff (Hz)", juce::NormalisableRange<float> (20.f, 16000.f, 1.f, 0.25f), 190.f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "minCutoff", 1 }, "Auto Mode Cutoff (Harmonics)", 0.01, 20, 3));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "cutoffAmount", 1 }, "Cutoff Envelope Amount", juce::NormalisableRange<float> (0.f, 10.f, 0.1f, 1), 7));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "resonance", 1 }, "Resonance", juce::NormalisableRange<float> (0.f, 10.f, 0.1f, 1), 7.f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "maxResonance", 1 }, "Resonance Envelope Amount", juce::NormalisableRange<float> (0.f, 10.f, 0.1f, 1), 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "cutoffTension", 1 }, "Cutoff envelope tension", juce::NormalisableRange<float> (-0.5f, 0.5f, 0.01f, 1), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "speedTension", 1 }, "Speed envelope tension", juce::NormalisableRange<float> (-0.5f, 0.5f, 0.01f, 1), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "resonanceTension", 1 }, "Resonance Envelope Tension", juce::NormalisableRange<float> (-0.5f, 0.5f, 0.01f, 1), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "driveTension", 1 }, "Drive Envelope Tension", juce::NormalisableRange<float> (-0.5f, 0.5f, 0.01f, 1), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "speedAmount", 1 }, "Speed Envelope Amount", juce::NormalisableRange<float> (0.f, 10.f, 0.1f, 1), 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "freqIsUp", 1 }, "Cutoff frequency Envelope Direction", freqDirection, 0, ""));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "resIsUp", 1 }, "Resonance Envelope Direction", resDirection, 1, ""));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "speedIsUp", 1 }, "Speed Envelope Direction", speedDirection, 1, ""));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "driveIsUp", 1 }, "Drive Envelope Direction", driveDirection, 1, ""));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "mix", 1 }, "Mix", juce::NormalisableRange<float> (0.f, 1.f, 0.01f), 1));
    ChainSettings settings;
    return layout;
}