/*

EECS 366/466 COMPUTER GRAPHICS
Template for Assignment 8-MAPPING
Spring 2006

*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>

#include <string.h>
#include "glut.h"
#include <windows.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include "glprocs.h"
# include "assign8_temp.h"
#include "read_tga.h"
#include "objects.h"


#define PI 3.14159265359

#define PrintOpenGLError()::PrintOGLError(__FILE__, __LINE__)

using namespace std;


//object related information
int verts, faces, norms;    // Number of vertices, faces and normals in the system
point *vertList, *normList; // Vertex and Normal Lists
faceStruct *faceList;	    // Face List


//Illimunation and shading related declerations
char *shaderFileRead(char *fn);
GLuint vertex_shader,fragment_shader,p;
int illimunationMode = 0;
int shadingMode = 0;
int lightSource = 0;
int program=-1;


//Parameters for Copper (From: "Computer Graphics Using OpenGL" BY F.S. Hill, Jr.) 
//GLfloat ambient_cont [] = {0.19125,0.0735,0.0225};
GLfloat ambient_cont [] = {0.1,0.1,0.1};
GLfloat diffuse_cont [] = {0.7038,0.27048,0.0828};
//GLfloat specular_cont [] = {0.256777,0.137622,0.086014};
GLfloat specular_cont [] = {1,1,1};
GLfloat exponent = 15;//set in setTexture, bump looks better with different value


//Projection, camera contral related declerations
int WindowWidth,WindowHeight;
bool LookAtObject = false;
bool ShowAxes = true;



float CameraRadius = 10;
float CameraTheta = PI / 2;
float CameraPhi = PI / 2;
int MouseX = 0;
int MouseY = 0;
bool MouseLeft = false;
bool MouseRight = false;

GLuint myTexture;
GLuint colorTexture, bumpTexture;


point origin = {0,0,0};
int algSelect = 0;
int algCount = 8;
char algType [] = {'t','t','t','t','t','e','e'/*,'e','e'*/,'b'/*,'b'*/};//texture, environment, bump
char objType [] = {'p','s','t','s','t','s','t'/*,'s','t'*/,'p'/*,'s'*/};//plane, sphere, teapot
char mapType [] = {'p','p','p','s','s','s','s'/*,'c','c'*/,'p'/*,'s'*/};//planar, spherical
char* textureName;
char* grayName;
bool bumpMap = false;
bool envMap = false;
MeshObject* obj;
int objSelect = 0;

GLuint LoadTexture( const char * filename)
{
	GLuint id;	
	// Load image from tga file
	//TGA *TGAImage	= new TGA("./sphericalenvironmentmap/house2.tga");
	TGA *TGAImage	= new TGA(filename);
	//TGA *TGAImage	= new TGA("./cubicenvironmentmap/cm_right.tga");

	// Use to dimensions of the image as the texture dimensions
	uint width	= TGAImage->GetWidth();
	uint height	= TGAImage->GetHeigth();
	
	// The parameters for actual textures are changed

	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_2D, id);


	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


	// Finaly build the mipmaps
	glTexImage2D (GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, TGAImage->GetPixels());

	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, TGAImage->GetPixels());


	glEnable( GL_TEXTURE_2D );

	glBindTexture (GL_TEXTURE_2D, id); 

    delete TGAImage;
	return id;
}



void setObject(char obj)
{
	if(obj == 't')
		objSelect = 2;
	else if (obj == 'p')
		objSelect = 0;
	else
		objSelect = 1;
}
void setTexture(char alg, char obj, char map)
{
	bumpMap = false;
	envMap = false;
	exponent = 15;

	if(alg == 't')
	{
		if(map == 'p')
			textureName = "./planartexturemap/abstract2.tga";
		else if (map == 's')
			textureName = "./sphericaltexturemap/earth2.tga";
	}
	else if (alg == 'e')
	{
		envMap = true;
		if(map == 's')
			textureName = "./sphericalenvironmentmap/house2.tga";
	}
	else if (alg == 'b')
	{
		exponent = 30;
		if(map == 'p')
		{
			textureName = "./planarbumpmap/abstract2.tga";
			grayName = "./planarbumpmap/abstract_gray2.tga";
			//myTexture = LoadTexture(grayName);
			bumpMap = true;
		}
		else if (map == 's')
		{
			textureName = "./sphericalbumpmap/earth2.tga";
			grayName = "./planarbumpmap/earth_gray2.tga";
			bumpMap = true;
		}
	}
	myTexture= LoadTexture(textureName);
}

