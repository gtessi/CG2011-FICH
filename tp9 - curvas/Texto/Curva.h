#ifndef CURVA_H
#include <cstring>
#include <cmath>
using namespace std;
#define CURVA_H

class punto {
	float c[4];
	friend class Spline;
public:
	punto(double x=0, double y=0, double z=0, double w=1){ c[0]=x; c[1]=y; c[2]=z; c[3]=w; }
	punto(const punto &p){ memcpy(c,p.c,4*sizeof(float)); }
	
	operator const float*() const {return c;}
	
	punto& operator=(const punto &p){memcpy(c,p.c,4*sizeof(float)); return *this;}
	
	float& operator[](int i) {return c[i];}
	const float& operator[](int i) const {return c[i];}
	
	punto& neg(){c[0]=-c[0]; c[1]=-c[1]; c[2]=-c[2]; c[3]=-c[3]; return *this;}
	
	punto operator -() const {return punto(-c[0],-c[1],-c[2],-c[3]);}
	
	punto& lerp(const punto& p0,const punto& p1,float u){
		c[0]=(1-u)*p0[0]+u*p1[0];
		c[1]=(1-u)*p0[1]+u*p1[1];
		c[2]=(1-u)*p0[2]+u*p1[2];
		c[3]=(1-u)*p0[3]+u*p1[3];
		return *this;
	}
	
	float mod() const {
		if (c[3]!=0) return sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2])/c[3]; // punto: distancia al origen
		else return sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]); // vector: modulo
	}
	float mod2() const {
		return c[0]*c[0]+c[1]*c[1]+c[2]*c[2]+c[3]*c[3];
	}
	punto& dir() {return operator/=(mod());}
	
	punto operator -(const punto &p) const{
		return punto(c[0]-p[0],c[1]-p[1],c[2]-p[2],c[3]-p[3]);
	}
	punto& operator -=(const punto &p) {
		c[0]-=p[0];c[1]-=p[1];c[2]-=p[2];c[3]-=p[3]; return *this;
	}
	punto operator +(const punto &p) const{
		return punto(c[0]+p[0],c[1]+p[1],c[2]+p[2],c[3]+p[3]);
	}
	punto& operator +=(const punto &p) {
		c[0]+=p[0];c[1]+=p[1];c[2]+=p[2];c[3]+=p[3]; return *this;
	}
	punto operator *(float f) const{
		return punto(c[0]*f,c[1]*f,c[2]*f,c[3]*f);
	}
	float operator *(punto &p) const{
		return c[0]*p.c[0]+c[1]*p.c[1]+c[2]*p.c[2]+c[3]*p.c[3];
	}
	punto &operator *=(float f) {
		c[0]*=f;c[1]*=f;c[2]*=f;c[3]*=f; return *this;
	}
	friend punto operator*(float f,const punto &p) {return (p*f);}
	punto operator /(float f) const{
		return punto(c[0]/f,c[1]/f,c[2]/f,c[3]/f);
	}
	punto &operator /=(float f) {
		c[0]/=f;c[1]/=f;c[2]/=f;c[3]/=f; return *this;
	}
	friend punto operator/(float f,const punto &p) {return (p/f);}
};


#define MAX_TRAMOS 20

class Spline {
private:
	punto p[3*MAX_TRAMOS+1]; // coordenadas de los puntos (todos, interpolados y de control)
	float l[MAX_TRAMOS]; // longitud de cada tramo
	float t[MAX_TRAMOS+1]; // valor parametro acumulado en cada punto interpolado (t[0]=0, t[np]=t_max)
	int np; // cantidad de puntos interpolados
	void ElevaGrado(int i); // para convertir las partes de grado 2 al equivalente grado 3
	void Recalc(int i1, int i2); // recalcula todos los puntos entre los puntos interpolados i1 e i2
	void Recalc(int i); // recalcula los puntos de control adicionales del i-ésimo tramo bezier
	float Longitud(int i); // devuelve la longitud aproximada de la curva de bezier que va de i a i+1
	void Reparametrizar(); // llena t en base a l ya calculado
	float MaxT(); // devuelve el valor máximo al que llega el parametro t
	punto Evaluar(int i, float ti); // evalua un tramo de bezier para un t dado y obtiene el pto
	void Evaluar(int i, float ti, punto &x, punto &d); // evalua un tramo de bezier para un t dado y obtiene el pto y su derivada
public:
	Spline();
	int Agregar(punto x); // agrega un punto más a interpolar al final de la curva
	int Agregar(int i, punto x); // agrega un punto más a interpolar antes del i-ésimo
	void Mover(int i, punto x); // mueve el punto i-ésimo a una nueva posición
	void Quitar(int i); // quita el punto i-ésimo
	int Cerca(punto x, float tol2); // busca un punto interpolado cerca de ese coordenadas, devuelve -1 si no hay
	void Dibujar(int detail); // dibuja la curva en un contexto OpenGL
	punto Evaluar(float ti); // obtiene para un t dado el punto sobre la curva (t in [0;1])
	void Evaluar(float ti, punto &x, punto &d); // obtiene para un t dado el punto sobre la curva y su derivada (t in [0;1])
	punto &operator[](int i); // devuelve uno de los puntos interpolados
	float Longitud(); // devuelve una mala aproximación de la longitud de arco
	int CantPuntos(); // devuelve la cantidad de puntos de control
};

#endif

