#include "Material.h"
#include "CsgTree.h"
#include "Primitive.h"
#include "Raytrace.h"
#include <cmath>
#include "Line.h"
#include <iostream>
#include "Ray.h"
#include "GLTool.h"
#include <ctime>
#include <cmath>


/**
@file
@author Fabrice Aubert
*/


using namespace p3d;
using namespace std;


/**
Calcul de la couleur pour un rayon donné : la fonction sera récursive lors de l'ajout des rayons réfléchis
 - level détermine la profondeur de récursivité (pour limiter la profondeur de récursivité);
 - contribution donne le pourcentage (contrib \in [0,1]) de la contribution à l'éclairage total (cumul par appels récursif) : si la contribution devient trop faible, on arrête la récursivité
*/
Vector3 Raytrace::computeRayColor(const Ray &ray, int level, double contribution) {

    /**
  * Remarques :
  - inter->material().reflexionCoefficient() donnera le coefficient de contribution à la réflexion (i.e. 1=réflexion totale, 0=réflexion nulle)
  - inter->computeReflectRay() : permet de calculer le rayon réfléchi
  */


    Vector3 color=Vector3(0,0,0);

    if (level>0 && contribution>0.001) {

        // Intersection de la scène avec le rayon (donne uniquement l'intersection la "plus proche").
        Intersection *inter=_scene->intersection(ray,0.1); // 0.1 pour prendre une intersection qui se trouve un peu devant le "point de départ" (origine) du rayon

        if (inter!=NULL) { // existe-t-il une intersection avec la scène ?
            color=computeLocalColor(*inter); // calcul de la couleur par Phong

            //E4Q2 : on vérifie si l'objet est réfléchissant
            double r = inter->material().reflectionCoefficient();
            if(r > 0.) {
                //on renvoie un rayon réfléchi et on récupère la couleur renvoyée
                Vector3 cr = computeRayColor(inter->computeReflectRay(),level -1, 1.);

                //E4Q3 : on fait la moyenne de la couleur locale et réfléchie
                color = (1-r) * color + r * cr;
            }

            // libération mémoire de inter
            delete inter;
        }

    }


    return color;
}


/**
  Calcul de l'éclairement local d'un point d'intersection => Phong avec ombres portées.
  - toutes les données nécessaires au point d'intersection sont dans le paramêtre intersection (point, normale, noeud CSG intersecté)
  - les données de la scène (sources lumineuses) sont accessibles par scene()->...
*/
Vector3 Raytrace::computeLocalColor(const Intersection &intersection) {
    /**
  * P est le point d'intersection (Vector3)
  * L est le vecteur d'éclairement (Vector3)
  * N est la normale au point d'intersection (Vector3)
  * V est le vecteur d'observation
  * m contient le materiel au point : m.diffuse() donne le matériel diffus (de type Vector3 : on peut utiliser les opérateurs *, +, etc), de même m.specular(), m.shininess()
  * intersection.incident() donne le rayon qui a provoqué l'intersection
  * Pour les sources :
  *   - _scene->nbLight() donne le nombre de source lumineuses
  *   - _scene->lightPosition(i) donne la position de la source i (uniquement des sources ponctuelles).
  * Remarque : il faut faire la somme des couleurs obtenues pour chacune des sources (risque de saturation si plusieurs sources lumineuses).
  */

    Vector3 P;
    Vector3 L;
    Vector3 N;
    Vector3 V;

    //N = normale au point d'intersection (normalisé, voir cours chap6 p13)
    N=intersection.normal();
    N.normalize();

    //P = coordonnées du point d'intersection
    P=intersection.point();

    //V = vecteur point-oeil
    V = Vector3(P, _scene->camera().position());
    V.normalize();

    // m = propriété du matériau de l'objet (couleurs refletées etc.)
    Material m=intersection.node()->primitive()->material();

    //result contient juste la couleur du matériau, sans lumière s'y additionnant
    Vector3 result = m.ambient().xyz();

    //Pour toutes les sources de lumières
    for(int i = 0; i < _scene->nbLight(); i++) {
        //L = vecteur d'éclairement / angle du rayon => Vecteur entre la position de la lumière et le "point d'impact"
        L = Vector3(P, _scene->lightPosition(i));
        L.normalize();

        //E4Q1
        //on créé un rayon de direction L qui frappe P
        Ray shadow(P, L);
        //J'imagine qu'on cherche une intersection entre P + 0.1 et la source ?
        Intersection *nearestIntersection=_scene->intersection(shadow, 0.1);

        //E4Q1 : s'il n'existe pas d'intersection entre P et la source, on éclaire
        if(nearestIntersection == nullptr) {

            //E2Q2 : on calcule l'intensité du rayon diffusé, l'angle du rayon par rapport à la normale est un bon indicateur de l'intensité (chap6 p13)
            double LdotN = L.dot(N);

            //E4Q5 : on calcule le rayon R miroir de L (pour la spécularité) (chap6 p21)
            Vector3 R = 2 * (L.dot(N)) * N - L ;
            //E4Q5 : on calcule l'intensité du rayon spéculaire (V.R) pour la même raison (chap6 p22)
            //E4Q5 : on prend aussi en compte la brillance du matériau (chap6 p23)
            double coeff = pow(V.dot(R), m.shininess());

            //E2Q2 : si l'angle est inférieur à 0, on est de l'autre coté, si = 0 on est tangeant => on éclaire pas
            if(LdotN > 0.)
                //E2Q2 : on multiplie la couleur du matériau de base par l'intensité du rayon, puis on l'additionne aux rayons déjà reçus pour simuler l'éclairement
                result = result + m.diffuse() * LdotN;

            //E4Q5 : idem pour la spécularité
            if(coeff > 0.) {
                result = result + m.specular() * coeff;
            }
        }

        //E4Q1
        delete nearestIntersection;
    }

    return result;
}


