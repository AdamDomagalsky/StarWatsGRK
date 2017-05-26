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
	MENU_GRID,
	MENU_TEXTURING,
	MENU_EXIT
};
typedef int BOOL;
#define TRUE 1
#define FALSE 0

static BOOL g_bLightingEnabled = TRUE;
static BOOL g_bFillPolygons = TRUE;
static BOOL g_bTexture = TRUE;
static BOOL g_bButton1Down = FALSE;
static BOOL g_bGrid = TRUE;
static GLfloat g_orbit = 0.0;
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
static GLfloat g_ZpositionArr[100], g_XpositionArr[100];
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


//multiplayer
#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "game_message.hpp"

using boost::asio::ip::tcp;

typedef std::deque<game_message> game_message_queue;

class game_client
{
public:
	game_client() = default;

		game_client(boost::asio::io_service& io_service,
		tcp::resolver::iterator endpoint_iterator)
		: io_service_(io_service),
		socket_(io_service)
	{
		do_connect(endpoint_iterator);
	}

	void write(const game_message& msg)
	{
		io_service_.post(
			[this, msg]()
		{
			bool write_in_progress = !write_msgs_.empty();
			write_msgs_.push_back(msg);
			if (!write_in_progress)
			{
				do_write();
			}
		});
	}

	void close()
	{
		io_service_.post([this]() { socket_.close(); });
	}

private:
	void do_connect(tcp::resolver::iterator endpoint_iterator)
	{
		boost::asio::async_connect(socket_,
			endpoint_iterator,
			[this](boost::system::error_code ec, tcp::resolver::iterator)
		{
			if (!ec)
			{
				do_read_header();
			}
		});
	}

	void do_read_header()
	{
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.data(), game_message::header_length),
			[this](boost::system::error_code ec, std::size_t)
		{
			if (!ec && read_msg_.decode_header())
			{
				do_read_body();
			}
			else
			{
				socket_.close();
			}
		});
	}

	void do_read_body()
	{
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
			[this](boost::system::error_code ec, std::size_t)
		{
			if (!ec)
			{
				std::cout.write(read_msg_.body(), read_msg_.body_length());
				std::cout << "\tKlient1\n";
				do_read_header();
			}
			else
			{
				socket_.close();
			}
		});
	}

	void do_write()
	{
		boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front().data(),
				write_msgs_.front().length()),
			[this](boost::system::error_code ec, std::size_t)
		{
			if (!ec)
			{
				write_msgs_.pop_front();
				if (!write_msgs_.empty())
				{
					do_write();
				}
			}
			else
			{
				socket_.close();
			}
		});
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	game_message read_msg_;
	game_message_queue write_msgs_;
/*
	game_client& operator=(game_client other)
	{
		io_service_ = other.io_service_;
		
		return *this;
	}*/


};
//multiplayer

//global ptr for game_client object.
game_client *c;
/**/

