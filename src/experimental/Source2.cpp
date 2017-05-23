#include <windows.h>
#include <stdio.h>

#include <glut.h>
#include <math.h>
#include <cstdlib>

float anglex = 35;
float angley = -35;
float anglez = 0;
float locZ = 0.0;
float locY = 0.0;
float locX = 0.0;

GLUquadric *earth;
GLuint earthTexture;


void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	case 'a':
		anglex += 5;
		break;
	case 'A':
		anglex -= 5;
		break;
	case 's':
		angley -= 5;
		break;
	case 'S':
		angley += 5;
		break;
	case 'd':
		anglez += 5;
		break;
	case 'D':
		anglez -= 5;
		break;
	case 'u':
		locZ -= 0.05;
		break;
	case 'U':
		locZ += 0.05;
		break;
	case 'k':
		locY += 0.05;
		break;
	case 'j':
		locY -= 0.05;
		break;
	case 'l':
		locX += 0.05;
		break;
	case 'h':
		locX -= 0.05;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, earthTexture);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//rotate/translate entire scene with a,A,s,S,D,D and h,l,j,k,u,U keys
	glRotatef(anglez, 0.0, 0.0, 1.0);
	glRotatef(angley, 0.0, 1.0, 0.0);
	glRotatef(anglex, 1.0, 0.0, 0.0);

	//draw textured sphere
	glPushMatrix();
	glTranslatef(locX, locY, locZ);
	glScalef(0.5, 0.5, 0.5);
	gluSphere(earth, 0.9, 36, 72);
	glPopMatrix();

	//draw textured rectangle
	glPushMatrix();
	glTranslatef(0.0, 0.0, 1.0);
	glScalef(0.5, 0.5, 0.5);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(-1.0, -1.0);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(1.0, -1.0);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(1.0, 1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(-1.0, 1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(-1.0, -1.0);
	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);

	glutSwapBuffers();
}

void init(void)
{
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -2.0, 2.0);

	earth = gluNewQuadric();
	gluQuadricTexture(earth, GL_TRUE);
	//earthTexture = LoadBitmap("data/EarthTexture.bmp");
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Testing Texture");
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}