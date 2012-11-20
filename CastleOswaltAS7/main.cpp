#include <stdio.h>
#include <stdlib.h>
#include "glut.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#include "frame_buffer.h"
#include "primitives.h"
#include "color.h"
#include "objects.h"
#include "tri_intersect.h"
#include <vector>

#include <iostream>
#include <fstream>

#define ON 1
#define OFF 0


using namespace std;



// Global variables
int window_width, window_height;    // Window dimensions

const int INITIAL_RES = 400;

float imgPlnSize, imgPlnDist;

FrameBuffer* fb;
MeshObject* meshList;
int meshCount;

intersection getIntersections(ray *myRay);
void localIllumination(float &r, float &g, float &b, intersection objIntersection, ray *myRay);

typedef struct sphere : GraphicsObject {
	point center;
	double radius;

	intersection intersects(ray r)
	{
		intersection i;
		i.type = type_none;
		double diffX = r.origin.x - center.x;
		double diffY = r.origin.y - center.y;
		double diffZ = r.origin.z - center.z;
		double a = r.direction.x * r.direction.x + 
			r.direction.y * r.direction.y +
			r.direction.z * r.direction.z;
		double b = 2.0 * (r.direction.x * diffX +
			r.direction.y * diffY + 
			r.direction.z * diffZ);
		double c = diffX * diffX + diffY * diffY + diffZ * diffZ - radius * radius;

		double disc = b * b - 4 * a * c;
		if (disc < 0)
			return i;
		double t = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
		if (t >= 0)
		{
			double inv = 1.0 / radius;
			i.location.x = r.origin.x + t * r.direction.x;
			i.location.y = r.origin.y + t * r.direction.y;
			i.location.z = r.origin.z + t * r.direction.z;
			i.normal.x = (i.location.x - center.x) / inv;
			i.normal.y = (i.location.y - center.y) / inv;
			i.normal.z = (i.location.z - center.z) / inv;
			i.distance = t;
			i.object = (GraphicsObject)*this;
			i.type = type_circle;
			return i;
		}

		t = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);

		if (t < 0)
			return i;

		double inv = 1.0 / radius;
		i.location.x = r.origin.x + t * r.direction.x;
		i.location.y = r.origin.y + t * r.direction.y;
		i.location.z = r.origin.z + t * r.direction.z;
		i.normal.x = (i.location.x - center.x) / inv;
		i.normal.y = (i.location.y - center.y) / inv;
		i.normal.z = (i.location.z - center.z) / inv;
		i.distance = t;
		i.object = (GraphicsObject)*this;

		return i;

	}
} sphere;
void calcReflectedRay(intersection i, ray *r);
void calcRefractedRay(intersection i, ray *r);
void shootRay(ray *myRay);

LightSource *lightList;
sphere *sphereList;
int sphereCount, lightCount;

