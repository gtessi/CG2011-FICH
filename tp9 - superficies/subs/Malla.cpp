#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include "Malla.h"
using namespace std;

void Malla::Load(const char *fname) {
  e.clear(); p.clear();
  ifstream f(fname);
  if (!f.is_open()) return;
  int i,nv;
  f>>nv;
  float x,y,z;
  for (i=0;i<nv;i++) {
    f>>x>>y>>z;
    p.push_back(Nodo(x,y,z));
  }
  int ne;
  f>>ne;
  int v0,v1,v2,v3;
  for (i=0;i<ne;i++) {
    f>>nv>>v0>>v1>>v2;
    if (nv==3) { AgregarElemento(v0,v1,v2); }
    else { f>>v3; AgregarElemento(v0,v1,v2,v3); }
  }
  f.close();
  MakeVecinos();
  MakeNormales();
}

Malla::Malla(const char *fname) {
  if (fname) Load(fname);
}

void Malla::Save(const char *fname) {
  ofstream f(fname);
  f<<p.size()<<endl;
  unsigned int i,j;
  for (i=0;i<p.size();i++)
    f<<p[i].x[0]<<' '<<p[i].x[1]<<' '<<p[i].x[2]<<endl;
  f<<e.size()<<endl;
  for (i=0;i<e.size();i++) {
    f<<e[i].nv;
    for (j=0;j<e[i].nv;j++)
      f<<' '<<e[i][j];
    f<<endl;
  }
  f.close();
}

void Malla::AgregarElemento(int n0, int n1, int n2, int n3) {
  int ie=e.size(); e.push_back(Elemento(n0,n1,n2,n3)); // agrega el Elemento
  // avisa a cada nodo que ahora es vertice de este elemento
  p[n0].e.push_back(ie); p[n1].e.push_back(ie);
  p[n2].e.push_back(ie); if (n3>=0) p[n3].e.push_back(ie);
  
}

void Malla::ReemplazarElemento(int ie, int n0, int n1, int n2, int n3) {
  Elemento &ei=e[ie];
  // estos nodos ya no seran vertices de este elemento
  for (unsigned int i=0;i<ei.nv;i++) {
    vector<int> &ve=p[ei[i]].e;
    ve.erase(find(ve.begin(),ve.end(),ie));
  }
  ei.SetNodos(n0,n1,n2,n3);
  // estos nodos ahora son vertices
  p[n0].e.push_back(ie); p[n1].e.push_back(ie); p[n2].e.push_back(ie); 
  if (n3>=0) p[n3].e.push_back(ie); 
}

void Malla::MakeNormales() {
  normal.resize(p.size());
  for (unsigned int i=0;i<p.size();i++) {// "promedio" de normales de cara
    vector<int> &en=p[i].e;
    Punto n(0,0,0); int k;
    for (unsigned int j=0;j<en.size();j++) {
      Elemento &ej=e[en[j]];
      k=ej.Indice(i);
      n+=(p[ej[k]]-p[ej[k-1]])%(p[ej[k+1]]-p[ej[k]]);
    }
    float m=n.mod(); if (m>1e-10) n/=m; else n.zero();
    normal[i]=n;
  }
}

void Malla::Draw(bool relleno) {
  // dibuja los Elementos
  unsigned int i;
  if (relleno) {
    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  } else {
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  }
  for (i=0;i<e.size();i++) {
    if (e[i].nv==4) {
      glBegin(GL_QUADS);
      glNormal3fv(normal[e[i][0]].x); glVertex3fv(p[e[i][0]].x); 
      glNormal3fv(normal[e[i][1]].x); glVertex3fv(p[e[i][1]].x); 
      glNormal3fv(normal[e[i][2]].x); glVertex3fv(p[e[i][2]].x); 
      glNormal3fv(normal[e[i][3]].x); glVertex3fv(p[e[i][3]].x); 
    } else {
      glBegin(GL_TRIANGLES);
      glNormal3fv(normal[e[i][0]].x); glVertex3fv(p[e[i][0]].x);
      glNormal3fv(normal[e[i][1]].x); glVertex3fv(p[e[i][1]].x);
      glNormal3fv(normal[e[i][2]].x); glVertex3fv(p[e[i][2]].x);
    }
    glEnd();
  }
  // dibuja solo los nodos sueltos
  glDisable(GL_LIGHTING);
  glBegin(GL_POINTS);
  for (i=0;i<p.size();i++) {
    if (p[i].e.size()==0) glVertex3fv(p[i].x);
  }
  glEnd();
}

