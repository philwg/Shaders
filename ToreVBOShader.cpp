#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include "shader.hpp"
#include <string.h>

#include "./glm/glm.hpp"
#include "./glm/gtc/matrix_transform.hpp"

using namespace glm;
using namespace std;

#define P_SIZE 3
#define N_SIZE 3
#define C_SIZE 3

#define N_VERTS  8
#define N_VERTS_BY_FACE  3
#define N_FACES  12

#define NB_R 40
#define NB_r 20
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLuint indices[NB_R*NB_r*6];				// x 6 car pour chaque face quadrangulaire on a 6 indices (2 triangles=2x 3 indices)
GLfloat sommets[(NB_R+1)*(NB_r+1)*3];		// x 3 coordonnées (+1 acr on double les dernierspoints pour avoir des coord de textures <> pour les points de jonctions)
GLfloat coordTexture[(NB_R+1)*(NB_r+1)*2]; 	// x 2 car U+V par sommets
GLfloat normales[(NB_R+1)*(NB_r+1)*3];

/** Prototypes ********************************************************/

void genereVBO();
void deleteVBO();
void traceObjet();
void affichage();
void clavier(unsigned char,int,int);
void mouse(int, int, int, int);
void mouseMotion(int, int);
void reshape(int,int);
void drawString(const char *str, int x, int y, float color[4], void *font);
void showInfo();

/** Variables OpenGL **************************************************/

void *font = GLUT_BITMAP_8_BY_13;
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance = 0.0f;
GLuint programID;
GLuint MatrixIDMVP, MatrixIDView, MatrixIDModel, MatrixIDPerspective;
GLuint VBO_sommets, VBO_normales, VBO_indices, VBO_UVtext, VAO;
GLuint locTextured;
GLuint locGama;
GLuint locCameraPosition;
GLuint locmaterialShininess;
GLuint locmaterialSpecularColor;
GLuint locLightPosition;
GLuint locLightIntensities;
GLuint locLightAttenuation;
GLuint locLightAmbientCoefficient;
GLuint indexVertex = 0, indexUVTexture = 2, indexNormale = 3 ;
vec3 cameraPosition(0.0, 0.0, 3.0);
GLfloat materialShininess = 6.0f;
GLint textured = 1;
GLfloat gama = 2.4f;
vec3 materialSpecularColor(0.6, 0.6, 0.6);

vec3 LightPosition(1.0, 0.0, 0.5);
vec3 LightIntensities(1.0, 1.0, 1.0);
GLfloat LightAttenuation = 1.0f;
GLfloat LightAmbientCoefficient = 0.6f;

glm::mat4 MVP;
glm::mat4 Model, View, Projection;  // Matrices constituant MVP

int screenHeight = 500;
int screenWidth = 500;
int intex = 0;
char texture_Path[][36] = {	"./Textures/Metalcolor.ppm",
							"./Textures/Bricks01_COL_VAR1_1K.ppm",
							"./Textures/opengl.ppm" };
char normal_Path[][36] = {	"./Textures/MetalNRM.ppm",
							"./Textures/Bricks01_NRM_1K.ppm",
							"./Textures/Bricks01_NRM_1K.ppm"	};
GLuint image ;
GLuint bufTexture,bufNormalMap;
GLuint locationTexture,locationNormalMap;

/** Méthodes **********************************************************/

//----------------------------------------------------------
void createTorus(float R, float r )
//----------------------------------------------------------
{
	float theta, phi;
	theta = ((float)radians(360.0f))/((float)NB_R);
	phi = ((float)(radians(360.0f)))/((float)NB_r);
	float pasU, pasV;
	pasU= 1.0f/NB_R;
	pasV= 1.0f/NB_r;
	for (int i=0; i<=NB_R; i++) {
		for (int j=0; j<=NB_r; j++) {
			
			sommets[(i*(NB_r+1)*3)+(j*3)]	= (R+r*cos((float)j*phi))*cos((float)i*theta);
			sommets[(i*(NB_r+1)*3)+(j*3)+1] = (R+r*cos((float)j*phi))*sin((float)i*theta);
			sommets[(i*(NB_r+1)*3)+(j*3)+2] = r*sin((float)j*phi);
			
			normales[(i*(NB_r+1)*3)+(j*3)]	 = cos((float)j*phi)*cos((float)i*theta);
			normales[(i*(NB_r+1)*3)+(j*3)+1] = cos((float)j*phi)*sin((float)i*theta);
			normales[(i*(NB_r+1)*3)+(j*3)+2] = sin((float)j*phi);
			
			coordTexture[(i*(NB_r+1)*2)+ (j*2)]= ((float)i)*pasU;
			coordTexture[(i*(NB_r+1)*2)+ (j*2)+1]= ((float)j)*pasV;
		}
	}
	int indiceMaxI = ((NB_R+1)*(NB_r))-1;
	int indiceMaxJ = (NB_r+1);
	for (int i=0; i<NB_R; i++) {
		for (int j=0; j<NB_r; j++) {
			indices[(i*NB_r*6)+(j*6)]   = (unsigned int)((i*(NB_r+1))+j);
			indices[(i*NB_r*6)+(j*6)+1] = (unsigned int)((i+1)*(NB_r+1)+(j));
			indices[(i*NB_r*6)+(j*6)+2] = (unsigned int)(((i+1)*(NB_r+1))+(j+1));
			indices[(i*NB_r*6)+(j*6)+3] = (unsigned int)((i*(NB_r+1))+j);
			indices[(i*NB_r*6)+(j*6)+4] = (unsigned int)(((i+1)*(NB_r+1))+(j+1));
			indices[(i*NB_r*6)+(j*6)+5] = (unsigned int)(((i)*(NB_r+1))+(j+1));
		}
	}
}

