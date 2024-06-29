//  ========================================================================
//  COSC422: Advanced Computer Graphics;  University of Canterbury (2023)
//
//  FILE NAME: Skeleton Animation.cpp
//  See Exer04_SkeletalAnimation.pdf for details 
//  ========================================================================

#include <iostream>
#include <fstream>
#include <GL/freeglut_std.h>
#include <GL/freeglut.h>
#include <IL/il.h>
using namespace std;

//#include "background.h"
//#include "assimp_ModelLoader.h"
#include <assimp/cimport.h>
#include <assimp/types.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assimp_extras.h"

//----------Globals----------------------------
const aiScene* scene = NULL;
const aiScene* sceneModel = NULL;
const aiScene* character = NULL;

aiVector3D scene_min, scene_max, scene_center;
float scene_scale;
bool modelRotn = false;

GLuint* textureIds = NULL;
GLuint* textureIdC = NULL;

float toRad = 3.14159265 / 180.0;   //Conversion from degrees to radians

//timer
int currentTick = 0;
bool pauseAni = false;
int tDuration;   //Animation duration in ticks.
int tDurationC;
int tick = 0;
int tickC = 0;
float fpsC;
float fps;
int timeStep;
int timeStepC;

//Camera
float camAngle = 0, camDist = 10, camNear = 0, camFov = 60.0;		//Camera's parameters
float camX = 0;
float camY = 2;
float camZ = 30;
float move_speed = 0.5;
float rot_speed = 5;

const int numVertices = 100;  // Number of vertices on the skydome
const float radius = 100.0f;   // Radius of the skydome
const float pi = 3.14159265359f;

//------------Modify the following as needed----------------------
float materialdefault[4] = { 0.0, 0.0, 0.0, 1.0 };   //Default material colour (not used if model's colour is available)
float materialCol[4] = { 0.0, 1.0, 0.02 , 1.0 };




//----A basic mesh structure that stores initial values of-----
//----vertex coordinates and normal components-----------------
struct meshInit
{
	int mNumVertices;
	aiVector3D* mVertices;
	aiVector3D* mNormals;
};
meshInit* initData;

const aiScene* loadModel(const char* fileName);
void loadGLTextures(const aiScene* sc);
void renderModel(const aiScene* sc, const aiNode* node);
void renderSkeleton(const aiNode* node);

const aiScene* loadModel(const char* fileName)
{
	const aiScene* temp = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality);
	if (temp == NULL) exit(1);
	return temp;
}

//----Loads the character model and stores mesh data------- 
const aiScene* loadCharacterModel(const char* fileName)
{
	int numVert;
	aiMesh* mesh;

	const aiScene* chr = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality);
	if (chr == NULL) exit(1);
	//Data structure for storing vertices and normal vectors of the initial mesh.
	initData = new meshInit[chr->mNumMeshes];

	//Allocate space using number of vertices in each mesh object
	//and copy data
	for (int i = 0; i < chr->mNumMeshes; i++)
	{
		mesh = chr->mMeshes[i];
		numVert = mesh->mNumVertices;
		(initData + i)->mNumVertices = numVert;
		(initData + i)->mVertices = new aiVector3D[numVert];
		(initData + i)->mNormals = new aiVector3D[numVert];
		for (int k = 0; k < numVert; k++) {
			(initData + i)->mVertices[k] = mesh->mVertices[k];
			(initData + i)->mNormals[k] = mesh->mNormals[k];
		}
	}

	//Compute camera parameters using model's dimensions
	return chr;
}

