/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

    ------------------------------------------------------------------

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

#ifndef __RECORDNODE_H_FB9B1CA7__
#define __RECORDNODE_H_FB9B1CA7__


#include "../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>

#include "GenericProcessor.h"

/**
  
  --UNDER CONSTRUCTION--

  Receives inputs from all processors that want to save their data,
  and writes it to disk using a FileOutputStream.

  Receives a signal from the ControlPanel to begin recording.

  @see GenericProcessor, ControlPanel

*/


class RecordNode : public GenericProcessor
{
public:
	
	RecordNode();
	~RecordNode();
	
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);

	void setParameter (int parameterIndex, float newValue);

	bool enable();
	bool disable();

	float getFreeSpace();
	
private:

	File outputFile;
	FileOutputStream* outputStream;

	bool isRecording;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordNode);

};



#endif  // __RECORDNODE_H_FB9B1CA7__
