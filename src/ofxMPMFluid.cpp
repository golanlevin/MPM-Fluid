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

#include "ofxMPMFluid.h"

//TODO make varying
#define gridSizeX 160
#define gridSizeY 120 

ofxMPMFluid::ofxMPMFluid() 
:	densitySetting(5.0),
	stiffness(.5),
	bulkViscosity(3.0),
	elasticity(1.0),    
	viscosity(1.0),    
	yieldRate(1.0),     
	gravity(.002),	  
	bGradient(false),	 
	bDoObstacles(true),
	elapsed(0.0),
	scaleFactor(1.0),
	smoothing(1.0)
{
	//
}

void ofxMPMFluid::setup(int maxParticles){
	maxNumParticles = maxParticles;
	
	// This creates a 2-dimensional array (i.e. grid) of Node objects.
	for (int i=0; i<gridSizeX; i++){
		grid.push_back( vector<ofxMPMNode*>() );
		for (int j=0; j<gridSizeY; j++){
			grid[i].push_back( new ofxMPMNode() );
		}
	}
	
	for (int i=0; i<(gridSizeX * gridSizeY); i++){
		activeNodes.push_back( new ofxMPMNode() );
	}
	
	for (int i=0; i < maxParticles; i++) {
		int x0 = 5;
		int x1 = gridSizeX-5;
		float rx = ofRandom(x0,x1); 
		float ry = ofRandom(5,gridSizeY/5);
		particles.push_back( new ofxMPMParticle(rx,ry, 0.0, 0.0) );
	}
	
	//TODO: JG add and remove obistacles through API
	obstacles.push_back( new ofxMPMObstacle(gridSizeX * 0.75, gridSizeY * 0.75, gridSizeX * 0.075) );
}