/*Procedular scene starts here*/
void cubepositions(void) { //set the positions of the cubes
	int sgn = 1;
	for (int i = 0;i<100;i++)
	{
		g_ZpositionArr[i] = (rand() % 15 + 1)*sgn;
		g_XpositionArr[i] = (rand() % 15 + 1)*sgn;
		sgn *= -1;
	}
}
//drawing cubes
void cube(void) {
	for (int i = 0;i<100 - 1;i++)
	{
		glPushMatrix();
		glTranslated(-g_XpositionArr[i + 1] * 10, 0, -g_ZpositionArr[i + 1] * 10); //translate the cube
		glutSolidCube(2); //draw the cube
		glPopMatrix();
	}
}
/*Grid proc*/
void drawGrid()
{
	GLfloat d3[] = { 0.4, 0.2, 0.2, 1.0 };
	glBegin(GL_LINES);
	for (float i = -500; i <= 500; i += 5)
	{
		glColor4fv(d3);
		glVertex3f(-500, 0, i);
		glVertex3f(500, 0, i);
		glVertex3f(i, 0, -500);
		glVertex3f(i, 0, 500);	
	}
	glEnd();
}
/*Whole scene*/
void RenderObjects(void)
{
	/*Colors*/
	float colorBronzeDiff[4] = { 0.8, 0.6, 0.0, 1.0 };
	float colorBronzeSpec[4] = { 1.0, 1.0, 0.4, 1.0 };

	float colorBlue[4] = { 0.0, 0.2, 1.0, 1.0 };
	float colorRed[4] = { 1.0,0.0,0.0,1.0 };
	float colorWhite[4] = { 1.0,1.0,1.0,1.0 };
	float colorGreen[4] = { 0.0,0.9,0.0, 1.0 };
	float colorNone[4] = { 0.0, 0.0, 0.0, 0.0 };
	/*START draw*/
	glMatrixMode(GL_MODELVIEW);
	
	if (g_bGrid)
		drawGrid();
	cube();
	/*SUN*/
	// Main object (cube) ... transform to its coordinates, and render
	glPushMatrix();
	//glRotatef(orbit, 0, 1,0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, colorRed);
	glMaterialfv(GL_FRONT, GL_SPECULAR, colorBronzeSpec);
	glMaterialf(GL_FRONT, GL_SHININESS, 5.0);
	glColor4fv(colorRed);
	glBindTexture(GL_TEXTURE_2D, TEXTURE_ID_CUBE);
	glutSolidSphere(1.0, 50, 35);
	//glPopMatrix();

	glRotatef(g_orbit, 0, 1, 0);
	/*Earth*/
	// Child object (teapot) ... relative transform, and render
	glPushMatrix();
	glTranslatef(2, 0, 0);
	glRotatef(g_orbit, 0, 1, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, colorGreen);
	glMaterialfv(GL_FRONT, GL_SPECULAR, colorWhite);
	glMaterialf(GL_FRONT, GL_SHININESS, 50.0);
	glColor4fv(colorBronzeDiff);
	glBindTexture(GL_TEXTURE_2D, 2);
	glutSolidTeapot(0.3);

	//Ring
	glPushMatrix();
	glRotatef(g_orbit / 2, 1, 0, 0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
	glMaterialfv(GL_FRONT, GL_SPECULAR, colorWhite);
	glMaterialf(GL_FRONT, GL_SHININESS, 50.0);
	glColor4fv(colorBronzeDiff);
	glBindTexture(GL_TEXTURE_2D, 3);
	glutSolidTorus(0.1, 0.5, 2, 40);

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -2, 0);
	glScalef(1, cos(g_orbit / 10), sin(g_orbit / 20));
	glRotatef(g_orbit / 2, 0, -1, 1);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, colorRed);
	glMaterialfv(GL_FRONT, GL_SPECULAR, colorWhite);
	glMaterialf(GL_FRONT, GL_SHININESS, 50.0);
	glColor4fv(colorBronzeDiff);
	glBindTexture(GL_TEXTURE_2D, 3);
	glutSolidCube(2.0);



	//glutSolidSphere(0.3,50,35); // size, slices quality
	//glutSolidTorus(1, 2, 20, 20);

	glPopMatrix();
}

void display(void)
{
	
	//char line[game_message::max_body_length + 1];
	//while (std::cin.getline(line, game_message::max_body_length + 1)){
	//if (std::cin.getline(line, game_message::max_body_length + 1))
	//{
		//sprintf(line, "Xxxx:%f Z:%f", g_Pos.x, g_Pos.z);
		//sprintf(line, "\n");
		game_message msg;
		
		//msg.body_length(std::strlen(line));
		//std::memcpy(msg.body(), line, msg.length());
		//msg.encode_header();

		msg.setPos(g_Pos);
		(*c).write(msg);
		glm::vec3 tmp = msg.getPos();
		std::cout << "X:" << tmp.x << "\tY:" << tmp.y << std::endl;
	//}
		
	//}

	// Clear frame buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glLoadIdentity();
	/*TODO ship model*/
	//std::cout << "\yX: " << g_Pos.x << "\tZ: " << g_Pos.z << std::endl;
	glTranslatef(0.0f, 0.0f, -g_fViewDistance); // 0,0,0 if we want 1st person camera
	glRotatef(g_Rot.x, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 1.0f, 0.0f);
	//glutSolidCube(1); //Our character to follow
	glutSolidTeapot(1);
	
	
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
	g_orbit += dt*25.0;
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
	case MENU_GRID:
		g_bGrid = !g_bGrid;
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
		case 'g':
			SelectFromMenu(MENU_GRID);
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
	glutAddMenuEntry("Toggle mesh grid\tp", MENU_GRID);
	glutAddMenuEntry("Toggle texturing\tt", MENU_TEXTURING);
	glutAddMenuEntry("Exit demo\tEsc", MENU_EXIT);
	return menu;
}
void GRKmain(int argc, char** argv)
{

	boost::asio::io_service io_service;

	tcp::resolver resolver(io_service);
	auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
	//global ptr at game_client;
	c = new game_client(io_service, endpoint_iterator);

	std::thread t([&io_service]() { io_service.run(); });

	/* see display() ^^^^
	char line[game_message::max_body_length + 1];
	while (std::cin.getline(line, game_message::max_body_length + 1))
	{
	game_message msg;
	msg.body_length(std::strlen(line));
	std::memcpy(msg.body(), line, msg.length());
	msg.encode_header();
	(*c).write(msg);
	}*/

	// GLUT Window Initialization:
	glutInit(&argc, argv);
	glutInitWindowSize(g_Width, g_Height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Project GRK");
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
	
	(*c).close();
	//prevent memory leak
	delete c;
	t.join();
	//return 0;
}