void loadGLTextures(const aiScene* sc)
{
	ILuint imageId;

	if (sc == nullptr) exit(1);
	if (sc->HasTextures())
	{
		std::cout << "Support for meshes with embedded textures is not implemented" << endl;
		exit(1);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glEnable(GL_TEXTURE_2D);
	ilInit();   //DevIL initialization

	textureIds = new GLuint[sc->mNumMaterials];
	glGenTextures(sc->mNumMaterials, textureIds);

	for (unsigned int m = 0; m < sc->mNumMaterials; ++m)
	{
		aiString path;  // file path, name

		if (sc->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			ilGenImages(1, &imageId);
			ilBindImage(imageId); /* Binding of DevIL image name */
			ilEnable(IL_ORIGIN_SET);
			ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
			if (ilLoadImage((ILstring)path.data))   //if success
			{
				/* Convert image to RGBA */
				ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
				glBindTexture(GL_TEXTURE_2D, textureIds[m]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
					ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				cout << "  Texture:" << path.data << " successfully loaded." << endl;
			}
			else
			{
				cout << "Couldn't load Image: " << path.data << endl;
			}
		}

	}  //loop for material
}

void loadGLTexturesChar(const aiScene* sc)
{
	ILuint imageId;

	if (sc == nullptr) exit(1);
	if (sc->HasTextures())
	{
		std::cout << "Support for meshes with embedded textures is not implemented" << endl;
		exit(1);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glEnable(GL_TEXTURE_2D);
	ilInit();   //DevIL initialization

	textureIdC = new GLuint[sc->mNumMaterials];
	glGenTextures(sc->mNumMaterials, textureIdC);

	for (unsigned int m = 0; m < sc->mNumMaterials; ++m)
	{
		aiString path;  // file path, name

		if (sc->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			ilGenImages(1, &imageId);
			ilBindImage(imageId); /* Binding of DevIL image name */
			ilEnable(IL_ORIGIN_SET);
			ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
			if (ilLoadImage((ILstring)"T_M_MED_Banana_Smooth_Body_D.png"))   //if success
			{
				/* Convert image to RGBA */
				ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
				glBindTexture(GL_TEXTURE_2D, textureIdC[m]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
					ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				cout << "  Texture:" << "T_M_MED_Banana_Smooth_Body_D.png" << " successfully loaded." << endl;
			}
			else
			{
				cout << "Couldn't load Image: " << "T_M_MED_Banana_Smooth_Body_D.png" << endl;
			}
		}

	}  //loop for material
	
}

void renderCharacter(const aiScene* sc, const aiNode* node)
{
	aiMatrix4x4 m = node->mTransformation;
	aiMesh* mesh;
	aiFace* face;
	aiColor4D diffuse;
	aiMaterial* mtl;
	int meshIndex, materialIndex;

	m.Transpose();   //Convert to column-major order
	glPushMatrix();
	glMultMatrixf((float*)&m);   //Multiply by the transformation matrix for this node

	// Draw all meshes
	for (int n = 0; n < node->mNumMeshes; n++)
	{
		meshIndex = node->mMeshes[n];
		mesh = sc->mMeshes[meshIndex];

		materialIndex = mesh->mMaterialIndex;  //Get material index attached to the mesh
		mtl = sc->mMaterials[materialIndex];
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))  //Get material colour from model
			glColor4f(diffuse.r, diffuse.g, diffuse.b, 1.0);
		else
			glColor4fv(materialCol);   //Default material colour

		if (mesh->HasTextureCoords(0))
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textureIdC[materialIndex]);
		}
		else
			glDisable(GL_TEXTURE_2D);


		//Get the polygons from each mesh and draw them
		for (int k = 0; k < mesh->mNumFaces; k++)
		{
			face = &mesh->mFaces[k];
			GLenum face_mode;

			switch (face->mNumIndices)
			{
			case 1: face_mode = GL_POINTS; break;
			case 2: face_mode = GL_LINES; break;
			case 3: face_mode = GL_TRIANGLES; break;
			default: face_mode = GL_POLYGON; break;
			}

			glBegin(face_mode);

			for (int i = 0; i < face->mNumIndices; i++) {
				int vertexIndex = face->mIndices[i];

				if (mesh->HasVertexColors(0))
					glColor4fv((GLfloat*)&mesh->mColors[0][vertexIndex]);

				if (mesh->HasTextureCoords(0))
					glTexCoord2f(mesh->mTextureCoords[0][vertexIndex].x, mesh->mTextureCoords[0][vertexIndex].y);

				if (mesh->HasNormals())
					glNormal3fv(&mesh->mNormals[vertexIndex].x);

				glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}
			
			glEnd();
		}

	}

	// Draw all children of the current node
	for (int i = 0; i < node->mNumChildren; i++)
		renderCharacter(sc, node->mChildren[i]);
	
	glPopMatrix();
	//glColor4fv(materialdefault);
	
}

