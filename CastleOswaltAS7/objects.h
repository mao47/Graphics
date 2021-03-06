#pragma once


#define type_none 0
#define type_circle 1
#define type_face 2

#define light_direction 0
#define light_point 1


class Vertex
{
public:
	Vertex();
	void Normalize();
	float x, y, z, h;
};

//class Vector
//{
//public:
//	float i, j, k;
//};
typedef struct Vector
{
	float i, j, k;
} Vector;

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
	int id;
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
	ray() {
		reflected = NULL;
		refracted = NULL;
		r = 0;
		b = 0;
		g = 0;
		krg = 1.0;
		indRefr = 1.0;
		inside = false;
		kRefl = 0.0;
		kRefr = 0.0;
	}
	point origin;
	point direction;
	float r, g, b;
	int depth;
	double krg;
	double indRefr;
	bool inside;
	ray *reflected;
	float kRefl, kRefr;
	ray *refracted;

	void calculateValues()
	{
		double reflR = 0;
		double reflG = 0;
		double reflB = 0;
		double refrR = 0;
		double refrG = 0;
		double refrB = 0;
		if (reflected)
		{
			reflected->calculateValues();
			reflR = reflected->r;
			reflG = reflected->g;
			reflB = reflected->b;
		}
		if (refracted)
		{
			refracted->calculateValues();
			refrR = refracted->r;
			refrG = refracted->g;
			refrB = refracted->b;
		}
		r = r * krg + reflR * kRefl  + refrR * kRefr;
		g = g * krg + reflG * kRefl  + refrG * kRefr;
		b = b * krg + reflB * kRefl  + refrB * kRefr;
	}

} ray;


class MeshObject : public GraphicsObject
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
	Vertex* pNormList;
	int VertexCount;
	Face* pFaceList;
	int FaceCount;
	float ModelMatrix[16];
};