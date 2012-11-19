#define _USE_MATH_DEFINES

#include <math.h>
#include <iostream>
#include "objects.h"
#include "tri_intersect.h"

using namespace std;

Vertex::Vertex()
{
	x = y = z = 0;
	h = 1;
}

void Vertex::Normalize()
{
	x = x/h;
	y = y/h;
	z = z/h;
	h = 1;
}
Vertex Transform(float* matrix, Vertex& point)
{
	Vertex temp;
	temp.x = matrix[0]*point.x + matrix[4]*point.y + matrix[8]*point.z + matrix[12]*point.h;
	temp.y = matrix[1]*point.x + matrix[5]*point.y + matrix[9]*point.z + matrix[13]*point.h;
	temp.z = matrix[2]*point.x + matrix[6]*point.y + matrix[10]*point.z + matrix[14]*point.h;
	temp.h = matrix[3]*point.x + matrix[7]*point.y + matrix[11]*point.z + matrix[15]*point.h;
	return temp;
}

MeshObject::MeshObject()
{
	pBoundingBox = new Vertex[8];
	// Load the identity for the initial modeling matrix
	ModelMatrix[0] = ModelMatrix[5] = ModelMatrix[10] = ModelMatrix[15] = 1;
	ModelMatrix[1] = ModelMatrix[2] = ModelMatrix[3] = ModelMatrix[4] =
		ModelMatrix[6] = ModelMatrix[7] = ModelMatrix[8] = ModelMatrix[9] =
		ModelMatrix[11] = ModelMatrix[12] = ModelMatrix[13] = ModelMatrix[14]= 0;
}

MeshObject::~MeshObject()
{
	delete [] pBoundingBox;
	delete [] pVertexList;
	delete [] pFaceList;
}


intersection MeshObject::intersects(ray myRay)
{
	int i = 0;
	intersection besti;
	besti.type = type_none;
	float orig[3], dir[3], vert0[3], vert1[3], vert2[3];
	float t, u, v;
		
	orig[0] = myRay.origin.x; orig[1] = myRay.origin.y; orig[2] = myRay.origin.z;
	dir[0] = myRay.direction.x; dir[1] = myRay.direction.y; dir[2] = myRay.direction.z;

	for(i = 0; i < FaceCount; i ++)
	{
		Vertex ver = Transform(ModelMatrix, pVertexList[ pFaceList[i].v1 ]);
		vert0[0] = ver.x;	vert0[1] = ver.y;	vert0[2] = ver.z;

		ver = Transform(ModelMatrix, pVertexList[ pFaceList[i].v2 ]);
		vert1[0] = ver.x;	vert1[1] = ver.y;	vert1[2] = ver.z;

		ver = Transform(ModelMatrix, pVertexList[ pFaceList[i].v3 ]);
		vert2[0] = ver.x;	vert2[1] = ver.y;	vert2[2] = ver.z;

		if(intersect_triangle(orig, dir, vert0, vert1, vert2, &t, &u, &v) == 1)
		{
			intersection tempi;
			tempi.type = type_face;
			tempi.distance = (double)t;
			if((besti.type == type_none || besti.distance > tempi.distance) && tempi.distance > 0)
			{
				//new best intersection
				tempi.object = (GraphicsObject)*this;
				tempi.location.x = (1-u-v)*vert0[0] + u*vert1[0] + v*vert2[0];
				tempi.location.y = (1-u-v)*vert0[1] + u*vert1[1] + v*vert2[1];
				tempi.location.z = (1-u-v)*vert0[2] + u*vert1[2] + v*vert2[2];

				Vector U, V;
				U.i = vert1[0]-vert0[0]; U.j = vert1[1]-vert0[1]; U.k = vert1[2]-vert0[2];
				V.i = vert2[0]-vert0[0]; V.j = vert2[1]-vert0[1]; V.k = vert2[2]-vert0[2];
				tempi.normal.x = (U.j * V.k) - (U.k * V.j);
				tempi.normal.y = (U.k * V.i) - (U.i * V.k);
				tempi.normal.z = (U.i * V.j) - (U.j * V.i);
				float normalMag = sqrt(tempi.normal.x * tempi.normal.x + 
					tempi.normal.y * tempi.normal.y + tempi.normal.z * tempi.normal.z);
				tempi.normal.x /= normalMag;
				tempi.normal.y /= normalMag;
				tempi.normal.z /= normalMag;

				besti = tempi;
			}
		}
	}
	return besti;
}

