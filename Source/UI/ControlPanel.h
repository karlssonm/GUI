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

#ifndef __CONTROLPANEL_H_AD81E528__
#define __CONTROLPANEL_H_AD81E528__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Audio/AudioComponent.h"
#include "../Processors/Editors/AudioEditor.h"
#include "../Processors/ProcessorGraph.h"
#include "../Processors/RecordNode.h"
#include "CustomLookAndFeel.h"

#include "../OpenGL.h"

//------------------------------------------------------------------------

/**
  
  Displays information and provides buttons to control acquistion and recording.

  The ControlPanel is located along the top of the application window.

  @see UIComponent

*/


class PlayButton : public DrawableButton
{
	public:
		PlayButton();
		~PlayButton();
};

class RecordButton : public DrawableButton
{
	public:
		RecordButton();
		~RecordButton();
};

class CPUMeter : public Label
{
	public:
		CPUMeter();
		~CPUMeter();

		void updateCPU(float usage);

		void paint (Graphics& g);
	
	private:

		Font font;

		float cpu;
		float lastCpu;

};

class DiskSpaceMeter : public Component
{
public:
	DiskSpaceMeter();
	~DiskSpaceMeter();

	void updateDiskSpace(float percent);

	void paint (Graphics& g);

private:

	Font font;

	float diskFree;
	ProcessorGraph* graph;
	
};

class Clock : public OpenGLComponent
{
	public:
		Clock();
		~Clock();

		void newOpenGLContextCreated();
		void renderOpenGL();

		void start();
		void stop();

		void startRecording();
		void stopRecording();

	private:

		void drawTime();

		int64 lastTime;

		int64 totalTime;
		int64 totalRecordTime;

		bool isRunning;
		bool isRecording;

		FTPixmapFont* font;
};



class ControlPanel : public Component, 
					 public Button::Listener,
					 public ActionListener,
					 public Timer

{
public:
	ControlPanel(ProcessorGraph* graph, AudioComponent* audio);
	~ControlPanel();

	void disableCallbacks();

private:	
	PlayButton* playButton;
	RecordButton* recordButton;
	Clock* masterClock;
	CPUMeter* cpuMeter;
	DiskSpaceMeter* diskMeter;
	AudioComponent* audio;
	ProcessorGraph* graph;
	AudioEditor* audioEditor;

	void paint(Graphics& g);

	void resized();
	void buttonClicked(Button* button);

	void actionListenerCallback(const String& msg);

	void timerCallback();

	Font font;

};


#endif  // __CONTROLPANEL_H_AD81E528__