//----------------------------------------------------------
GLubyte* glmReadPPM(char* filename, int* width, int* height)
//----------------------------------------------------------
{
    FILE* fp;
    int i, w, h, d;
    unsigned char* image;
    char head[70];          /* max line <= 70 in PPM (per spec). */
    
    fp = fopen(filename, "rb");
    if (!fp) {
        perror(filename);
        return NULL;
    }
    
    /* grab first two chars of the file and make sure that it has the
       correct magic cookie for a raw PPM file. */
    fgets(head, 70, fp);
    if (strncmp(head, "P6", 2)) {
        fprintf(stderr, "%s: Not a raw PPM file\n", filename);
        return NULL;
    }
    
    /* grab the three elements in the header (width, height, maxval). */
    i = 0;
    while(i < 3) {
        fgets(head, 70, fp);
        if (head[0] == '#') continue;
        if (i == 0) i += sscanf(head, "%d %d %d", &w, &h, &d);
        else if (i == 1) i += sscanf(head, "%d %d", &h, &d);
        else if (i == 2) i += sscanf(head, "%d", &d);
    }
    
    /* grab all the image data in one fell swoop. */
    image = new unsigned char[w*h*3];
    fread(image, sizeof(unsigned char), w*h*3, fp);
    fclose(fp);
    
    *width = w;
    *height = h;
    return image;
}

//----------------------------------------------------------
void initTexture(void)
//----------------------------------------------------------
{
	int iwidth, iheight;
	GLubyte *  image = NULL;
	image = glmReadPPM(texture_Path[intex], &iwidth, &iheight);
	glGenTextures(1, &bufTexture);
	glBindTexture(GL_TEXTURE_2D, bufTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, iwidth, iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	locationTexture = glGetUniformLocation(programID, "myTextureSampler");
	glBindAttribLocation(programID, indexUVTexture, "vertexUV");
}

//----------------------------------------------------------
void initNormalMap(void)
//----------------------------------------------------------
{
	int iwidth, iheight;
	GLubyte *  image = NULL;
	image = glmReadPPM(normal_Path[intex], &iwidth, &iheight);
	glGenTextures(1, &bufTexture);
	glBindTexture(GL_TEXTURE_2D, bufTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, iwidth, iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	locationNormalMap = glGetUniformLocation(programID, "myNormalSampler");
	glBindAttribLocation(programID, indexUVTexture, "normalUV");
}

//----------------------------------------------------------
void initOpenGL(void)
//----------------------------------------------------------
{
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST); 
  
	// le shader
	programID = LoadShaders("PhongShader.vert", "PhongShader.frag");
  
	// Get handles for our matrix transformations "MVP" VIEW  MODEL uniform
	MatrixIDMVP = glGetUniformLocation(programID, "MVP");
	MatrixIDView = glGetUniformLocation(programID, "VIEW");
	MatrixIDModel = glGetUniformLocation(programID, "MODEL");
	MatrixIDPerspective = glGetUniformLocation(programID, "PERSPECTIVE");

	// Projection matrix : 65 Field of View, 1:1 ratio, display range : 1 unit <-> 1000 units
	// ATTENTIOn l'angle est donné en radians si f GLM_FORCE_RADIANS est défini sinon en degré
	Projection = glm::perspective( glm::radians(60.f), 1.0f, 1.0f, 1000.0f);

	locGama = glGetUniformLocation(programID, "gama");
	locTextured = glGetUniformLocation(programID, "textured");
	locCameraPosition = glGetUniformLocation(programID, "cameraPosition");
	locmaterialShininess = glGetUniformLocation(programID, "materialShininess");
	locmaterialSpecularColor = glGetUniformLocation(programID, "materialSpecularColor");
	locLightAttenuation = glGetUniformLocation(programID, "light.attenuation");
	locLightAmbientCoefficient = glGetUniformLocation(programID, "light.ambientCoefficient");
	locLightIntensities = glGetUniformLocation(programID, "light.intensities");
	locLightPosition = glGetUniformLocation(programID, "light.position");
}

//----------------------------------------------------------
int main(int argc,char **argv)
//----------------------------------------------------------
{
	/* initialisation de glut et creation de la fenetre */
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE|GLUT_RGB);
	glutInitWindowPosition(200,200);
	glutInitWindowSize(screenWidth,screenHeight);
	glutCreateWindow("CUBE VBO SHADER ");
	
	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
	
	//info version GLSL
	std::cout << "***** Info GPU *****" << std::endl;
	std::cout << "Fabricant : " << glGetString (GL_VENDOR) << std::endl;
	std::cout << "Carte graphique: " << glGetString (GL_RENDERER) << std::endl;
	std::cout << "Version : " << glGetString (GL_VERSION) << std::endl;
	std::cout << "Version GLSL : " << glGetString (GL_SHADING_LANGUAGE_VERSION) << std::endl << std::endl;
	initOpenGL();    
    createTorus(1.0f, 0.3f);

	// construction des VBO
	genereVBO();
	initNormalMap();
	initTexture();
		
	/* enregistrement des fonctions de rappel */
	glutDisplayFunc(affichage);
	glutKeyboardFunc(clavier);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	
	/* Entree dans la boucle principale glut */
	glutMainLoop();
	glDeleteProgram(programID);
	deleteVBO();
	return 0;
}

