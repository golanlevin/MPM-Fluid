#include "testApp.h"


/*
 * MPM FLuid Simulation Demo
 * OpenFrameworks version by Golan Levin
 * http://www.flong.com
 
 * Original Java version:
 * http://grantkot.com/MPM/Liquid.html
 
 * Flash version:
 * Copyright iunpin ( http://wonderfl.net/user/iunpin )
 * MIT License ( http://www.opensource.org/licenses/mit-license.php )
 * Downloaded from: http://wonderfl.net/c/6eu4
 
 * Javascript version:
 * Copyright Stephen Sinclair (radarsat1) ( http://www.music.mcgill.ca/~sinclair )
 * MIT License ( http://www.opensource.org/licenses/mit-license.php )
 * Downloaded from: http://www.music.mcgill.ca/~sinclair/blog
 */


//--------------------------------------------------------------
void testApp::setup(){
	
	ofSetWindowTitle("OpenFrameworks MPM Fluid Demo");
	ofBackground(0,0,0);
	
	currMousePressed = prevMousePressed = false;
	prevMouseX = prevMouseY = 0; 
	elapsed = 0.0;
	
	bEpsCapture = false;
	bFullscreen = false;
	bShowControlPanel = true;
	ofSetFullscreen(bFullscreen);

	maxNParticles = 100000; 
	initGridAndParticles( maxNParticles );	
	scaleFactor = 6.4;	// FYI: this is computed from (screensize/gridSizeX)
 
	setupControlPanel();
}  
 
 
//=========================================================================
// Set up the controls in the ofxAutoControlPanel
void testApp::setupControlPanel(){
	
	// Note: the other primary parameters for this simulation are
	// gridSizeX and gridSizeY, in testApp.h
	
	gui.setup("MPM Fluid Demo", 10, 10, 350, 640);
	gui.addPanel(" MPM Simulation Parameters", 1);
	gui.addLabel("'f' to toggle fullscreen \n'h' to toggle this panel \n'p' to print postscript");
	
	gui.addSlider("# Particles",		"N_PARTICLES",		maxNParticles/4,  1000, maxNParticles, true); 
	gui.addSlider("Density",			"DENSITY",			5.0,	0, 30.0,	false);	
	gui.addSlider("Stiffness",			"STIFFNESS",		0.5,	0, 2.0,		false);
	gui.addSlider("Bulk Viscosity",		"BULK_VISCOSITY",	3.0,	0, 10.0,	false);
	gui.addSlider("Elasticity",			"ELASTICITY",		1.0,	0, 4.0,		false);
	gui.addSlider("Viscosity",			"VISCOSITY",		1.0,	0, 4.0,		false);
	gui.addSlider("Yield Rate",			"YIELD_RATE",		1.0,	0, 2.0,		false);
	gui.addSlider("Gravity",			"GRAVITY",			0.002,	0, 0.02,	false);
	gui.addSlider("Smoothing",			"SMOOTHING",		1.0,	0, 3.0,		false);
	
	gui.addToggle("Do Obstacles?",		"DO_OBSTACLES",		true); 
	
	// Now, this horizontal gradient in the Density parameter is just for yuks.
	// It demonstrates that spatial variations in the Density parameter can yield interesting results. 
	// For an interesting experiment, try making Density proportional to the luminance of a photograph.
	gui.addToggle("Horizontal Density Gradient?",	"DENSITY_GRADIENT", false);
	
	// This can eliminate screen tearing. 
	gui.addToggle("Vertical Sync?",					"VERTICAL_SYNC", true);
	
	gui.loadSettings("settings/controlPanelSettings.xml");
}

 
//=========================================================================
void testApp::initGridAndParticles (int np){

	// This creates a 2-dimensional array (i.e. grid) of Node objects.
	grid = new Node**[gridSizeX];
	for (int i=0; i<gridSizeX; i++) {
		grid[i] = new Node*[gridSizeY];
		for (int j=0; j<gridSizeY; j++){
			grid[i][j] = new Node();
		}
	}

	nActiveNodes = 0; 
	activeNodeArray = new Node*[gridSizeX * gridSizeY];
	for (int i=0; i<(gridSizeX * gridSizeY); i++){
		activeNodeArray[i] = new Node();
	}
  
	nParticles = np;
	particles = new Particle*[maxNParticles]; 
	for (int i=0; i<maxNParticles; i++) {
		int x0 = 5;
		int x1 = gridSizeX-5;
		float rx = ofRandom(x0,x1); 
		float ry = ofRandom(5,gridSizeY/5);
		particles[i] = new Particle (rx,ry, 0.0, 0.0);
	}
	
	obstacle = new Obstacle (gridSizeX * 0.75, gridSizeY * 0.75, gridSizeX * 0.075);
}


