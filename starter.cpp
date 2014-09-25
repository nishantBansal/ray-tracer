#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"


// The Width & Height have been reduced to help you debug
//	more quickly by reducing the resolution 
//  Your code should work for any dimension, and should be set back
//	to 640x480 for submission.

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 400
#define EPSILON 0.0001

#include "Scene.h"
#include "RayTrace.h"
#include "NormalRenderer.h"

/* --- Global State Variables --- */

/* - Menu State Identifier - */
int g_iMenuId;

/* - Mouse State Variables - */
int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

/* - RayTrace Variable for the RayTrace Image Generation - */
RayTrace g_RayTrace1;
RayTrace g_RayTrace2;
RayTrace g_RayTrace3;
RayTrace g_RayTrace4;

/* - NormalRenderer Variable for drawing with OpenGL calls instead of the RayTracer - */
NormalRenderer g_NormalRenderer1;
NormalRenderer g_NormalRenderer2;
NormalRenderer g_NormalRenderer3;
NormalRenderer g_NormalRenderer4;

/* - RayTrace Buffer - */
Vector g_ScreenBuffer[WINDOW_HEIGHT][WINDOW_WIDTH];

unsigned int g_X = 0, g_Y = 0;
bool g_bRayTrace = true;
bool g_bRenderNormal = false;
int scene_id = 1;

void myinit()
{
	// Default to these camera settings
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, 1, -1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set the Scene Variable for the NormalRenderer
	g_NormalRenderer1.SetScene (&g_RayTrace1.m_Scene);
	g_NormalRenderer2.SetScene (&g_RayTrace2.m_Scene);
	g_NormalRenderer3.SetScene (&g_RayTrace3.m_Scene);
	g_NormalRenderer4.SetScene (&g_RayTrace4.m_Scene);

	glClearColor(0, 0, 0, 0);
}

void display()
{
	if (g_bRenderNormal)
	{
		if(scene_id == 1)
			g_NormalRenderer1.RenderScene ();
		else if(scene_id == 2)
			g_NormalRenderer2.RenderScene ();
		else if(scene_id == 3)
			g_NormalRenderer3.RenderScene ();
		else
			g_NormalRenderer4.RenderScene ();
	}
	else
	{
		if(scene_id == 1)
			g_RayTrace1.RenderScene();
		else if(scene_id == 2)
			g_RayTrace2.RenderScene();
		else if(scene_id == 3)
			g_RayTrace3.RenderScene();
		else
			g_RayTrace4.RenderScene();
	}

	glFlush();

	glutSwapBuffers ();
}

/* 
 * The rountine reshape() gets called each time the window is reshaped.
 */
void reshape(int width_new, int height_new)
{
	g_RayTrace1.win_width = (float)width_new;
	g_RayTrace1.win_height = (float)height_new;
	g_RayTrace2.win_width = (float)width_new;
	g_RayTrace2.win_height = (float)height_new;
	g_RayTrace3.win_width = (float)width_new;
	g_RayTrace3.win_height = (float)height_new;
	g_RayTrace4.win_width = (float)width_new;
	g_RayTrace4.win_height = (float)height_new;
}

void menufunc(int value)
{
	switch (value)
	{
	case 0:
		// Start the Ray Tracing
		g_bRayTrace = true;
		g_bRenderNormal = false;
		glutPostRedisplay ();
		break;
	case 1:
		// Render Normal
		g_bRayTrace = false;
		g_X = 0;
		g_Y = 0;
		g_bRenderNormal = true;
		glutPostRedisplay ();
		break;
	case 2:
		scene_id = 1;
		glutPostRedisplay ();
		break;
	case 3:
		scene_id = 2;
		glutPostRedisplay ();
		break;
	case 4:
		scene_id = 3;
		glutPostRedisplay ();
		break;
	case 5:
		scene_id = 4;
		glutPostRedisplay ();
		break;
	case 6:
		// Quit Program
		exit(0);
		break;
	}
}

void mousebutton(int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		g_iLeftMouseButton = (state==GLUT_DOWN);
		break;
	case GLUT_MIDDLE_BUTTON:
		g_iMiddleMouseButton = (state==GLUT_DOWN);
		break;
	case GLUT_RIGHT_BUTTON:
		g_iRightMouseButton = (state==GLUT_DOWN);
		break;
	}

	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

int main (int argc, char ** argv)
{

	//You will be creating a menu to load in scenes
	//The test.xml is the default scene and you will modify this code
	if (!g_RayTrace1.m_Scene.Load ("scene1.xml"))
	{
		printf ("failed to load scene: 1\n");
		exit(1);
	}
	if (!g_RayTrace2.m_Scene.Load ("scene2.xml"))
	{
		printf ("failed to load scene: 2\n");
		exit(1);
	}
	if (!g_RayTrace3.m_Scene.Load ("scene3.xml"))
	{
		printf ("failed to load scene: 3\n");
		exit(1);
	}
	if (!g_RayTrace4.m_Scene.Load ("scene4.xml"))
	{
		printf ("failed to load scene: 4\n");
		exit(1);
	}
	glutInit(&argc,argv);

	/* create a window */
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	glutCreateWindow("Assignment 5 - Ray Tracer");

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);

	/* create a right mouse button menu */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Render RayTrace",0);
	glutAddMenuEntry("Render Normal",1);
	glutAddMenuEntry("Scene 1: Normal",2);
	glutAddMenuEntry("Scene 2: Two Lights",3);
	glutAddMenuEntry("Scene 3: Soft Shadow",4);
	glutAddMenuEntry("Scene 4: Refraction",5);
	glutAddMenuEntry("Quit",6);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);

	/* created a callback routine for window reshape*/
	glutReshapeFunc(reshape);
	
	/* do initialization */
	myinit();

	glutMainLoop();
	return 0;
}