// Identifica los pares de elementos vecinos y las aristas de frontera
// Actualiza el atributo v (lista de vecinos) de cada elemento y el atributo es_frontera de cada nodo
void Malla::MakeVecinos() {
  unsigned int i,j,k,ie,iev;
  int ix; // puede dar -1
  // inicializa
  for (i=0;i<p.size();i++) p[i].es_frontera=false; // le dice a todos los nodos que no son frontera
  for (i=0;i<e.size();i++) e[i].v[0]=e[i].v[1]=e[i].v[2]=e[i].v[3]=-1; // le dice a todos los elementos que no tienen vecinos
  // identificacion de vecinos
  for (ie=0;ie<e.size();ie++) { // por cada elemento
    for (j=0;j<e[ie].nv;j++) { // por cada arista
      if (e[ie].v[j]>=0) continue; // ya se hizo
      int in0=e[ie][j], in1=e[ie][j+1]; // 1er y 2do nodo de la arista
      for (k=0;k<p[in0].e.size();k++) { // recorro los elementos del primer nodo
        iev=p[in0].e[k];
        if (iev==ie) continue; // es este mismo
        // se fija si tiene a in1 (el 2do nodo)
        ix=e[iev].Indice(in1);
        if (ix<0) continue; 
        // tiene al 2do
        e[ie].v[j]=p[in0].e[k]; // ese es el vecino
        e[iev].v[ix]=ie;
        break; // solo dos posibles vecinos para una arista
      }
      if (k==p[in0].e.size()) // no encontro vecino ==> frontera
        p[in0].es_frontera=p[in1].es_frontera=true;
    }
  }
}

// da los nodos "vecinos" de un nodo (otros nodos que comparten con este una aristas)
//const vector<int> &Malla::NodosVecinos(unsigned int in) {
//  static vector<int> vd; vd.clear();
//  vector<int> &eni=p[in].e; // elementos del nodo i
//  for (unsigned int i=0;i<eni.size();i++) {
//    int ie=eni[i]; Elemento &q=e[ie];
//    int ii=q.Indice(in);
//    if (q.v[ii]<ie) vd.push_back(q[ii+1]);
//    if (q.v[(ii+q.nv-1)%q.nv]<ie) vd.push_back(q[(ii+q.nv-1)%q.nv]); // por si hay fronteras
//  }
//  return vd;
//}

