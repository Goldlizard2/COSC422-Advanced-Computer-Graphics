//  ========================================================================
//  COSC422: Computer Graphics (2023);  University of Canterbury.
//  FILE NAME: TerrainP.cpp
//  See Exer14.pdf for details.
//
//	The program generates and loads the mesh data for a terrain floor (100 verts, 81 elems).
//  Requires files  Terrain.vert, Terrain.frag
//                  Terrain.cont, Terrains.eval
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h" 
#include "stb_image.h"

using namespace std;

GLuint vaoID;
GLuint skyBoxVao;
GLuint mvpMatrixLoc, eyeLoc;
GLuint lightPosLoc;
GLuint cameraPositionLoc;
GLuint tickLoc;
GLuint projLoc, viewLoc, SkyboxLoc;
int tick;
glm::vec3 light;

GLuint program;
GLuint program1;

GLuint wireframeLoc;
int wireframeBool = 0;

GLuint waterHeightLoc;
float waterLevel = 1.8;
GLuint snowHeightLoc;
float snowLevel = 7;

GLuint texIDs[6];
float eye_x = 0, eye_y = 20, eye_z = 30;      //Initial camera position
float look_x = 0, look_y = 0, look_z = -40;    //"Look-at" point along -z direction
float theta = 0;                              //Look angle
float toRad = 3.14159265/180.0;     //Conversion from degrees to rad
glm::vec4 cameraPosn;
//skybox
unsigned int skyboxID;

static	vector<std::string> faces
{
	"./skyBox/right.tga",
	"./skyBox/left.tga",
	"./skyBox/top.tga",
	"./skyBox/bottom.tga",
	"./skyBox/front.tga",
	"./skyBox/back.tga"
};
float skyBoxVerts[] = {
	// skybox verticies
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};


float verts[100*3];       //10x10 grid (100 vertices)
GLushort elems[81*4];     //Element array for 9x9 = 81 quad patches

glm::mat4 projView;

float dir_x, dir_z;

//Generate vertex and element data for the terrain floor
void generateData()
{
	int indx, start;
	//verts array
	for(int i = 0; i < 10; i++)   //100 vertices on a 10x10 grid
	{
		for(int j = 0; j < 10; j++)
		{
			indx = 10*i + j;
			verts[3*indx] = 10*i - 45;		//x
			verts[3*indx+1] = 0;			//y
			verts[3*indx+2] = -10*j;		//z
		}
	}

	//elems array
	for(int i = 0; i < 9; i++)
	{
		for(int j = 0; j < 9; j++)
		{
			indx = 9*i +j;
			start = 10*i + j;
			elems[4*indx] = start;
			elems[4*indx+1] = start + 10;
			elems[4*indx+2] = start + 11;
			elems[4*indx+3] = start + 1;			
		}
	}
	

}

void loadTexture()
{
	
	glGenTextures(5, texIDs);

	for (int i = 0; i < 5; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texIDs[i]);

		// Load texture data (use different textures for each level)
		if (i == 0) loadTGA("Water.tga");
		else if (i == 1) loadTGA("Rock1.tga");
		else if (i == 2) loadTGA("Grass.tga");
		else if (i == 3) loadTGA("Snow.tga");
		else if (i == 4) loadTGA("mtk.tga");

		// Set texture parameters

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}


}

//Loads a shader file and returns the reference to a shader object
GLuint loadShader(GLenum shaderType, string filename)
{
	ifstream shaderFile(filename.c_str());
	if(!shaderFile.good()) cout << "Error opening shader file." << endl;
	stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();
	string shaderStr = shaderData.str();
	const char* shaderTxt = shaderStr.c_str();

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderTxt, NULL);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		const char *strShaderType = NULL;
		cerr <<  "Compile failure in shader: " << strInfoLog << endl;
		delete[] strInfoLog;
	}
	return shader;
}

void loadCubemap(vector<std::string> faces)
{
	
	glGenTextures(1, &skyboxID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		cout << faces[i].c_str() << endl;
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void scaleSkybox() {
	for (int i = 0; i < 108; i++) {
		skyBoxVerts[i] = 1 * skyBoxVerts[i];
	}
}

//Initialise the shader program, create and load buffer data
void initialise()
{
	loadTexture();
	loadCubemap(faces);
	scaleSkybox();
	

	//--------Load shaders----------------------
	program = glCreateProgram();
	GLuint shaderv = loadShader(GL_VERTEX_SHADER, "Terrain.vert");
	GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "Terrain.frag");
	GLuint shaderc = loadShader(GL_TESS_CONTROL_SHADER, "Terrain.cont");
	GLuint shadere = loadShader(GL_TESS_EVALUATION_SHADER, "Terrain.eval");
	GLuint shaderg = loadShader(GL_GEOMETRY_SHADER, "Terrain.geom");	
	glAttachShader(program, shaderv);
	glAttachShader(program, shaderf);
	glAttachShader(program, shaderc);
	glAttachShader(program, shadere);
	glAttachShader(program, shaderg);

	glLinkProgram(program);

	program1 = glCreateProgram();
	GLuint shaderv1 = loadShader(GL_VERTEX_SHADER, "skybox.vert");
	GLuint shaderf1 = loadShader(GL_FRAGMENT_SHADER, "skybox.frag");
	glAttachShader(program1, shaderv1);
	glAttachShader(program1, shaderf1);

	glLinkProgram(program1);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);


	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	generateData();
	GLuint vboID[2];
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	glGenBuffers(2, vboID);

	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);  // Vertex position

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glPatchParameteri(GL_PATCH_VERTICES, 4);

	GLuint skyBoxVBO;
	glGenVertexArrays(1, &skyBoxVao);
	glGenBuffers(1, &skyBoxVBO);
	glBindVertexArray(skyBoxVao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyBoxVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyBoxVerts), skyBoxVerts, GL_STATIC_DRAW);

	// Define vertex attributes for the shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	light = glm::vec3(500.0, 1000.0, 500.0);
	lightPosLoc = glGetUniformLocation(program, "lightPos");
	wireframeLoc = glGetUniformLocation(program, "toggleWireframe");
	mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
	waterHeightLoc = glGetUniformLocation(program, "waterLevel");
	snowHeightLoc = glGetUniformLocation(program, "snowLevel");
	cameraPositionLoc = glGetUniformLocation(program, "cameraPosition");
	tickLoc = glGetUniformLocation(program, "Tick");

	
}