void layoutReader(char *filename)
{
	FILE *fp;
	int lights, spheres, meshes;
	int i;
	char letter;
	float x,y,z,r,g,b,radius;
	float rAmb, gAmb, bAmb;
	float rDiff, gDiff, bDiff;
	float rSpec, gSpec, bSpec;
	float kAmb, kDiff, kSpec;
	float specExp, indRefr;
	float kRefl, kRefr;
	int lightType;

	fp = fopen(filename, "r");
	if(fp == NULL)
	{
		printf("Cannot open %s\n!", filename);
		exit(0);
	}
	if(!feof(fp))
	{
		fscanf(fp, "%d %d %d\n", &lights, &spheres, &meshes);
	}
	meshCount = meshes;
	lightCount = lights;
	sphereCount = spheres;

	sphereList = new sphere[sphereCount];
	meshList = new MeshObject[meshCount];//(MeshObject *)malloc(sizeof(MeshObject)*meshCount);
	lightList = (LightSource *)malloc(sizeof(LightSource)*lights);
 
	//read in light sources
	i = 0;
	while(!feof(fp) && i < lights)
	{
		fscanf(fp, "%c %d %f %f %f %f %f %f\n", &letter, &lightType,
			&x, &y, &z, &r, &g, &b);

		if(letter == 'L')
		{
			lightList[i].type = lightType;
			lightList[i].x = x;
			lightList[i].y = y;
			lightList[i].z = z;
			lightList[i].r = r;
			lightList[i].g = g;
			lightList[i].b = b;
		}
		else
		{
			printf("Expected L but read %c\n", letter);
		}
		i++;
	}
	i = 0;
	while (!feof(fp) && i < spheres)
	{
		fscanf(fp, "%c %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", &letter, 
			&x, &y, &z, &radius, 
			&rAmb, &gAmb, &bAmb, 
			&rDiff, &gDiff, &bDiff,
			&rSpec, &gSpec, &bSpec,
			&kAmb, &kDiff, &kSpec,
			&specExp, &indRefr,
			&kRefl, &kRefr);
		if (letter == 'S')
		{
			sphere s;
			s.center.x = x;
			s.center.y = y;
			s.center.z = z;
			s.radius = radius;
			s.rAmb = rAmb;
			s.gAmb = gAmb;
			s.bAmb = bAmb;
			s.rSpec = rSpec;
			s.gSpec = gSpec;
			s.bSpec = bSpec;
			s.rDiff = rDiff;
			s.gDiff = gDiff;
			s.bDiff = bDiff;
			s.kAmb = kAmb;
			s.kDiff = kDiff;
			s.kSpec = kSpec;
			s.specExp = specExp;
			s.indRefr = indRefr;
			s.kRefl = kRefl;
			s.kRefr = kRefr;
			sphereList[i] = s;
		}
		else
		{
			printf("Expected S but read %c\n", letter);
		}
		i++;
	}

	i = 0;
	while(!feof(fp) && i < meshes)
	{
		char MeshFile[255];
		float scale, rx, ry, rz, tx, ty, tz,	/*scale, rotation, and transformation*/
			ar, ag, ab, dr, dg, db, sr, sg, sb, /*ambient, diffuse, specular reflection*/ 
			ka, kd, ks, sexp,					/*coefficients, specular exponent*/ 
			r, krefl, krefr;					/*index of refraction, k reflect & refract */ 

		fscanf(fp, "%c %s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
			&letter, &MeshFile, &scale, &rx, &ry, &rz, &tx, &ty, &tz, &ar, &ag, &ab,
			&dr, &dg, &db, &sr, &sg, &sb, &ka, &kd, &ks, &sexp, &r, &krefl, &krefr);
		if(letter == 'M')
		{
			meshList[i].rAmb = ar;
			meshList[i].gAmb = ag;
			meshList[i].bAmb = ab;
			meshList[i].rDiff = dr;
			meshList[i].gDiff = dg;
			meshList[i].bDiff = db;
			meshList[i].rSpec = sr;
			meshList[i].gSpec = sg;
			meshList[i].bSpec = sb;
			meshList[i].kAmb = ka;
			meshList[i].kDiff = kd;
			meshList[i].kSpec = ks;
			meshList[i].specExp = sexp;
			meshList[i].indRefr = r;
			meshList[i].kRefl = krefl;
			meshList[i].kRefr = krefr;
			meshList[i].Load(MeshFile, scale, rx, ry, rz, tx, ty, tz);
		}
		else
		{
			printf("Expected M but read %c\n", letter);
		}
		i++;
	}
	fclose(fp);
}


void drawRect(double x, double y, double w, double h)
{
	glVertex2f(x,y);
	glVertex2f(x+w,y);
	glVertex2f(x+w,y+h);
	glVertex2f(x, y+h);
}