//----------------------------------------------------------
void genereVBO ()
//----------------------------------------------------------
{
    glGenBuffers(1, &VAO);
    glBindVertexArray(VAO);
    
    if (glIsBuffer(VBO_sommets)==GL_TRUE) glDeleteBuffers(1, &VBO_sommets);
    glGenBuffers(1, &VBO_sommets);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_sommets);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sommets),sommets , GL_STATIC_DRAW);
    glVertexAttribPointer(indexVertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    if (glIsBuffer(VBO_normales)==GL_TRUE) glDeleteBuffers(1, &VBO_normales);
    glGenBuffers(1, &VBO_normales);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_normales);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normales),normales , GL_STATIC_DRAW);
    glVertexAttribPointer(indexNormale, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    if (glIsBuffer(VBO_indices)==GL_TRUE) glDeleteBuffers(1, &VBO_indices);
    glGenBuffers(1, &VBO_indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices , GL_STATIC_DRAW);
    
    if (glIsBuffer(VBO_UVtext)==GL_TRUE) glDeleteBuffers(1, &VBO_UVtext);
    glGenBuffers(1, &VBO_UVtext);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_UVtext);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordTexture), coordTexture, GL_STATIC_DRAW);
    glVertexAttribPointer(indexUVTexture, 2, GL_FLOAT, GL_FALSE, 0,  (void*)0);
    
    glEnableVertexAttribArray(indexVertex);
    glEnableVertexAttribArray(indexNormale);
    glEnableVertexAttribArray(indexUVTexture);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

//----------------------------------------------------------
void deleteVBO ()
//----------------------------------------------------------
{
	glDeleteBuffers(1, &VBO_sommets);
	glDeleteBuffers(1, &VBO_normales);
	glDeleteBuffers(1, &VBO_indices);
	glDeleteBuffers(1, &VBO_UVtext);
	glDeleteBuffers(1, &VAO);
}

//----------------------------------------------------------
void affichage()
//----------------------------------------------------------
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(10.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
	glPointSize(2.0f);
	View = glm::lookAt(cameraPosition, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, glm::vec3(0,0,cameraDistance));
	Model = glm::rotate(Model, glm::radians(cameraAngleX), glm::vec3(1.0, 0.0, 0.0) );
	Model = glm::rotate(Model, glm::radians(cameraAngleY), glm::vec3(0.0, 1.0, 0.0) );
	Model = glm::scale(Model, glm::vec3(0.8, 0.8, 0.8));
	MVP = Projection * View * Model;
	traceObjet();
	glutPostRedisplay();
	glutSwapBuffers();
}