// Load an MeshObject (.obj) file
void MeshObject::Load(char* file, float s, float rx, float ry, float rz,
				  float tx, float ty, float tz)
{
	FILE* pObjectFile = fopen(file, "r");
	if(!pObjectFile)
		cout << "Failed to load " << file << "." << endl;
	else
		cout << "Successfully loaded " << file << "." << endl;

	char DataType;
	float a, b, c;

	// Scan the file and count the faces and vertices
	VertexCount = FaceCount = 0;
	while(!feof(pObjectFile))
	{
		fscanf(pObjectFile, "%c %f %f %f\n", &DataType, &a, &b, &c);
		if(DataType == 'v')
            VertexCount++;
		else if(DataType == 'f')
			FaceCount++;
	}
	pVertexList = new Vertex[VertexCount];
	pFaceList = new Face[FaceCount];

	fseek(pObjectFile, 0L, SEEK_SET);

	cout << "Number of vertices: " << VertexCount << endl;
	cout << "Number of faces: " << FaceCount << endl;

	// Load and create the faces and vertices
	int CurrentVertex = 0, CurrentFace = 0;
	float MinimumX, MaximumX, MinimumY, MaximumY, MinimumZ, MaximumZ;
	while(!feof(pObjectFile))
	{
		fscanf(pObjectFile, "%c %f %f %f\n", &DataType, &a, &b, &c);
		if(DataType == 'v')
		{
			pVertexList[CurrentVertex].x = a;
			pVertexList[CurrentVertex].y = b;
			pVertexList[CurrentVertex].z = c;
			// Track maximum and minimum coordinates for use in bounding boxes
			if(CurrentVertex == 0)
			{
				MinimumX = MaximumX = a;
				MinimumY = MaximumY = b;
				MinimumZ = MaximumZ = c;
			}
			else
			{
				if(a < MinimumX)
					MinimumX = a;
				else if(a > MaximumX)
					MaximumX = a;
				if(b < MinimumY)
					MinimumY = b;
				else if(b > MaximumY)
					MaximumY = b;
				if(c < MinimumZ)
					MinimumZ = c;
				else if(c > MaximumZ)
					MaximumZ = c;
			}
			CurrentVertex++;
		}
		else if(DataType == 'f')
		{
			// Convert to a zero-based index for convenience
			pFaceList[CurrentFace].v1 = (int)a - 1;
			pFaceList[CurrentFace].v2 = (int)b - 1;
			pFaceList[CurrentFace].v3 = (int)c - 1;
			CurrentFace++;
		}
	}

	// Initialize the bounding box vertices
	pBoundingBox[0].x = MinimumX; pBoundingBox[0].y = MinimumY; pBoundingBox[0].z = MinimumZ;
	pBoundingBox[1].x = MaximumX; pBoundingBox[1].y = MinimumY; pBoundingBox[1].z = MinimumZ;
	pBoundingBox[2].x = MinimumX; pBoundingBox[2].y = MaximumY; pBoundingBox[2].z = MinimumZ;
	pBoundingBox[3].x = MaximumX; pBoundingBox[3].y = MaximumY; pBoundingBox[3].z = MinimumZ;
	pBoundingBox[4].x = MinimumX; pBoundingBox[4].y = MinimumY; pBoundingBox[4].z = MaximumZ;
	pBoundingBox[5].x = MaximumX; pBoundingBox[5].y = MinimumY; pBoundingBox[5].z = MaximumZ;
	pBoundingBox[6].x = MinimumX; pBoundingBox[6].y = MaximumY; pBoundingBox[6].z = MaximumZ;
	pBoundingBox[7].x = MaximumX; pBoundingBox[7].y = MaximumY; pBoundingBox[7].z = MaximumZ;

	// Apply the initial transformations in order
	LocalScale(s);
	WorldRotate((float)(M_PI*rx/180.0), (float)(M_PI*ry/180.0), (float)(M_PI*rz/180.0));
	WorldTranslate(tx, ty, tz);	
}

// Do world-based translation
void MeshObject::WorldTranslate(float tx, float ty, float tz)
{
	ModelMatrix[12] += tx;
	ModelMatrix[13] += ty;
	ModelMatrix[14] += tz;
}

