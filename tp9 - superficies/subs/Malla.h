#ifndef MALLA_H
#define MALLA_H

#include <algorithm>
#include <map>
#include <vector>
#include <cmath>
using namespace std;

// el Punto tiene un arreglo con 3 coordenadas (x,y,z)
struct Punto {
  float x[3]; // coordenadas
  Punto(float ax=0, float ay=0, float az=0) {
    x[0]=ax; x[1]=ay; x[2]=az;
  }
  Punto &operator=(const Punto &p2) {
    x[0]=p2.x[0]; x[1]=p2.x[1]; x[2]=p2.x[2];
    return *this;
  }
  bool operator==(const Punto &p2) {
    return (x[0]==p2.x[0] && x[1]==p2.x[1] && x[2]==p2.x[2]);
  }
  Punto operator+(const Punto &p2) {
    return Punto(x[0]+p2.x[0],x[1]+p2.x[1],x[2]+p2.x[2]);
  }
  Punto &operator+=(const Punto &p2) {
    x[0]+=p2.x[0]; x[1]+=p2.x[1]; x[2]+=p2.x[2]; return *this;
  }
  Punto operator-(const Punto &p2) {
    return Punto(x[0]-p2.x[0],x[1]-p2.x[1],x[2]-p2.x[2]);
  }
  Punto &operator-=(const Punto &p2) {
    x[0]-=p2.x[0]; x[1]-=p2.x[1]; x[2]-=p2.x[2]; return *this;
  }
  Punto &operator*=(float n) {
    x[0]*=n; x[1]*=n; x[2]*=n; return *this;
  }
  Punto operator*(float n) {
    return Punto(x[0]*n,x[1]*n,x[2]*n);
  }
  Punto &operator/=(float n) {
    x[0]/=n; x[1]/=n; x[2]/=n; return *this;
  }
  Punto operator/(float n) {
    return Punto(x[0]/n,x[1]/n,x[2]/n);
  }
  Punto operator%(const Punto &n2) { // producto vectorial
    return Punto(
      x[1]*n2.x[2]-x[2]*n2.x[1],
      x[2]*n2.x[0]-x[0]*n2.x[2],
      x[0]*n2.x[1]-x[1]*n2.x[0]
      );
  }
  float mod() {return sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);}
  Punto &zero() {
    x[0]=0; x[1]=0; x[2]=0; return *this;
  }
};

// Nodo o vértice: punto mas datos para usar en una malla
struct Nodo : public Punto{
  bool es_frontera;
  vector<int> e; // en que elementos esta este Punto
  Nodo(float ax=0, float ay=0, float az=0) {
    x[0]=ax; x[1]=ay; x[2]=az;
    es_frontera=false;
  }
  Nodo(const Punto &p2) {
    x[0]=p2.x[0]; x[1]=p2.x[1]; x[2]=p2.x[2];
    es_frontera=false;
  }
  Nodo &operator=(const Punto &p2) { // para que no cambie es_frontera ni e
    x[0]=p2.x[0]; x[1]=p2.x[1]; x[2]=p2.x[2];
    return *this;
  }
};

// El Elemento guarda los indice de los 3/4 nodos de un triangulo/cuadrilatero 
//  y los indices de los 3/4 elementos vecinos
// El orden es importante pues indica la orientacion
// Notar que no conoce las posiciones, solo indices en listas de una malla que tampoco conoce
struct Elemento {
  unsigned int nv; // cantidad de vertices 3 o 4, me dice que tipo de elemento es
  int n[4]; // los tres/cuatro nodos
  int v[4]; // los tres/cuatro elementos vecinos por arista (-1 indica arista de frontera)
  Elemento(int n0=0, int n1=0, int n2=0, int n3=-1) {
    n[0]=n0; n[1]=n1; n[2]=n2; n[3]=n3; nv=n3<0?3:4;
  }
  void SetNodos(int n0=0, int n1=0, int n2=0, int n3=-1) {
    n[0]=n0; n[1]=n1; n[2]=n2; n[3]=n3; nv=n3<0?3:4;
  }
  inline int &operator[](int i) { 
    return n[(i+nv)%nv]; // para poer ciclar (anterior, posterior, diagonal)
  }
  bool Tiene(int i) { // pregunta si el Elemento tiene al nodo i
    return n[0]==i||n[1]==i||n[2]==i||(nv==4&&n[3]==i);
  }
  int Indice(int i) { // devuelve en que posicion el elemento tiene al nodo i (o -1, si no)
    int j=nv-1; while (j>=0 && n[j]!=i) j--; return j;
  }
};

// La struct Arista guarda los dos indices de nodos de una arista
// Siempre pone primero el menor indice, para facilitar la búsqueda en lista ordenada;
//    es para usar con el Mapa de más abajo, para asociar un nodo nuevo a una arista vieja
struct Arista {
  int n[2];
  Arista(int n1, int n2) {
    n[0]=n1; n[1]=n2;
    if (n[0]>n[1]) swap(n[0],n[1]);
  }
  Arista(Elemento &e, int i) { // i-esima arista de un elemento
    n[0]=e[i]; n[1]=e[i+1];
    if (n[0]>n[1]) swap(n[0],n[1]); // pierde el orden del elemento
  }
  const bool operator<(const Arista &a) const {
    return (n[0]<a.n[0]||(n[0]==a.n[0]&&n[1]<a.n[1]));
  }
};

// Mapa sirve para guardar una asociación entre una arista y un indice de nodo (que no es de la arista)
typedef map<Arista,int> Mapa;

// Malla guarda principalmente una lista de nodos y elementos
struct Malla {
  vector<Nodo> p;
  vector<Elemento> e;
  vector<Punto> normal; // solo para iluminacion
  Malla(const char *fname=NULL);
  void Load(const char *fname);
  void Save(const char *fname);
  void MakeVecinos();
  void MakeNormales();
  void AgregarElemento(int n0, int n1, int n2, int n3=-1);
  void ReemplazarElemento(int ie, int n0, int n1, int n2, int n3=-1);
  void Draw(bool relleno=true);
  void Subdivide();
//  const vector<int>& NodosVecinos(unsigned int in);
};

#endif

