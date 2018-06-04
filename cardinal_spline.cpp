/*
================================================================================================================================
 Cardinal Spline Interpolation and Drawing with OpenGL

 This program allows a user to select curve control points via mouse click
 in a window. The points are used to calculate a sequence of cardinal spline
 interpolation vertices. The cardinal spline is rendered by drawing straight
 lines between consecutive vertices.

 Roman Smirnov

 03/06/2018
================================================================================================================================
*/



/*-------- includes --------*/
#include <vector>
#include <cmath>
using std::vector;
using std::begin;
using std::end;

/* macos uses a different header than linux/owindows */
#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif


/*-------- constants --------*/
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const char *WINDOW_TITLE = "Cardinal Splines - Roman Smirnov ";
const int DEFAULT_2D_ZNEAR = -1;
const int DEFAULT_2D_ZFAR = 1;
const GLfloat CTRL_POINT_SIZE = 10.0f;
const GLfloat LINE_WIDTH = 5.0f;
const int NUM_OF_SEGMENTS = 100; // between each two consecutive control points
const double TENSION = 0.5; // controls the amount of curviness

/*-------- global vars --------*/
vector<vector<GLdouble>> controlVertices; // holds the control points


/*
================================================================================================
 Calculations
================================================================================================
*/


/*
----------------------------------------------------------------
 interpolates segment points on curve between vert1 and vert2
----------------------------------------------------------------
*/
vector<vector<GLdouble>>
interpVerts(vector<GLdouble> &vert0, vector<GLdouble> &vert1, size_t numSegments, vector<GLdouble> &tangs) {

	/* init the output vector */
	vector<vector<GLdouble>> curveVerts;
	curveVerts.reserve(numSegments);

	/* interpolate points on curve path from vert0 to vert1 */
	for (int i = 0; i < numSegments; ++i) {
		double u = (double) i / numSegments;

		/* calc the polynomial coefficients */
		auto h0 = 2 * pow(u, 3) - 3 * pow(u, 2) + 1;
		auto h1 = -2 * pow(u, 3) + 3 * pow(u, 2);
		auto h2 = pow(u, 3) - 2 * pow(u, 2) + u;
		auto h3 = pow(u, 3) - pow(u, 2);

		/* calc interpolated point */
		auto vx = h0 * vert0[0] + h1 * vert1[0] + h2 * tangs[0] + h3 * tangs[2];
		auto vy = h0 * vert0[1] + h1 * vert1[1] + h2 * tangs[1] + h3 * tangs[3];

		curveVerts.push_back(vector<GLdouble>{vx, vy});
	}

	return curveVerts;
}


/*
----------------------------------------------------------------
 calculates the tangentials (i.e sort of the slope/derivative) of v1,v2 endpoints
----------------------------------------------------------------
*/
vector<GLdouble>
calcTangs(vector<GLdouble> &v0, vector<GLdouble> &v1, vector<GLdouble> &v2, vector<GLdouble> &v3, double t) {
	/* calc the tangentials at end points */
	vector<GLdouble> tangs{t * (v2[0] - v0[0]), t * (v2[1] - v0[1]), t * (v3[0] - v1[0]), t * (v3[1] - v1[1])};
	return tangs;
}