// Perform world-based rotations in x,y,z order (intended for one-at-a-time use)
void MeshObject::WorldRotate(float rx, float ry, float rz)
{
	float temp[16];

	if(rx != 0)
	{
		float cosx = cos(rx), sinx = sin(rx);
		for(int i = 0; i < 16; i++)
			temp[i] = ModelMatrix[i];
		ModelMatrix[1] = temp[1]*cosx - temp[2]*sinx;
		ModelMatrix[2] = temp[2]*cosx + temp[1]*sinx;
		ModelMatrix[5] = temp[5]*cosx - temp[6]*sinx;
		ModelMatrix[6] = temp[6]*cosx + temp[5]*sinx;
		ModelMatrix[9] = temp[9]*cosx - temp[10]*sinx;
		ModelMatrix[10] = temp[10]*cosx + temp[9]*sinx;
		ModelMatrix[13] = temp[13]*cosx - temp[14]*sinx;
		ModelMatrix[14] = temp[14]*cosx + temp[13]*sinx;
	}

	if(ry != 0)
	{
		float cosy = cos(ry), siny = sin(ry);
		for(int i = 0; i < 16; i++)
			temp[i] = ModelMatrix[i];
		ModelMatrix[0] = temp[0]*cosy + temp[2]*siny;
		ModelMatrix[2] = temp[2]*cosy - temp[0]*siny;
		ModelMatrix[4] = temp[4]*cosy + temp[6]*siny;
		ModelMatrix[6] = temp[6]*cosy - temp[4]*siny;
		ModelMatrix[8] = temp[8]*cosy + temp[10]*siny;
		ModelMatrix[10] = temp[10]*cosy - temp[8]*siny;
		ModelMatrix[12] = temp[12]*cosy + temp[14]*siny;
		ModelMatrix[14] = temp[14]*cosy - temp[12]*siny;
	}

	if(rz != 0)
	{
		float cosz = cos(rz), sinz = sin(rz);
		for(int i = 0; i < 16; i++)
			temp[i] = ModelMatrix[i];
		ModelMatrix[0] = temp[0]*cosz - temp[1]*sinz;
		ModelMatrix[1] = temp[1]*cosz + temp[0]*sinz;
		ModelMatrix[4] = temp[4]*cosz - temp[5]*sinz;
		ModelMatrix[5] = temp[5]*cosz + temp[4]*sinz;
		ModelMatrix[8] = temp[8]*cosz - temp[9]*sinz;
		ModelMatrix[9] = temp[9]*cosz + temp[8]*sinz;
		ModelMatrix[12] = temp[12]*cosz - temp[13]*sinz;
		ModelMatrix[13] = temp[13]*cosz + temp[12]*sinz;
	}
}

// Perform locally-based rotations in x,y,z order (intended for one-at-a-time use)
void MeshObject::LocalRotate(float rx, float ry, float rz)
{
	float temp[16];

	if(rx != 0)
	{
		float cosx = cos(rx), sinx = sin(rx);
		for(int i = 0; i < 16; i++)
			temp[i] = ModelMatrix[i];
		ModelMatrix[4] = temp[4]*cosx + temp[8]*sinx;
		ModelMatrix[5] = temp[5]*cosx + temp[9]*sinx;
		ModelMatrix[6] = temp[6]*cosx + temp[10]*sinx;
		ModelMatrix[7] = temp[7]*cosx + temp[11]*sinx;
		ModelMatrix[8] = temp[8]*cosx - temp[4]*sinx;
		ModelMatrix[9] = temp[9]*cosx - temp[5]*sinx;
		ModelMatrix[10] = temp[10]*cosx - temp[6]*sinx;
		ModelMatrix[11] = temp[11]*cosx - temp[7]*sinx;
	}

	if(ry != 0)
	{
        float cosy = cos(ry), siny = sin(ry);
		for(int i = 0; i < 16; i++)
			temp[i] = ModelMatrix[i];
		ModelMatrix[0] = temp[0]*cosy - temp[8]*siny;
		ModelMatrix[1] = temp[1]*cosy - temp[9]*siny;
		ModelMatrix[2] = temp[2]*cosy - temp[10]*siny;
		ModelMatrix[3] = temp[3]*cosy - temp[11]*siny;
		ModelMatrix[8] = temp[8]*cosy + temp[0]*siny;
		ModelMatrix[9] = temp[9]*cosy + temp[1]*siny;
		ModelMatrix[10] = temp[10]*cosy + temp[2]*siny;
		ModelMatrix[11] = temp[11]*cosy + temp[3]*siny;
	}

	if(rz != 0)
	{
		float cosz = cos(rz), sinz = sin(rz);
		for(int i = 0; i < 16; i++)
			temp[i] = ModelMatrix[i];
		ModelMatrix[0] = temp[0]*cosz + temp[4]*sinz;
		ModelMatrix[1] = temp[1]*cosz + temp[5]*sinz;
		ModelMatrix[2] = temp[2]*cosz + temp[6]*sinz;
		ModelMatrix[3] = temp[3]*cosz + temp[7]*sinz;
		ModelMatrix[4] = temp[4]*cosz - temp[0]*sinz;
		ModelMatrix[5] = temp[5]*cosz - temp[1]*sinz;
		ModelMatrix[6] = temp[6]*cosz - temp[2]*sinz;
		ModelMatrix[7] = temp[7]*cosz - temp[3]*sinz;
	}
}

// Do locally-based uniform scaling
void MeshObject::LocalScale(float s)
{
	for(int i = 0; i <= 11; i++)
        ModelMatrix[i] = s*ModelMatrix[i];
}