void renderSkeleton(const aiNode* node)
{
	aiMatrix4x4 m = node->mTransformation;
	aiMesh* mesh;
	aiFace* face;
	int meshIndex;

	m.Transpose();   //Convert to column-major order
	glPushMatrix();
	glMultMatrixf((float*)&m);   //Multiply by the transformation matrix for this node

	//The scene graph for a skeleton contains at most one mesh per node
	if ((strcmp((node->mName).data, "Chest") == 0))
	{
		glPushMatrix();
		glTranslatef(0, 7, 0);
		glScalef(14, 20, 4);
		glutSolidCube(1);
		glPopMatrix();
	}
	else if ((strcmp((node->mName).data, "Hips") == 0))
	{
		glPushMatrix();
		glScalef(14, 4, 4);
		glutSolidCube(1);
		glPopMatrix();
	}
	else if ((strcmp((node->mName).data, "RightCollar") == 0) || (strcmp((node->mName).data, "LeftCollar") == 0))
	{
		int side = (strcmp((node->mName).data, "RightCollar") == 0) ? -1 : 1;
		glPushMatrix();
		glTranslatef(side * 7, 0, 0);
		glutSolidSphere(3, 20, 20);
		glPopMatrix();
	}
	else if ((strcmp((node->mName).data, "RightUpLeg") == 0) || (strcmp((node->mName).data, "LeftUpLeg") == 0))
	{
		glPushMatrix();
		glTranslatef(0, -9, 0);
		glScalef(3, 18, 3);
		glutSolidCube(1);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0, -18, 0);
		glutSolidSphere(3, 20, 20);
		glPopMatrix();
	}
	else if ((strcmp((node->mName).data, "RightLowLeg") == 0) || (strcmp((node->mName).data, "LeftLowLeg") == 0))
	{
		glPushMatrix();
		glTranslatef(0, -9, 0);
		glScalef(3, 18, 3);
		glutSolidCube(1);
		glPopMatrix();
	}
	else if ((strcmp((node->mName).data, "RightFoot") == 0) || (strcmp((node->mName).data, "LeftFoot") == 0))
	{
		/*
			Calculates the world coordinates of the Right Foot
		*/
		glPushMatrix();
		glTranslatef(0, -1.5, 2);
		glScalef(3, 3, 7);
		glutSolidCube(1);
		glPopMatrix();
	}
	else if ((strcmp((node->mName).data, "RightUpArm") == 0) || (strcmp((node->mName).data, "LeftUpArm") == 0))
	{
		glPushMatrix();
		glTranslatef(0, -6.5, 0);
		glScalef(2.5, 13, 2.5);
		glutSolidCube(1);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0, -13, 0);
		glutSolidSphere(3, 20, 20);
		glPopMatrix();
	}
	else if ((strcmp((node->mName).data, "RightHand") == 0) || (strcmp((node->mName).data, "LeftHand") == 0))
	{
		glPushMatrix();
		glTranslatef(0, 0, 0);
		glScalef(2.5, 14, 2.5);
		glutSolidCube(1);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0, -8, 0);
		glutSolidSphere(3, 20, 20);
		glPopMatrix();
	}
	else if ((strcmp((node->mName).data, "Neck") == 0))
	{
		glPushMatrix();
		glTranslatef(0, 3, 0);
		glRotatef(90, 1, 0, 0);
		glutSolidCylinder(1.5, 5, 50, 10);
		glPopMatrix();
	}
	else if ((strcmp((node->mName).data, "Head") == 0))
	{
		glPushMatrix();
		glTranslatef(0, 2, 0);
		glutSolidSphere(5, 20, 20);
		glPopMatrix();
	}
	// Recursively draw all children of the current node
	for (int i = 0; i < node->mNumChildren; i++)
		renderSkeleton(node->mChildren[i]);

	glPopMatrix();
}