void normalize(point &p)
{
	double invlength = 1.0 / sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

void calcRefractedRay(intersection i, ray *r)
{
	if (i.object.kRefr <= 0)
		return;

	ray *refracted = new ray();

	// normalize vectors
	normalize(i.normal);

	double ratioIndRefr;

	if (r->inside)
	{
		ratioIndRefr = i.object.indRefr;
	}
	else
	{
		ratioIndRefr = i.object.indRefr;
	}

	double rdotn = r->direction.x * i.normal.x + r->direction.y * i.normal.y + r->direction.z * i.normal.z;
	double k = 1.0 - ratioIndRefr * ratioIndRefr * (1.0 - rdotn * rdotn);



	if (k >= 0.0)
	{
		refracted->direction.x = ratioIndRefr * r->direction.x - (ratioIndRefr * rdotn + sqrt(k)) * i.normal.x;
		refracted->direction.y = ratioIndRefr * r->direction.y - (ratioIndRefr * rdotn + sqrt(k)) * i.normal.y;
		refracted->direction.z = ratioIndRefr * r->direction.z - (ratioIndRefr * rdotn + sqrt(k)) * i.normal.z;
		refracted->origin = i.location;

		refracted->inside = !r->inside;

		r->refracted = refracted;
		r->kRefl = i.object.kRefl;
		r->kRefr = i.object.kRefr;
		refracted->depth = r->depth;


		shootRay(refracted);
	}
	else
	{
		delete refracted;
	}

}

void calcReflectedRay(intersection i, ray *r)
{
	if (i.object.kRefl > 0) // object is reflecting
	{
		//calculate reflection vector
		ray * reflected = new ray();
		reflected->depth = r->depth ;
		// normalize normal vector
		normalize(i.normal);

		double rdotn = r->direction.x * i.normal.x + r->direction.y * i.normal.y + r->direction.z * i.normal.z;

		reflected->direction.x	= r->direction.x - 2.0 * rdotn * i.normal.x;
		reflected->direction.y	= r->direction.y - 2.0 * rdotn * i.normal.y;
		reflected->direction.z	= r->direction.z - 2.0 * rdotn * i.normal.z;

		//normalize reflection direction
		normalize(r->direction);

		reflected->origin = i.location;

		r->reflected = reflected;
		r->kRefl = i.object.kRefl;
		r->kRefr = i.object.kRefr;
		reflected->depth = r->depth;

		shootRay(reflected);
	}
}

void shootRay(ray *myRay)
{
	// normalize ray 
	normalize(myRay->direction);
	
	myRay->r = 0;
	myRay->g = 0;
	myRay->b = 0;

	intersection objIntersection = getIntersections(myRay);

	//select closest point and object
	if(objIntersection.type == type_none)
	{
		myRay = NULL;
		return;
	}
	myRay->kRefl = objIntersection.object.kRefl;
	myRay->kRefr = objIntersection.object.kRefr;

	localIllumination(myRay->r, myRay->g, myRay->b, objIntersection, myRay);
	
	// attenuate
	myRay->krg = 1.0;// / (objIntersection.distance * objIntersection.distance);
	//myRay->r = objIntersection.object.

	myRay->depth --;
	if(myRay->depth > 0)
	{
		calcReflectedRay(objIntersection, myRay);
		calcRefractedRay(objIntersection, myRay);
	}
	else
		myRay = NULL;
}

void localIllumination(float &r, float &g, float &b, intersection objIntersection, ray *myRay)
{
	//ambient part independent of light sources
	r = 0.3 * objIntersection.object.rAmb * objIntersection.object.kAmb;
	g = 0.3 * objIntersection.object.gAmb * objIntersection.object.kAmb;
	b = 0.3 * objIntersection.object.bAmb * objIntersection.object.kAmb;
	
	//sum over light sources for specular and diffuse
	int i;
	for(i = 0; i < lightCount; i ++)
	{
		Vector L;
		if(lightList[i].type == light_direction)
		{
			L.i = -lightList[i].x;
			L.j = -lightList[i].y;
			L.k = -lightList[i].z;
		}
		else //point source
		{
			L.i = lightList[i].x - objIntersection.location.x;
			L.j = lightList[i].y - objIntersection.location.y;
			L.k = lightList[i].z - objIntersection.location.z;
		}

		float magnL = sqrt(L.i*L.i + L.j*L.j + L.k*L.k);
		L.i /= magnL;
		L.j /= magnL;
		L.k /= magnL;

		//N dot L for diffuse component
		float ndotl = (L.i*objIntersection.normal.x + L.j*objIntersection.normal.y
			+ L.k*objIntersection.normal.z);

		Vector R; // perfect reflection vector;
		R.i = L.i - 2*ndotl*objIntersection.normal.x;
		R.j = L.j - 2*ndotl*objIntersection.normal.y;
		R.k = L.k - 2*ndotl*objIntersection.normal.z;

		Vector V;
		V.i = -myRay->direction.x;
		V.j = -myRay->direction.y;
		V.k = -myRay->direction.z;
		float magV = sqrt(V.i*V.i + V.j*V.j + V.k*V.k);
		V.i /= magV;	V.j /= magV;	V.k /= magV;

		float rdotv = (V.i*R.i + V.j*R.j + V.k*R.k);
		float rdotvexp = pow(rdotv, (float)objIntersection.object.specExp);

		r += lightList[i].r * (objIntersection.object.kDiff * objIntersection.object.rDiff
			* ndotl + objIntersection.object.kSpec * objIntersection.object.rSpec * rdotvexp);
		g += lightList[i].g * (objIntersection.object.kDiff * objIntersection.object.gDiff
			* ndotl + objIntersection.object.kSpec * objIntersection.object.gSpec * rdotvexp);
		b += lightList[i].b * (objIntersection.object.kDiff * objIntersection.object.bDiff
			* ndotl + objIntersection.object.kSpec * objIntersection.object.bSpec * rdotvexp);

	}
}

intersection getIntersections(ray *myRay)
{
	
	//get intersections
	double distance = 10000000000;
	//spheres
	int i = 0;
	intersection objIntersection;
	objIntersection.type = type_none;
	
	for( i = 0; i < sphereCount; i++)
	{
		double tempdist;
		intersection inter = sphereList[i].intersects(*myRay);
		if(inter.type != type_none)
		{
			if(inter.distance < distance && inter.distance > 0.001)
			{
				distance = inter.distance;
				objIntersection = inter;
			}
		}
	}
	//meshes
	
	for(i = 0; i < meshCount; i++)
	{
		intersection inter = meshList[i].intersects(*myRay);
		if(inter.type != type_none)
		{
			if(inter.distance < distance && inter.distance > 0.001)
			{
				distance = inter.distance;
				objIntersection = inter;
			}
		}
	}
	return objIntersection;
}

// The display function. It is called whenever the window needs
// redrawing (ie: overlapping window moves, resize, maximize)
// You should redraw your polygons here
void	display(void)
{
    // Clear the background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	double w = 10/double(fb->GetWidth());
	double h = 10/double(fb->GetHeight());

	Color cl;
	glColor3f(0,0,1);

	glBegin(GL_QUADS);

	printf("width %d, height %d\n", fb->GetWidth(), fb->GetHeight());

	point worldPoint;

	for(int y = 0; y < fb->GetHeight(); y++)
	{
		for(int x = 0; x < fb->GetWidth(); x++)
		{
			cl = fb->GetPixel(x, y).color;
			glColor3f(cl.r, cl.g, cl.b);
			drawRect(w*x, h*y, w, h);
		}
	}

	glEnd();
    glutSwapBuffers();
}


// This function is called whenever the window is resized. 
// Parameters are the new dimentions of the window
void	resize(int x,int y)
{
    glViewport(0,0,x,y);
    window_width = x;
    window_height = y;
    
    printf("Resized to %d %d\n",x,y);
}


// This function is called whenever the mouse is pressed or released
// button is a number 0 to 2 designating the button
// state is 1 for release 0 for press event
// x and y are the location of the mouse (in window-relative coordinates)
void	mouseButton(int button,int state,int x,int y)
{
   ;
}


//This function is called whenever the mouse is moved with a mouse button held down.
// x and y are the location of the mouse (in window-relative coordinates)
void	mouseMotion(int x, int y)
{
	;
}


void renderScene()
{
	printf("rendering...");
	Color cl;
		ray *r;
	float width = fb->GetWidth() / 2.0;
	float height = fb->GetHeight() / 2.0;
	for(int y = 0; y < fb->GetHeight(); y++)
	{
		for(int x = 0; x < fb->GetHeight(); x++)
		{
			r = new ray();
			r->depth = 5;
			r->inside = false;
			r->direction.x = imgPlnSize * (x + 0.5 - width) / width;
			r->direction.y = imgPlnSize * (y + 0.5 - height) / height;
			r->direction.z = -imgPlnDist;
			shootRay(r);
			r->calculateValues();
			cl.r = r->r;
			cl.g = r->g;
			cl.b = r->b;
			fb->SetPixel(x, y, cl);
			//cl = fb->buffer[x][y].color;
			//glColor3f(cl.r, cl.g, cl.b);

			//drawRect(w*x, h*y, w, h);
		}
	}
	printf("\n  render complete\n");
}


// This function is called whenever there is a keyboard input
// key is the ASCII value of the key pressed
// x and y are the location of the mouse
void	keyboard(unsigned char key, int x, int y)
{
    switch(key) {
    case 'q':                           /* Quit */
		exit(1);
		break;
	case '-':
		fb->Resize(fb->GetHeight()/2, fb->GetWidth()/2);
		//BresenhamLine(fb, fb->GetWidth()*0.1, fb->GetHeight()*0.1, fb->GetWidth()*0.9, fb->GetHeight()*0.9, Color(1,0,0));
		break;
	case '=':
		fb->Resize(fb->GetHeight()*2, fb->GetWidth()*2);
		//BresenhamLine(fb, fb->GetWidth()*0.1, fb->GetHeight()*0.1, fb->GetWidth()*0.9, fb->GetHeight()*0.9, Color(1,0,0));
		break;
	case '[':
		imgPlnDist = imgPlnDist / 1.25;
		printf("Image plane distance: %f \n", imgPlnDist); 
		break;
	case ']':
		imgPlnDist = imgPlnDist * 1.25;
		printf("Image plane distance: %f \n", imgPlnDist); 
		break;
	case ',':
		imgPlnSize = imgPlnSize / 1.25;
		printf("Image plane size: %f \n", imgPlnSize); 
		break;
	case '.':
		imgPlnSize = imgPlnSize * 1.25;
		printf("Image plane size: %f \n", imgPlnSize); 
		break;
	case 'r':
		renderScene();
		break;
    default:
		break;
    }

    // Schedule a new display event
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{    
	imgPlnSize = 5.0;
	imgPlnDist = 8.0;
	fb = new FrameBuffer(256, 256);

	//BresenhamLine(fb, fb->GetWidth()*0.1, fb->GetHeight()*0.1, fb->GetWidth()*0.9, fb->GetHeight()*0.9, Color(1,0,0));

	layoutReader("../samples/red_sphere_and_teapot.rtl");
	

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Raytracer");
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);

    // Initialize GL
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,10,0,10,-10000,10000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);

	renderScene();

    // Switch to main loop
    glutMainLoop();
    return 0;        
}
