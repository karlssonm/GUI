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

#include "MainWindow.h"
#include <stdio.h>

//-----------------------------------------------------------------------

MainWindow::MainWindow()
    : DocumentWindow (JUCEApplication::getInstance()->getApplicationName(),
                      Colour(Colours::black),
                      DocumentWindow::allButtons)
{
    //centreWithSize (500, 400);
    setResizable (true,     // isResizable
                  false);   // useBottomCornerRisizer -- doesn't work very well
    
    // Constraining the window's size doesn't seem to work:
    //setResizeLimits(500, 400, 10000, 10000);

    // Create ProcessorGraph and AudioComponent, and connect them.
    // Callbacks will be set by the play button in the control panel
    processorGraph = new ProcessorGraph();
    audioComponent = new AudioComponent();
    audioComponent->connectToProcessorGraph(processorGraph);

    setContentComponent (new UIComponent(this, processorGraph, audioComponent), true, true);

    // commandManager.registerAllCommandsForTarget (getContentComponent());
    // commandManager.registerAllCommandsForTarget (JUCEApplication::getInstance());
    
    // setMenuBar ((MenuBarModel*) getContentComponent());

    // addKeyListener( commandManager.getKeyMappings());

    loadWindowBounds();
    setVisible (true);

}

MainWindow::~MainWindow()
{

  saveWindowBounds();
  processorGraph->saveState();

  audioComponent->disconnectProcessorGraph();

   deleteAndZero(processorGraph);
   deleteAndZero(audioComponent);

  setContentComponent (0);
  setMenuBar(0);

}

void MainWindow::closeButtonPressed()
{ 
    if (audioComponent->callbacksAreActive()) {
      audioComponent->endCallbacks();
      processorGraph->disableProcessors();
    }

    JUCEApplication::getInstance()->systemRequestedQuit();

}

void MainWindow::saveWindowBounds()
{

    std::cout << "Saving window bounds." << std::endl;

    File file = File("./windowState.xml");


    XmlElement* xml = new XmlElement("MAINWINDOW");

    XmlElement* bounds = new XmlElement("BOUNDS");
    bounds->setAttribute("x",getScreenX());
    bounds->setAttribute("y",getScreenY());
    bounds->setAttribute("w",getContentComponent()->getWidth());
    bounds->setAttribute("h",getContentComponent()->getHeight());
    bounds->setAttribute("fullscreen",isFullScreen());

    xml->addChildElement(bounds);

    String error;
    
    if (! xml->writeToFile (file, String::empty))
        error = "Couldn't write to file";
    
    delete xml;
}

void MainWindow::loadWindowBounds()
{
  
    std::cout << "Loading window bounds." << std::endl;
    
    File file = File("./windowState.xml");

    XmlDocument doc (file);
    XmlElement* xml = doc.getDocumentElement();

    if (xml == 0 || ! xml->hasTagName (T("MAINWINDOW")))
    {
        
        std::cout << "File not found." << std::endl;
        delete xml;
        centreWithSize (800, 600);

    } else {

        String description;// = T(" ");

        forEachXmlChildElement (*xml, e)
        {

            int x = e->getIntAttribute("x");
            int y = e->getIntAttribute("y");
            int w = e->getIntAttribute("w");
            int h = e->getIntAttribute("h");

            bool fs = e->getBoolAttribute("fullscreen");

            // without the correction, you get drift over time
            setTopLeftPosition(x,y-27);
            getContentComponent()->setBounds(0,0,w,h-33);
            //setFullScreen(fs);

        }

        delete xml;
    }
   // return "Everything went ok.";
}