void ofxMPMFluid::update(){
	// Important: can't exceed maxNParticles!
	numParticles = MIN(numParticles, maxNumParticles);
	
	//-------------------------
	// Clear the grid. Necessary to begin the simulation.
	for (int i=0; i<gridSizeX; i++) {
		for (int j=0; j<gridSizeY; j++) {
			grid[i][j]->clear();
		}
	}
	
	numActiveNodes = 0; 
		
	long t0 = ofGetElapsedTimeMillis();
	//-------------------------
	// Particles pass 1
	float phi;
	int pcxTmp, pcyTmp;
	for (int ip=0; ip<numParticles; ip++) {
		
		ofxMPMParticle *p = particles[ip];
		
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
				vector<ofxMPMNode*>& nrow = grid[pcxi]; // potential for array index out of bounds here if simulation explodes.
				float pxi = px[i];
				float gxi = gx[i];
				
				for (int j=0; j<3; j++) {
					pcyj = pcy+j;
					
					if ((pcyj >= 0) && (pcyj < gridSizeY)){
						ofxMPMNode *n = nrow[pcyj]; // potential for array index out of bounds here if simulation explodes.
						
						if (!n->active) {
							n->active = true;
							activeNodes[numActiveNodes] = n;
							numActiveNodes++;
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
	
	
	for (int ni=0; ni<numActiveNodes; ni++) {
		ofxMPMNode *n = activeNodes[ni]; 
		if (n->m > 0.0) {
			n->u /= n->m;
			n->v /= n->m;
		}
	}
	
	
	long t1 = ofGetElapsedTimeMillis();
	//-------------------------
	// Particles pass 2
	float stiffnessBulk  = stiffness * bulkViscosity;	
	int nBounced = 0; 
	
	for (int ip=0; ip<numParticles; ip++) {
		ofxMPMParticle *p = particles[ip];
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
		
		int pcxi;
		for (int i=0; i<3; i++) {
			
			vector<ofxMPMNode*>& nrow = grid[pcx+i];
			gxi = gx[i];
			pxi = px[i];
			
			for (int j=0; j<3; j++){
				ofxMPMNode *nj = nrow[pcy + j]; 
				gxf   =   gxi * py[j];
				gyf   =   pxi * gy[j];
				dudx += nj->u * gxf;
				dudy += nj->u * gyf;
				dvdx += nj->v * gxf;
				dvdy += nj->v * gyf;
			}
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
		ofxMPMNode *n00  = grid[cx0][cy0];
		ofxMPMNode *n01  = grid[cx0][cy1];
		ofxMPMNode *n10  = grid[cx1][cy0];
		ofxMPMNode *n11  = grid[cx1][cy1];
		
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
		if (bDoObstacles && obstacles.size() > 0){
			
			// circular obstacle
			float oR  = obstacles[0]->radius;
			float oR2 = obstacles[0]->radius2;
			float odx = obstacles[0]->cx - p->x;
			float ody = obstacles[0]->cy - p->y;
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
				vector<ofxMPMNode*>& nrow = grid[pcx + i];
				float ppxi = px[i];
				float pgxi = gx[i];
				
				for (int j=0; j<3; j++){
					ofxMPMNode *nj = nrow[pcy + j ];
					phi = ppxi * py[j];
					dx  = pgxi * py[j];
					dy  = ppxi * gy[j];
					nj->ax += fx * phi -(dx * T00 + dy * T01);
					nj->ay += fy * phi -(dx * T01 + dy * T11);
				}
			}
			
		} else {
			
			float *pppxi = &px[0];
			float *ppgxi = &gx[0];
			
			for (int i=0; i<3; i++) {
				vector<ofxMPMNode*>& nrow = grid[pcx + i];

				float ppxi = *(pppxi++); //px[i]; 
				float pgxi = *(ppgxi++); //gx[i];
				for (int j=0; j<3; j++){
					ofxMPMNode *nj = nrow[pcy+j];
					dx  = pgxi * py[j];
					dy  = ppxi * gy[j];
					nj->ax -= (dx * T00 + dy * T01);
					nj->ay -= (dx * T01 + dy * T11);
				}
				
			}
		}
	}
	
	
	for (int ni=0; ni<numActiveNodes; ni++) {
		ofxMPMNode *n = activeNodes[ni]; 
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
	const float rightEdge  = gridSizeX - 3.0;
	const float bottomEdge = gridSizeY - 3.0;
	
	for (int ip=0; ip<numParticles; ip++) {
		ofxMPMParticle *p = particles[ip];
		
		float *px = p->px;
		float *py = p->py;
		int pcy = p->cy;
		int pcx = p->cx;
		for (int i=0; i<3; i++) {
			vector<ofxMPMNode*>& nrow = grid[pcx + i];
			float ppxi = px[i];
			for (int j=0; j<3; j++){
				ofxMPMNode *nj = nrow[pcy + j];
				phi   = ppxi * py[j];
				p->u += phi * nj->ax;
				p->v += phi * nj->ay;
			}
		}
		
		p->v += gravity;
		//if (isMouseDragging) {
		if (ofGetMousePressed(0)) {
			float vx = abs(p->x - ofGetMouseX()/scaleFactor);
			float vy = abs(p->y - ofGetMouseY()/scaleFactor);
			float mdx = (ofGetMouseX() - ofGetPreviousMouseX())/scaleFactor;
			float mdy = (ofGetMouseY() - ofGetPreviousMouseY())/scaleFactor;
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
			vector<ofxMPMNode*>& nrow = grid[pcx + i];
			float ppxi = px[i];
			for (int j=0; j<3; j++){
				ofxMPMNode *nj = nrow[pcy + j];
				phi = ppxi * py[j];
				nj->u += phi * pu;
				nj->v += phi * pv;
			}
		}
	}
	
	for (int ni=0; ni<numActiveNodes; ni++) { 
		ofxMPMNode *n = activeNodes[ni]; 
		if (n->m > 0.0F) {
			n->u /= n->m;
			n->v /= n->m;
		}
	}
	
	
	long t3 = ofGetElapsedTimeMillis();
	//-------------------------
	// Particles pass 4
	float gu, gv;
	for (int ip=0; ip<numParticles; ip++) {
		ofxMPMParticle *p = particles[ip];
		
		gu = 0.0F;
		gv = 0.0F;
		
		float *px = p->px;
		float *py = p->py;
		int pcy = p->cy;
		int pcx = p->cx;
		for (int i=0; i<3; i++) {
			vector<ofxMPMNode*>& nrow = grid[pcx + i];
			float ppxi = px[i]; 
			for (int j=0; j<3; j++){
				ofxMPMNode *nj = nrow[pcy + j];
				phi = ppxi * py[j];
				gu += phi * nj->u;
				gv += phi * nj->v;
			}
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

void ofxMPMFluid::draw(){
	
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
	vector<ofVec2f> verts;

	for (int ip=0; ip<numParticles; ip++) {
		ofxMPMParticle* p = particles[ip];
		verts.push_back(ofVec2f(p->x, p->y));
		verts.push_back(ofVec2f(p->x - p->u, p->y - p->v));
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, &(verts[0].x));
	glDrawArrays(GL_LINES, 0, verts.size());
	glDisableClientState(GL_VERTEX_ARRAY);
	ofPopMatrix();
}

vector<ofxMPMParticle*>& ofxMPMFluid::getParticles(){
	return particles;
}

int ofxMPMFluid::getGridSizeX(){
	return gridSizeX;
}

int ofxMPMFluid::getGridSizeY(){
	return gridSizeY;
}

