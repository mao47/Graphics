#pragma once

class Vertex
{
public:
	Vertex();
	void Normalize();
	float x, y, z, h;
};

class Vector
{
public:
	float i, j, k;
};

class Face
{
public:
	int v1, v2, v3;
};

class GraphicsObject 
{
public:
	double rSpec;
	double gSpec;
	double bSpec;
	double rDiff;
	double gDiff;
	double bDiff;
	double rAmb;
	double bAmb;
	double gAmb;
	double kSpec;
	double kAmb;
	double kDiff;
	double specExp;
	double indRefr;
	double kRefl;
	double kRefr;
};

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
	double indRefr;
	ray *reflected;
	ray *refracted;

	void calculateValues()
	{
		reflected->calculateValues();
		refracted->calculateValues();
		r = r * krg + reflected->r + refracted->r;
		g = g * krg + reflected->g + refracted->g;
		b = b * krg + reflected->b + refracted->b;
	}

} ray;


class MeshObject : GraphicsObject
{
public:
	MeshObject();
	~MeshObject();
	intersection intersects(ray myRay);
	void Load(char* file, float s, float rx, float ry, float rz,
				  float tx, float ty, float tz);
	void WorldTranslate(float tx, float ty, float tz);
	void WorldRotate(float rx, float ry, float rz);
	void LocalRotate(float rx, float ry, float rz);
	void LocalScale(float s);
	Vertex* pBoundingBox;
	Vertex* pVertexList;
	int VertexCount;
	Face* pFaceList;
	int FaceCount;
	float ModelMatrix[16];
	double rSpec;
	double gSpec;
	double bSpec;
	double rDiff;
	double gDiff;
	double bDiff;
	double rAmb;
	double bAmb;
	double gAmb;
	double kSpec;
	double kAmb;
	double kDiff;
	double specExp;
	double indRefr;
	double kRefl;
	double kRefr;
};