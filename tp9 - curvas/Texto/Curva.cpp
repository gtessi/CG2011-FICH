#include "Curva.h"
#include <GL/gl.h>


Spline::Spline () {
	np=0;
}

void Spline::Recalc (int i) {
	if (i==0) { // si es el primer tramo...
		if (np==2) { // si hay un solo tramo, es una recta
			punto d=p[3]-p[0];
			p[1]=p[0]+d/3;
			p[2]=p[0]+2*d/3;
		} else { // primer tramo, se construye grado 2 y despues se eleva a 3
			p[1]=p[3]-(p[6]-p[0])/4;
			ElevaGrado(0);
		}
	} else if (i==np-2) { // ultimo tramo, tambien se construye grado 2 y se eleva
		p[3*i+1]=p[3*i]+(p[3*i+3]-p[3*i-3])/4;
		ElevaGrado(np-2);
	} else {
		punto d1=p[3*(i+1)]-p[3*(i-1)];
		punto d2=p[3*(i+2)]-p[3*i];
		p[3*i+1]=p[3*i]+d1/6;
		p[3*i+2]=p[3*i+3]-d2/6;
	}
	l[i]=Longitud(i);
}

int Spline::Agregar (int i, punto x) {
	if (np>MAX_TRAMOS) return -1;
	memmove(p+3*i,p+3*(i+1),(np-i)*3*sizeof(punto));
	p[3*i]=x; np++;
	Recalc(i-2,i);
	Reparametrizar();
	return i;
}

int Spline::Agregar (punto x) {
	return Agregar(np,x);
}

void Spline::Mover (int i, punto x) {
	p[3*i]=x;
	Recalc(i-2,i+2);
	Reparametrizar();
}

void Spline::Quitar (int i) {
	memmove(p+3*(i+1),p+3*i,(np-i-1)*3*sizeof(punto));
	np--;
	if (i!=0) Recalc(i-1); // tramo anterior al punto
	if (i+1!=np) Recalc(i); // tramo posterior al punto
}

int Spline::Cerca (punto x, float tol2) {
	for (int i=0;i<np;i++) {
		if ((p[i*3]-x).mod2()<tol2) return i;
	}
	return -1;
}

void Spline::Evaluar (int i, float ti, punto & x, punto & d) {
	punto &p0=p[3*i], &p1=p[3*i+1], &p2=p[3*i+2], &p3=p[3*i+3];
	float um_ti=1-ti;
	punto p01=um_ti*p0+ti*p1;
	punto p12=um_ti*p1+ti*p2;
	punto p23=um_ti*p2+ti*p3;
	punto p012=um_ti*p01+ti*p12;
	punto p123=um_ti*p12+ti*p23;
	d=(p123-p012)/3;
	x=um_ti*p012+ti*p123;
}

punto Spline::Evaluar (int i, float ti) {
	punto &p0=p[3*i], &p1=p[3*i+1], &p2=p[3*i+2], &p3=p[3*i+3];
	float um_ti=1-ti;
	punto p01=um_ti*p0+ti*p1;
	punto p12=um_ti*p1+ti*p2;
	punto p23=um_ti*p2+ti*p3;
	punto p012=um_ti*p01+ti*p12;
	punto p123=um_ti*p12+ti*p23;
	return um_ti*p012+ti*p123;
}

void Spline::Evaluar (float ti, punto & x, punto & d) {
	// busqueda binaria para ver en que tramo cae
	ti*=t[np-1];
	int i0=0, iN=np-1, im;
	while (i0+1<iN) {
		im=(i0+iN)/2;
		if (t[im]>ti)
			iN=im;
		else
			i0=im;
	}
	Evaluar(i0,(ti-t[i0])/l[i0],x,d);
}

punto Spline::Evaluar (float ti) {
	// busqueda binaria para ver en que tramo cae
	ti*=t[np-1];
	int i0=0, iN=np-1, im;
	while (i0+1<iN) {
		im=(i0+iN)/2;
		if (t[im]>ti)
			iN=im;
		else
			i0=im;
	}
	return Evaluar(i0,(ti-t[i0])/l[i0]);
}

punto & Spline::operator[] (int i) {
	return p[3*i];
}

float Spline::Longitud (int i) {
	// esto es truchísimo, no se mide asi la longitud de arco, pero da un resultado sensato solo para el practico
	float a=.5,b=.5; int i3=i*3;
	return a*(p[i3+3]-p[i3]).mod()+b*((p[i3+1]-p[i3]).mod()+(p[i3+2]-p[i3+1]).mod()+(p[i3+3]-p[i3+2]).mod());
}


void Spline::ElevaGrado (int i) {
	punto d1=(p[3*i+1]-p[3*i])/2;
	punto d2=(p[3*i+1]-p[3*i+3])/2;
	p[3*i+1]=p[3*i]+d1;
	p[3*i+2]=p[3*i+3]+d2;
}

void Spline::Dibujar (int detail) {
	glLineWidth(1); glColor4f(0,0,.5,1);
	
	glEnable(GL_MAP1_VERTEX_4);
	for (int i=0;i<np-1;i++){
		glMap1f(GL_MAP1_VERTEX_4, 0.0, 1.0, 4, 4, (const float*)(&p[3*i]));
		glMapGrid1f(detail,0,1); glEvalMesh1(GL_LINE,0,detail);
	}
	
	glPointSize(3); glColor4f(.5,0,0,.5);
	glBegin(GL_POINTS);
		for (int i=3*np;i>=0;i--)
			glVertex2fv(p[i].c);
	glEnd();
	glPointSize(5); glColor3f(.5,0,0);
	glBegin(GL_POINTS);
		for (int i=0;i<np;i++)
			glVertex2fv(p[i*3].c);
	glEnd();
	
//	glPointSize(4); glColor3f(0,.5,0);
//	glBegin(GL_POINTS);
//		punto x,d;
//		for (float t=0;t<=1;t+=0.025) {
//			Evaluar(t,x,d);
//			glVertex2fv(x.c);
//		}
//	glEnd();
}

float Spline::MaxT ( ) {
	if (!np) return 0;
	return t[np];
}

void Spline::Recalc (int i1, int i2) {
	if (i1<0) i1=0;
	if (i2>np-1) i2=np-1;
	while (i1<i2) Recalc(i1++);
}

void Spline::Reparametrizar() {
	t[0]=0;
	for (int i=0;i<np-1;i++)
		t[i+1]=t[i]+l[i];
}

float Spline::Longitud ( ) {
	return t[np-1];
}

int Spline::CantPuntos ( ) {
	return np;
}

