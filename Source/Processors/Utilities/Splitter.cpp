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

#include "Splitter.h"
#include "../Editors/SplitterEditor.h"

Splitter::Splitter()
	: GenericProcessor("Splitter"), 
		destNodeA(0), destNodeB(0), activePath(0)
	{
		
	}

Splitter::~Splitter()
{
	
}

AudioProcessorEditor* Splitter::createEditor()
{
	SplitterEditor* editor = new SplitterEditor(this);
	setEditor(editor);
	
	std::cout << "Creating editor." << std::endl;
	return editor;
}

void Splitter::setDestNode(GenericProcessor* dn)
{

	destNode = dn;

	if (activePath == 0) {
		std::cout << "Setting destination node A." << std::endl;
		destNodeA = dn;
	} else {
		destNodeB = dn;
		std::cout << "Setting destination node B." << std::endl;

	}
}

void Splitter::switchDest(int destNum) {
	
	activePath = destNum;

	if (destNum == 0) 
	{
		setDestNode(destNodeA);
	} else 
	{
		setDestNode(destNodeB);
	}

	//viewport->setActiveEditor((GenericEditor*) getEditor());
	//viewport->updateVisibleEditors();
}