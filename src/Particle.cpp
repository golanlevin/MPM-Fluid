/*
 *  Particle.cpp
 *  graphicsExample
 *
 *  Created by Golan Levin on 12/7/11.
 *
 */

#include "Particle.h"


Particle::Particle ( float inx, float iny, float inu, float inv) {
	x   = inx;
	y   = iny;
	u   = inu;
	v   = inv;
	pu = 0;
	pv = 0; 
	cx  = 0; 
	cy  = 0; 

	//px = new float[3];
	//py = new float[3];
	//gx = new float[3];
	//gy = new float[3];
}