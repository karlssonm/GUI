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

#ifndef __SPLITTEREDITOR_H_33F644A8__
#define __SPLITTEREDITOR_H_33F644A8__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

class FilterViewport;

// class PipelineSelectorButton : public DrawableButton
// {
// 	public:
// 		PipelineSelectorButton();
// 		~PipelineSelectorButton();	
// };

class SplitterEditor : public GenericEditor,
					   public Button::Listener
{
public:
	SplitterEditor (GenericProcessor* parentNode);
	virtual ~SplitterEditor();

	void buttonClicked (Button* button);

private:	
	
	ImageButton* pipelineSelectorA;
	ImageButton* pipelineSelectorB;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SplitterEditor);

};


#endif  // __SPLITTEREDITOR_H_33F644A8__