void renderModel(const aiScene* sc, const aiNode* node)
{
	aiMatrix4x4 m = node->mTransformation;
	aiMesh* mesh;
	aiFace* face;
	aiMaterial* mtl;
	aiColor4D diffuse;
	int meshIndex, materialIndex;
	int texNum = 0;

	aiTransposeMatrix4(&m);   //Convert to column-major order
	glPushMatrix();
	glMultMatrixf((float*)&m);   //Multiply by the transformation matrix for this node

	// Draw all meshes assigned to this node
	for (int n = 0; n < node->mNumMeshes; n++)
	{
		meshIndex = node->mMeshes[n];          //Get the mesh indices from the current node
		mesh = sceneModel->mMeshes[meshIndex];    //Using mesh index, get the mesh object

		materialIndex = mesh->mMaterialIndex;  //Get material index attached to the mesh
		mtl = sc->mMaterials[materialIndex];

		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))  //Get material colour from model
		{
			glColor4f(diffuse.r, diffuse.g, diffuse.b, 1.0);
		}
		else
			glColor4fv(materialCol);   //Default material colour


		if (mesh->HasTextureCoords(0))
		{
			aiString path;
			const char* cpath;
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textureIds[materialIndex]);
		}
		else
			glDisable(GL_TEXTURE_2D);

		//Get the polygons from each mesh and draw them
		for (int k = 0; k < mesh->mNumFaces; k++)
		{
			face = &mesh->mFaces[k];
			GLenum face_mode;

			switch (face->mNumIndices)
			{
			case 1: face_mode = GL_POINTS; break;
			case 2: face_mode = GL_LINES; break;
			case 3: face_mode = GL_TRIANGLES; break;
			default: face_mode = GL_POLYGON; break;
			}

			glBegin(face_mode);
			for (int i = 0; i < face->mNumIndices; i++) {
				int vertexIndex = face->mIndices[i];
				if (mesh->HasTextureCoords(0))
					glTexCoord2f(mesh->mTextureCoords[0][vertexIndex].x, mesh->mTextureCoords[0][vertexIndex].y);
				if (mesh->HasNormals())
					glNormal3fv(&mesh->mNormals[vertexIndex].x);
				glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}
			glEnd();
		}
	}

	// Draw all children
	for (int i = 0; i < node->mNumChildren; i++)
		renderModel(sc, node->mChildren[i]);

	glPopMatrix();
}
//----- Function to update node matrices for each tick ------
// Complete this function
void updateNodeMatrices(int tick)
{
	aiAnimation* anim = scene->mAnimations[0];
	aiMatrix4x4 matPos, matRot, matProd;
	aiMatrix3x3 matRot3;
	aiNode* node;
	int nPoskeys, nRotkeys, index;
	aiVector3D posn1, posn2, posn;
	aiQuaternion rotn1, rotn2, rotn;
	float factor, time1, time2;

	for (int i = 0; i < anim->mNumChannels; i++)
	{
		matPos = aiMatrix4x4();   //Identity
		matRot = aiMatrix4x4();
		aiNodeAnim* channel = anim->mChannels[i];
		nPoskeys = channel->mNumPositionKeys;
		nRotkeys = channel->mNumRotationKeys;

		if (tick < (channel->mPositionKeys[nPoskeys - 1]).mTime)
		{
			index = 1;
			while (tick >= (channel->mPositionKeys[index]).mTime) index++;
			posn1 = (channel->mPositionKeys[index - 1]).mValue;
			posn2 = (channel->mPositionKeys[index]).mValue;
			time1 = (channel->mPositionKeys[index - 1]).mTime;
			time2 = (channel->mPositionKeys[index]).mTime;
			factor = (tick - time1) / (time2 - time1);
			posn = (1 - factor) * posn1 + factor * posn2;
		}
		else
			posn = (channel->mPositionKeys[nPoskeys - 1]).mValue;

		if (tick < (channel->mRotationKeys[nRotkeys - 1]).mTime)
		{
			index = 1;
			while (tick >= (channel->mRotationKeys[index]).mTime) index++;
			rotn1 = (channel->mRotationKeys[index - 1]).mValue;
			rotn2 = (channel->mRotationKeys[index]).mValue;
			time1 = (channel->mRotationKeys[index - 1]).mTime;
			time2 = (channel->mRotationKeys[index]).mTime;
			factor = (tick - time1) / (time2 - time1);
			rotn.Interpolate(rotn, rotn1, rotn2, factor);
		}
		else
			rotn = (channel->mRotationKeys[nRotkeys - 1]).mValue;

		matPos.Translation(posn, matPos);
		matRot3 = rotn.GetMatrix();
		matRot = aiMatrix4x4(matRot3);

		matProd = matPos * matRot;
		node = scene->mRootNode->FindNode(channel->mNodeName);
		node->mTransformation = matProd;
	}
}

