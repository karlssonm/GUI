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


#include "ProcessorList.h"
#include <stdio.h>

#include "UIComponent.h"


ProcessorList::ProcessorList() : isDragging(false),
                           itemHeight(32),
                           subItemHeight(22),
                           totalHeight(800),
                           xBuffer(1),
                           yBuffer(1)
{

	ProcessorListItem* sources = new ProcessorListItem("Sources");
	sources->addSubItem(new ProcessorListItem("Intan Demo Board"));
	sources->addSubItem(new ProcessorListItem("Signal Generator"));
	//sources->addSubItem(new ProcessorListItem("Custom FPGA"));
	//sources->addSubItem(new ProcessorListItem("File Reader"));
	sources->addSubItem(new ProcessorListItem("Event Generator"));

	ProcessorListItem* filters = new ProcessorListItem("Filters");
	filters->addSubItem(new ProcessorListItem("Bandpass Filter"));
	//filters->addSubItem(new ProcessorListItem("Resampler"));
	//filters->addSubItem(new ProcessorListItem("Spike Detector"));

	ProcessorListItem* sinks = new ProcessorListItem("Sinks");
	sinks->addSubItem(new ProcessorListItem("LFP Viewer"));
	//sinks->addSubItem(new ProcessorListItem("Spike Display"));
	sinks->addSubItem(new ProcessorListItem("WiFi Output"));

	//ProcessorListItem* utilities = new ProcessorListItem("Utilities");
	//utilities->addSubItem(new ProcessorListItem("Splitter"));
	//utilities->addSubItem(new ProcessorListItem("Merger"));

	baseItem = new ProcessorListItem("Processors");
	baseItem->addSubItem(sources);
	baseItem->addSubItem(filters);
	baseItem->addSubItem(sinks);
	//baseItem->addSubItem(utilities);

	// set parent names / colors
	baseItem->setParentName("Processors");

	for (int n = 0; n < baseItem->getNumSubItems(); n++)
	{

		const String category = baseItem->getSubItem(n)->getName();
		baseItem->getSubItem(n)->setParentName(category);

			for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
			{

				baseItem->getSubItem(n)->getSubItem(m)->setParentName(category);// = category;

			}
			
	}

}

ProcessorList::~ProcessorList()
{
	deleteAndZero(baseItem);
}

bool ProcessorList::isOpen()
{
	return baseItem->isOpen();
}


void ProcessorList::newOpenGLContextCreated()
{

	setUp2DCanvas();
	activateAntiAliasing();

	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
	resized();

}

void ProcessorList::renderOpenGL()
{
	
	glClear(GL_COLOR_BUFFER_BIT); // clear buffers to preset values
	drawItems();
	drawScrollBars();
}


void ProcessorList::drawItems()
{
	int itemNum = 0;
	totalHeight = yBuffer;

	setViewport(true);

	category = baseItem->getName();

	drawItem(baseItem);

	if (baseItem->isOpen())
	{
		for (int n = 0; n < baseItem->getNumSubItems(); n++)
		{
			setViewport(baseItem->hasSubItems());
			category = baseItem->getSubItem(n)->getName();
			drawItem(baseItem->getSubItem(n));
			
			if (baseItem->getSubItem(n)->isOpen())
			{
				for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
				{

					setViewport(baseItem->
								 getSubItem(n)->
								 getSubItem(m)->
								 hasSubItems());
					drawItem(baseItem->getSubItem(n)->getSubItem(m));

				}
			}			
		}
	}

}

void ProcessorList::drawItem(ProcessorListItem* item)
{
	
	glColor4f(item->color.getFloatRed(),
		      item->color.getFloatGreen(),
		      item->color.getFloatBlue(),
		      1.0f);

	glBegin(GL_POLYGON);
	glVertex2f(0,0);
	glVertex2f(1,0);
	glVertex2f(1,1);
	glVertex2f(0,1);
	glEnd();

	drawItemName(item);

	if (item->hasSubItems())
	{
		drawButton(item->isOpen());
	}

}

