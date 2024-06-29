//  ========================================================================
//  COSC422: Advanced Computer Graphics;  University of Canterbury (2023)
//
//  FILE NAME: Skeleton Animation.cpp
//  See Exer04_SkeletalAnimation.pdf for details 
//  ========================================================================

#include <iostream>
#include <fstream>
#include <GL/freeglut.h>
using namespace std;

#include <assimp/cimport.h>
#include <assimp/types.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assimp_extras.h"

//----------Globals----------------------------
const aiScene* scene = NULL;
aiVector3D scene_min, scene_max, scene_center;
float scene_scale;

// ------A recursive function to traverse scene graph and render each mesh----------
// Simplified version for rendering a skeleton mesh
void render(const aiNode* node)
{
	aiMatrix4x4 m = node->mTransformation;
	aiMesh* mesh;
	aiFace* face;
	float materialCol[4] = { 1, 0, 1, 1 };
	int meshIndex;

	m.Transpose();   //Convert to column-major order
	glPushMatrix();
	glMultMatrixf((float*)&m);   //Multiply by the transformation matrix for this node

	//The scene graph for a skeleton contains at most one mesh per node
	//Skeleton meshes are always triangle meshes
	if(node->mNumMeshes > 0)
	{
		meshIndex = node->mMeshes[0];          //Get the mesh indices from the current node
		mesh = scene->mMeshes[meshIndex];    //Using mesh index, get the mesh object
		glColor4fv(materialCol);   //Default material colour

		//Draw the mesh in the current node
		for (int k = 0; k < mesh->mNumFaces; k++)
		{
			face = &mesh->mFaces[k];
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < face->mNumIndices; i++) {
				int vertexIndex = face->mIndices[i];
				if (mesh->HasNormals())
					glNormal3fv(&mesh->mNormals[vertexIndex].x);
				glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}
			glEnd();
		}
	}

	// Recursively draw all children of the current node
	for (int i = 0; i < node->mNumChildren; i++)
		render(node->mChildren[i]);

	glPopMatrix();
}

//----- Function to update node matrices for each tick ------
// Complete this function
void updateNodeMatrices(int tick)
{
	aiAnimation* anim;   //Animation object
	aiMatrix4x4 matPos, matRot, matProd;  //Position, rotation, product matrices
	aiMatrix3x3 matRot3;
	aiNode* node;

	anim = scene->mAnimations[0];
	for (int i = 0; i < anim->mNumChannels; i++)
	{
		matPos = aiMatrix4x4();   //Identity
		matRot = aiMatrix4x4();
		aiNodeAnim* chnl = anim->mChannels[i];      //Channel

		//See Lec04:Slides 31, 32  and complete this section


		node = scene->mRootNode->FindNode(chnl->mNodeName);
		node->mTransformation = matProd;
	}
}

//--------------------OpenGL initialization------------------------
void initialise()
{
	float ambient[4] = { 0.2, 0.2, 0.2, 1.0 };  //Ambient light
	float white[4] = { 1, 1, 1, 1 };		    //Light's colour
	float black[4] = { 0, 0, 0, 1 };
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, black);   //Disable specular light
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, 1, 1.0, 500.0);

	//---- Load the model ------
	scene = aiImportFile("Test.bvh", aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Debone);
	if (scene == NULL) exit(1);
	//printTreeInfo(scene->mRootNode);
	//printAnimInfo(scene, 0);

	get_bounding_box(scene, &scene_min, &scene_max);
	scene_center = (scene_min + scene_max) * 0.5f;
	aiVector3D scene_diag = scene_max - scene_center;
	scene_scale = 1.0 / scene_diag.Length();
}


//------The main display function---------
void display()
{
	float lightPosn[4] = { -5, 10, 10, 1 };
	aiMatrix4x4 m = scene->mRootNode->mTransformation;
	float xpos = m.a4;   //Root joint's position in world space
	float ypos = m.b4;
	float zpos = m.c4;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 7, 0, 0, 0, 0, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosn);

	glPushMatrix();
	   glScalef(scene_scale, scene_scale, scene_scale);
	   glTranslatef(-xpos, 0, -zpos);   //Move model to origin
	   render(scene->mRootNode);
	glPopMatrix();

	glutSwapBuffers();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Skeleton Animation");

	initialise();
	glutDisplayFunc(display);
	glutMainLoop();
}