void updateCharacterNodeMatricies(int tickC)
{
	aiAnimation* anim = character->mAnimations[0];
	aiMatrix4x4 matPos, matRot, matProd;
	aiMatrix3x3 matRot3;
	aiNode* node;
	int nPoskeys, nRotkeys, index;
	aiVector3D posn1, posn2, posn;
	aiQuaternion rotn1, rotn2, rotn;
	float factor, time1, time2;

	for (int i = 0; i < anim->mNumChannels; i++)
	{
		matPos = aiMatrix4x4();   //Identity
		matRot = aiMatrix4x4();
		aiNodeAnim* channel = anim->mChannels[i];
		nPoskeys = channel->mNumPositionKeys;
		nRotkeys = channel->mNumRotationKeys;

		if (tickC < (channel->mPositionKeys[nPoskeys - 1]).mTime)
		{
			index = 1;
			while (tickC >= (channel->mPositionKeys[index]).mTime) index++;
			posn1 = (channel->mPositionKeys[index - 1]).mValue;
			posn2 = (channel->mPositionKeys[index]).mValue;
			time1 = (channel->mPositionKeys[index - 1]).mTime;
			time2 = (channel->mPositionKeys[index]).mTime;
			factor = (tickC - time1) / (time2 - time1);
			posn = (1 - factor) * posn1 + factor * posn2;
		}
		else
			posn = (channel->mPositionKeys[nPoskeys - 1]).mValue;

		if (tickC < (channel->mRotationKeys[nRotkeys - 1]).mTime)
		{
			index = 1;
			while (tickC >= (channel->mRotationKeys[index]).mTime) index++;
			rotn1 = (channel->mRotationKeys[index - 1]).mValue;
			rotn2 = (channel->mRotationKeys[index]).mValue;
			time1 = (channel->mRotationKeys[index - 1]).mTime;
			time2 = (channel->mRotationKeys[index]).mTime;
			factor = (tickC - time1) / (time2 - time1);
			rotn.Interpolate(rotn, rotn1, rotn2, factor);
		}
		else
			rotn = (channel->mRotationKeys[nRotkeys - 1]).mValue;

		matPos.Translation(posn, matPos);
		matRot3 = rotn.GetMatrix();
		matRot = aiMatrix4x4(matRot3);

		matProd = matPos * matRot;
		node = character->mRootNode->FindNode(channel->mNodeName);
		node->mTransformation = matProd;
	}
}



//From 3D Mesh Processing And Charcter Animation By: Ramkrishnan Mukundan
void transformVertices()
{
	aiMesh* mesh;
	aiVector3D vert, norm;
	aiMatrix4x4 offset, nodeTransf, matProd;
	aiMatrix3x3 norMat;
	aiNode* node;
	aiBone* bone;
	int vertId;
	for (int imesh = 0; imesh < character->mNumMeshes; imesh++)
	{
		mesh = character->mMeshes[imesh];
		for (int i = 0; i < mesh->mNumBones; i++)
		{
			bone = mesh->mBones[i];
			offset = bone->mOffsetMatrix;
			node = character->mRootNode->FindNode(bone->mName);
			nodeTransf = node->mTransformation;
			while (node->mParent != NULL)
			{
				node = node->mParent;
				nodeTransf = (node->mTransformation) * nodeTransf;
			}
			matProd = nodeTransf * offset;
			norMat = aiMatrix3x3(matProd);
			norMat.Transpose();
			norMat.Inverse();
			for (int k = 0; k < bone->mNumWeights; k++)
			{
				vertId = (bone->mWeights[k]).mVertexId;
				vert = (initData + imesh)->mVertices[vertId];
				norm = (initData + imesh)->mNormals[vertId];
				mesh->mVertices[vertId] = matProd * vert;
				mesh->mNormals[vertId] = norMat * norm;

			}
		}
	}
}

void update(int value) {
	if (tick <= tDuration) {
		updateNodeMatrices(tick);
		tick++;
		glutTimerFunc(timeStep, update, 0);
	}
	else {
		tick = 0;
		updateNodeMatrices(0);
		glutTimerFunc(timeStep, update, 0);
	}
	glutPostRedisplay();
}