void ProcessorList::drawItemName(ProcessorListItem* item)
{

	String name; 

	glColor4f(1.0f,1.0f,1.0f,1.0f);

	float offsetX, offsetY;

	if (item->getNumSubItems() == 0) 
	{
		if (item->isSelected())
		{
			glRasterPos2f(9.0/getWidth(),0.72);
			getFont(String("cpmono-plain"))->FaceSize(15);
			getFont(String("cpmono-plain"))->Render(">");
		}

		name = item->getName();

		offsetX = 20.0f;

		offsetY = 0.72f;
	}
	else {
		name = item->getName().toUpperCase();
		offsetX = 5.0f;
		offsetY = 0.75f;
	}

	
	glRasterPos2f(offsetX/getWidth(),offsetY);

	if (item->getNumSubItems() == 0) {
		getFont(String("cpmono-plain"))->FaceSize(15);
		getFont(String("cpmono-plain"))->Render(name);
	} else {
		getFont(String("cpmono-light"))->FaceSize(23);
		getFont(String("cpmono-light"))->Render(name);
	}
}

void ProcessorList::drawButton(bool isOpen)
{
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);

	if (isOpen)
	{
		glVertex2f(0.875,0.35);
		glVertex2f(0.9,0.65);
	} else {
		glVertex2f(0.925,0.65);
		glVertex2f(0.875,0.5);
	}
	glVertex2f(0.925,0.35);
	glEnd();

}

void ProcessorList::clearSelectionState()
{
	baseItem->setSelected(false);

	for (int n = 0; n < baseItem->getNumSubItems(); n++)
	{
		baseItem->getSubItem(n)->setSelected(false);

		for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
		{
			baseItem->getSubItem(n)->getSubItem(m)->setSelected(false);
		}
	}
}

ProcessorListItem* ProcessorList::getListItemForYPos(int y)
{
	int bottom = (yBuffer + itemHeight) - getScrollAmount();

	//std::cout << "Bottom: " << bottom << std::endl;
	//std::cout << "Y coordinate: " << y << std::endl;

	if (y < bottom)
	{
		return baseItem;

	} else {
		
		if (baseItem->isOpen())
		{
		for (int n = 0; n < baseItem->getNumSubItems(); n++)
		{
			bottom += (yBuffer + itemHeight);

			if (y < bottom)
			{
				return baseItem->getSubItem(n);
			}
				
			if (baseItem->getSubItem(n)->isOpen())
				{
					for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
					{
						bottom += (yBuffer + subItemHeight);

						if (y < bottom)
						{
							return baseItem->getSubItem(n)->getSubItem(m);
						}

					}
				}			
			}
		}

	}

	return 0;

}

void ProcessorList::setViewport(bool hasSubItems)
{

	int height;

	if (hasSubItems)
	{
		height = itemHeight;
	} else {
		height = subItemHeight;
	}

	glViewport(xBuffer,
			   getHeight()-(totalHeight) - height + getScrollAmount(),
	           getWidth()-2*xBuffer,
	           height);

	totalHeight += yBuffer + height;
}

int ProcessorList::getTotalHeight()
{
 	return totalHeight;
}

void ProcessorList::resized() {canvasWasResized();}

void ProcessorList::mouseDown(const MouseEvent& e) 
{

	isDragging = false;

	Point<int> pos = e.getPosition();
	int xcoord = pos.getX();
	int ycoord = pos.getY();

	//std::cout << xcoord << " " << ycoord << std::endl;

	ProcessorListItem* fli = getListItemForYPos(ycoord);

	if (fli != 0) 
	{
		//std::cout << "Selecting: " << fli->getName() << std::endl;
		if (!fli->hasSubItems()){
			clearSelectionState();
			fli->setSelected(true);
		}
			
	} else {
		//std::cout << "No selection." << std::endl;
	}

	if (fli != 0) {
		if (xcoord < getWidth() - getScrollBarWidth())
		{
			fli->reverseOpenState();
		}

		if (fli == baseItem)
		{
			if (fli->isOpen()) {
				getUIComponent()->childComponentChanged();
			}
			else
			{
				getUIComponent()->childComponentChanged();
				//setBounds(0,0,225,itemHeight + 2*yBuffer); 
				totalHeight = itemHeight + 2*yBuffer;
			}
			
		}
	}

	mouseDownInCanvas(e);

	repaint();
}

