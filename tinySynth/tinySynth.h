/*
==============================================================================
tinySynth.h - header file for tinySynth.cpp
Copyright (C) 2016 Aldo Ciaccini

Description:
This module can be considered as the baseline Synthetizer of the tinySynth project.
It extends the audio processing capabilities implemented inside the 
tinySynthProcessor::processBlock().
Being based on the JUCE Synthetizer, tinySynth is able to generate a sound 
composed by a configurable number of polifony voices.
the tinySynth::renderNextBlock() handles the audio generation process for 
the incoming MIDI notes according to the number of active voices.  
Each voice produces an audio output generated by three indipendent Oscillators,
and according to:
- Oscillator parameters: waveform type, frequency, octave and gain
- LFO effects: vibrato and tremolo effects
- ADSR modifications: attach, decay, release and sustain
- Filters parameters: filtering and routing modifications
- Noise level
==============================================================================
Copyright:
The tinySynth architecture was inspired by the vstSynth project developed 
by Gabriel Olochwoszcz in 2012 under the terms of the GNU Public Lic. v.2.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
==============================================================================
*/

#include "tinySynthDoc.h"
#include "tinySynthStkIncludes.h"
#include "tinySynthOscillator.h"
#include "tinySynthFilter.h"
#include "tinySynthLFO.h"


/** \class tinySynthSound
 *  \brief This class implements the "Sound" element of the Synthetizer used inside the tinySynth plugin.
 *  \details The tinySynth plugin uses the JUCE Synthesiser class to generate audio. \n
 * JUCE Synthetizer needs Sounds and Voices classes before to be used.\n
 * A synthesiser can contain one or more sounds, and a sound can choose which midi notes and channels can trigger it.\n
 * The tinySynthSound class is configured to play all the MIDI notes and channels.\n
 * The tinySynthSound is a passive class that just describes what the sound is - the actual audio rendering for a sound is done by a tinySynthVoice.\n
 * More voices can be added to the sound for poliphony functionalities.\n
 */
class tinySynthSound : public SynthesiserSound
{
public:
    tinySynthSound(); /**< \brief Class constructor*/  
    bool appliesToNote(const int midiNoteNumber) {return true;}; /**< \brief return a fixed true. all MIDI notes are associated*/ 
    bool appliesToChannel(const int midiChannel) {return true;}; /**< \brief return a fixed true. the sound is triggered by any MIDI events on every channel*/ 
}; 

/** \class tinySynthVoice
 *  \brief This class implements the "Voice" element of the Synthetizer used inside the tinySynth plugin.
 *  \details The tinySynth plugin uses the JUCE Synthesiser class to generate audio. \n
 * JUCE Synthetizer needs Sounds and Voices classes before to be used.\n
 * The tinySynthSound is a passive class that just describes what the sound is - the actual audio rendering for a sound is done by the tinySynthVoice.\n
 * The tinySynth plugin supports one sound with poliphony functionalities generating up to 8 voices concurrently.
 * Each voice is handled by a dedicated renderNextBlock().
 * The renderNextBlock() extends the audio processing capabilities implemented inside the tinySynthProcessor::processBlock(). \n
 * Each voice produces an audio output generated by three indipendent Oscillators, and according to:\n
 * - oscillator parameters: waveform type, frequency, octave and gain \n 
 * - LFO effects: vibrato and tremolo effects \n
 * - ADSR modifications: attach, decay, release and sustain \n
 * - Filters parameters: filtering and routing modifications \n
 * - Noise level \n <P>
 */