//=========================================================================
void testApp::draw(){
	
	// These improve the appearance of small lines and/or points.
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_POINT_SMOOTH); // in case you want it
	glEnable (GL_MULTISAMPLE);
	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	 
	
	ofSetColor(255,255,255, 204); 
	glLineWidth(1.0); // or thicker, if you prefer
	
	ofPushMatrix();
	ofScale(scaleFactor, scaleFactor, 1.0);
	
	// Draw the active particles as a short line, 
	// using their velocity for their length. 
	Particle *p;
	glBegin(GL_LINES); 
	for (int ip=0; ip<nParticles; ip++) {
		p = particles[ip];
		glVertex2f ( p->x,          p->y);
		glVertex2f ((p->x - p->u), (p->y - p->v));
	}
	glEnd();
	ofPopMatrix();
	
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
		float pageScale = printAreaInMillimeters / (gridSizeX-5);
		Particle *p;
			
		for (int ip=0; ip<nParticles; ip++) {
			p = particles[ip];
			float x0 = p->x;
			float y0 = p->y;
			
			if ((x0>0) && (x0<gridSizeX) && (y0>0) && (y0<gridSizeY)){
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
	
	//-------------------------
	// Update mouse information; used for interaction with the simulation.
	bool isMouseDragging = false;
	float mdx = 0.0;
	float mdy = 0.0;
	if (currMousePressed && prevMousePressed) {
		isMouseDragging = true;
		mdx = (mouseX - prevMouseX)/scaleFactor;
		mdy = (mouseY - prevMouseY)/scaleFactor;
	}
	prevMousePressed = currMousePressed;
	prevMouseX = mouseX;
	prevMouseY = mouseY;
	
	//-------------------------
	// Clear the grid. Necessary to begin the simulation.
	for (int i=0; i<gridSizeX; i++) {
		for (int j=0; j<gridSizeY; j++) {
			grid[i][j]->clear();
		}
	}
	nActiveNodes = 0; 
	
	
	// Fetch the number of particles to simulate, from the control panel. 
	// Important: can't exceed maxNParticles!
	nParticles = gui.getValueI("N_PARTICLES", 0); 

	
	long t0 = ofGetElapsedTimeMillis();
	//-------------------------
	// Particles pass 1
	float phi;
	int pcxTmp, pcyTmp;
	for (int ip=0; ip<nParticles; ip++) {

		Particle *p = particles[ip];

		int pcx = p->cx = (int)(p->x - 0.5F);
		int pcy = p->cy = (int)(p->y - 0.5F);
		
		float *px = p->px;
		float *py = p->py;
		float *gx = p->gx;
		float *gy = p->gy;
		float pu  = p->u;
		float pv  = p->v;
		p->pu = pu;
		p->pv = pv;

		
		// N.B.: The constants below are not playthings.
		float x = (float) p->cx - p->x;
		px[0] = (0.5F * x * x + 1.5F * x) + 1.125f;
		gx[0] = x + 1.5F;
		x++;
		px[1] = -x * x + 0.75F;
		gx[1] = -2.0F * x;
		x++;
		px[2] = (0.5F * x * x - 1.5F * x) + 1.125f;
		gx[2] = x - 1.5F;

		float y = (float) p->cy - p->y;
		py[0] = (0.5F * y * y + 1.5F * y) + 1.125f;
		gy[0] = y + 1.5F;
		y++;
		py[1] = -y * y + 0.75F;
		gy[1] = -2.0F * y;
		y++;
		py[2] = (0.5F * y * y - 1.5F * y) + 1.125f;
		gy[2] = y - 1.5F;

		
		int pcxi, pcyj;
		for (int i=0; i<3; i++) {
			pcxi = pcx+i;
			
			if ((pcxi >= 0) && (pcxi < gridSizeX)){ 
				Node **nrow = grid[pcxi]; // potential for array index out of bounds here if simulation explodes.
				float pxi = px[i];
				float gxi = gx[i];

				for (int j=0; j<3; j++) {
					pcyj = pcy+j;
					
					if ((pcyj >= 0) && (pcyj < gridSizeY)){
						Node *n = nrow[pcyj]; // potential for array index out of bounds here if simulation explodes.

						if (!n->active) {
							n->active = true;
							activeNodeArray[nActiveNodes] = n;
							nActiveNodes++;
						}
						phi    = pxi * py[j];
						n->m  += phi;
						n->gx += gxi * py[j];
						n->gy += pxi * gy[j];
						n->u  += phi * pu;
						n->v  += phi * pv;
					}
				}
			}
		}
	}

	
	for (int ni=0; ni<nActiveNodes; ni++) {
		Node *n = activeNodeArray[ni]; 
		if (n->m > 0.0) {
			n->u /= n->m;
			n->v /= n->m;
		}
	}

	
	long t1 = ofGetElapsedTimeMillis();
	//-------------------------
	// Particles pass 2
	float densitySetting = gui.getValueF("DENSITY",			0); 
	float stiffness      = gui.getValueF("STIFFNESS",		0); 
	float bulkViscosity  = gui.getValueF("BULK_VISCOSITY",	0); 
	float elasticity     = gui.getValueF("ELASTICITY",		0); 
	float viscosity      = gui.getValueF("VISCOSITY",		0); 
	float yieldRate      = gui.getValueF("YIELD_RATE",		0);
	bool  bGradient		 = gui.getValueB("DENSITY_GRADIENT", 0);
	bool  bDoObstacles   = gui.getValueB("DO_OBSTACLES",	0); 
	float stiffnessBulk  = stiffness * bulkViscosity;
 
	int nBounced = 0; 
	
	for (int ip=0; ip<nParticles; ip++) {
		Particle *p = particles[ip];
		float *px = p->px;
		float *py = p->py;
		float *gx = p->gx;
		float *gy = p->gy;
		int   pcy = p->cy;
		int   pcx = p->cx;

		float dudx = 0.0F;
		float dudy = 0.0F;
		float dvdx = 0.0F;
		float dvdy = 0.0F;

		float gxi, pxi;
		float gxf, gyf;

		float py0 = py[0];
		float gy0 = gy[0];
		float py1 = py[1];
		float gy1 = gy[1];
		float py2 = py[2];
		float gy2 = gy[2];
	
		int pcxi;
		for (int i=0; i<3; i++) {
				
			Node **nrow = grid[pcx+i];
			gxi = gx[i];
			pxi = px[i];
			
			// Here, an unrolled loop. Used to be pcy+j as j:[0,1,2]
			Node *n0 = nrow[pcy  ]; 
			gxf   =   gxi * py0;
			gyf   =   pxi * gy0;
			dudx += n0->u * gxf;
			dudy += n0->u * gyf;
			dvdx += n0->v * gxf;
			dvdy += n0->v * gyf;

			Node *n1 = nrow[pcy+1]; 
			gxf   =   gxi * py1;
			gyf   =   pxi * gy1;
			dudx += n1->u * gxf;
			dudy += n1->u * gyf;
			dvdx += n1->v * gxf;
			dvdy += n1->v * gyf;

			Node *n2 = nrow[pcy+2];
			gxf   =   gxi * py2;
			gyf   =   pxi * gy2;
			dudx += n2->u * gxf;
			dudy += n2->u * gyf;
			dvdx += n2->v * gxf;
			dvdy += n2->v * gyf;
		} 
	  
		float w1  = dudy - dvdx;
		float wT0 = w1 * p->T01;
		float wT1 = 0.5F * w1 * (p->T00 - p->T11);
		float D00 = dudx;
		float D01 = 0.5F * (dudy + dvdx);
		float D11 = dvdy;
		float trace = 0.5F * (D00 + D11);
		D00 -= trace;
		D11 -= trace;

		p->T00 += (-wT0 + D00) - yieldRate * p->T00;
		p->T01 += ( wT1 + D01) - yieldRate * p->T01;
		p->T11 += ( wT0 + D11) - yieldRate * p->T11;
		
		// here's our protection against exploding simulations...
		float norma = p->T00 * p->T00 + 2.0F * p->T01 * p->T01 + p->T11 * p->T11;
		if (norma > 10.0F) {
			p->T00 = p->T01 = p->T11 = 0.0F;
		}

		int cx0 = (int) p->x;
		int cy0 = (int) p->y;
		int cx1 = cx0 + 1;
		int cy1 = cy0 + 1;
		Node *n00  = grid[cx0][cy0];
		Node *n01  = grid[cx0][cy1];
		Node *n10  = grid[cx1][cy0];
		Node *n11  = grid[cx1][cy1];

		float p00 = n00->m;
		float x00 = n00->gx;
		float y00 = n00->gy;
		float p01 = n01->m;
		float x01 = n01->gx;
		float y01 = n01->gy;
		float p10 = n10->m;
		float x10 = n10->gx;
		float y10 = n10->gy;
		float p11 = n11->m;
		float x11 = n11->gx;
		float y11 = n11->gy;

		float pdx =  p10 - p00;
		float pdy =  p01 - p00;
		float C20 =  3.0F * pdx - x10 - 2.0F * x00;
		float C02 =  3.0F * pdy - y01 - 2.0F * y00;
		float C30 = -2.0F * pdx + x10 + x00;
		float C03 = -2.0F * pdy + y01 + y00;
		float csum1 = p00 + y00 + C02 + C03;
		float csum2 = p00 + x00 + C20 + C30;
		float C21 =   3.0F * p11 - 2.0F * x01 - x11 - 3.0F * csum1  - C20;
		float C31 = (-2.0F * p11 +        x01 + x11 + 2.0F * csum1) - C30;
		float C12 =   3.0F * p11 - 2.0F * y10 - y11 - 3.0F * csum2  - C02;
		float C13 = (-2.0F * p11 +        y10 + y11 + 2.0F * csum2) - C03;
		float C11 = x01 - C13 - C12 - x00;

		float u1 = p->x - (float)cx0;
		float u2 = u1 * u1;
		float u3 = u1 * u2;
		float v1 = p->y - (float)cy0;
		float v2 = v1 * v1;
		float v3 = v1 * v2;
		float density = 
			p00 + 
			x00 * u1 + 
			y00 * v1 + 
			C20 * u2 + 
			C02 * v2 + 
			C30 * u3 + 
			C03 * v3 + 
			C21 * u2 * v1 + 
			C31 * u3 * v1 +
			C12 * u1 * v2 + 
			C13 * u1 * v3 + 
			C11 * u1 * v1 ;

	  
		float DS = densitySetting; 
		if (bGradient){
			// Just for yuks, a spatially varying density function
			DS = densitySetting * ( powf(p->x / (float) gridSizeX, 4.0) );
		} 
	  
		float pressure = (stiffness / max (1.0F, DS)) * (density - DS);
		if (pressure > 2.0F) { 
			pressure = 2.0F;
		}

		p->d = 1.0/MAX(0.001, density);
	  
		// COLLISIONS-1
		// Determine if there has been a collision with the wall. 
		float fx = 0.0F;
		float fy = 0.0F;
		bool bounced = false;
	  
		if (p->x < 3.0F) {
			fx += 3.0F - p->x;
			bounced = true;
		} else if (p->x > (float)(gridSizeX - 3)) {
			fx += (gridSizeX - 3.0) - p->x;
			bounced = true;
		}

		if (p->y < 3.0F) {
			fy += 3.0F - p->y;
			bounced = true;
		} else if (p->y > (float)(gridSizeY - 3)) {
			fy += (gridSizeY - 3.0) - p->y;
			bounced = true;
		}
		
		
		// Interact with a simple demonstration obstacle.
		// Note: an accurate obstacle implementation would also need to implement
		// some velocity fiddling as in the section labeled "COLLISIONS-2" below.
		// Otherwise, this obstacle is "soft"; particles can enter it slightly. 
		if (bDoObstacles){
			
			// circular obstacle
			float oR  = obstacle->radius;
			float oR2 = obstacle->radius2;
			float odx = obstacle->cx - p->x;
			float ody = obstacle->cy - p->y;
			float oD2 = odx*odx + ody*ody;
			if (oD2 < oR2){
				float oD = sqrtf(oD2);
				float dR = oR-oD;
				fx -= dR * (odx/oD); 
				fy -= dR * (ody/oD); 
				bounced = true;
			}
		}
		
	
		trace *= stiffnessBulk;
		float T00 = elasticity * p->T00 + viscosity * D00 + pressure + trace;
		float T01 = elasticity * p->T01 + viscosity * D01;
		float T11 = elasticity * p->T11 + viscosity * D11 + pressure + trace;
		float dx, dy;
	
		if (bounced){
			for (int i=0; i<3; i++) {
				Node **nrow = grid[pcx + i];
				float ppxi = px[i];
				float pgxi = gx[i];
				
				// Here, an unrolled loop. Used to be pcy+j as j:[0,1,2]
				Node *n0 = nrow[pcy    ];
				phi = ppxi * py0;
				dx  = pgxi * py0;
				dy  = ppxi * gy0;
				n0->ax += fx * phi -(dx * T00 + dy * T01);
				n0->ay += fy * phi -(dx * T01 + dy * T11);

				Node *n1 = nrow[pcy + 1];
				phi = ppxi * py1;
				dx  = pgxi * py1;
				dy  = ppxi * gy1;
				n1->ax += fx * phi -(dx * T00 + dy * T01);
				n1->ay += fy * phi -(dx * T01 + dy * T11);

				Node *n2 = nrow[pcy + 2];
				phi = ppxi * py2;
				dx  = pgxi * py2;
				dy  = ppxi * gy2;
				n2->ax += fx * phi -(dx * T00 + dy * T01);
				n2->ay += fy * phi -(dx * T01 + dy * T11);
			}
			
		} else {

			float *pppxi = &px[0];
			float *ppgxi = &gx[0];

			for (int i=0; i<3; i++) {
				Node **nrow = grid[pcx + i];

				float ppxi = *(pppxi++); //px[i]; 
				float pgxi = *(ppgxi++); //gx[i];

				// Here, an unrolled loop. Used to be pcy+j as j:[0,1,2]
				Node *n0 = nrow[pcy  ];
				dx  = pgxi * py0;
				dy  = ppxi * gy0;
				n0->ax -= (dx * T00 + dy * T01);
				n0->ay -= (dx * T01 + dy * T11);

				Node *n1 = nrow[pcy+1];
				dx  = pgxi * py1;
				dy  = ppxi * gy1;
				n1->ax -= (dx * T00 + dy * T01);
				n1->ay -= (dx * T01 + dy * T11);

				Node *n2 = nrow[pcy+2];
				dx  = pgxi * py2;
				dy  = ppxi * gy2;
				n2->ax -= (dx * T00 + dy * T01);
				n2->ay -= (dx * T01 + dy * T11);

			}
		}
	}
	

	for (int ni=0; ni<nActiveNodes; ni++) {
		Node *n = activeNodeArray[ni]; 
		if (n->m > 0.0F) {
			n->ax /= n->m;
			n->ay /= n->m;
			n->u = 0.0; 
			n->v = 0.0;
		}
	}

	
	long t2 = ofGetElapsedTimeMillis();
	//-------------------------
	// Particles pass 3
	float gravity = gui.getValueF("GRAVITY",		0);
	const float rightEdge  = gridSizeX - 3.0;
	const float bottomEdge = gridSizeY - 3.0;

	for (int ip=0; ip<nParticles; ip++) {
		Particle *p = particles[ip];

		float *px = p->px;
		float *py = p->py;
		int pcy = p->cy;
		int pcx = p->cx;
		for (int i=0; i<3; i++) {
			Node **nrow = grid[pcx + i];
			float ppxi = px[i];

			// Here, an unrolled loop. Used to be pcy+j as j:[0,1,2]
			Node *n0 = nrow[pcy    ];
			phi   = ppxi * py[0];
			p->u += phi * n0->ax;
			p->v += phi * n0->ay;

			Node *n1 = nrow[pcy + 1];
			phi   = ppxi * py[1];
			p->u += phi * n1->ax;
			p->v += phi * n1->ay;

			Node *n2 = nrow[pcy + 2];
			phi   = ppxi * py[2];
			p->u += phi * n2->ax;
			p->v += phi * n2->ay;
		}

		p->v += gravity;
		if (isMouseDragging) {
			float vx = abs(p->x - mouseX/scaleFactor);
			float vy = abs(p->y - mouseY/scaleFactor);
			if (vx < 10.0F && vy < 10.0F) {
				float weight = (1.0F - vx / 10.0F) * (1.0F - vy / 10.0F);
				p->u += weight * (mdx - p->u);
				p->v += weight * (mdy - p->v);
			}
		}

		// COLLISIONS-2
		// Plus, an opportunity to add randomness when accounting for wall collisions. 
		float xf = p->x + p->u;
		float yf = p->y + p->v;
		float wallBounceMaxRandomness = 0.03;
		if (xf < 2.0F) {
			p->u +=     (2.0F - xf) + ofRandom(wallBounceMaxRandomness);
		} else if (xf > rightEdge) { 
			p->u +=  rightEdge - xf - ofRandom(wallBounceMaxRandomness);
		} 
		if (yf < 2.0F) {
			p->v +=     (2.0F - yf) + ofRandom(wallBounceMaxRandomness);
		} else if (yf > bottomEdge) {
			p->v += bottomEdge - yf - ofRandom(wallBounceMaxRandomness);
		} 
		
		
		


		float pu = p->u;
		float pv = p->v;
		for (int i=0; i<3; i++) {
			Node **nrow = grid[pcx + i];
			float ppxi = px[i];

			// Here, an unrolled loop. Used to be pcy+j as j:[0,1,2]
			Node *n0 = nrow[pcy    ];
			phi = ppxi * py[0];
			n0->u += phi * pu;
			n0->v += phi * pv;

			Node *n1 = nrow[pcy + 1];
			phi = ppxi * py[1];
			n1->u += phi * pu;
			n1->v += phi * pv;

			Node *n2 = nrow[pcy + 2];
			phi = ppxi * py[2];
			n2->u += phi * pu;
			n2->v += phi * pv;
		}
	}

	for (int ni=0; ni<nActiveNodes; ni++) { 
		Node *n = activeNodeArray[ni]; 
		if (n->m > 0.0F) {
			n->u /= n->m;
			n->v /= n->m;
		}
	}

	
	long t3 = ofGetElapsedTimeMillis();
	//-------------------------
	// Particles pass 4
	float gu, gv;
	float smoothing = gui.getValueF("SMOOTHING",		0); 
	for (int ip=0; ip<nParticles; ip++) {
		Particle *p = particles[ip];

		gu = 0.0F;
		gv = 0.0F;

		float *px = p->px;
		float *py = p->py;
		int pcy = p->cy;
		int pcx = p->cx;
		for (int i=0; i<3; i++) {
			Node **nrow = grid[pcx + i];
			float ppxi = px[i]; 

			// Here, an unrolled loop. Used to be pcy+j as j:[0,1,2]
			Node *n0 = nrow[pcy    ];
			phi = ppxi * py[0];
			gu += phi * n0->u;
			gv += phi * n0->v;

			Node *n1 = nrow[pcy + 1];
			phi = ppxi * py[1];
			gu += phi * n1->u;
			gv += phi * n1->v;

			Node *n2 = nrow[pcy + 2];
			phi = ppxi * py[2];
			gu += phi * n2->u;
			gv += phi * n2->v;
		}

		p->x += (p->gu = gu);
		p->y += (p->gv = gv);
		p->u += smoothing * (gu - p->u);
		p->v += smoothing * (gv - p->v);
	}


	//----------------------------------
	long t4 = ofGetElapsedTimeMillis();

	long dt0 = t1-t0;
	long dt1 = t2-t1;
	long dt2 = t3-t2;
	long dt3 = t4-t3;
	long dt =  t4 - t0;
	elapsed = 0.95*elapsed + 0.05*(dt);
	// Timing: in case you're curious about CPU consumption, uncomment this:
	// printf("Elapsed = %d	%d	%d	%d	%f\n", dt0, dt1, dt2, dt3, elapsed); 
}


//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	currMousePressed = false;
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	currMousePressed = true;
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	currMousePressed = true;
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	currMousePressed = false;
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