void updateCharacter(int value)
{
	if (tickC < tDurationC)
	{
		updateCharacterNodeMatricies(tickC);
		transformVertices();
		glutTimerFunc(timeStepC, updateCharacter, 0);
		tickC++;
	}
	else {
		tickC = 0;
		updateCharacterNodeMatricies(0);
		glutTimerFunc(timeStepC, updateCharacter, 0);
	}
	glutPostRedisplay();
}


/*
	Adjusts the distance of the camera to the character.
*/
void zoomCamera(int direction) {
	float moveX = -camX;
	float moveZ = -camZ;
	if (abs(moveX) > abs(moveZ)) {
		moveZ /= abs(moveX);
		moveX /= abs(moveX);
	}
	else {
		moveX /= abs(moveZ);
		moveZ /= abs(moveZ);
	}


	float xDist = scene_center.x - camX;
	float yDist = scene_center.y - camY;
	float zDist = scene_center.z - camZ;
	float distance = sqrt(pow(xDist, 2) + pow(yDist, 2) + pow(zDist, 2));
	if (distance > 7 || direction < 0)
	{
		camX += moveX * direction * move_speed;
		camZ += moveZ * direction * move_speed;
	}
}

void generateCustomBackground(void) {
	int numRows = 200;
	int numCols = 200;
	float tileSize = 1.0f;

	glNormal3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_QUADS);

	for (int i = 0; i < numRows; ++i) {
		for (int j = 0; j < numCols; ++j) {
			float x0 = i * tileSize - numCols * tileSize * 0.5f;
			float z0 = j * tileSize - numRows * tileSize * 0.5f;
			float x1 = x0 + tileSize;
			float z1 = z0 + tileSize;

			if ((i + j) % 2 == 0) {
				glColor3f(0.20f, 0.79f, 0.0f); // leaf green
			}
			else {
				glColor3f(0.337f, 0.4902f, 0.2745f); // Grass Green
			}

			glVertex3f(x0, 0, z0);
			glVertex3f(x1, 0, z0);
			glVertex3f(x1, 0, z1);
			glVertex3f(x0, 0, z1);
		}
	}
	glEnd();
}

void drawSkydome() {
	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i <= numVertices; ++i) {
		for (int j = 0; j <= numVertices; ++j) {
			float theta = 2.0f * pi * static_cast<float>(i) / static_cast<float>(numVertices);
			float phi = pi * static_cast<float>(j) / static_cast<float>(numVertices);

			float x = radius * sin(phi) * cos(theta);
			float y = radius * cos(phi);
			float z = radius * sin(phi) * sin(theta);

			glNormal3f(x, y, z);  // Use vertex as normal for simplicity
			glVertex3f(x, y, z);

			theta = 2.0f * pi * static_cast<float>(i + 1) / static_cast<float>(numVertices);

			x = radius * sin(phi) * cos(theta);
			y = radius * cos(phi);
			z = radius * sin(phi) * sin(theta);

			glNormal3f(x, y, z);
			glVertex3f(x, y, z);
		}
	}
	glEnd();
}

//--------------------OpenGL initialization------------------------
void initialise()
{
	float ambient[4] = { 0.2, 0.2, 0.2, 1.0 };  //Ambient light
	float white[4] = { 1, 1, 1, 1 };		    //Light's colour
	float black[4] = { 0, 0, 0, 1 };
	float grey[4] = { 0.2, 0.2, 0.2, 1.0 };
    
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, grey);
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);   //Disable specular light
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	//---- Load the models ------
	
	
	sceneModel = loadModel("stage.gltf");
	character = loadCharacterModel("BananaDancing.fbx");

	loadGLTextures(sceneModel);
	loadGLTexturesChar(character);
	scene = loadModel("Dance.bvh");

	gluPerspective(40, 1, 1.0, 500.0);
	
	//Scene dimensions based on skeleton
	get_bounding_box(scene, &scene_min, &scene_max);
	scene_center = (scene_min + scene_max) * 0.5f;
	aiVector3D scene_diag = scene_max - scene_center;
	scene_scale = 1.0 / scene_diag.Length();

	//Skeletal timer
	tDuration = scene->mAnimations[0]->mDuration;
	float fps = scene->mAnimations[0]->mTicksPerSecond;
	if (fps < 10) fps = 30;
	timeStep = 1200.0 / fps;     //Animation time step in m.Sec
	//Character timer 
	tDurationC = character->mAnimations[0]->mDuration;
	float fpsC = character->mAnimations[0]->mTicksPerSecond;
	if (fpsC < 10) fpsC = 30;
	timeStepC = 1500.0 / fpsC;     //Animation time step in m.Sec
	
}