class tinySynthVoice : public SynthesiserVoice
{
public:
	tinySynthVoice(float* parameters);  /**< \brief Class constructor*/
    ~tinySynthVoice(); /**< \brief Class destructor*/
    float getParameter(int index); /**< \brief used to get local parameters*/ 
	bool canPlaySound (SynthesiserSound* sound); /**< \brief Return true if this voice is capable of playing the given sound*/ 
	void startNote (int midiNoteNumber,  float velocity, SynthesiserSound* sound,  int currrentPitchWheelPosition); /**< \brief called to start a new note during the rendering callback*/ 
	void stopNote ( bool allowTailOff); /**< \brief called to stop a note during the rendering callback*/
	void pitchWheelMoved (int newValue); /**< \brief called to let the voice know that the pitch wheel has been moved*/
	void controllerMoved (int controllerNumber, int newValue); /**< \brief called to let the voice know that a MIDI control has been moved*/
	void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples); /**< \brief render the next block of data for this voice processing process the region of the buffer between the start and (startSample+numSamples) */
	void setOscillatorParams(float newGain1, float newGain2, float newGain3,
		int newWaveform1,int newWaveform2,int newWaveform3,
		int newOctave1, int newOctave2, int newOctave3,
		int newSemiTone1, int newSemiTone2, int newSemiTone3); /**< \brief set the gain, the type of waveform and the pitch for the 3 oscillators*/
	
private: //Private Data, helper methods etc.
	
	/* 
	enum Parameters is an enumerative index used to get or set the float values of parameters[] array.
	Each enumerative is associated with a correspondent UI object and its naming convention follows the rules:
	the intial part of the name for both enumerative and UI object is the same
	enumerative's name ends with "Param"
	UI name ends according to object type (i.e "Silder" for sliders, "Box" for boxes..)
	*/
	enum Parameters 
    {
        // oscillators
        osc1WaveParam=0, osc1OctaveParam, osc1LevelParam, osc1LfoParam, osc1EnvParam, osc1OnParam,
		osc2WaveParam, osc2OctaveParam, osc2LevelParam, osc2LfoParam, osc2EnvParam, osc2OnParam,
		osc3WaveParam, osc3OctaveParam, osc3LevelParam, osc3LfoParam, osc3EnvParam, osc3OnParam,

		// envelopes
        adsr1AttackParam, adsr1DecayParam, adsr1SustainParam, adsr1ReleaseParam,
        adsr2AttackParam, adsr2DecayParam, adsr2SustainParam, adsr2ReleaseParam,
		adsr3AttackParam, adsr3DecayParam, adsr3SustainParam, adsr3ReleaseParam,

        // LFOs
        lfo1DestParam, lfo1WaveParam, lfo1FreqParam, lfo1DevParam,
		lfo2DestParam, lfo2WaveParam, lfo2FreqParam, lfo2DevParam,
        
        // Filters 
        filter1TypeParam, filter1CutoffParam, filter1ResonanceParam, filter1EnvModDepthParam, filter1EnvParam,
		filter2TypeParam, filter2CutoffParam, filter2ResonanceParam, filter2EnvModDepthParam, filter2EnvParam,
        
        // delay
        delayTimeParam, delayFeedbackParam, delayGainParam, delayOnParam, 
        
        // output
        noiseParam, driveParam,outputGainParam,
		
		synthVoiceParam,
		
		// From here to be ordered by functionality
		
		//filter 
		filterSequenceParam, filter1LfoParam, filter2LfoParam,

		// reverb
		reverbDryWetParam, reverbSizeParam, reverbDampParam, reverbOnParam,

		//semitones
		osc1SemiToneParam, osc2SemiToneParam, osc3SemiToneParam,
        totalNumParams
       
    };
	
    
	/*
	The localParameters is a pointer to the parameters[] array allocated by tinySynthProcessor class. 
	The values of UI objects are mantenied hooked with the ones in parameters[] by the tinySynthEditor class.
	The parameters values are used by SynthesiserVoice with the aim to controls its internal resources. 
	*/
	float* localParameters; 
	
	// Internal resources
	tinySynthOscillator oscillator1, oscillator2, oscillator3;
    stk::ADSR envelope1, envelope2, envelope3;
	tinySynthFilter hpeq1Filter, hpeq2Filter;
	
	double freq, keyLevel;
	LFO lfo0, lfo1, lfo2;

	float osc1Level, osc2Level, osc3Level, osc1Octave, osc2Octave, osc3Octave, osc1Wave, osc2Wave, osc3Wave, osc1SemiTone, osc2SemiTone, osc3SemiTone;
	float filter1Type, filter2Type, filter1EnvModDepth, filter2EnvModDepth;

};