//----------------------------------------------------------
void traceObjet()
//----------------------------------------------------------
{
	glUseProgram(programID);

	glUniformMatrix4fv(MatrixIDMVP, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(MatrixIDView, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(MatrixIDModel, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(MatrixIDPerspective, 1, GL_FALSE, &Projection[0][0]);
	
	glUniform1f(locGama, gama);
	glUniform1i(locTextured, textured);
	glUniform1f(locmaterialShininess, materialShininess);
	glUniform1f(locLightAttenuation, LightAttenuation);
	glUniform1f(locLightAmbientCoefficient, LightAmbientCoefficient);
	
	glUniform3f(locCameraPosition, cameraPosition.x, cameraPosition.y, cameraPosition.z);
	glUniform3f(locmaterialSpecularColor, materialSpecularColor.x, materialSpecularColor.y, materialSpecularColor.z);
	glUniform3f(locLightIntensities, LightIntensities.x, LightIntensities.y, LightIntensities.z);
	glUniform3f(locLightPosition, LightPosition.x, LightPosition.y, LightPosition.z);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

//----------------------------------------------------------
void reshape(int w, int h)
//----------------------------------------------------------
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	float aspectRatio = (float)w / h;
	Projection = glm::perspective(glm::radians(60.0f), (float)(w)/(float)h, 1.0f, 1000.0f);
}

//----------------------------------------------------------
void clavier(unsigned char touche,int x,int y)
//----------------------------------------------------------
{
  switch (touche) {
	  
    case 'f':	/* affichage du carre plein */
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      glutPostRedisplay();
      break;
      
    case 'e':	/* affichage en mode fil de fer */
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      glutPostRedisplay();
      break;
      
    case 'v' :	/* Affichage en mode sommets seuls */
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      glutPostRedisplay();
      break;
      
    case 's' :	/* diminution de la Shininess */
      materialShininess -= 0.5f;
      glutPostRedisplay();
      break;
      
    case 'S' :	/* Augmentation de la Shininess */
      materialShininess += 0.5f;
      glutPostRedisplay();
      break;
      
    case 'x' :	/* Déplacement en x de la lumière */
      LightPosition.x -= 0.2f;
      glutPostRedisplay();
      break;
      
    case 'X' :	/* Déplacement en x de la lumière */
      LightPosition.x += 0.2f;
      glutPostRedisplay();
      break;
      
    case 'y' :	/* Déplacement en y de la lumière */
      LightPosition.y -= 0.2f;
      glutPostRedisplay();
      break;
      
    case 'Y' : 	/* Déplacement en y de la lumière */
      LightPosition.y += 0.2f;
      glutPostRedisplay();
      break;
      
    case 'z' :	/* Déplacement en z de la lumière */
      LightPosition.z -= 0.2f;
      glutPostRedisplay();
      break;
      
    case 'Z' :	/* Déplacement en z de la lumière */
      LightPosition.z += 0.2f;
      glutPostRedisplay();
      break;
      
    case 'a' :	/* Diminution de la lumière ambiante */
      LightAmbientCoefficient -= 0.1f;
      glutPostRedisplay();
      break;
      
    case 'A' :	/* Augmentation de la lumière ambiante */
      LightAmbientCoefficient += 0.1f;
      glutPostRedisplay();
      break;
      
    case 't' :	/* Texturé ou non */
		if (textured!=0) textured = 0;
		else textured = 1;
		glutPostRedisplay();
		break;
		
	case 'T' :	/* Switch de texture */
		if (textured!=0) {
			if (intex<2) intex++;
			else intex = 0;
			genereVBO();
			initNormalMap();
			initTexture();
		}
		glutPostRedisplay();
		break;
		
    case 'G' :	/* Augmentation du Gama */
		if (gama>0.2f) gama -= 0.1f;
		glutPostRedisplay();
		break;
		
    case 'g' :	/* Switch Texture ou non */
		if (gama<3.0f) gama += 0.1f;
		glutPostRedisplay();
		break;
		
	case 'q' : /* Quitter */
      exit(0);
    }
}

//----------------------------------------------------------
void mouse(int button, int state, int x, int y)
//----------------------------------------------------------
{
    mouseX = x;
    mouseY = y;
    if(button == GLUT_LEFT_BUTTON) {
        if(state == GLUT_DOWN) mouseLeftDown = true;
        else if(state == GLUT_UP) mouseLeftDown = false;
	}
    else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) mouseRightDown = true;
        else if(state == GLUT_UP) mouseRightDown = false;
    }
    else if(button == GLUT_MIDDLE_BUTTON) {
		if(state == GLUT_DOWN) mouseMiddleDown = true;
		else if(state == GLUT_UP) mouseMiddleDown = false;
    }
}


//----------------------------------------------------------
void mouseMotion(int x, int y)
//----------------------------------------------------------
{
    if(mouseLeftDown) {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown) {
        cameraDistance += (y - mouseY) * 0.2f;
        mouseY = y;
    }
    glutPostRedisplay();
}
