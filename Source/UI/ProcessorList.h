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

#ifndef __PROCESSORLIST_H_C3A661E9__
#define __PROCESSORLIST_H_C3A661E9__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/Visualization/OpenGLCanvas.h"
#include "../AccessClass.h"

/**
  
  Holds a list of processors that can be used to build the signal
  chain.

  Must be manually updated every time a new processor is created,
  and the names must match those recognized by the ProcessorGraph.

  @see ProcessorViewport, ProcessorGraph

*/

class ProcessorListItem;
class UIComponent;

class ProcessorList : public OpenGLCanvas,
				   public DragAndDropContainer,
				   public AccessClass

{
public:

	ProcessorList();
	~ProcessorList();
	void newOpenGLContextCreated();
	void renderOpenGL();

	//void setUIComponent(UIComponent* ui) {UI = ui;}


	bool isOpen();

private:

	void drawItems();
	void drawItem(ProcessorListItem*);
	void drawItemName(ProcessorListItem*);
	void drawButton(bool isOpen);

	ProcessorListItem* getListItemForYPos(int y);

	void setViewport(bool);

	int getTotalHeight();
	void clearSelectionState();

	bool isDragging;
	int totalHeight, itemHeight, subItemHeight;
	int xBuffer, yBuffer;

	//UIComponent* UI;

	String category;
	
	void resized();
	void mouseDown(const MouseEvent& e);
	void mouseDrag(const MouseEvent& e);
	void mouseMove(const MouseEvent& e);
	void mouseUp(const MouseEvent& e);
	void mouseWheelMove(const MouseEvent&, float, float);

	ProcessorListItem* baseItem;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorList);	

};

class ProcessorListItem
{
public:
	ProcessorListItem(const String& name);
	~ProcessorListItem();

	int getNumSubItems();
	ProcessorListItem* getSubItem (int index);

	void clearSubItems();
	void addSubItem (ProcessorListItem* newItem);
	void removeSubItem (int index);
	bool hasSubItems();

	bool isOpen();
	void setOpen(bool);
	bool isSelected() {return selected;}
	void setSelected(bool b) {selected = b;}

	void reverseOpenState() {open = !open;}

	const String& getName();
	const String& getParentName();
	void setParentName(const String& name);

	Colour color;

private:

	bool selected;
	bool open;
	const String name;
	String parentName;
	OwnedArray<ProcessorListItem> subItems;
	
};

#endif  // __PROCESSORLIST_H_C3A661E9__