//void renderSkybox(){
//	glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
//	glDepthFunc(GL_LEQUAL);
//	projLoc = glGetUniformLocation(program1, "mvpMatrix");
//	SkyboxLoc = glGetUniformLocation(program1, "skybox");
//	glUseProgram(program1);
//	glUniform1i(SkyboxLoc, 0);
//	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projView[0][0]);
//
//	// Render the skybox
//	glBindVertexArray(skyBoxVao);
//	glActiveTexture(GL_TEXTURE5);
//	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxID);
//	
//	glDrawArrays(GL_TRIANGLES, 0, sizeof(skyBoxVerts) / 3);
//	glDepthFunc(GL_LESS);
//}

void renderTerrain() {
	glUseProgram(program);
	GLuint waterTexLoc = glGetUniformLocation(program, "waterTex");
	GLuint grassTexLox = glGetUniformLocation(program, "grassTex");
	GLuint rockTexLoc = glGetUniformLocation(program, "rockTex");
	GLuint snowTexLoc = glGetUniformLocation(program, "snowTex");
	GLuint texLoc = glGetUniformLocation(program, "heightMap");
	glUniform1i(waterTexLoc, 0);
	glUniform1i(grassTexLox, 1);
	glUniform1i(rockTexLoc, 2);
	glUniform1i(snowTexLoc, 3);
	glUniform1i(texLoc, 4);
	

	glUniform3fv(lightPosLoc, 1, &light[0]);
	glUniform4fv(eyeLoc, 1, &cameraPosn[0]);
	glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
	glUniform1i(wireframeLoc, wireframeBool);
	glUniform1i(tickLoc, tick);
	//set water level
	glUniform1f(waterHeightLoc, waterLevel);
	//set snow level
	glUniform1f(snowHeightLoc, snowLevel);
	glBindVertexArray(vaoID);
	glDrawElements(GL_PATCHES, 81 * 4, GL_UNSIGNED_SHORT, NULL);

}

//Display function to compute uniform values based on transformation parameters and to draw the scene
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, wireframeBool ? GL_LINE : GL_FILL);
	//--------Compute matrices----------------------
	glm::mat4 proj = glm::perspective(30.0f * toRad, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
	glm::mat4 view = lookAt(glm::vec3(eye_x, eye_y, eye_z), glm::vec3(look_x, look_y, look_z), glm::vec3(0.0, 1.0, 0.0)); //view matri
	projView = proj * view;  //Product matrix
	cameraPosn = glm::vec4(eye_x, eye_y, eye_z, 1.0);
	
	renderTerrain();
	//renderSkybox();
	glutSwapBuffers();
	
	
}

void special(int key, int x, int y)
{
	int step = 0;
	if (key == GLUT_KEY_LEFT) theta += 0.1;   //in radians
	else if (key == GLUT_KEY_RIGHT) theta -= 0.1;
	else if (key == GLUT_KEY_DOWN) step = -5;
	else if (key == GLUT_KEY_UP) step = 5;
	
	dir_x = -sin(theta);
	dir_z = -cos(theta);
	eye_x += step * 0.1 * dir_x;
	eye_z += step * 0.1 * dir_z;
	look_x = eye_x + 70 * dir_x;
	look_z = eye_z + 70 * dir_z;
	
	cout << dir_x << dir_z << eye_x << eye_z << look_x << look_z << endl;

	glutPostRedisplay();
}




void keyboardEvent(unsigned char key, int x, int y)
{
	if (key == 'w')
	{
		wireframeBool = ~wireframeBool;
	}
}

void update(int tick) {
	tick++;
	glUniform1i(tickLoc, tick);
	glutTimerFunc(50, update, tick);
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Okoko Anainga - COSC422 Assignment 2 Terrain Viewer");
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
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboardEvent);
	glutTimerFunc(100, update, tick);
	glutMainLoop();
	return 0;
}

