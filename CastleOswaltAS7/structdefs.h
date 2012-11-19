#pragma once

#include "objects.h"


class point
{
public:
	double x,y,z,w;

	point(){ x = 0; y = 0; z = 0; w = 1;}
	point(double xa, double ya, double za)
	{
		x = xa; y = ya; z = za; w = 1.0;
	}
	point(double xa, double ya, double za, double wa)
	{
		x = xa; y = ya; z = za; w = wa;
	}
};

typedef struct intersection {
	int type;
	GraphicsObject object;
	point normal;
	point location;
	double distance;
} intersection;


typedef struct _faceStruct {
  int v1,v2,v3;
  int n1,n2,n3;
} faceStruct;

typedef struct LightSource {
	int type;
	float x, y, z;
	float r, g, b;
} LightSource;

typedef struct ray {
	point origin;
	point direction;
	float r, g, b;
	int depth;
	double krg;
	ray *reflected;
	ray *refracted;
} ray;