/*
----------------------------------------------------------------
 calculates and returns a vector of all interpolated spline points
----------------------------------------------------------------
*/
vector<vector<GLdouble>> cardinalSpline(vector<vector<GLdouble>> &ctrlVrts, size_t numSegments, double tension) {

	auto numVrts = ctrlVrts.size();

	/* there can't be a spline curve between fewer than 3 points (between 2 it's just a line ) */
	if (numVrts < 3) {
		return ctrlVrts;
	}

	/* init the output vector */
	vector<vector<GLdouble>> splineVerts;
	splineVerts.reserve(numVrts * numSegments);

	/* f prefix - calc spline path points between first 2 control points */
	auto fTangs = calcTangs(ctrlVrts[numVrts - 1], ctrlVrts[0], ctrlVrts[1], ctrlVrts[2], tension);
	auto fInterpVerts = interpVerts(ctrlVrts[0], ctrlVrts[1], numSegments, fTangs);
	splineVerts.insert(end(splineVerts), begin(fInterpVerts), end(fInterpVerts));

	/* calc spline path points */
	for (int i = 0; i < numVrts - 3; ++i) {

		/* calc the tangentials at end points */
		auto tangs = calcTangs(ctrlVrts[i], ctrlVrts[i + 1], ctrlVrts[i + 2], ctrlVrts[i + 3], tension);

		/* calc the interpolated segment vertices between two control points */
		auto interpolatedVerts = interpVerts(ctrlVrts[i + 1], ctrlVrts[i + 2], numSegments, tangs);
		splineVerts.insert(end(splineVerts), begin(interpolatedVerts), end(interpolatedVerts));
	}

	/* l suffix - calc spline path points between last 2 control points */
	auto lTangs = calcTangs(ctrlVrts[numVrts - 3], ctrlVrts[numVrts - 2], ctrlVrts[numVrts - 1], ctrlVrts[0], tension);
	auto lInterpVerts = interpVerts(ctrlVrts[numVrts - 2], ctrlVrts[numVrts - 1], numSegments, lTangs);
	splineVerts.insert(end(splineVerts), begin(lInterpVerts), end(lInterpVerts));

	return splineVerts;
}


/*
================================================================================================
Drawing and OpenGL callbacks 
================================================================================================
*/



/*
----------------------------------------------------------------
 GLUT display function - it's where we draw
----------------------------------------------------------------
*/
void handleDisplay() {

	glClear(GL_COLOR_BUFFER_BIT);

	/* draw the vertices */
	glPointSize(CTRL_POINT_SIZE);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	for (auto &vert : controlVertices) {
		glVertex2dv(&vert[0]);
	}
	glEnd();

	/* calculate all spline curve points */
	auto spline = cardinalSpline(controlVertices, NUM_OF_SEGMENTS, TENSION);

	/* draw the spline */
	glLineWidth(LINE_WIDTH);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_STRIP);
	for (auto &vert : spline) {
		glVertex2d(vert[0], vert[1]);
	}
	glEnd();

	glutSwapBuffers();
}


/*
----------------------------------------------------------------
 GLUT reshape function - handles window reshape and resize
----------------------------------------------------------------
*/
void handleReshape(int w, int h) {
	/* setup viewport and projection */
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (double) w, (double) WINDOW_WIDTH - h, (double) WINDOW_HEIGHT, DEFAULT_2D_ZNEAR, DEFAULT_2D_ZFAR);
	glMatrixMode(GL_MODELVIEW);
}



/*
----------------------------------------------------------------
  GLUT mouse function - handles mouse clicks
----------------------------------------------------------------
*/
void handleMouse(int button, int state, int x, int y) {

	/* we only handle left mouse clicks on release */
	if (button != GLUT_LEFT_BUTTON or state != GLUT_UP) {
		return;
	}

	int mousex = x;
	int mousey = WINDOW_WIDTH - y;

	/* create a vertex and add to vertex vector */
	controlVertices.emplace_back(vector<GLdouble>(2));
	controlVertices[controlVertices.size() - 1][0] = mousex;
	controlVertices[controlVertices.size() - 1][1] = mousey;

	/* redraw screen */
	glutPostRedisplay();
}


/*
----------------------------------------------------------------
 initialization function
----------------------------------------------------------------
*/
void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0); // black
}


/*
----------------------------------------------------------------
  main function
----------------------------------------------------------------
*/
int main(int argc, char **argv) {

	/* init opengl and glut */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	/* create and setup a window */
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(50, 50);
	glutCreateWindow(WINDOW_TITLE);

	/* setup framework callback functions */
	glutDisplayFunc(handleDisplay);
	glutReshapeFunc(handleReshape);
	glutMouseFunc(handleMouse);

	/* handle program specific initializations here */
	init();

	/* start draw loop */
	glutMainLoop();

	return 0;
}