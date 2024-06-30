/*
 * 3d test
*/

#pragma once
#include <stdlib.h>
#include "triangleTree.h"

#include <TFT_eSPI.h>

class Engine3D
{
	public:
	TriangleTree *triangleBuffer;
	TriangleTree *triangleRoot;
	int trinagleBufferSize;
	int triangleCount;

	Engine3D(const int initialTrinagleBufferSize = 1)
	{
		trinagleBufferSize = initialTrinagleBufferSize;
		triangleBuffer = (TriangleTree*)malloc(sizeof(TriangleTree) * trinagleBufferSize);
		//if(!triangleBuffer)
			//ERROR("Not enough memory for triangleBuffer");
		triangleRoot = 0;
		triangleCount = 0;
	}


	void enqueueTriangle(short *v0, short *v1, short *v2, uint32_t color)
	{
		if (triangleCount >= trinagleBufferSize)
			return;
		TriangleTree &t = triangleBuffer[triangleCount++];
		t.set(v0, v1, v2, color);
		if (triangleRoot)
			triangleRoot->add(&triangleRoot, t);
		else
			triangleRoot = &t;
	}
	
	void drawTriangleTree(TFT_eSprite &tft, TriangleTree *t)
	{
		if (t->left)
			drawTriangleTree(tft, t->left);
		//g.triangle(t->v[0], t->v[1], t->v[2], t->color);
		tft.fillTriangle(t->v[0][0], t->v[0][1], t->v[1][0], t->v[1][1], t->v[2][0], t->v[2][1], t->color);
		if (t->right)
			drawTriangleTree(tft, t->right);
	}

	virtual void begin()
	{
		triangleCount = 0;
		triangleRoot = 0;
	}

	virtual void end(TFT_eSprite &tft)
	{
		if (triangleRoot)
			drawTriangleTree(tft, triangleRoot);
	}
};