//------The main display function---------
void display()
{
	float lightPosn[4] = { 20, 50, 0, 1 };
	float shadowMat[16] = { lightPosn[1], 0, 0, 0, -lightPosn[0], 0, -lightPosn[2],
	-1, 0, 0, lightPosn[1], 0, 0, 0, 0, lightPosn[1] };

	float ex = camDist * sin(camAngle * toRad);
	float ez = camDist * cos(camAngle * toRad);

	// Clear the color buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camX, camDist*0.5, camZ, 0, 0, 0, 0, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosn);
	glEnable(GL_LIGHTING);

	//Stage Model
	glPushMatrix();
		glScalef(0.008, 0.008, 0.008);
		glTranslatef(-100.0f, 0.0f, -100.0f);   //Move model to origin
		renderModel(sceneModel, sceneModel->mRootNode);
	glPopMatrix(); 

	glNormal3f(0, 1, 0);
	glPushMatrix();
		glTranslatef(0.0f, -2.0f, 0.0f);
		generateCustomBackground();
	glPopMatrix();

	glPushMatrix();
		glColor4f(0.3, 0.7, 1.0, 1);
		drawSkydome();
	glPopMatrix(); 

	//Dance Skeletal
	glPushMatrix();
	   glColor4f(1, 0.75, 0.79, 1.0);
	   glScalef(scene_scale, scene_scale, scene_scale);
	   glTranslatef(-160.0f, -5.0f, -100);   //Move model to origin
	   renderSkeleton(scene->mRootNode);
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glColor3f(0.2, 0.2, 0.2);
	glPushMatrix();
		glMultMatrixf(shadowMat);
		glScalef(scene_scale, scene_scale, scene_scale);
		glTranslatef(-160.0f, -5.0f, -100);   //Move model to origin
		renderSkeleton(scene->mRootNode);
	glPopMatrix();

	//Dance Skeletal
	glPushMatrix();
		glColor4f(1, 0.75, 0.79, 1.0);
		glScalef(scene_scale, scene_scale, scene_scale);
		glTranslatef(70.0f, -5.0f, -100); // Translate to the right
		renderSkeleton(scene->mRootNode);
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glColor3f(0.2, 0.2, 0.2);
	glPushMatrix();
		glMultMatrixf(shadowMat);
		glScalef(scene_scale, scene_scale, scene_scale);
		glTranslatef(70.0f, -5.0f, -100); // Translate to the right
		renderSkeleton(scene->mRootNode);
	glPopMatrix();
	
	//Banana Character 
	glPushMatrix();
		glScalef(0.02, 0.02, 0.02);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(-35.0f, -300.0f, -12.0f);   //move the character above floor plane
		renderCharacter(character, character->mRootNode);
	glPopMatrix();

	glEnable(GL_LIGHTING);
	glutSwapBuffers();
}

void special(int key, int x, int y) {
	if (key == GLUT_KEY_PAGE_DOWN && camDist > 0) camDist -= 5;
	else if (key == GLUT_KEY_PAGE_UP && camDist < 90) camDist += 5;
	else if (key == GLUT_KEY_UP && camZ > 5) camZ -= 5;
	else if (key == GLUT_KEY_DOWN && camZ < 80) camZ += 5;
	else if (key == GLUT_KEY_LEFT && camX > -40) camX -= 5;
	else if (key == GLUT_KEY_RIGHT && camX < 40) camX += 5;
	glutPostRedisplay();
}


int main(int argc, char** argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1000, 1000);
	glutCreateWindow("Skeleton Animation");

	initialise();
	glutDisplayFunc(display);
	glutTimerFunc(timeStepC, updateCharacter, 0);
	glutTimerFunc(timeStep, update, 0);
	glutSpecialFunc(special);
	glutMainLoop();

	aiReleaseImport(sceneModel);
	aiReleaseImport(scene); 
	aiReleaseImport(character);
	
}

