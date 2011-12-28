/*
 *  Node.h
 *  graphicsExample
 *
 *  Created by Golan Levin on 12/7/11.
 *
 */

#pragma once

//============================================================================
class Node {

public:
	float m;
	float d;
	float gx;
	float gy;
	float u;
	float v;
	float ax;
	float ay;
	bool active;
	
	void clear();
	Node();

};