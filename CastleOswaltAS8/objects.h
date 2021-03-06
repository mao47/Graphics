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

//class point
//{
//public:
//	double x,y,z,w;
//
//	point(){ x = 0; y = 0; z = 0; w = 1;}
//	point(double xa, double ya, double za)
//	{
//		x = xa; y = ya; z = za; w = 1.0;
//	}
//	point(double xa, double ya, double za, double wa)
//	{
//		x = xa; y = ya; z = za; w = wa;
//	}
//};

//
//typedef struct _faceStruct {
//  int v1,v2,v3;
//  int n1,n2,n3;
//} faceStruct;



class MeshObject : public GraphicsObject
{
public:
	MeshObject();
	~MeshObject();
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