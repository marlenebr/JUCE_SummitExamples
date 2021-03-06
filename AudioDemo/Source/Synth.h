/*
  ==============================================================================

   This file is part of the Projucer live coding demo created for
   the JUCE Summit 2015.
   Copyright (c) 2015 - ROLI Ltd.

   Use this code at your own risk & have some fun with C++ live coding!

   This code is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.

  ==============================================================================
*/

#ifndef SYNTH_H_INCLUDED
#define SYNTH_H_INCLUDED


//==============================================================================
/** Our demo synth sound is just a basic sine wave.. */
struct SineWaveSound : public SynthesiserSound
{
    SineWaveSound() {}
    
    bool appliesToNote (int /*midiNoteNumber*/) override        { return true; }
    bool appliesToChannel (int /*midiChannel*/) override        { return true; }
};

//==============================================================================
/** Our demo synth voice just plays a sine wave.. */
struct SineWaveVoice  : public SynthesiserVoice
{
    SineWaveVoice()   : currentAngle (0), angleDelta (0), level (0), tailOff (0)
    {
    }
    
    bool canPlaySound (SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }
    
    void startNote (int midiNoteNumber, float velocity,
                    SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;
        
        double cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double cyclesPerSample = cyclesPerSecond / getSampleRate();
        
        angleDelta = cyclesPerSample * 2.0 * double_Pi;
    }
    
    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.
            
            if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
                // stopNote method could be called more than once.
                tailOff = 1.0;
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..
            
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }
    
    void pitchWheelMoved (int /*newValue*/) override
    {
        // can't be bothered implementing this for the demo!
    }
    
    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override
    {
        // not interested in controllers in this case.
    }
    
    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta != 0.0)
        {
            if (tailOff > 0)
            {
                while (--numSamples >= 0)
                {
                    const float currentSample = (float) (std::sin (currentAngle) * level * tailOff);
                    
                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);
                    
                    currentAngle += angleDelta;
                    ++startSample;
                    
                    tailOff *= 0.99;
                    
                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote();
                        
                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    const float currentSample = (float) (std::sin (currentAngle) * level);
                    
                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);
                    
                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }
    
private:
    double currentAngle, angleDelta, level, tailOff;
};


#endif  // SYNTH_H_INCLUDED
