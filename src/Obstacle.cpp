/*
 *  Obstacle.cpp
 *  mpm-fluid
 *
 *  Created by Golan Levin on 12/28/11.
 *
 */

#include "Obstacle.h"


Obstacle::Obstacle ( float inx, float iny, float inr) {
	cx		= inx;
	cy		= iny;
	radius	= inr;
	radius2 = radius * radius;
}