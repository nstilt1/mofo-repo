#include "PluginProcessor.h"
#include "PluginEditor.h"

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

    float maxDrive = chainSettings.maxDrive;
    float am = chainSettings.amount;
    float maxRes = chainSettings.maxResonance;
    float maxSpeed = chainSettings.maxSpeed;

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

        float ratio;
        if (max != 0)
            ratio = currentVolume / max;
        else
            ratio = 0.000000001f;

        float resRatio = 0.5f;
        if (maxRes != 0.f)
            resRatio = curveConvert (ratio, chainSettings.resShape);

        float driveRatio = 0.5f;
        if (maxDrive != 0)
            driveRatio = curveConvert (ratio, chainSettings.driveShape);

        float speedRatio = 0.5f;
        if (am != 1.f || maxSpeed != 0.f)
        {
            speedRatio = curveConvert (ratio, chainSettings.speedShape);
            ratio = curveConvert (ratio, chainSettings.shape);
        }

        float resonance = chainSettings.resonance;

        if (maxRes != 0.f)
        {
            if (resonance + maxRes > 1) maxRes = 1;
            else maxRes = resonance + maxRes;
            if (chainSettings.resIsUp)
                newRes = maxRes - ((maxRes - resonance) * resRatio);
            else
                newRes = resonance + ((maxRes - resonance) * resRatio);
        }
        else
            newRes = resonance;


        // So, this part is kind of hard to conceptualize and describe.
        // The envelope that is based on the volume/max ratio and the "cutoff amount" knob is applied to the cutoff frequency. 
        // The "speed" is an envelope which is (still) based on the volume/max ratio, but it is applied to the "cutoff amount". 
        //
        // I wasn't sure what to call this, so I refer to it as "speed". I'm sure there's a more technical term.
        float amount = chainSettings.amount;
        float maxAmount = chainSettings.maxSpeed;
        if (maxAmount != 0)
            if (maxAmount != 0)
            {
                if (amount + maxAmount > 32) maxAmount = 32;
                else maxAmount = amount + maxAmount;
                if (treeState.getRawParameterValue ("speedIsUp")->load ())
                    amount = maxAmount - ((maxAmount - amount) * speedRatio);
                else amount = amount + ((maxAmount - amount) * speedRatio);
            }

        float drive = chainSettings.drive;

        if (maxDrive != 0)
        {
            if (drive + maxDrive > 100) maxDrive = 100;
            else maxDrive = drive + maxDrive;
            if (chainSettings.driveIsUp)
                newDrive = maxDrive - ((maxDrive - drive) * driveRatio);
            else 
                newDrive = drive + ((maxDrive - drive) * driveRatio);
        }
        else newDrive = drive;

        if (amount != 1.f)
            if (chainSettings.isAuto == 1)
            {
                float minC = chainSettings.minCutoff * frequency;
                float maxC = minC * amount;

                if (maxC > 18000) maxC = 18000;
                float difference = maxC - minC;
                if (chainSettings.freqIsUp == 1)
                    newFreq = maxC - (difference * ratio);
                else
                    newFreq = (difference * ratio) + minC;
            }
            else
            {
                float cut = chainSettings.cutoff;
                float am = amount * cut;;
                if (am > 18000)
                {
                    am = 18000;
                }
                float difference = am - cut;

                if (chainSettings.freqIsUp)
                    newFreq = am - (difference * ratio);
                else
                    newFreq = (difference * ratio) + cut;
            }
        else
            if (chainSettings.isAuto == 1)
                newFreq = chainSettings.minCutoff * frequency;
            else
                newFreq = chainSettings.cutoff;

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

    settings.maxDrive = treeState.getRawParameterValue("maxDrive")->load();
    settings.cutoff = treeState.getRawParameterValue("cutoff")->load();
    settings.minCutoff = treeState.getRawParameterValue("minCutoff")->load();


    settings.amount = std::pow(2.f, treeState.getRawParameterValue("amount")->load() / 2.f);
    settings.resonance = treeState.getRawParameterValue("resonance")->load() / 10.f;
    settings.maxResonance = treeState.getRawParameterValue("maxResonance")->load() / 10.f;
    settings.shape = treeState.getRawParameterValue("shape")->load() + 0.5f;
    if (settings.shape == 0.0f)
        settings.shape = 0.01f;
    else if (settings.shape == 1.f)
        settings.shape = 0.99f;
    settings.speedShape = treeState.getRawParameterValue("speedShape")->load() + 0.5f;
    if (settings.speedShape == 0.0f)
        settings.speedShape = 0.01f;
    else if (settings.speedShape == 1.f)
        settings.speedShape = 0.99f; 
    settings.resShape = treeState.getRawParameterValue("resShape")->load() + 0.5f;
    if (settings.resShape == 0.0f)
        settings.resShape = 0.01f;
    else if (settings.resShape == 1.f)
        settings.resShape = 0.99f;
    settings.driveShape = treeState.getRawParameterValue("driveShape")->load() + 0.5f;
    if (settings.driveShape == 0.0f)
        settings.driveShape = 0.01f;
    else if (settings.driveShape == 1.f)
        settings.driveShape = 0.99f;
    settings.maxSpeed = std::pow(2.f,treeState.getRawParameterValue("maxSpeed")->load() / 2.f) - 1.f;
    settings.is2Pole = treeState.getRawParameterValue("is2Pole")->load();
    settings.isHighPass = treeState.getRawParameterValue("isHighPass")->load();
    settings.isAuto = treeState.getRawParameterValue("isAuto")->load();
    settings.freqIsUp = treeState.getRawParameterValue("freqIsUp")->load();
    settings.resIsUp = treeState.getRawParameterValue("resIsUp")->load();
    settings.amountIsUp = treeState.getRawParameterValue("amountIsUp")->load();
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
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "maxDrive", 1 }, "Drive Follower", juce::NormalisableRange<float> (0.f, 100.f, 0.1f, 1), 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "cutoff", 1 }, "Classic Mode Cutoff (Hz)", juce::NormalisableRange<float> (20.f, 16000.f, 1.f, 0.25f), 190.f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "minCutoff", 1 }, "Auto Mode Cutoff (Harmonics)", 0.01, 20, 3));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "amount", 1 }, "Cutoff Envelope Amount", juce::NormalisableRange<float> (0.f, 10.f, 0.1f, 1), 7));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "resonance", 1 }, "Resonance", juce::NormalisableRange<float> (0.f, 10.f, 0.1f, 1), 7.f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "maxResonance", 1 }, "Resonance Envelope Amount", juce::NormalisableRange<float> (0.f, 10.f, 0.1f, 1), 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "shape", 1 }, "Shape", juce::NormalisableRange<float> (-0.5f, 0.5f, 0.01f, 1), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "speedShape", 1 }, "Shape", juce::NormalisableRange<float> (-0.5f, 0.5f, 0.01f, 1), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "resShape", 1 }, "Shape", juce::NormalisableRange<float> (-0.5f, 0.5f, 0.01f, 1), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "driveShape", 1 }, "Shape", juce::NormalisableRange<float> (-0.5f, 0.5f, 0.01f, 1), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "maxSpeed", 1 }, "Speed Envelope Amount", juce::NormalisableRange<float> (0.f, 10.f, 0.1f, 1), 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "freqIsUp", 1 }, "Frequency Envelope Direction", freqDirection, 0, ""));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "resIsUp", 1 }, "Resonance Envelope Direction", resDirection, 1, ""));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "amountIsUp", 1 }, "Amount Envelope Direction", amountDirection, 1, ""));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "speedIsUp", 1 }, "Speed Envelope Direction", speedDirection, 1, ""));
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{ "driveIsUp", 1 }, "Drive Envelope Direction", driveDirection, 1, ""));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID{ "mix", 1 }, "Mix", juce::NormalisableRange<float> (0.f, 1.f, 0.01f), 1));
    ChainSettings settings;
    return layout;
}


