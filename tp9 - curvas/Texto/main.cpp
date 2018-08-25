#include <GL/glut.h>
#include "Curva.h"
#include <cmath>
#include <iostream>
#include <cctype>
#include "uglyfont.h"
using namespace std;

Spline curva;
int win_w=600,win_h=400, sel=-1;

// dibuja un caracter en un cuadrado de 1x1 con su vertice inferior iquierdo en 0,0
#define G2R(rad) ((rad)*180/M_PI)

char texto[256];
int texto_len=strlen(texto);
bool ver_curva=true, ver_texto=true, edit_mode=false, use_deriv=false;

void dibujar_caracter(const char chr);

void reshape_cb (int w, int h) {
	win_w=w; win_h=h;
	if (w==0||h==0) return;
	glViewport(0,0,w,h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D(0,w,0,h);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

void DibujarTexto() {

	if (!use_deriv) {
		// versión sin derivadas
		/// @@@ Analizar que hace y cuales son los problemas
		glColor3f(0,.5,0); glLineWidth(2);
		float f = 1.f/texto_len;
		for (int i=0;i<texto_len;i++) {
			glPushMatrix();
			punto x1 = curva.Evaluar(i*f);
			punto x2 = curva.Evaluar((i+1)*f);
			punto d=x2-x1; float m=d.mod();
			glTranslatef(x1[0],x1[1],0);
			glRotatef(G2R(atan2(d[1],d[0])),0,0,1);
			glScalef(m/100,m/100,1);
			glTranslatef(0,-50,0);
			dibujar_caracter(texto[i]);
			glPopMatrix();
		}
	} else {
		// con derivadas, apoyar las letras sobre puntos de la curva con la
		// direccion que da la derivada. El tamaño será fijo y sale de dividir
		// la longitud de arco de la curva (que es una aproximación muy tosca,
		// pensar, sin implementar, como podría mejorarse el calculo de la 
		// longitud de arco)
		/// @@@ Implementar esta version
		glColor3f(0,.5,0); glLineWidth(2);
		float lc = curva.Longitud()/texto_len; // calcula la longitud de arco (aprox.)
		float u = 1.f/texto_len; // calcula el parámetro u para interpolar
		for (int i=0;i<texto_len;i++) {
			glPushMatrix();
			punto x, d; // define dos puntos, x y d
			// busqueda binaria para ver en que tramo cae
			// u*(i+0.5) acomoda mejor la letra sobre la curva
			curva.Evaluar(u*(i+0.5),x,d); 
			glTranslatef(x[0],x[1],0);
			glRotatef(G2R(atan2(d[1],d[0])),0,0,1); // roto las letras sobre la curva
			glScalef(lc/100,lc/100,1); // escalo las letras
			glTranslatef(-50,-50,0);
			dibujar_caracter(texto[i]);
			glPopMatrix();
		}
	}
	
}

void display_cb() {
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	if (edit_mode) {
		glColor3f(1,0,0);
		glLineWidth(1);
		glPushMatrix();
		glTranslatef(10,10,0);
		glScalef(10,10,10);
		YsDrawUglyFont("edit mode",0,0);
		glPopMatrix();
	}
	
	if (curva.CantPuntos()) {
		if (ver_curva) curva.Dibujar(10);
		if (ver_texto && texto_len) DibujarTexto();
	}
	
	glutSwapBuffers();
}

void motion_cb(int x, int y) {
	y=win_h-y;
	curva.Mover(sel,punto(x,y));
	glutPostRedisplay();
}

void mouse_cb(int button, int state, int x, int y) {
	y=win_h-y;
	if (button==GLUT_LEFT_BUTTON) {
		if (state==GLUT_DOWN) {
			sel=curva.Cerca(punto(x,y),100);
			if (sel==-1) {
				sel=curva.Agregar(punto(x,y));
				if (sel==-1) return;
				glutPostRedisplay();
			} 
			glutMotionFunc(motion_cb);
		} else 
			glutMotionFunc(NULL);
	}
}

void keyboard_cb(unsigned char key, int x, int y) {
	if (edit_mode) {
		if (key=='\b') {
			if (texto_len) texto[--texto_len]='\0';
		} else if (key<32) edit_mode=false;
		else if (texto_len<255) {
			texto[texto_len++]=key;
			texto[texto_len]='\0';
		}
	} else {
		key=tolower(key);
		if (key=='e') edit_mode=true;
		else if (key=='t') ver_texto=!ver_texto;
		else if (key=='c') ver_curva=!ver_curva;
		else if (key=='d') use_deriv=!use_deriv;
	}
	glutPostRedisplay();
}

void menu_cb(int key) {
	if (key=='e') edit_mode=!edit_mode;
	else if (key=='t') ver_texto=!ver_texto;
	else if (key=='c') ver_curva=!ver_curva;
	else if (key=='d') use_deriv=!use_deriv;
	glutPostRedisplay();
}

void initialize() {
	glutInitDisplayMode (GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize (win_w,win_h);
	glutInitWindowPosition (100,100);
	glutCreateWindow ("Ventana OpenGL");
	glutDisplayFunc (display_cb);
	glutReshapeFunc (reshape_cb);
	glutMouseFunc(mouse_cb);
	glutKeyboardFunc(keyboard_cb);
	glutCreateMenu(menu_cb);
		glutAddMenuEntry("[e] Editar Texto", 'e');
		glutAddMenuEntry("[t] Mostrar/Ocultar Texto", 't');
		glutAddMenuEntry("[c] Mostrar/Ocultar Curva", 'c');
		glutAddMenuEntry("[d] Usar/No Usar Derivadas", 'd');
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glClearColor(1.f,1.f,1.f,1.f);
}

int main (int argc, char **argv) {
	glutInit (&argc, argv);
	initialize();
	strcpy(texto,"Computacion Grafica");
	texto_len=strlen(texto);
	glutMainLoop();
	return 0;
}