/** *************************************************************** **/
/** *************************************************************** **/
/** *************************************************************** **/
/** *************************************************************** **/
/** *************************************************************** **/
/** *************************************************************** **/
/** *************************************************************** **/
void Raytrace::run() {
    _stopRequest=false;
    computeImage();
}


/**
Boucle principale du lancer de rayon

*/
void Raytrace::computeImage() {
    _camera=_scene->camera();
    _camera.viewport(0,0,_width,_height);
    Vector3 eye(0.0,0.0,0.0);
    Vector3 pixel_eye; // pixel dans le repère observateur
    _image->fill(Qt::black);

    clock_t clockStart=clock();

    Matrix4 csg2Camera=_scene->localWorld();
    csg2Camera.mul(_camera.worldCamera());


    for(unsigned int y=0; y<_height; ++y) {
        for(unsigned int x=0; x<_width; ++x) {
            if (_stopRequest) goto fin;
            pixel_eye=_camera.windowToCamera(x,y); // exprime le rayon dans le repère de l'observateur.

            Ray rayon=Ray(eye,pixel_eye);  // rayon primaire
            rayon.transform(csg2Camera);
            Vector3 c=computeRayColor(rayon,4,1.0); // calcule la couleur du pixel; 10=profondeur max de récursion, 1.0=contribution; tous les calculs sont entendus dans le repère G
            // mise à jour de la couleur du pixel dans l'image résultante
            c.clamp(0,1);
            QColor color=QColor::fromRgbF(c.r(),c.g(),c.b());

            _image->setPixel(x,y,color.rgba()); // affecte à l'image la couleur calculée
        }
    }
fin:
    clock_t clockElapsed=clock()-clockStart;
    cout << "Raytracing finished in " << double(clockElapsed)/CLOCKS_PER_SEC << " seconds" << endl;
}



/** *********************************************************************************************************** */
/** *********************************************************************************************************** */
/** *********************************************************************************************************** */

Raytrace::Raytrace(unsigned int width,unsigned int height) {
    _image=new QImage(width,height,QImage::Format_ARGB32);
    _width=width;
    _height=height;
}

Raytrace::Raytrace() : Raytrace(512,512) {
}


Raytrace::~Raytrace() {
    close();
    delete _image;
}



void Raytrace::close() {
    if (isRunning()) {
        _stopRequest=true;
        while (isRunning()) {
            cout << "Im waiting" << endl;
            usleep(100);
        }
    }
}