/*/

juce::AudioProcessorValueTreeState::ParameterLayout Mothereffer1AudioProcessor::createParameters()
{
    //juce::AudioParameterBool* isHighPass;
    //bool previousIsHighPass;

    //juce::AudioParameterFloat* harmonicThreshold;
    //bool previousHarmonicThreshold;

    //juce::AudioParameterFloat* threshold;
    //float previousThreshold;

    //juce::AudioParameterFloat* drive;
    //float previousDrive;

    //juce::AudioParameterFloat* mix;
    //float previousMix;

    //juce::AudioParameterFloat* cutoff;
    //float previousCutoff;

    //juce::AudioParameterFloat* minCutoff;
    //float previousMinCutoff;

    //juce::AudioParameterFloat* maxCutoff;
    //float previousMaxCutoff;

    //juce::AudioParameterFloat* amount;
    //float previousAmount;

    //juce::AudioParameterFloat* resonance;
    //float previousResonance;

    //juce::AudioParameterFloat* speed;
    //float previousSpeed;

    //juce::AudioParameterBool* is2Pole;
    //bool previousIs2Pole;

    //juce::AudioParameterBool* freqIsUp;
    //bool previousFreqIsUp;

    //juce::AudioParameterBool* resIsUp;
    //bool previousResIsUp;


    juce::StringArray polarChoices = { "4-pole", "2-pole" };
    juce::StringArray filterChoices = { "LP", "HP" };
    juce::StringArray modeChoices = { "Classic", "Auto" };
    juce::StringArray freqDirection = { "Down", "Up" };
    juce::StringArray resDirection = { "Down", "Up" };
    juce::StringArray amountDirection = { "Down", "Up" };
    juce::StringArray driveDirection = { "Down", "Up" };
    juce::StringArray speedDirection = { "Down", "Up" };



        
    


    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params; 


    params.push_back(std::make_unique<juce::AudioParameterChoice>("is2Pole", "Half/Full", polarChoices, 1, "Half/Full"));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("isHighPass", "Filter Type", filterChoices, 0, "LP/HP"));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("isAuto", "Frequency Mode", filterChoices, 0, "Frequency Mode"));


    params.push_back(std::make_unique<juce::AudioParameterFloat>("harmonicThreshold", "Harmonic Threshold", 0, 1, 0.88));

    
    params.push_back(std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", 0, 1, 0.3));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 0.01, 25, 4));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("maxDrive", "Drive EF Amount", 0, 25, 4));
    

    //params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0, 1, 1));
    //params.push_back(std::make_unique<juce::AudioParameterFloat>("maxMix", "Max Mix", 0, 1, 1));

    
    params.push_back(std::make_unique<juce::AudioParameterFloat>("cutoff", "Cutoff", juce::NormalisableRange<float>(20.f, 16000.f, 1.f, 0.25f),250.f));

    //params.push_back(std::make_unique<juce::AudioParameterFloat>("cutoff", "Cutoff", 20, 16000, 250));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("minCutoff", "Minimum Cutoff", 0.01, 10, 3));
    

    //params.push_back(std::make_unique<juce::AudioParameterFloat>("maxCutoff", "Maximum Cutoff", 1, 10, 8));

    
    params.push_back(std::make_unique<juce::AudioParameterFloat>("amount", "Amount", 1, 32, 32));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("maxAmount", "Amount EF Amt.", 0, 9, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("resonance", "Resonance", 0, 10, 6));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("maxResonance", "Resonance EF Amt.", 0, 10, 6));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("speed", "Speed", 0, 1, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("maxSpeed", "Speed EF Amt.", 0, 1, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("freqIsUp", "Frequency EF Direction", freqDirection, 1, "Frequency EF Direction"));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("resIsUp", "Resonance EF Direction", resDirection, 1, "Frequency EF Direction"));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("amountIsUp", "Amount EF Direction", amountDirection, 1, "Amount EF Direction"));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("speedIsUp", "Speed EF Direction", speedDirection, 1, "Speed EF Direction"));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("driveIsUp", "Drive EF Direction", driveDirection, 1, "Drive EF Direction"));
    

    //params.push_back(std::make_unique<juce::AudioParameterBool>("mixIsUp", "Mix Follower Direction", true, "Mix Follower Direction"));
    //params.push_back(std::make_unique<juce::AudioParameter>("", "", , , ));




    return { params.begin(), params.end() };

    
}
//*/