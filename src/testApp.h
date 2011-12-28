#pragma once

#include "ofMain.h"
#include "Particle.h"
#include "Node.h"
#include "Obstacle.h"

#define gridSizeX 160
#define gridSizeY 120 

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
		
		//-----------------------------------
		// For the fluid simulation 
		Particle **particles;
		int nParticles;
		int maxNParticles;
		void initGridAndParticles (int np);
	
		Node ***grid; // a two-dimensional grid of Node objects
		Node **activeNodeArray; // the Nodes which actually have stuff in them
		int nActiveNodes;
	
		Obstacle *obstacle;
	
		//-----------------------------------
		// For interaction and display
		bool  currMousePressed;
		bool  prevMousePressed;
		float prevMouseX;
		float prevMouseY; 
		
		float elapsed;
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



