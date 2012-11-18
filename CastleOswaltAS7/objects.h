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

class MeshObject
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