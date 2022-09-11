//modified by: Jason Rodriguez
//date: 8/31/2022
//
//
//author: Gordon Griesel
//date: Spring 2022
//purpose: get openGL working on your personal computer
//
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

//constants
const int MAX_PARTICLES = 1000;
const int MAX_BOXES = 5;
//some structures


class Global {
public:
	int xres, yres;
	Global();
} g;
class Circle {
public: 

	float radius;
	float theta;
	float pi;
	float pos[2];
	Circle() {
	
		radius = 220.0f;
		pi = 3.14159f;
		pos[0] = g.xres * 3.25f;
		pos[1] = g.yres / 2.75f;
	}
}circle;
class Box {
public:
	float w,l;
	float dir;
	float vel[2];
	float pos[2];
	Box() {

		w = 20.0f;
		l = 40.0f;
		dir = 25.0f; 
		pos[0] = g.xres / 6.0f;
		pos[1] = g.yres / 0.5f;
		vel[0] = vel[1] = 0.0;

	}
	Box(float wid, float d, float p0, float p1) {
	 	w = wid;
	 	dir = d;
	 	pos[0] = p0;
	 	pos[1] = p1;
		vel[0] = vel[1] = 0.0;
	}
} box, particle(2.0,0.0,g.xres/2.0,g.yres/4.0*3.0);

Box particles[MAX_PARTICLES];
Box boxes[MAX_BOXES];
int n = 0;
int m = 0;

class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	~X11_wrapper();
	X11_wrapper();
	void set_title();
	bool getXPending();
	XEvent getXNextEvent();
	void swapBuffers();
	void reshape_window(int width, int height);
	void check_resize(XEvent *e);
	void check_mouse(XEvent *e);
	int check_keys(XEvent *e);
} x11;

//Function prototypes
void init_opengl(void);
void physics(void);
void render(void);



//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
	init_opengl();
	//Main loop
	int done = 0;
	while (!done) {
		//Process external events.
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			x11.check_mouse(&e);
			done = x11.check_keys(&e);
		}
		physics();
		render();
		x11.swapBuffers();
		usleep(200);
	}
	return 0;
}

Global::Global()
{
	xres = 400; 
	yres = 200; 
}

X11_wrapper::~X11_wrapper()
{
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w = g.xres, h = g.yres;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		cout << "\n\tcannot connect to X server\n" << endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		cout << "\n\tno appropriate visual found\n" << endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "3350 Lab2");
}

bool X11_wrapper::getXPending()
{
	//See if there are pending events.
	return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
	//Get a pending event.
	XEvent e;
	XNextEvent(dpy, &e);
	return e;
}

void X11_wrapper::swapBuffers()
{
	glXSwapBuffers(dpy, win);
}

void X11_wrapper::reshape_window(int width, int height)
{
	//window has been resized.
	g.xres = width;
	g.yres = height;
	//
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
}

void X11_wrapper::check_resize(XEvent *e)
{
	//The ConfigureNotify is sent by the
	//server if the window is resized.
	if (e->type != ConfigureNotify)
		return;
	XConfigureEvent xce = e->xconfigure;
	if (xce.width != g.xres || xce.height != g.yres) {
		//Window size did change.
		reshape_window(xce.width, xce.height);
	}
}
//-----------------------------------------------------------------------------

void make_particle(int x, int y)
{
	if ( n >= MAX_PARTICLES)
	    return;
    	particles[n].w = 2.0;
	particles[n].pos[0] = x;
	particles[n].pos[1] = y;
	particles[n].vel[0] = particles[n].vel[1] = 0.0;
	n++;
	
}
void make_boxes(int max) {
	float x = 0.25; 
	float y = 4;
	while ( m != max ) {
		boxes[m].w = 20.0;
		boxes[m].l = 100.0;
		boxes[m].pos[0] = g.xres*x; 
		boxes[m].pos[1] = g.yres*y;
		boxes[m].vel[0] = boxes[m].vel[1] = 0.0;
		cout << "box number " << m << ": position (" << boxes[m].pos[0] 
             << ", " << boxes[m].pos[1] << ") and velocity : " << boxes[m].vel[0] << endl;
		cout << "repeating " << endl;
		x = x + 0.50f;
		y = y - 0.5f;
		m++;	
	}
}
void X11_wrapper::check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	//Weed out non-mouse events
	if (e->type != ButtonRelease &&
		e->type != ButtonPress &&
		e->type != MotionNotify) {
		//This is not a mouse event that we care about.
		return;
	}
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed.
			int y = g.yres - e->xbutton.y;
			int x = e ->xbutton.x;
			make_particle(x,y);
		    	return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed.
			return;
		}
	}
	if (e->type == MotionNotify) {
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			savex = e->xbutton.x;
			savey = e->xbutton.y;
			//Code placed here will execute whenever the mouse moves.


		}
	}
}

