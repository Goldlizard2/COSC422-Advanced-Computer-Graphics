//====================================================================================
//  Mesh viewer using OpenMesh and OpenGL
//  COSC422 Advanced Computer Graphics, University of Canterbury (2023)
//  Key bindings:
//  'w'  : Toggle wireframe mode (Wireframe displayed as an overlay on filled model)
//  Arrow keys: Rotate model about x, y axes.
//====================================================================================
#define _USE_MATH_DEFINES // for C++  
#include <cmath>  
#include <iostream>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <GL/freeglut.h>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
using namespace std;

//----- Globals -----
typedef OpenMesh::TriMesh_ArrayKernelT<>  MyMesh;   //Mesh converted to triangle mesh
typedef OpenMesh::Decimater::DecimaterT<MyMesh> MyDecimater;
typedef OpenMesh::Decimater::ModQuadricT<MyMesh>::Handle HModQuadric;
MyMesh mesh;
float modelScale;
float xc, yc, zc;
float angleY = 0, angleX = 0;
bool wireframe = true;

void getBoundingBox(float& xmin, float& xmax, float& ymin, float& ymax, float& zmin, float& zmax)
{
	MyMesh::VertexIter vit = mesh.vertices_begin();
	MyMesh::Point pmin, pmax;

	pmin = pmax = mesh.point(*vit);

	//Iterate over the mesh using a vertex iterator
	for (vit = mesh.vertices_begin() + 1; vit != mesh.vertices_end(); vit++)
	{
		pmin.minimize(mesh.point(*vit));
		pmax.maximize(mesh.point(*vit));
	}
	xmin = pmin[0];  ymin = pmin[1];  zmin = pmin[2];
	xmax = pmax[0];  ymax = pmax[1];  zmax = pmax[2];
}


void initialize()
{
	float xmin, xmax, ymin, ymax, zmin, zmax;

	if (!OpenMesh::IO::read_mesh(mesh, "Dragon.obj"))
	{
		cerr << "read error\n";
	}
	getBoundingBox(xmin, xmax, ymin, ymax, zmin, zmax);
	cout << "Number of vertices: " << mesh.n_vertices() << endl;
	cout << "Number of faces: " << mesh.n_faces() << endl;

	if (mesh.has_face_normals())
	{
		cout << ">> Mesh has face normals" << endl;
	}
	else
	{
		cout << ">>Mesh does not have face normals" << endl;
		mesh.request_face_normals();
	}
	if (mesh.has_vertex_normals())
	{
		cout << ">> Mesh has vertex normals" << endl;
	}
	else
	{
		cout << ">>Mesh does not have vertex normals" << endl;
		mesh.request_vertex_normals();
	}

	MyDecimater decimater(mesh); // a decimater object, connected to a mesh
	HModQuadric hModQuadric; // QEM module
	decimater.add(hModQuadric); // register QEM module at the decimater
	decimater.initialize();
	cout << "Simplifying mesh ..." << endl;
	decimater.decimate_to(500); //Reduce vertex count
	cout << "Mesh simplification completed." << endl;

	mesh.garbage_collection();
	mesh.update_face_normals();
	mesh.update_vertex_normals();

	cout << "xmin, xmax = " << xmin << " " << xmax << endl;
	cout << "ymin, ymax = " << ymin << " " << ymax << endl;
	cout << "zmin, zmax = " << zmin << " " << zmax << endl;
	OpenMesh::Vec3f box = { xmax - xmin,  ymax - ymin, zmax - zmin };
	modelScale = 1.0/box.max();
	cout << "Model scale = " << modelScale << endl;
	xc = (xmin + xmax)*0.5;
	yc = (ymin + ymax)*0.5;
	zc = (zmin + zmax)*0.5;

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);	//Background colour

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(10, 1.0, 10, 100);
}

void special(int key, int x, int y)
{
	if (key == GLUT_KEY_LEFT) angleY--;
	else if (key == GLUT_KEY_RIGHT) angleY++;
	else if (key == GLUT_KEY_UP) angleX--;
	else if (key == GLUT_KEY_DOWN) angleX++;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 'w') wireframe = !wireframe;
	glutPostRedisplay();
}

void drawObject()
{
	MyMesh::FaceIter fit;
	MyMesh::FaceHandle feh;
	MyMesh::FaceVertexIter fvit;
	MyMesh::Point p;
	MyMesh::Normal norm;
	int nvert;
	for (fit = mesh.faces_begin(); fit != mesh.faces_end(); fit++)
	{
		feh = *fit;
		norm = mesh.normal(feh);   //using face normals
		glNormal3fv(norm.data());

		glBegin(GL_TRIANGLES);
		for (fvit = mesh.fv_iter(feh); fvit.is_valid(); fvit++)
		{
			p = mesh.point(*fvit);
			glVertex3fv(p.data());
		}
		glEnd();
	}
}


void display()
{
	float lpos[4] = { 5.0, 0.0, 10.0, 1.0 };

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0, 1, 1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 11.0, 0, 0, 0, 0.0, 1.0, 0.0);
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	glPushMatrix();
	    if(wireframe){
		  glColor3f(1, 1, 1);
		  glEnable(GL_POLYGON_OFFSET_FILL);
		  glPolygonOffset(1., 1.);
		}
		glRotatef(angleY, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glScalef(modelScale, modelScale, modelScale);
		glTranslatef(-xc, -yc, -zc);
		drawObject();
	glPopMatrix();

	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPushMatrix();
		glColor3f(0, 0, 1);
		glRotatef(angleY, 0, 1, 0);
		glRotatef(angleX, 1, 0, 0);
		glScalef(modelScale, modelScale, modelScale);
		glTranslatef(-xc, -yc, -zc);
		drawObject();
		glPopMatrix();
	}

	glFlush();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB |GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("OpenMesh Based Viewer");
	initialize();

	glutDisplayFunc(display);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}