void ProcessorList::mouseDrag(const MouseEvent& e) 
{

	if (e.getMouseDownX() < getWidth()-getScrollBarWidth() && !(isDragging))
	{

		ProcessorListItem* fli = getListItemForYPos(e.getMouseDownY());

		if (fli != 0)
		{

			if (!fli->hasSubItems())
			{
				isDragging = true;

				String b = fli->getParentName();
				b += "/";
				b += fli->getName();

				const String dragDescription = b;

				//std::cout << dragDescription << std::endl;

				if (dragDescription.isNotEmpty())
				{
					DragAndDropContainer* const dragContainer
						= DragAndDropContainer::findParentDragContainerFor (this);

					if (dragContainer != 0)
					{
						//pos.setSize (pos.getWidth(), 10);

						Image dragImage (Image::ARGB, 100, 15, true);

						Graphics g(dragImage);
						g.setColour (fli->color);
						g.fillAll();
						g.setColour(Colours::white);
						g.setFont(14);
						g.drawSingleLineText(fli->getName(),10,12);//,75,15,Justification::centredRight,true);

						dragImage.multiplyAllAlphas(0.6f);

						Point<int> imageOffset (20,10);
						dragContainer->startDragging(dragDescription, this,
											         dragImage, true, &imageOffset);
					}
				}
			}
		}
	}

	mouseDragInCanvas(e);
}

void ProcessorList::mouseMove(const MouseEvent& e) {mouseMoveInCanvas(e);}
void ProcessorList::mouseUp(const MouseEvent& e) 	{mouseUpInCanvas(e);}
void ProcessorList::mouseWheelMove(const MouseEvent& e, float a, float b) {mouseWheelMoveInCanvas(e,a,b);}


ProcessorListItem::ProcessorListItem(const String& name_) : name(name_), open(true), selected(false)
{
}

ProcessorListItem::~ProcessorListItem()
{ }

bool ProcessorListItem::hasSubItems()
{
	if (subItems.size() > 0)
	{
		return true;
	} else {
		return false;
	}
}

int ProcessorListItem::getNumSubItems()
{
	return subItems.size();
}

ProcessorListItem* ProcessorListItem::getSubItem (int index)
{
	return subItems[index];
}

void ProcessorListItem::clearSubItems()
{
	subItems.clear();
}

void ProcessorListItem::addSubItem (ProcessorListItem* newItem)
{
	subItems.add(newItem);
}

void ProcessorListItem::removeSubItem (int index)
{
	subItems.remove(index);
}

bool ProcessorListItem::isOpen()
{
	return open;
}

void ProcessorListItem::setOpen(bool t)
{
	open = t;
}

const String& ProcessorListItem::getName()
{
	return name;
}


const String& ProcessorListItem::getParentName()
{
	return parentName;
}

void ProcessorListItem::setParentName(const String& name)
{
	parentName = name;

	if (parentName.equalsIgnoreCase("Processors"))
	{
		color = Colour(59, 59, 59);

	} else if (parentName.equalsIgnoreCase("Filters"))
	{
		color = Colour(255, 89, 0);
	} else if (parentName.equalsIgnoreCase("Sinks"))
	{
		color = Colour(255, 149, 0);
	} else if (parentName.equalsIgnoreCase("Sources"))
	{
		color = Colour(255, 0, 0);

	} else {
		color = Colour(90, 80, 80);
	}
}

	// Blue slate:
	// if (parentName.equalsIgnoreCase("Processors"))
	// {
	// 	color = Colour(59, 59, 59);
	// } else if (parentName.equalsIgnoreCase("Filters"))
	// {
	// 	color = Colour(82, 101, 163);
	// } else if (parentName.equalsIgnoreCase("Sinks"))
	// {
	// 	color = Colour(48, 61, 102);
	// } else if (parentName.equalsIgnoreCase("Sources"))
	// {
	// 	color = Colour(151, 170, 230);

	// } else {
	// 	color = Colour(20, 37, 92);
	// }