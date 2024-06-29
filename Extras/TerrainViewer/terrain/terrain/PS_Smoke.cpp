//  ============================================================================
//  COSC422: Advanced Computer Graphics (2022);  University of Canterbury.
//
//  FILE NAME: PS_Smoke
//  Program to demonstrate the construction and rendering of particle systems.
//  =============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include "Shader.h"
#include "loadTGA.h"
using namespace std;

const int NPART = 10;    //number of particles
GLuint vaoID, texID;
GLuint theProgram;
GLuint timeLoc;
float simTime = 0;   //simulation time
float CDR=3.14159265/180.0;

void loadTextures()
{
    glGenTextures(1, &texID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    loadTGA("Smoke1.tga");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void initialise()
{
	loadTextures();
	GLuint program = createShaderProg("PS_Smoke.vert", "PS_Smoke.frag");
	GLuint matrixLoc = glGetUniformLocation(program, "mvpMatrix");
	timeLoc = glGetUniformLocation(program, "simt");
	glUniform1f(timeLoc, 0);

	GLuint texLoc = glGetUniformLocation(program, "smokeTex");
	glUniform1i(texLoc, 0);

	glm::mat4 proj = glm::ortho(-5.0f, 5.0f, -1.0f, 9.0f);  //2D orthographic projection matrix
	glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, &proj[0][0]);

	float vert[NPART], vel[NPART], startTime[NPART], angle[NPART], mag[NPART], omeg[NPART];
	float  texindx[NPART];

	//Particle path parameters to be stored in 6 VBOs
	float t = 0;
	for(int i = 0; i < NPART; i ++)
	{
		vert[i] = glm::linearRand(-1.0f, 1.0f);
		vel[i] = glm::linearRand(1.0f, 3.f);
		angle[i] = glm::linearRand(-10.0f, 10.0f);
		mag[i] = glm::linearRand(0.1f, 1.0f);
		omeg[i] = glm::linearRand(0.2f, 1.0f);
		startTime[i] = t;
		t += 0.1;
	}
	
	GLuint vboID[6];

	glGenVertexArrays(1, &vaoID);

    glBindVertexArray(vaoID);
    glGenBuffers(6, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);      // Vertex position

    glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vel), vel, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);      // Velocity

    glBindBuffer(GL_ARRAY_BUFFER, vboID[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(startTime), startTime, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);      // Starttime

	glBindBuffer(GL_ARRAY_BUFFER, vboID[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(angle),angle, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(3);      // angle

	glBindBuffer(GL_ARRAY_BUFFER, vboID[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mag), mag, GL_STATIC_DRAW);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(4);      // mag

	glBindBuffer(GL_ARRAY_BUFFER, vboID[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(omeg), omeg, GL_STATIC_DRAW);
	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(5);      // omeg

    glBindVertexArray(0);
	glDisable(GL_DEPTH_TEST);   //++++++++++++++++++++++
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1, 1, 1, 1);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);  //Important!
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glUniform1f(timeLoc, simTime);

	glBindVertexArray(vaoID);
	glDrawArrays(GL_POINTS, 0, NPART);

	glutSwapBuffers();
}

void update(int value)
{
	simTime += 0.05;
	glutTimerFunc(50, update, 0);
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Smoke sprites");
	glutInitContextVersion (4, 2);
	glutInitContextProfile ( GLUT_CORE_PROFILE );

	if(glewInit() == GLEW_OK)
	{
		cout << "GLEW initialization successful! " << endl;
		cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
	}
	else
	{
		cerr << "Unable to initialize GLEW  ...exiting." << endl;
		exit(EXIT_FAILURE);
	}

	initialise();
	glutDisplayFunc(display);
	glutTimerFunc(50, update, 0);
	glutMainLoop();
}

