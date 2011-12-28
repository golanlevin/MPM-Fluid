/*
 *  Node.cpp
 *  graphicsExample
 *
 *  Created by Golan Levin on 12/7/11.
 *
 */


#include "Node.h"

Node::Node () {
	m  = 0;
	d  = 0;
	gx = 0;
	gy = 0;
	u  = 0;
	v  = 0;
	ax = 0;
	ay = 0;
	active = false;
}

void Node::clear () {
	m = d = gx = gy = u = v = ax = ay = 0.0;
	active = false;
}