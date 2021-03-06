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

#ifndef __SOURCENODE_H_DCE798F1__
#define __SOURCENODE_H_DCE798F1__

#include "../../JuceLibraryCode/JuceHeader.h"
#include <ftdi.h>
#include <stdio.h>
#include "DataThreads/DataBuffer.h"
#include "DataThreads/IntanThread.h"
#include "DataThreads/FPGAThread.h"
#include "DataThreads/FileReaderThread.h"
#include "GenericProcessor.h"
#include "../UI/UIComponent.h"

/**

  Creates and controls a thread for reading data from external sources.

  @see GenericProcessor, SourceNodeEditor, DataThread, IntanThread

*/

class SourceNode : public GenericProcessor,
				   public Timer

{
public:
	
	// real member functions:
	SourceNode(const String& name);
	~SourceNode();

	void enabledState(bool t);

	//void setName(const String name_);
	
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);

	void setParameter (int parameterIndex, float newValue);

	void setConfiguration(Configuration* cf);

	float getSampleRate();
	float getDefaultSampleRate();
	int getDefaultNumOutputs();

	// void setSourceNode(GenericProcessor* sn);
	// void setDestNode(GenericProcessor* dn);

	AudioProcessorEditor* createEditor();
	bool hasEditor() const {return true;}

	bool enable();
	bool disable();

	bool isReady();

	bool isSource() {return true;}

	void acquisitionStopped();
	
private:

	int sourceCheckInterval;

	bool wasDisabled;

	//const String name;
	void timerCallback();

	DataThread* dataThread;
	DataBuffer* inputBuffer;

	int* numSamplesInThisBuffer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceNode);

};


#endif  // __SOURCENODE_H_DCE798F1__