void Malla::Subdivide() {  
  // @@@@@: Implementar Catmull-Clark... lineamientos:
  //  0) Los nodos originales van de 0 a Nn-1, los elementos orignales de 0 a Ne-1
  //  1) Por cada elemento, agregar el centroide (nuevos nodos: Nn a Nn+Ne-1)
  //  2) Por cada arista de cada cara, agregar un pto en el medio que es
  //      promedio de los vertices de la arista y los centroides de las caras 
  //      adyacentes. Aca hay que usar los elementos vecinos.
  //      En los bordes, cuando no hay vecinos, es simplemente el promedio de los 
  //      vertices de la arista
  //      Hay que evitar procesar dos veces la misma arista (como?)
  //      Mas adelante vamos a necesitar encontrar los puntos que agregamos para 
  //      cada arista, y ya que no se puede relacionar el indice de los nodos nuevos
  //      (la cantidad de aristas por nodo es variable), se sugiere usar Mapa como 
  //      estructura auxiliar
  //  3) Armar los elementos nuevos
  //      Los quads se dividen en 4, (uno reemplaza al original, los otros 3 se agregan)
  //      Los triangulos se dividen en 3, (uno reemplaza al original, los otros 2 se agregan)
  //      Para encontrar los nodos de las aristas usar el mapa que armaron en el paso 2
  //  4) Calcular las nuevas posiciones de los nodos originales
  //      Para nodos interiores: (4r-f+(n-3)p)/n
  //         f=promedio de nodos interiores de las caras (los agregados en el paso 1)
  //         r=promedio de los pts medios de las aristas (los agregados en el paso 2)
  //         p=posicion del nodo original
  //         n=cantidad de elementos para ese nodo
  //      Para nodos del borde: (r+p)/2
  //         r=promedio de los dos pts medios de las aristas
  //         p=posicion del nodo original
  //      Ojo: en el paso 3 cambio toda la malla, analizar donde quedan en los nuevos 
  //      elementos (¿de que tipo son?) los nodos de las caras y los de las aristas 
  //      que se agregaron antes.
  // tips:
  //   no es necesario cambiar ni agregar nada fuera de este método, (con Mapa como 
  //     estructura auxiliar alcanza)
  //   sugerencia: probar primero usando el cubo (es cerrado y solo tiene quads)
  //               despues usando la piramide (tambien cerrada, y solo triangulos)
  //               despues el ejemplo plano (para ver que pasa en los bordes)
  //               finalmente el mono (tiene mezcla y elementos sin vecinos)
  //   repaso de como usar un mapa:
  //     para asociar un indice (i) de nodo a una arista (n1-n2): elmapa[Arista(n1,n2)]=i;
  //     para saber si hay un indice asociado a una arista:  ¿elmapa.find(Arista(n1,n2))!=elmapa.end()?
  //     para recuperar el indice (en j) asociado a una arista: int j=elmapa[Arista(n1,n2)];
  
  int Nn=p.size(); // número de nodos original
  Mapa M; // para formar el par arista-índice k
  int k=0; // variable para calcular índices
  
  // Paso 1, agregar los centroides al vector de nodos (nuevos nodos: Nn a Nn+Ne-1)
  int Ne=e.size(); // total de elementos
  for (int ie=0;ie<Ne;ie++){ // recorro la lista de elementos
    Punto centroide(0,0,0);
    for (int j=0;j<e[ie].nv;j++) // recorro la lista de nodos de cada elemento (3 o 4)
      centroide+=p[e[ie][j]]; // suma los nodos de cada elemento
    centroide/=e[ie].nv; // hace el promedio según la cantidad de nodos
    p.push_back(centroide); // agrega el centroide a la lista de nodos
  }
    
  // Paso 2, por cada arista de cada cara, agregar un punto en el medio que es
  // promedio de los vértices de la arista y los centroides de las caras adyacentes
  for (int ie=0;ie<Ne;ie++){ // recorro la lista de elementos
    Punto promedio(0,0,0);  
    Punto n0, n1, n2, n3;
    for (int j=0;j<(e[ie].nv);j++){ // recorro la lista de nodos de cada elemento (3 o 4)      
      n0=p[e[ie][j]]; // nodo actual
      n1=p[e[ie][(j+1)%e[ie].nv]]; // nodo siguiente, corregído el límite del contador con el módulo
      Arista a(e[ie][j],e[ie][(j+1)%e[ie].nv]); // crea la arista j -- j+1
      
      if (e[ie].v[j]==-1){ // si el vecino es frontera, promedio entre los nodos
        promedio=(n0+n1)/2;
        p.push_back(promedio); // agrega el nodo promedio a la lista de nodos
        M[a]=p.size()-1; // mapea la arista con el índice p.size()-1        
        continue;
      }      
      // sino es frontera, promedio entre los nodos y los centroides
      n2=p[Nn+ie]; // centroide del elemento
      n3=p[Nn+e[ie].v[j]]; // centroide del elemento vecino
      promedio=(n0+n1+n2+n3)/4;
      // control de arista en mapa
      if (M.find(a)==M.end()){ // pregunta si la arista no está mapeada
        p.push_back(promedio); // agrega el nodo promedio a la lista de nodos
        M[a]=p.size()-1; // mapea la arista con el índice p.size()-1
      }
    }

    // Paso 3, armar los elementos nuevos    
    for (int j=1;j<(e[ie].nv);j++) // recorre la lista de elementos
      AgregarElemento(Nn+ie, // centroide
                      M[Arista(e[ie][j],e[ie][(j+1)%e[ie].nv])], // centro de arista j -- j+1
                      e[ie][(j+1)%e[ie].nv], // nodo del elemento
                      M[Arista(e[ie][(j+1)%e[ie].nv],e[ie][(j+2)%e[ie].nv])]); // centro de arista j+1 -- j+2
    // Reemplazo el elemento común (igual para quads/triángulos) 
    ReemplazarElemento(ie, // índice del elemento actual
                       Nn+ie, // centroide
                       M[Arista(e[ie][0],e[ie][1])], // centro de arista 0 -- 1
                       e[ie][1], // nodo 1
                       M[Arista(e[ie][1],e[ie][2])]); // centro de arista 1 -- 2
  }

  MakeVecinos(); // actualizo la lista de vecinos de cada elemento
  MakeNormales(); // actualizo las normales de cada elemento

  // Paso 4, mover los nodos originales
  //   Para nodos interiores: (4r-f+(n-3)p)/n
  //         f=promedio de nodos interiores de las caras (los agregados en el paso 1)
  //         r=promedio de los pts medios de las aristas (los agregados en el paso 2)
  //         p=posicion del nodo original
  //         n=cantidad de elementos para ese nodo
  //   Para nodos del borde: (r+p)/2
  //         r=promedio de los dos pts medios de las aristas
  //         p=posicion del nodo original

  for (int in=0;in<Nn;in++){ // recorro la lista de nodos originales
    vector<int> &en=p[in].e; // me devuelve la lista de elementos para ese nodo
    Nodo f(0,0,0), r(0,0,0), po=p[in]; // contadores para los promedios de los nodos
    
    if (p[in].es_frontera){ // pregunto si el nodo actual está en la frontera
      for (int j=0;j<en.size();j++){ // recorro la lista de nodos del elemento actual
        k=e[en[j]].Indice(in); // obtengo el índice del nodo actual (P) en el elemento actual
        if (p[e[en[j]][(k+1)%4]].es_frontera) // pregunto si el nodo siguiente (R+1) es frontera
          r+=p[e[en[j]][(k+1)%4]]; // si es frontera, lo sumo
        if (p[e[en[j]][(k+3)%4]].es_frontera) // pregunto si el nodo anterior (R-1=(R+3)%4) es frontera
          r+=p[e[en[j]][(k+3)%4]]; // si es frontera, lo sumo      
      }       
      r/=2; // hago el promedio de R (el promedio de (R) es siempre entre 2 nodos)
      p[in]=(r+po)/2; // actualizo el nodo original con el promedio 
    } else{
      for (int j=0;j<en.size();j++){ // recorro la lista de nodos del elemento actual
        int k=e[en[j]].Indice(in); // obtengo el índice del nodo actual (P) en el elemento actual
        r+=p[e[en[j]][(k+1)%4]]; // e[en[j]][k+1] me da el índice del nodo siguiente (R) en el elemento actual
        f+=p[e[en[j]][(k+2)%4]]; // e[en[j]][k+2] me da el índice del nodo opuesto (F) al nodo actual (P) en el elemento actual
      }       
      r/=en.size(); // hago el promedio de R (como sumé los nodos en sentido "antihorario", el promedio no se multiplica por 2)
      f/=en.size(); // hago el promedio de los nodos opuestos (F) al nodo actual (Punto P)
      p[in]=(r*4-f+po*(int(en.size())-3))/en.size(); // actualizo el nodo original con el promedio
    }
  }
}
