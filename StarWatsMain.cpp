// glut_example.c
// Stanford University, CS248, Fall 2000
//
// Demonstrates basic use of GLUT toolkit for CS248 video game assignment.
// More GLUT details at http://reality.sgi.com/mjk_asd/spec3/spec3.html
// Here you'll find examples of initialization, basic viewing transformations,
// mouse and keyboard callbacks, menus, some rendering primitives, lighting,
// double buffering, Z buffering, and texturing.
//
// Matt Ginzton -- magi@cs.stanford.edu
#include <glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <glm.hpp>
#ifdef _WIN32
#	include <windows.h>
#else
#	include <sys/time.h>
#endif
//#include "texture.h"
#define VIEWING_DISTANCE_MIN  3.0
#define VIEWING_CAMERA_ANGLE_MIN -50
#define VIEWING_CAMERA_ANGLE_MAX 50
#define TEXTURE_ID_CUBE 1
#define M_PI 3.14159265358979323846f
#define degreesToRadians(angleDegrees) (angleDegrees * M_PI / 180.0)

enum {
	MENU_LIGHTING = 1,
	MENU_POLYMODE,
	MENU_TEXTURING,
	MENU_EXIT
};
typedef int BOOL;
#define TRUE 1
#define FALSE 0
static BOOL g_bLightingEnabled = TRUE;
static BOOL g_bFillPolygons = TRUE;
static BOOL g_bTexture = FALSE;
static BOOL g_bButton1Down = FALSE;
static GLfloat g_fTeapotAngle = 0.0;
static GLfloat g_fTeapotAngle2 = 0.0;
static GLfloat g_fViewDistance = 3 * VIEWING_DISTANCE_MIN;
static GLfloat g_nearPlane = 1;
static GLfloat g_farPlane = 1000;
glm::vec3 pos;
//Camera params
static glm::vec3 g_Pos; // Current coordinates x y z
static glm::vec2 g_Rot; // Current rotation x y
static GLfloat g_angle = 0;								// our radius distance from our character
static GLfloat g_ZpositionArr[10], g_XpositionArr[10];
static glm::vec2 g_lastPos;	// Last coordinates

static int g_Width = 600;                          // Initial window width
static int g_Height = 600;                         // Initial window height
static int g_yClick = 0;
static float g_lightPos[4] = { 10, 100, -100, 1 };  // Position of light
#ifdef _WIN32
static DWORD last_idle_time;
#else
static struct timeval last_idle_time;
#endif

/*Procedular scene starts here*/
void cubepositions(void) { //set the positions of the cubes

	for (int i = 0;i<10;i++)
	{
		g_ZpositionArr[i] = rand() % 5 + 1;
		g_XpositionArr[i] = rand() % 5 + 1;
	}
}

//drawing cubes
void cube(void) {
	for (int i = 0;i<10 - 1;i++)
	{
		glPushMatrix();
		glTranslated(-g_XpositionArr[i + 1] * 10, 0, -g_ZpositionArr[i + 1] *
			10); //translate the cube
		glutSolidCube(2); //draw the cube
		glPopMatrix();
	}
}

/*Whole scene*/
void RenderObjects(void)
{
	cube();
	GLfloat colorBronzeDiff[4] = { 0.8, 0.6, 0.0, 1.0 };
	GLfloat colorBronzeSpec[4] = { 1.0, 1.0, 0.4, 1.0 };
	GLfloat colorBlue[4] = { 0.0, 0.2, 1.0, 1.0 };
	GLfloat colorNone[4] = { 0.0, 0.0, 0.0, 0.0 };
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	// Main object (cube) ... transform to its coordinates, and render
	glRotatef(15, 1, 0, 0);
	glRotatef(45, 0, 1, 0);
	glRotatef(g_fTeapotAngle, 0, 0, 1);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, colorBlue);
	glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
	glColor4fv(colorBlue);
	glBindTexture(GL_TEXTURE_2D, TEXTURE_ID_CUBE);
	glutSolidCube(1.0);
	// Child object (teapot) ... relative transform, and render
	glPushMatrix();
	glTranslatef(2, 0, 0);
	glRotatef(g_fTeapotAngle2, 1, 1, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, colorBronzeDiff);
	glMaterialfv(GL_FRONT, GL_SPECULAR, colorBronzeSpec);
	glMaterialf(GL_FRONT, GL_SHININESS, 50.0);
	glColor4fv(colorBronzeDiff);
	glBindTexture(GL_TEXTURE_2D, 0);
	glutSolidTeapot(0.3);
	glPopMatrix();
	glPopMatrix();
}