float magnitude(point p)
{
	return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

point normalize(point &v)
{
	float mag = magnitude(v);
	v.x /= mag;
	v.y /= mag;
	v.z /= mag;
	return v;
}

point calcHalfwayVector(point view, point light)
{
	point combined;
	combined.x = view.x + light.x;
	combined.y = view.y + light.y;
	combined.z = view.z + light.z;
	
	float combinedMagnitude = magnitude(combined) * 2.0;
	combined.x /= combinedMagnitude;
	combined.y /= combinedMagnitude;
	combined.z /= combinedMagnitude;

	return combined;
}

point getSphericalTextureCoordinates(point pos, point center) // note: only x and y are returned
{
	point p;
	point normal = {pos.x - center.x, pos.y - center.y, pos.z - center.z};
	normalize(normal);
	p.x = asin(normal.x) / PI + 0.5;
	p.y = (asin(normal.y) / PI + 0.5);
	return p;
}

point getSphericalTextureCoordinates(point pos) // assumes center is (0, 0, 0)
{
	point p;
	normalize(pos) ;
	p.x = -(atan(pos.z/pos.x) /(2*PI) + PI/2);
	//if(pos.x > 0) p.x += 0.5;
	p.y = -(acos(pos.y) /(PI));
	//p.y = acos(pos.z) / PI; //z = R cos(v)
	//p.x = acos(pos.x / (sin(PI*p.y)))/ (2*PI);
	p.y *= 1;
	p.y += 0;
	p.x *= 1;
	p.x += 0;
	return p;

}

point getPlanarTextureCoordinates(point pos, float rangeX, float rangeY)
{
	point p;
	p.z = 0;
	p.x = pos.x / rangeX;
	p.x += 0.5;
	p.y = pos.y / rangeY;
	p.y += 0.5;
	return p;
}

point texCoord(char map, point vert)
{
	if(map == 'p')
		return getPlanarTextureCoordinates(vert,2,2);
	else
		return getSphericalTextureCoordinates(vert);
}

//convert vertex to point
point v2p(Vertex vert)
{
	point p;
	p.x = vert.x;
	p.y = vert.y;
	p.z = vert.z;
	return p;
}
void DisplayFunc(void) 
{
    GLuint id ;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//load projection and viewing transforms
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
        
	gluPerspective(60,(GLdouble) WindowWidth/WindowHeight,0.01,10000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(CameraRadius*cos(CameraTheta)*sin(CameraPhi),
			  CameraRadius*cos(CameraPhi),
			  CameraRadius*sin(CameraTheta)*sin(CameraPhi),
			  0,0,0,
			  0,1,0);

	glEnable(GL_DEPTH_TEST);	
	glEnable(GL_TEXTURE_2D);

	setParameters(program);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for(int i = 0; i < obj[objSelect].FaceCount; i ++)
	{
		glBegin(GL_TRIANGLES);
			point v1, v2, v3, n1, n2, n3, tx1, tx2, tx3;
			v1 = v2p(	obj[objSelect].pVertexList[obj[objSelect].pFaceList[i].v1] );	//v1 = vertList[faceList[i].v1];
			v2 = v2p(	obj[objSelect].pVertexList[obj[objSelect].pFaceList[i].v2] );
			v3 = v2p(	obj[objSelect].pVertexList[obj[objSelect].pFaceList[i].v3] );
			n1 = v2p(	obj[objSelect].pNormList  [obj[objSelect].pFaceList[i].v1] ); //vertList[faceList[i].v1];
			n2 = v2p(	obj[objSelect].pNormList  [obj[objSelect].pFaceList[i].v2] );
			n3 = v2p(	obj[objSelect].pNormList  [obj[objSelect].pFaceList[i].v3] );
			tx1 = texCoord(mapType[algSelect], v1);
			tx2 = texCoord(mapType[algSelect], v2);
			tx3 = texCoord(mapType[algSelect], v3);

			if(objSelect == 0) //plane
			{
				n1.x = n1.y = 0;
				if(CameraRadius*sin(CameraTheta)*sin(CameraPhi) > 0)
					n1.z = 1;
				else
					n1.z = -1;
				n2 = n1;
				n3 = n2;
			}
			if(mapType[algSelect] == 's')
			{
				
				if(v1.x < 0 && v1.z > 0)
					tx1.x -= .5;
				if(v2.x < 0 && v2.z > 0)
					tx2.x -= .5;
				if(v3.x < 0 && v3.z > 0)
					tx3.x -= .5;

				if(v1.x < 0 && v1.z < 0)
					tx1.x += .5;
				if(v2.x < 0 && v2.z < 0)
					tx2.x += .5;
				if(v3.x < 0 && v3.z < 0)
					tx3.x += .5;


				//no matter what I try, can't get continuity in pacific ocean
				//if(v1.x < 0 && v1.z < 0)
				//{
				//	if(v2.z > 0) tx2.z += 2.;
				//	if(v3.z > 0) tx3.z += 2.;
				//}
				//else if(v2.x < 0 && v2.z < 0)
				//{
				//	if(v1.z > 0) tx1.z += 2.;
				//	if(v3.z > 0) tx3.z += 2.;
				//}
				//else if(v3.x < 0 && v3.z < 0)
				//{
				//	if(v1.z > 0) tx1.z += 2.;
				//	if(v2.z > 0) tx2.z += 2.;
				//}

			}


			glNormal3f(n1.x, n1.y, n1.z);
			glTexCoord2f (tx1.x, tx1.y);
			glVertex3f(v1.x, v1.y, v1.z);
			glNormal3f(n2.x, n2.y, n2.z);
			glTexCoord2f (tx2.x, tx2.y);
			glVertex3f(v2.x, v2.y, v2.z);
			glNormal3f(n3.x, n3.y, n3.z);
			glTexCoord2f (tx3.x, tx3.y);
			glVertex3f(v3.x, v3.y, v3.z);
		glEnd();
	}

	//glutSolidTeapot(1);
//	setParameters(program);
	glutSwapBuffers();
}

void ReshapeFunc(int x,int y)
{
    glViewport(0,0,x,y);
    WindowWidth = x;
    WindowHeight = y;
}


void MouseFunc(int button,int state,int x,int y)
{
	MouseX = x;
	MouseY = y;

    if(button == GLUT_LEFT_BUTTON)
		MouseLeft = !(bool) state;
	if(button == GLUT_RIGHT_BUTTON)
		MouseRight = !(bool) state;
}

void MotionFunc(int x, int y)
{
	if(MouseLeft)
	{
        CameraTheta += 0.01*PI*(MouseX - x);
		CameraPhi += 0.01*PI*(MouseY - y);
		if (CameraPhi > (PI - 0.01))
			CameraPhi = PI - 0.01;
		if (CameraPhi < 0.01)
			CameraPhi = 0.01;
	}
	if(MouseRight)
	{
        CameraRadius += 0.2*(MouseY-y);
		if(CameraRadius <= 0)
			CameraRadius = 0.2;
	}
    
	MouseX = x;
	MouseY = y;

	glutPostRedisplay();
}




//Motion and camera controls
void KeyboardFunc(unsigned char key, int x, int y)
{
    switch(key)
	{
	case 'A':
	case 'a':
		algSelect = (algSelect+1) % algCount;//ShowAxes = !ShowAxes;
		break;
	case 'Q':
	case 'q':
		exit(1);
		break;
	case 'w':
	case 'W':
		if (illimunationMode == 0)
		{
			illimunationMode = 1;
		}
		else
		{
			illimunationMode = 0;
		}
		break;
	case 'e':
	case 'E':
		if (shadingMode == 0)
		{
			shadingMode =1;
		}
		else
		{
			shadingMode =0;
		}
		break;
	case 'd':
	case 'D':
		if (lightSource == 0)
		{
			lightSource =1;
		}
		else
		{
			lightSource =0;
		}
		break;
	case 'f':
	case 'F':
		if (lightSource == 1)
		{
			//change color of the secondary light source at each key press, 
			//light color cycling through pure red, green, blue, and white.
		}
		break;

    default:
		break;
    }

	glutPostRedisplay();
}

int main(int argc, char **argv) 
{			  

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(320,320);
	glutCreateWindow("Assignment 8");



	glutDisplayFunc(DisplayFunc);
	glutReshapeFunc(ReshapeFunc);
	glutMouseFunc(MouseFunc);
    glutMotionFunc(MotionFunc);
    glutKeyboardFunc(KeyboardFunc);


	setShaders();
	
	meshReader("sphere.obj", 1);
	obj = new MeshObject[3];
	obj[0].Load("plane.obj",1,0,0,0,0,0,0);
	obj[1].Load("sphere.obj",1,0,0,0,0,0,0);
	obj[2].Load("teapot.obj",1,0,0,0,0,0,0);
	glutMainLoop();

	return 0;
}

/*************************************************************
Shader related methods,
Setting the shader files
Setting the shader variables
*************************************************************/

void error_exit(int status, char *text)
{

	// Print error message

	fprintf(stderr,"Internal Error %i: ", status);
	fprintf(stderr,text);
	printf("\nTerminating as Result of Internal Error.\nPress Enter to exit.\n");

	// Keep the terminal open

	int anyKey = getchar();

	// Exit program

	exit(status);
}

int PrintOGLError(char *file, int line)
{
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}


void setShaders() 
{

	char *vs = NULL,*fs = NULL;

	//create the empty shader objects and get their handles
	vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	

	//read the shader files and store the strings in corresponding char. arrays.
	vs = shaderFileRead("sampleshader.vert");
	fs = shaderFileRead("sampleshader.frag");

	const char * vv = vs;
	const char * ff = fs;

	GLint       vertCompiled, fragCompiled;

	//set the shader's source code by using the strings read from the shader files.
	glShaderSourceARB(vertex_shader, 1, &vv,NULL);
	glShaderSourceARB(fragment_shader, 1, &ff,NULL);

	free(vs);free(fs);

	//Compile the shader objects
	glCompileShaderARB(vertex_shader);
	glCompileShaderARB(fragment_shader);

	glGetObjectParameterivARB(fragment_shader, GL_OBJECT_COMPILE_STATUS_ARB, &fragCompiled);
	glGetObjectParameterivARB(vertex_shader, GL_OBJECT_COMPILE_STATUS_ARB, &vertCompiled);

    if (!vertCompiled || !fragCompiled)
	{
        cout<<"not compiled"<<endl;
	}
	
	//create an empty program object to attach the shader objects
	p = glCreateProgramObjectARB();

	//attach the shader objects to the program object
	glAttachObjectARB(p,vertex_shader);
	glAttachObjectARB(p,fragment_shader);

	/*
	**************
	Programming Tip:
	***************
	Delete the attached shader objects once they are attached.
	They will be flagged for removal and will be freed when they are no more used.
	*/
	glDeleteObjectARB(vertex_shader);
	glDeleteObjectARB(fragment_shader);

	//Link the created program.
	/*
	**************
	Programming Tip:
	***************
	You can trace the status of link operation by calling 
	"glGetObjectParameterARB(p,GL_OBJECT_LINK_STATUS_ARB)"
	*/
	glLinkProgramARB(p);
	GLint result;
	glGetObjectParameterivARB(p, GL_OBJECT_LINK_STATUS_ARB, &result);

	if (!result)
		printf("Failed to link file.\n");

	//Start to use the program object, which is the part of the current rendering state
	glUseProgramObjectARB(p);

	    
	setParameters(p);

	program =p;
}

//Gets the location of the uniform variable given with "name" in the memory
//and tests whether the process was successfull.
//Returns the location of the queried uniform variable
int getUniformVariable(GLuint program,char *name)
{
	int location = glGetUniformLocationARB(program, name);
	
	if (location == -1)
	{
		printf("%s\n", name);
 		error_exit(1007, "No such uniform variable");
	}
	PrintOpenGLError();
	return location;
}

void update_Light_Position()
{
	
	// Create light components
	GLfloat light_position[] = { CameraRadius*cos(CameraTheta)*sin(CameraPhi),			  
			  CameraRadius*cos(CameraPhi) , 
			  CameraRadius*sin(CameraTheta)*sin(CameraPhi),0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	

}

//Sets the light positions, etc. parameters for the shaders
void setParameters(GLuint program)
{
	int light_loc;
	int ambient_loc,diffuse_loc,specular_loc;
	int exponent_loc;
	int bump_loc, env_loc;

	//sample variable used to demonstrate how attributes are used in vertex shaders.
	//can be defined as gloabal and can change per vertex
	float tangent = 0.0;
	float tangent_loc;

	setTexture(algType[algSelect], objType[algSelect], mapType[algSelect]);
	setObject(objType[algSelect]);
	update_Light_Position();

	//Access uniform variables in shaders
	
	ambient_loc = getUniformVariable(program, "AmbientContribution");	
	glUniform3fvARB(ambient_loc,1, ambient_cont);

	specular_loc = getUniformVariable(program, "SpecularContribution");
	glUniform3fvARB(specular_loc,1,specular_cont);
	
	exponent_loc = getUniformVariable(program, "exponent");
	glUniform1fARB(exponent_loc,exponent);
	

	bump_loc = getUniformVariable(program, "bumpmapMode");
	glUniform1iARB(bump_loc, bumpMap);

	env_loc = getUniformVariable(program, "envmapMode");
	glUniform1iARB(env_loc, envMap);
	//Access attributes in vertex shader
	//tangent_loc = glGetAttribLocationARB(program,"tang");
	//glVertexAttrib1fARB(tangent_loc,tangent);
	
	GLint texLoc = getUniformVariable(program, "texture");
	glActiveTexture(GL_TEXTURE0);
	//glUniform1iARB(texLoc, 0);
	glBindTexture(GL_TEXTURE_2D, myTexture);




}


/****************************************************************
Utility methods:
shader file reader
mesh reader for objectt
****************************************************************/
//Read the shader files, given as parameter.
char *shaderFileRead(char *fn) {


	FILE *fp = fopen(fn,"r");
	if(!fp)
	{
		cout<< "Failed to load " << fn << endl;
		return " ";
	}
	else
	{
		cout << "Successfully loaded " << fn << endl;
	}
	
	char *content = NULL;

	int count=0;

	if (fp != NULL) 
	{
		fseek(fp, 0, SEEK_END);
		count = ftell(fp);
		rewind(fp);

		if (count > 0) 
		{
			content = (char *)malloc(sizeof(char) * (count+1));
			count = fread(content,sizeof(char),count,fp);
			content[count] = '\0';
		}
		fclose(fp);
	}
	return content;
}

void meshReader (char *filename,int sign)
{
  float x,y,z,len;
  int i;
  char letter;
  point v1,v2,crossP;
  int ix,iy,iz;
  int *normCount;
  FILE *fp;

  fp = fopen(filename, "r");
  if (fp == NULL) { 
    printf("Cannot open %s\n!", filename);
    exit(0);
  }

  // Count the number of vertices and faces
  while(!feof(fp))
   {
      fscanf(fp,"%c %f %f %f\n",&letter,&x,&y,&z);
      if (letter == 'v')
	  {
		  verts++;
	  }
      else
	  {
		  faces++;
	  }
   }

  fclose(fp);

  printf("verts : %d\n", verts);
  printf("faces : %d\n", faces);

  // Dynamic allocation of vertex and face lists
  faceList = (faceStruct *)malloc(sizeof(faceStruct)*faces);
  vertList = (point *)malloc(sizeof(point)*verts);
  normList = (point *)malloc(sizeof(point)*verts);

  fp = fopen(filename, "r");

  // Read the veritces
  for(i = 0;i < verts;i++)
    {
      fscanf(fp,"%c %f %f %f\n",&letter,&x,&y,&z);
      vertList[i].x = x;
      vertList[i].y = y;
      vertList[i].z = z;
    }

  // Read the faces
  for(i = 0;i < faces;i++)
    {
      fscanf(fp,"%c %d %d %d\n",&letter,&ix,&iy,&iz);
      faceList[i].v1 = ix - 1;
      faceList[i].v2 = iy - 1;
      faceList[i].v3 = iz - 1;
    }
  fclose(fp);


  // The part below calculates the normals of each vertex
  normCount = (int *)malloc(sizeof(int)*verts);
  for (i = 0;i < verts;i++)
    {
      normList[i].x = normList[i].y = normList[i].z = 0.0;
      normCount[i] = 0;
    }

  for(i = 0;i < faces;i++)
    {
      v1.x = vertList[faceList[i].v2].x - vertList[faceList[i].v1].x;
      v1.y = vertList[faceList[i].v2].y - vertList[faceList[i].v1].y;
      v1.z = vertList[faceList[i].v2].z - vertList[faceList[i].v1].z;
      v2.x = vertList[faceList[i].v3].x - vertList[faceList[i].v2].x;
      v2.y = vertList[faceList[i].v3].y - vertList[faceList[i].v2].y;
      v2.z = vertList[faceList[i].v3].z - vertList[faceList[i].v2].z;

      crossP.x = v1.y*v2.z - v1.z*v2.y;
      crossP.y = v1.z*v2.x - v1.x*v2.z;
      crossP.z = v1.x*v2.y - v1.y*v2.x;

      len = sqrt(crossP.x*crossP.x + crossP.y*crossP.y + crossP.z*crossP.z);

      crossP.x = -crossP.x/len;
      crossP.y = -crossP.y/len;
      crossP.z = -crossP.z/len;

      normList[faceList[i].v1].x = normList[faceList[i].v1].x + crossP.x;
      normList[faceList[i].v1].y = normList[faceList[i].v1].y + crossP.y;
      normList[faceList[i].v1].z = normList[faceList[i].v1].z + crossP.z;
      normList[faceList[i].v2].x = normList[faceList[i].v2].x + crossP.x;
      normList[faceList[i].v2].y = normList[faceList[i].v2].y + crossP.y;
      normList[faceList[i].v2].z = normList[faceList[i].v2].z + crossP.z;
      normList[faceList[i].v3].x = normList[faceList[i].v3].x + crossP.x;
      normList[faceList[i].v3].y = normList[faceList[i].v3].y + crossP.y;
      normList[faceList[i].v3].z = normList[faceList[i].v3].z + crossP.z;
      normCount[faceList[i].v1]++;
      normCount[faceList[i].v2]++;
      normCount[faceList[i].v3]++;
    }
  for (i = 0;i < verts;i++)
    {
      normList[i].x = (float)sign*normList[i].x / (float)normCount[i];
      normList[i].y = (float)sign*normList[i].y / (float)normCount[i];
      normList[i].z = (float)sign*normList[i].z / (float)normCount[i];
    }

}
