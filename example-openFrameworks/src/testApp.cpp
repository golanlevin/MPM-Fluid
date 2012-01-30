#include "testApp.h"

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

//--------------------------------------------------------------
void testApp::setup(){
	
	ofSetWindowTitle("OpenFrameworks MPM Fluid Demo");
	ofBackground(0,0,0);
	
	bEpsCapture = false;
	bFullscreen = false;
	bShowControlPanel = true;
	ofSetFullscreen(bFullscreen);

	fluid.setup(100000);
	scaleFactor = 6.4;	// FYI: this is computed from (screensize/gridSizeX)
 
	setupControlPanel();
}  
 
 
//=========================================================================
// Set up the controls in the ofxAutoControlPanel
void testApp::setupControlPanel(){
		
	gui.setup("MPM Fluid Demo", 10, 10, 350, 640);
	gui.addPanel(" MPM Simulation Parameters", 1);
	gui.addLabel("'f' to toggle fullscreen \n'h' to toggle this panel \n'p' to print postscript");
	
	gui.addSlider("# Particles",	"N_PARTICLES",		100000/4,  1000, 100000, true); 
	gui.addSlider("Density",		"DENSITY",			5.0,	0, 30.0,	false);	
	gui.addSlider("Stiffness",		"STIFFNESS",		0.5,	0, 2.0,		false);
	gui.addSlider("Bulk Viscosity",	"BULK_VISCOSITY",	3.0,	0, 10.0,	false);
	gui.addSlider("Elasticity",		"ELASTICITY",		1.0,	0, 4.0,		false);
	gui.addSlider("Viscosity",		"VISCOSITY",		1.0,	0, 4.0,		false);
	gui.addSlider("Yield Rate",		"YIELD_RATE",		1.0,	0, 2.0,		false);
	gui.addSlider("Gravity",		"GRAVITY",			0.002,	0, 0.02,	false);
	gui.addSlider("Smoothing",		"SMOOTHING",		1.0,	0, 3.0,		false);
	
	gui.addToggle("Do Obstacles?",	"DO_OBSTACLES",		true); 
	
	// Now, this horizontal gradient in the Density parameter is just for yuks.
	// It demonstrates that spatial variations in the Density parameter can yield interesting results. 
	// For an interesting experiment, try making Density proportional to the luminance of a photograph.
	gui.addToggle("Horizontal Density Gradient?",	"DENSITY_GRADIENT", false);
	
	// This can eliminate screen tearing. 
	gui.addToggle("Vertical Sync?",					"VERTICAL_SYNC", true);
	
	gui.loadSettings("settings/controlPanelSettings.xml");
}

//=========================================================================
void testApp::draw(){

	fluid.draw();

	//-----------------
	outputPostscript();
}


//=========================================================================
void testApp::outputPostscript(){
	// When the user presses 'p', save out the image as an EPS (PostScript) file.
	// Each particle is rendered as a small circle. 
	
	if(bEpsCapture){
		epsOutput.beginEPS("fluid-output.ps");
		epsOutput.fill();
		epsOutput.setColor(0x000000); 
		epsOutput.setLineWidth(0.04); //mm!
		
		int printAreaInMillimeters = 11.0 * 2.54 * 10.0;
		float pageScale = printAreaInMillimeters / (fluid.getGridSizeX()-5);
		ofxMPMParticle *p;
		
		for (int ip=0; ip<fluid.getParticles().size(); ip++) {
			p = fluid.getParticles()[ip];
			float x0 = p->x;
			float y0 = p->y;
			
			if ((x0>0) && (x0<fluid.getGridSizeX()) && (y0>0) && (y0<fluid.getGridSizeY())){
				epsOutput.circle (pageScale*x0, pageScale*y0, 0.1);
			}
			
		}
		
		epsOutput.endEPS();
		bEpsCapture = false;
		
	}
}


//--------------------------------------------------------------
void testApp::update(){
	
	//-------------------------
	// Update the OpenGL vertical sync, based on the control panel.
	bool bSync = gui.getValueB("VERTICAL_SYNC", 0); 
	ofSetVerticalSync (bSync);

	fluid.numParticles	 = gui.getValueI("N_PARTICLES", 0); 
	fluid.densitySetting = gui.getValueF("DENSITY", 0); 
	fluid.stiffness      = gui.getValueF("STIFFNESS", 0); 
	fluid.bulkViscosity  = gui.getValueF("BULK_VISCOSITY", 0); 
	fluid.elasticity     = gui.getValueF("ELASTICITY", 0); 
	fluid.viscosity      = gui.getValueF("VISCOSITY", 0); 
	fluid.yieldRate      = gui.getValueF("YIELD_RATE", 0);
	fluid.bGradient	     = gui.getValueB("DENSITY_GRADIENT", 0);
	fluid.bDoObstacles   = gui.getValueB("DO_OBSTACLES", 0); 
	fluid.gravity		 = gui.getValueF("GRAVITY", 0);
	fluid.smoothing		 = gui.getValueF("SMOOTHING", 0); 

	fluid.scaleFactor	= scaleFactor;
	
	fluid.update();	
}


//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	
	if (key == 'p'){        // print
		// See outputPostscript()
		bEpsCapture = true;
		
	} else if (key == 'f'){ // fullscreen
		bFullscreen = !bFullscreen; 
		ofSetFullscreen (bFullscreen);
		
	} else if (key == 'h'){ // help
		bShowControlPanel = !bShowControlPanel;
		if (bShowControlPanel){
			gui.show();
			ofShowCursor();
		} else {
			gui.hide();
			ofShowCursor();
		}
	}
}

//--------------------------------------------------------------
void testApp::keyReleased  (int key){ 
}
//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}
//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
}
//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
}

/* EOF */