void display(void)
{
	// Clear frame buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	/*TODO ship model*/
	//glTranslatef(0.0f, 0.0f, -g_cRadius);
	glTranslatef(0.0f, 0.0f, -g_fViewDistance); // 0,0,0 if we want 1st person camera
	glRotatef(g_Rot.x, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 1.0f, 0.0f);
	glutSolidCube(1); //Our character to follow

	glRotatef(g_Rot.y, 0.0, 1.0, 0.0);  //rotate our camera on the 	y - axis(up and down)
	glTranslated(-g_Pos.x, 0.0f, -g_Pos.z); //translate the screen	to the position of our camera
	glColor3f(1.0f, 1.0f, 1.0f);
		
	// Render the scene
	RenderObjects();

	// Set up the stationary light
	glLightfv(GL_LIGHT0, GL_POSITION, g_lightPos);

	// Make sure changes appear onscreen
	glutSwapBuffers(); //swap the buffers
	g_angle++; //increase the angle
}
void reshape(GLint width, GLint height)
{
	g_Width = width;
	g_Height = height;
	glViewport(0, 0, g_Width, g_Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0, (float)g_Width / g_Height, g_nearPlane, g_farPlane);
	glMatrixMode(GL_MODELVIEW);
}
void InitGraphics(void)
{
	int width, height;
	int nComponents;
	void* pTextureImage;
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	cubepositions();

	/*Textures loading here*/


	// Create texture for cube; load marble texture from file and bind it
	/*pTextureImage = read_texture("marble.rgb", &width, &height, &nComponents);
	glBindTexture(GL_TEXTURE_2D, TEXTURE_ID_CUBE);
	gluBuild2DMipmaps(GL_TEXTURE_2D,     // texture to specify
		GL_RGBA,           // internal texture storage format
		width,             // texture width
		height,            // texture height
		GL_RGBA,           // pixel format
		GL_UNSIGNED_BYTE,	// color component format
		pTextureImage);    // pointer to texture image
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);*/
}
void MouseButton(int button, int state, int x, int y)
{
	// Respond to mouse button presses.
	// If button1 pressed, mark this state so we know in motion function.
	if (button == GLUT_LEFT_BUTTON)
	{
		g_bButton1Down = (state == GLUT_DOWN) ? TRUE : FALSE;
		g_yClick = y - 3 * g_fViewDistance;
	}
}
void MouseMotion(int x, int y)
{
	// If button1 pressed, zoom in/out if mouse is moved up/down.
	if (g_bButton1Down)
	{
		g_fViewDistance = (y - g_yClick) / 3.0;
		if (g_fViewDistance < VIEWING_DISTANCE_MIN)
			g_fViewDistance = VIEWING_DISTANCE_MIN;
		glutPostRedisplay();
		std::cout << g_fViewDistance;
	}
	else
	{
		int diffx = x - g_lastPos.x; //check the difference between the 	current x and the last x position
		int diffy = y - g_lastPos.y; //check the difference between the 	current y and the last y position
		g_lastPos.x = x; //set lastx to the current x position
		g_lastPos.y = y; //set lasty to the current y position
		//set the xrot to xrot with the addition	of the difference in the y position
		//x camera rotation limits
		if ( g_Rot.x > VIEWING_CAMERA_ANGLE_MAX ){
			g_Rot.x = VIEWING_CAMERA_ANGLE_MAX;
		}
		else if (g_Rot.x < VIEWING_CAMERA_ANGLE_MIN)
		{
			g_Rot.x = VIEWING_CAMERA_ANGLE_MIN;
		}
		else
		{
			g_Rot.x += (float)diffy;
		}
		g_Rot.y += (float)diffx;
	}	
	std::cout << pos.x << std::endl;
}
void AnimateScene(void)
{
	float dt;
#ifdef _WIN32
	DWORD time_now;
	time_now = GetTickCount();
	dt = (float)(time_now - last_idle_time) / 1000.0;
#else
	// Figure out time elapsed since last call to idle function
	struct timeval time_now;
	gettimeofday(&time_now, NULL);
	dt = (float)(time_now.tv_sec - last_idle_time.tv_sec) +
		1.0e-6*(time_now.tv_usec - last_idle_time.tv_usec);
#endif
	// Animate the teapot by updating its angles
	g_fTeapotAngle += dt * 30.0;
	g_fTeapotAngle2 += dt * 100.0;
	// Save time_now for next time
	last_idle_time = time_now;
	// Force redraw
	glutPostRedisplay();
}
void SelectFromMenu(int idCommand)
{
	switch (idCommand)
	{
	case MENU_LIGHTING:
		g_bLightingEnabled = !g_bLightingEnabled;
		if (g_bLightingEnabled)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);
		break;
	case MENU_POLYMODE:
		g_bFillPolygons = !g_bFillPolygons;
		glPolygonMode(GL_FRONT_AND_BACK, g_bFillPolygons ? GL_FILL : GL_LINE);
		break;
	case MENU_TEXTURING:
		g_bTexture = !g_bTexture;
		if (g_bTexture)
			glEnable(GL_TEXTURE_2D);
		else
			glDisable(GL_TEXTURE_2D);
		break;
	case MENU_EXIT:
		exit(0);
		break;
	}
	// Almost any menu selection requires a redraw
	glutPostRedisplay();
}
void Keyboard(unsigned char key, int x, int y)
{
	GLfloat xrotrad, yrotrad;
	switch (key)
	{
		case 'q':
			g_Rot.x += 1;
				if (g_Rot.x >360) g_Rot.x -= 360;
			break;
		case 'z':
			g_Rot.x -= 1;
			if (g_Rot.x < -360) g_Rot.x += 360;
			break;
		case 'w':
			yrotrad = degreesToRadians(g_Rot.y);
			xrotrad = degreesToRadians(g_Rot.x);
			g_Pos.x += float(sin(yrotrad));
			g_Pos.z -= float(cos(yrotrad));
			g_Pos.y -= float(sin(xrotrad));
			break;
		case 's':
			yrotrad = degreesToRadians(g_Rot.y);
			xrotrad = degreesToRadians(g_Rot.x);
			g_Pos.x -= float(sin(yrotrad));
			g_Pos.z += float(cos(yrotrad));
			g_Pos.y += float(sin(xrotrad));
			break;
		case 'd':
			yrotrad = degreesToRadians(g_Rot.y);
			g_Pos.x += float(cos(yrotrad)) * 0.2;
			g_Pos.z += float(sin(yrotrad)) * 0.2;
			break;
		case 'a':
			yrotrad = degreesToRadians(g_Rot.y);
			g_Pos.x -= float(cos(yrotrad)) * 0.2;
			g_Pos.z -= float(sin(yrotrad)) * 0.2;
			break;
		case 27:            // ESCAPE key
			exit(0);
			break;
		case 'l':
			SelectFromMenu(MENU_LIGHTING);
			break;
		case 'p':
			SelectFromMenu(MENU_POLYMODE);
			break;
		case 't':
			SelectFromMenu(MENU_TEXTURING);
			break;
	}
}
int BuildPopupMenu(void)
{
	int menu;
	menu = glutCreateMenu(SelectFromMenu);
	glutAddMenuEntry("Toggle lighting\tl", MENU_LIGHTING);
	glutAddMenuEntry("Toggle polygon fill\tp", MENU_POLYMODE);
	glutAddMenuEntry("Toggle texturing\tt", MENU_TEXTURING);
	glutAddMenuEntry("Exit demo\tEsc", MENU_EXIT);
	return menu;
}
int main(int argc, char** argv)
{
	// GLUT Window Initialization:
	glutInit(&argc, argv);
	glutInitWindowSize(g_Width, g_Height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("CS248 GLUT example");
	// Initialize OpenGL graphics state
	InitGraphics();
	// Register callbacks:
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);// to zoom in 'n' out from model
	glutPassiveMotionFunc(MouseMotion); // to move around camera
	glutIdleFunc(AnimateScene);
	// Create our popup menu
	BuildPopupMenu();
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	// Get the initial time, for use by animation
#ifdef _WIN32
	last_idle_time = GetTickCount();
#else
	gettimeofday(&last_idle_time, NULL);
#endif
	// Turn the flow of control over to GLUT
	glutMainLoop();
	return 0;
}