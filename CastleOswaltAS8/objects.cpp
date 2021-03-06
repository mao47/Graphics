#define _USE_MATH_DEFINES

#include <math.h>
#include <iostream>
#include "objects.h"


using namespace std;

//array defining triangles for bounding box intersection
int triangles[12][3];
int boxInit = 0;


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

void initBoundBoxTriangles()
{
	if(boxInit != 0 )
	{
		return;
	}
	//bottom
	triangles[0][0] = 0;
	triangles[0][1] = 1;
	triangles[0][2] = 2;

	triangles[1][0] = 2;
	triangles[1][1] = 3;
	triangles[1][2] = 4;

	//top
	triangles[2][0] = 4;
	triangles[2][1] = 5;
	triangles[2][2] = 6;

	triangles[3][0] = 5;
	triangles[3][1] = 6;
	triangles[3][2] = 7;

	//front
	triangles[4][0] = 0;
	triangles[4][1] = 2;
	triangles[4][2] = 4;

	triangles[5][0] = 2;
	triangles[5][1] = 4;
	triangles[5][2] = 6;

	//back
	triangles[6][0] = 1;
	triangles[6][1] = 3;
	triangles[6][2] = 5;

	triangles[7][0] = 3;
	triangles[7][1] = 5;
	triangles[7][2] = 7;

	//left
	triangles[8][0] = 0;
	triangles[8][1] = 1;
	triangles[8][2] = 5;

	triangles[9][0] = 0;
	triangles[9][1] = 4;
	triangles[9][2] = 5;

	//right
	triangles[10][0] = 2;
	triangles[10][1] = 3;
	triangles[10][2] = 6;

	triangles[11][0] = 3;
	triangles[11][1] = 6;
	triangles[11][2] = 7;
	
	
	boxInit = 1;

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


// Load an MeshObject (.obj) file
void MeshObject::Load(char* file, float s, float rx, float ry, float rz,
				  float tx, float ty, float tz)
{
	FILE* pObjectFile = fopen(file, "r");
	if(!pObjectFile)
		cout << "Failed to load " << file << "." << endl;
	else
		cout << "Successfully loaded " << file << "." << endl;
	int *normCount;
	char DataType;
	float a, b, c, len;
	int i;
	Vertex v1, v2, crossP;

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
	pNormList = new Vertex[VertexCount];

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

	// Apply the initial transformations in order
	LocalScale(s);
	WorldRotate((float)(M_PI*rx/180.0), (float)(M_PI*ry/180.0), (float)(M_PI*rz/180.0));
	WorldTranslate(tx, ty, tz);	
	
	normCount = (int *)malloc(sizeof(int)*VertexCount);
	for(i = 0; i < VertexCount; i++)
	{
		pVertexList[i] = Transform(ModelMatrix, pVertexList[i]);
		pNormList[i].x = pNormList[i].y = pNormList[i].z = 0.0;
		normCount[i] = 0;
	}
	for(i = 0; i < FaceCount; i++)
	{
		v1.x = pVertexList[pFaceList[i].v2].x - pVertexList[pFaceList[i].v1].x;
		v1.y = pVertexList[pFaceList[i].v2].y - pVertexList[pFaceList[i].v1].y;
		v1.z = pVertexList[pFaceList[i].v2].z - pVertexList[pFaceList[i].v1].z;
		v2.x = pVertexList[pFaceList[i].v3].x - pVertexList[pFaceList[i].v2].x;
		v2.y = pVertexList[pFaceList[i].v3].y - pVertexList[pFaceList[i].v2].y;
		v2.z = pVertexList[pFaceList[i].v3].z - pVertexList[pFaceList[i].v2].z;

		
      crossP.x = v1.y*v2.z - v1.z*v2.y;
      crossP.y = v1.z*v2.x - v1.x*v2.z;
      crossP.z = v1.x*v2.y - v1.y*v2.x;

      len = sqrt(crossP.x*crossP.x + crossP.y*crossP.y + crossP.z*crossP.z);

      crossP.x = -crossP.x/len;
      crossP.y = -crossP.y/len;
      crossP.z = -crossP.z/len;

      pNormList[pFaceList[i].v1].x = pNormList[pFaceList[i].v1].x + crossP.x;
      pNormList[pFaceList[i].v1].y = pNormList[pFaceList[i].v1].y + crossP.y;
      pNormList[pFaceList[i].v1].z = pNormList[pFaceList[i].v1].z + crossP.z;
      pNormList[pFaceList[i].v2].x = pNormList[pFaceList[i].v2].x + crossP.x;
      pNormList[pFaceList[i].v2].y = pNormList[pFaceList[i].v2].y + crossP.y;
      pNormList[pFaceList[i].v2].z = pNormList[pFaceList[i].v2].z + crossP.z;
      pNormList[pFaceList[i].v3].x = pNormList[pFaceList[i].v3].x + crossP.x;
      pNormList[pFaceList[i].v3].y = pNormList[pFaceList[i].v3].y + crossP.y;
      pNormList[pFaceList[i].v3].z = pNormList[pFaceList[i].v3].z + crossP.z;
      normCount[pFaceList[i].v1]++;
      normCount[pFaceList[i].v2]++;
      normCount[pFaceList[i].v3]++;
	}
	for (i = 0;i < VertexCount;i++)
    {
		float magn = sqrt(pNormList[i].x*pNormList[i].x + pNormList[i].y*pNormList[i].y + pNormList[i].z*pNormList[i].z);
      pNormList[i].x = -1.0*pNormList[i].x / magn;// (float)normCount[i];
      pNormList[i].y = -1.0*pNormList[i].y / magn;// (float)normCount[i];
      pNormList[i].z = -1.0*pNormList[i].z / magn;// (float)normCount[i];
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


	for(i = 0; i < 16; i ++)
	{
		ModelMatrix[i] = 0.0;
	}
	ModelMatrix[0] = ModelMatrix[5] = ModelMatrix[10] = ModelMatrix[15]= 1.0;
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


