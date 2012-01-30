/**
 * ofxMPMFluid.cpp
 * The MIT License (MIT)
 * Copyright (c) 2010 respective contributors
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 *****************************************
 * MPM FLuid Simulation Demo
 * OpenFrameworks version by Golan Levin
 * http://www.flong.com
 *
 * ofxAddon created by James George (@obviousjm)
 * http://www.jamesgeorge.org
 *
 * Original Java version:
 * http://grantkot.com/MPM/Liquid.html
 *
 * Flash version:
 * Copyright iunpin ( http://wonderfl.net/user/iunpin )
 * MIT License ( http://www.opensource.org/licenses/mit-license.php )
 * Downloaded from: http://wonderfl.net/c/6eu4
 *
 * Javascript version:
 * Copyright Stephen Sinclair (radarsat1) ( http://www.music.mcgill.ca/~sinclair )
 * MIT License ( http://www.opensource.org/licenses/mit-license.php )
 * Downloaded from: http://www.music.mcgill.ca/~sinclair/blog 
 */

#pragma once

#include "ofMain.h"

#include "ofxMPMFluid.h"
#include "ofxAutoControlPanel.h"  /* for the control panel */
#include "ofxVectorGraphics.h"    /* for PostScript output */

class testApp : public ofBaseApp {
	
	public:
		
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);		
		
	
	ofxMPMFluid fluid;
	
	bool  bFullscreen;
	bool  bShowControlPanel;
	float scaleFactor;
	
	ofxAutoControlPanel gui;
	void setupControlPanel();

	//-----------------------------------
	// For PostScript output, if you like that sorta thing
	ofxVectorGraphics epsOutput;
	void outputPostscript();
	bool bEpsCapture;
};