int X11_wrapper::check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_1:
				//Key 1 was pressed
				break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
		}
	}
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	//Allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void physics()
{
	particle.vel[1] -= 0.01;
    	particle.pos[0] += particle.vel[0];
	particle.pos[1] += particle.vel[1];
	//
	// check for collision
	if (particle.pos[1] < (box.pos[1] + box.w) && 
	    particle.pos[1] > (box.pos[1] - box.w) &&
	    particle.pos[0] > (box.pos[0] - box.w) &&
	    particle.pos[0] < (box.pos[0] + box.w)) { 
	    particle.vel[1] = 0.0;
	    particle.vel[0] += 0.01;
	}
	for (int i=0; i<n; i++ ) {
		particles[i].vel[1] -= 0.75;
    		particles[i].pos[0] += particles[i].vel[0];
		particles[i].pos[1] += particles[i].vel[1];
		//
		// check for collision
		for (int j=0; j<MAX_BOXES; j++) {
			if (particles[i].pos[1] < (boxes[j].pos[1] + boxes[j].w) && 
	       	    	particles[i].pos[1] > (boxes[j].pos[1] - boxes[j].w) &&
	     	    	particles[i].pos[0] > (boxes[j].pos[0] - boxes[j].l) &&
	    	    	particles[i].pos[0] < (boxes[j].pos[0] + boxes[j].l)) { 
	            	particles[i].vel[1] = 1;
	            	particles[i].vel[0] += 0.15;
			}
			
			if (particles[i].pos[1] < 0.0 ) {
		    	particles[i] = particles[--n];
			}
		}
	}
}

void render()
{
	Rect r;
	glClear(GL_COLOR_BUFFER_BIT);

	//
  	//Draw boxes
	make_boxes(MAX_BOXES);
	for ( int i = 0; i < MAX_BOXES; i++ ) {	
		glPushMatrix();
		glColor3ub(80, 160, 80);
		glTranslatef(boxes[i].pos[0], boxes[i].pos[1], 0.0f);
		glBegin(GL_QUADS);
			glVertex2f(-boxes[i].l, -boxes[i].w);
			glVertex2f(-boxes[i].l,  boxes[i].w);
			glVertex2f( boxes[i].l,  boxes[i].w);
			glVertex2f( boxes[i].l, -boxes[i].w);
		glEnd();
		glPopMatrix();
	}
	//
	//Print Words
	r.bot = g.yres - 20;
	r.left = 10;
	r.center = 0;
	ggprint8b(&r, 20, 0x00ff0000, " Waterfall Model ");
	ggprint8b(&r, 20, 0x00ff0000, " GO FULLSCREEN ");
	r.bot = g.yres * 0.80;
	r.left = 50;
	r.center = 0;
	ggprint8b(&r, 150, 0x00ffff00, " REQUIREMENTS ");
	r.bot = g.yres * 0.70;
	r.left = 270;
	r.center = 0;
	ggprint8b(&r, 16, 0x00ffff00, " DESIGN ");
	r.bot = g.yres * 0.60;
	r.left = 470;
	r.center = 0;
	ggprint8b(&r, 16, 0x00ffff00, "CODING ");
	r.bot = g.yres * 0.50;
	r.left = 670;
	r.center = 0;
	ggprint8b(&r, 16, 0x00ffff00, " TESTING ");
	r.bot = g.yres * 0.40;
	r.left = 860;
	r.center = 0;
	ggprint8b(&r, 16, 0x00ffff00, " MAINTENANCE ");
	//
	//Draw Circle
	glPushMatrix();
	glColor3ub(80, 160, 80);
	glBegin(GL_POLYGON);
	for (int i=0; i<360; i++ ) {
		circle.theta = i*circle.pi/180;
		glVertex2f(circle.pos[0] + circle.radius * cos(circle.theta),
                   circle.pos[1] + circle.radius * sin(circle.theta));	
	}	
	glEnd();
	glPopMatrix();
	
	//
	//Draw particle.
	glPushMatrix();
	glColor3ub(50,60, 255);
	glTranslatef(particle.pos[0], particle.pos[1], 0.0f);
	glBegin(GL_QUADS);
		glVertex2f(-particle.w, -particle.w);
		glVertex2f(-particle.w,  particle.w);
		glVertex2f( particle.w,  particle.w);
		glVertex2f( particle.w, -particle.w);
	glEnd();
	glPopMatrix();
	
	//Draw particles.
	for (int i=0; i<n ; i++ ) {
		glPushMatrix();
		glColor3ub(150,160, 255);
		glTranslatef(particles[i].pos[0], particles[i].pos[1], 0.0f);
		glBegin(GL_QUADS);
			glVertex2f(-particles[i].w, -particles[i].w);
			glVertex2f(-particles[i].w,  particles[i].w);
			glVertex2f( particles[i].w,  particles[i].w);
			glVertex2f( particles[i].w, -particles[i].w);
		glEnd();
		glPopMatrix();
	}
}






