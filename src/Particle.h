/*
 *  Particle.h
 *  graphicsExample
 *
 *  Created by Golan Levin on 12/7/11.
 *
 */

#pragma once

//============================================================================
class Particle {

	public:
	
	float x;
	float y;
	float u;
	float v;
	float pu; 
	float pv;
	float d;

	int   cx;
	int   cy;

	float gu;
	float gv;
	float T00;
	float T01;
	float T11;

	float px[3];
	float py[3];
	float gx[3];
	float gy[3];
	
	Particle ( float inx, float iny, float inu, float inv);


};
	