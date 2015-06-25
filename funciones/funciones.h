#ifndef FUNCIONES
#define FUNCIONES

#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <string>
#include <complex>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <time.h>       /* time */
#include <set>
#include <deque>
#include <queue>
#include "malloc.h"
#include <omp.h>

#define cimg_plugin "plugins/skeleton.h"
#include "CImg.h"
#include <PDI_functions.h>

using namespace cimg_library;
using namespace std;


#define EPS 0.00001

/// STRUCTS Y VARIABLES AUXILIARES

//Punto
typedef struct punto{
    float x;
    float y;
    float norma() {
        return sqrt(x*x + y*y);
    }
    float normaManhattan() {
        return x+y;
    }
    bool operator<=( const punto& p ) const {
        return (this->x <= p.x & this->y <= p.y);
    }
    bool operator>=( const punto& p ) const {
        return (this->x >= p.x & this->y >= p.y);
    }
    bool operator<( const punto& p ) const {
        return (this->x < p.x & this->y < p.y);
    }
    bool operator>( const punto& p ) const {
        return (this->x > p.x & this->y > p.y);
    }


}punto;

//Sort de Vector con puntos
bool myfunction (punto i,punto j) { return (i.normaManhattan()<j.normaManhattan()); }

//Ordenar coordenadas
vector<punto> ordenarCoordenadas(vector<punto> puntos){
    // using object as comp
    std::sort (puntos.begin(), puntos.end(),myfunction);
    return puntos;
}

//Para usar sort con vectores de puntos
bool compara_puntos(const punto &a, const punto &b)
{
    return a.x < b.x;
}

//Colores auxiliares

const unsigned char red[] = { 255,0,0 },
green[] = { 0,255,0 },
blue[] = { 0,0,255 },
white[] = { 255,255,255 },
black[] = { 0,0,0 },
yellow[] = { 255,255,0 };

struct Pixel {
    int x;
    int y;
    CImg<double>* img;

    Pixel() : x(0), y(0), img(NULL) {}
    Pixel(const Pixel& p) : x(p.x), y(p.y), img(p.img) {}
    Pixel(int x, int y, CImg<double>* img) : x(x), y(y), img(img) {}

    bool operator<( const Pixel& p ) const {
        return this->value() < p.value();
    }
    double value() const {
        return img->operator()(x,y);
    }
    bool AdentroImagen( ) {
        return x>=0 && y>=0 && x<img->width() && y<img->height();
    }
};


// transforma radianes a reg
double radToReg(double rad) {
    return rad*180.0/M_PI;
}

//transforma reg a rad
double degToRad(double deg) {
    return deg*M_PI/180.0;
}

//------------------------------FUNCIONES AUXILIARES--------------------------------------------

///****************************************
///FUNCION NEGATIVO - Devuelve el negativo de una imagen
///****************************************
template<typename T>
CImg<T> negativo(CImg<T> imagen){
    cimg_forXY(imagen,i,j)
            imagen(i,j) = abs(((int)*imagen.data(i,j,0,0))-255);
    return imagen.normalize(0,255);
}

///--------FUNCIONES LUT----------

//s=a*r+c
//con r: valor de entrada
//a = factor de ganancia
//c = offset

//Caso generico. Recibe un gris inicial y un final. Por tramos
//item 1
template<typename T>
CImg<T> generar_lut(T a,T c,T ini,T fin){
    CImg<T> lut(1,abs(fin-ini)+1,1,1);
    T s;
    for (int i=ini;i<abs(fin-ini)+1;i++){
        s=T(a*i+c);
        ///--------------------------    if (s>255) s=255;
        if (s<0) s=0;
        lut(i)=s;
    }
    return lut;
}

//Funcion curva Solo cuando se necesitan tramos
template<typename T>
CImg <T> generar_curva(CImg<T> lut,vector<punto> puntos){
    T x0,x1,y0,y1;
    T a,c;
    a=1;
    c=0;
    CImg<T> curva(lut.width(),lut.height(),1,1),aux(lut.width(),lut.height(),1,1);

    //Ordeno el vector para que los puntos esten ordenados
    //sort(puntos.begin(),puntos.end(),compara_puntos);

    if (puntos.size() == 2) return generar_lut(1,0,0,255);

    x0 = puntos[0].x;
    y0 = puntos[0].y;

    for (int i=1;i<puntos.size();i++){
        //Cargo los puntos
        x1 = puntos[i].x;
        y1 = puntos[i].y;
        a=(y1-y0)/(x1-x0);

        cout<<"pendiente: ";
        printf("%.12f", a);
        cout<<endl;

        aux = generar_lut(a,c,x0,x1);

        for(int i=x0;i<x1;i++){
            curva(1,i) = aux(1,i);
        }

        x0 = x1;
        y0 = y1;

    }

    return curva;
}

///LUT LOGARITMICA
// Si positiva es 1 Aclara la imagen
// Si positiva es 0 Oscurece la imagen
template<typename T>
CImg<T> generar_lut_logb(T c){
    CImg<T> lut(1,256,1,1);
    T s;
    for (int i=0;i<256;i++){
        //Aca mapeamos el rango total resultante para que despues se traduzca en un nivel de gris
        s=floor(c*(255*((log(1+i)/log(1+255)))));
        lut(i)=s;
    }
    return lut;

}



///LUT EXPONECIAL
// gamma es positivo
// 0 < gamma < 1 Aclara la imagenimgnueva
// gamma > 1 Oscurece la imagen
template<typename T>
CImg<T> generar_lut_expb(T c,T gamma){
    CImg<T> lut(1,256,1,1);
    T s;
    for (int i=0;i<256;i++){
        //s=floor(c*(pow(i,gamma))/255);
        s=c*(pow(i,gamma));
        lut(i)=s;
    }
    lut.normalize(0,255);
    return lut;
}


//aplica la transformacion que se la pase en "lut" a la imagen "img"
template<typename T>
CImg<T> transformacion(CImg<T> img,CImg<T> lut){
    cimg_forXY(img,i,j)
            img(i,j)=lut(img(i,j)); // "map"
    return img;
}

//aplica la transformacion que se la pase en "lut" a la imagen "img"
template<typename T>
CImg<T> transformacion3(CImg<T> img,CImg<T> lut){

    cimg_forXY(img,i,j){
        img(i,j,0,0)=lut(img(i,j,0,0)); // "map"
        img(i,j,0,1)=lut(img(i,j,0,1)); // "map"
        img(i,j,0,2)=lut(img(i,j,0,2)); // "map"
    }

    return img;
}

///****************************************
/// FUNCION DETECTAR_MEDIOTONO - Devuelve una imagen 3x3 del mediotono correspondiente a la intensidad recibida
///****************************************
template<typename T>
CImg<T> detectar_mediotono(T intensidad){
    
    CImg <T> nivel(3,3,1,1);
    
    //9 niveles de 28 cada uno. El ultimo tiene 31
    if(intensidad >=0 && intensidad<25){
        //nivel 0

        return nivel.fill(0);
    }
    if(intensidad >=25 && intensidad<50){
        //nivel 1

        nivel.fill(0);
        nivel(0,1)= 255;
        return nivel;
    }
    if(intensidad >=50 && intensidad<75){
        //nivel 2

        nivel.fill(0);
        nivel(0,1)= 255;
        nivel(2,2)= 255;
        return nivel;
    }
    if(intensidad >=75 && intensidad<100){
        //nivel 3

        nivel.fill(0);
        nivel(0,0)= 255;
        nivel(0,1)= 255;
        nivel(2,2)= 255;
        return nivel;
    }
    if(intensidad >=100 && intensidad<125){
        //nivel 4

        nivel.fill(0);
        nivel(0,0)= 255;
        nivel(0,1)= 255;
        nivel(2,0)= 255;
        nivel(2,2)= 255;
        return nivel;
    }
    if(intensidad >=125 && intensidad<150){
        //nivel 5

        nivel.fill(0);
        nivel(0,0)= 255;
        nivel(0,1)= 255;
        nivel(0,2)= 255;
        nivel(2,0)= 255;
        nivel(2,2)= 255;
        return nivel;
    }
    if(intensidad >=150 && intensidad<175){
        //nivel 6

        nivel.fill(0);
        nivel(0,0)= 255;
        nivel(0,1)= 255;
        nivel(0,2)= 255;
        nivel(1,2)= 255;
        nivel(2,0)= 255;
        nivel(2,2)= 255;
        return nivel;
    }
    if(intensidad >=175 && intensidad<200){
        //nivel 7

        nivel.fill(255);
        nivel(1,0)= 0;
        nivel(1,1)= 0;
        return nivel;
    }
    if(intensidad >=200 && intensidad<225){
        //nivel 8

        nivel.fill(255);
        nivel(1,1)= 0;
        return nivel;
    }
    if(intensidad >=225){
        //nivel 9

        return nivel.fill(255);
    }

}
///****************************************
///FUNCION MEDIOTONO - Recibe una imagen y la devuelve en binario con los medios tonos correspondientes
///****************************************
template<typename T>
CImg<T> mediotono(CImg<T> original){

    CImg<T> modificada(original.width()*3,original.height()*3,1,1), aux(3,3,1,1);
    T var_x,var_y;
    
    original.normalize(0,255);

    for (int x =0;x<original.width();x++){
        for (int y =0;y<original.height();y++){

            aux = detectar_mediotono((int)*original.data(x,y,0,0));
            //cout <<"Fija x: "<<x<<" Columna y: "<<y<<" Intensidad: "<<(int)*original.data(x,y,0,0)<<endl;

            var_x=3*x;
            var_y=3*y;

            modificada(var_x,var_y) = aux(0,0);
            modificada(var_x,var_y+1) = aux(0,1);
            modificada(var_x,var_y+2) = aux(0,2);
            modificada(var_x+1,var_y) = aux(1,0);
            modificada(var_x+1,var_y+1) = aux(1,1);
            modificada(var_x+1,var_y+2) = aux(1,2);
            modificada(var_x+2,var_y) = aux(2,0);
            modificada(var_x+2,var_y+1) = aux(2,1);
            modificada(var_x+2,var_y+2) = aux(2,2);
        }

    }

    modificada.resize(original.width(),original.height(),-100,-100,3);
    return modificada;
}

///****************************************
///FUNCION BINARIO - Caso que recibe un umbral (En la libreria esta la funcion get_threshold tambien)
///****************************************
template<typename T>
CImg<T> binario(CImg<T> imagen, T umbral){

    CImg<T> modificada(imagen.width(),imagen.height(),1,1);

    for (int i=0;i<imagen.width();i++){
        for(int j=0;j<imagen.height();j++){

            if( (int)*imagen.data(i,j,0,0) > umbral )
            {
                modificada(i,j)= 255;
            }
            else
            {
                modificada (i,j)=0;
            }

        }
    }
    return modificada;

}
///****************************************
///SUMA
///****************************************
//corrimiento=0 si quiero sumar sin corrimiento
CImg<double> sumaImg(CImg<double> img1, CImg<double> img2, double corrimiento=0 ){
    CImg<double> resultado(img1.width(),img1.height(),1,3);
    cimg_forXY(img1,i,j){
        if((i+corrimiento) >= 0 && (j+corrimiento)>=0){
            resultado(i,j)=(img1(i,j)+ img2(i+corrimiento,j+corrimiento))/2;
        }else
        {

            resultado(i,j,0,0)=(img1(i,j,0,0)+ img2(i,j,0,0))/2;
            resultado(i,j,0,1)=(img1(i,j,0,1)+ img2(i,j,0,1))/2;
            resultado(i,j,0,2)=(img1(i,j,0,2)+ img2(i,j,0,2))/2;
        }
    }
    //componer la imagen

    return resultado;
}
///****************************************
///DIFERENCIA
///****************************************
template<typename T>
CImg<T> DifImg(CImg<T> img1, CImg<T> img2){
    CImg<T> resultado(img1.width(),img1.height(),1,1);
    cimg_forXY(img1,i,j)
    {
        if((img1(i,j)- img2(i,j)/2) < 0)
        {
            resultado(i,j)= 0;
        }
        else
        {
            if((img1(i,j)- img2(i,j)/2) > 255){
                resultado(i,j)= 255;
            }else
            {
                resultado(i,j)= (img1(i,j)- img2(i,j))/2;
            }
        }
    }
    return resultado;
}
///****************************************
///MULTIPLICACION
///****************************************
template<typename T>
CImg<T> multiplicacion(CImg<T> img, CImg<T> masc){
    CImg<T> resultado(img.width(),img.height(),1,1);
    cimg_forXY(img,i,j) resultado(i,j)=img(i,j) * masc(i,j);
    return resultado;
}
///****************************************
///DIVISION
///****************************************
template<typename T>
CImg<T> division(CImg<T> &img, CImg<T> &masc){
    cimg_forXY(masc,i,j) masc(i,j) = 255 * 1/masc(i,j);
    return multiplicacion(img,masc);

}
///****************************************
///REDUCIR RUIDO //pasar una imagen con ruido en "img",  //genera la suma de "n" imagenes con ruido
///****************************************
//pasar lista de imagenes con ruido
CImg<float> reducRuido(CImgList<float>lista){
    CImg<float> suma(lista[0].width(),lista[0].height(),1,3,0);
    for(int i=0;i<lista.size();i++)
        suma=(suma+lista[i])/2;
    return suma;
}
///****************************************
///UMBRAL INVERTIDO
///****************************************
//recibe imagen en escala de gris
template<typename T>
CImg<T> umbral_invertido(CImg<T> &img, T p){
    return negativo(img.get_threshold(p));
}
///****************************************
///UMBRAL POR TRAMOS
///****************************************
//recibe imagen en escala de gris
template<typename T>
CImg<T> umbral_por_tramos(CImg<T> img, int p1,int p2){
    CImg<T> resultado(img.width(),img.height(),1,1);
    cimg_forXY(img,i,j){
        if (img(i,j)<=p1 || img(i,j)>=p2)
            resultado(i,j)=255;
        else
            resultado(i,j)=0;
    }
    return resultado;
}

//umbral por tramos booleano
//recibe imagen en escala de gris
template<typename T>
CImg<bool> umbral_por_tramos_bool(CImg<T> img, int p1,int p2){
    CImg<bool> resultado(img.width(),img.height(),1,1);
    cimg_forXY(img,i,j){
        if (img(i,j)<=p1 || img(i,j)>=p2) // todo lo que esta fuera de rango lo pongo a cero
            resultado(i,j)=0;
        else
            resultado(i,j)=1;
    }
    return resultado;
}
///****************************************
///CONTAR CANTIDAD DE GRISES DISTINTOS
///****************************************
template<typename T>
int cant_grises(CImg<T> imagen){
    CImg<T> hist = imagen.get_histogram(256,0,255);
    int contador=0;

    for(int g=1;g<254;g++){//no contar el negro y tampoco el blanco
        if(hist(g)>0){
            contador++;
        }
    }

    return contador;
}
///****************************************
///DEVOLVER GRISES DE UNA IMAGEN
///****************************************
template<typename T>
vector<int> grises_disponibles(CImg<T> imagen){
    CImg<T> hist = imagen.get_histogram(256,0,255);
    vector<int> grises;

    for(int g=1;g<254;g++){//no contar el negro y tampoco el blanco
        if(hist(g)>0){
            grises.push_back(g);
        }
    }

    return grises;
}
///****************************************
///GREYSLICING INTERACTIVO
///****************************************
//Devuelve una imagen binaria
template<typename T>
CImg<bool> greyg(CImg<T> imagen,int ancho=10,bool creg=false){
    CImg<bool> final;
    CImgDisplay v1(imagen,"Presione sobre el gris deseado"),v2(imagen,"Resultado");

    while(!v1.is_closed() || !v2.is_closed()){
        v1.wait();
        if(v1.button()==1){

            int mx=v1.mouse_x();
            int my=v1.mouse_y();

            if(!creg){
                //Defino los puntos de la zona media
                T p1 = imagen(mx,my)-(ancho/2) >= 0 ? imagen(mx,my)-(ancho/2):0;
                T p2 = imagen(mx,my)+(ancho/2) <= imagen.max() ? imagen(mx,my)+(ancho/2):imagen.max();

                final = umbral_por_tramos(imagen,p1,p2);
            }else{
                final = region_growing(imagen,mx,my,ancho,256);
            }
            v2.render(final);
            v2.paint();
        }
    }

    return final;
}
///****************************************
///MAYOR
///****************************************
// las imagenes en escala de griz
template<typename T>
CImg<T> MAYORimg(CImg<T> &img, CImg<T> &img2){
    CImg<T> resultado(img.width(),img.height(),1,1);
    cimg_forXY(img,i,j)
            if ( img(i,j)>img2(i,j) )
            resultado(i,j)=1;
    else
    resultado(i,j)=0;

    return resultado;
}
///****************************************
///MENOR
///****************************************
//imagenes en escala de griz
template<typename T>
CImg<T> MENORimg(CImg<T> &img, CImg<T> &img2){
    CImg<T> resultado(img.width(),img.height(),1,1);
    cimg_forXY(img,i,j)
            if ( img(i,j)<img2(i,j) )
            resultado(i,j)=1;
    else
    resultado(i,j)=0;

    return resultado;
}
///****************************************
///BINARIO
///****************************************
//Pasar numero a binario
template<typename T>
vector<T> binario(T numero) {
    vector<T> bin;
    while(numero>=1){
        bin.push_back(numero%2);//voy guardando el modulo
        numero=numero/2;

    }
    while(bin.size()<8)
        bin.push_back(0);

    return bin;
}
///****************************************
///EMBOSS
///****************************************
//Filtro emboss
template<typename T>
CImg<T> emboss(CImg<T> img,int corrimiento){
    CImg<T> img_neg(img.width(),img.height(),1,1);//
    img_neg = negativo(img);
    return sumaImg(img, img_neg,corrimiento);//suma a img su negativo un poquito desplazado
}
///****************************************
///BLISTER
///****************************************
//Detector de faltantes de pastillas en bliter,
//Recive:la imagen blister (es deseable que este umbralizada o con buen contraste)
//Retorna: vector con coordenadas en las que faltan pastillas
template<typename T>
vector<punto> blister(CImg<T> img)
{   vector<punto> vectorcoord;
    punto puntos;
    for (int j=53;j<142;j=j+48)
        for (int i=53;i<img.width();i=i+50)
            if(img.get_crop(i-18,j-18,i+18,j+18).mean()<0.2){
                puntos.x=i;puntos.y=j;
                vectorcoord.push_back(puntos);
            }
    return vectorcoord;
}
///****************************************
///BIT LIST
///****************************************
template<typename T>
CImgList<T> bitlist(CImg<T> original)
{
    CImg<T> img_b0,img_b1,img_b2,img_b3,img_b4,img_b5,img_b6,img_b7;
    img_b0=img_b1=img_b2=img_b3=img_b4=img_b5=img_b6=img_b7=original;

    vector<T> bit;
    cimg_forXY(original,i,j){
        bit= binario(original(i,j));
        img_b0(i,j) = bit[0]*pow(2.0,0.0);
        img_b1(i,j) = bit[1]*pow(2.0,1.0);
        img_b2(i,j) = bit[2]*pow(2.0,2.0);
        img_b3(i,j) = bit[3]*pow(2.0,3.0);
        img_b4(i,j) = bit[4]*pow(2.0,4.0);
        img_b5(i,j) = bit[5]*pow(2.0,5.0);
        img_b6(i,j) = bit[6]*pow(2.0,6.0);
        img_b7(i,j) = bit[7]*pow(2.0,7.0);
    }

    CImgList<T> lista;
    lista.push_back(img_b0);
    lista.push_back(img_b1);
    lista.push_back(img_b2);
    lista.push_back(img_b3);
    lista.push_back(img_b4);
    lista.push_back(img_b5);
    lista.push_back(img_b6);
    lista.push_back(img_b7);

    return lista;
}

///----------------Operaciones Logicas Binarias-------------------

///****************************************
///OR LOGICO (Union de conjuntos)
///****************************************
CImg<bool> ORimg(CImg<bool> A, CImg<bool> B){
    CImg<bool> resultado(A.width(),A.height(),1,1);
    cimg_forXY(A,i,j){
        if (A(i,j)==0 & B(i,j)==0)
            resultado(i,j)=0;
        else
            resultado(i,j)=1;
    }
    return resultado;
}
///****************************************
///AND LOGICO (Interseccion de conjuntos)
///****************************************
CImg<bool> ANDimg(CImg<bool> A, CImg<bool> B){
    CImg<bool> resultado(A.width(),A.height(),1,1);
    cimg_forXY(A,i,j)
            resultado(i,j)=(A(i,j)*B(i,j));
    return resultado;
}
///****************************************
///NOT LOGICO (Complemento de un conjunto)
///****************************************
CImg<bool> NOTimg(CImg<bool> A){
    cimg_forXY(A,i,j){
        if(A(i,j)==0){
            A(i,j)=1;
        }else{
            A(i,j)=0;
        }
    }
    return A;
}
///****************************************
///XOR LOGICO
///****************************************
CImg<bool> XORimg(CImg<bool> A, CImg<bool> B){
    CImg<bool> resultado(A.width(),A.height(),1,1);
    cimg_forXY(A,i,j){
        if ((A(i,j)==0 & B(i,j)==0) | (A(i,j)==1 & B(i,j)==1))
            resultado(i,j)=0;
        else
            resultado(i,j)=1;
    }
    return resultado;
}
///****************************************
///NOT-AND LOGICO
///****************************************
CImg<bool> NOTANDimg(CImg<bool> A, CImg<bool> B){
    return ANDimg(NOTimg(A),B);
}
///****************************************
///DIFERENCIA DE CONJUNTOS
///****************************************
CImg<bool> DIFERENCIAimg(CImg<bool> A, CImg<bool> B){
    return ANDimg(A,NOTimg(B));
}
///****************************************
///REFLEXION DE CONJUNTO
///****************************************
CImg<bool> REFLEXIONimg(CImg<bool> A){
    cimg_forXY(A,i,j){
        A(i,j)*=-1;
    }
    return A;
}
///****************************************
///TRANSLACION DE CONJUNTO
///****************************************
CImg<bool> TRANSLACIONimg(CImg<bool> A, punto z){
    CImg<bool> resultado(A.width()+abs(z.x),A.height()+abs(z.y),1,1);
    cimg_forXY(A,i,j){
        resultado(z.x+i,z.y+j)=A(i,j);
    }
    return resultado;
}
///****************************************
///MAXIMAS Y MINIMAS COORDENADAS DE LOS BLANCOS DE UNA MASCARA
///****************************************
vector<punto> maxmin_coord(CImg<bool> A){
    //Primero encuentro las coordenadas maximas y minimas
    int max_x=0,max_y=0,min_x=A.width(),min_y=A.height();
    vector<punto> maxmin_finales;
    cimg_forXY(A,i,j){
        if(A(i,j)==1){
            //Estoy en un pixel del blanco buscado - actualizo los maximos y los minimos
            i>max_x ? max_x=i:max_x=max_x;
            j>max_y ? max_y=j:max_y=max_y;
            i<min_x ? min_x=i:min_x=min_x;
            j<min_y ? min_y=j:min_x=min_x;
        }
    }
    punto max,min;
    max.x=max_x;max.y=max_y;
    min.x=min_x;min.y=min_y;
    maxmin_finales.push_back(min);
    maxmin_finales.push_back(max);
    return maxmin_finales;
}


///-------------FILTRADO ESPACIAL-------------------------

///****************************************
///KERNEL PROMEDIO
///****************************************
//Funcion que devuelve el kernel promediado en func. del tamaño
//template<typename T>
CImg<double> mask(double tamanio){

    CImg<double> mascara (tamanio,tamanio,1,1,1);

    //Retorno el kernel.Tamaño
    return mascara/(tamanio*tamanio);

}
///****************************************
///GENERADOR DE KERNEL GAUSSIANO
///****************************************
/**
 * \file         gauss_filter.h
 * \author       Alain Lehmann <lehmann at vision.ee.ethz.ch>
 * \version      $Id: gauss_filter.h 337 2009-09-22 16:01:15Z lehmanal $
 * \date         2009-09-22 created
 */

/** compute Gaussian derivatives filter weights
 * \param sigma = bandwidth of the Gaussian
 * \param deriv = computing the 'deriv'-th derivatives of a Gaussian
 * the width of the filter is automatically determined from sigma.
 * g  = \frac{1}{\sqrt{2\pi}\sigma}   \exp(-0.5 \frac{x^2}{\sigma^2} )
 * g' = \frac{x}{\sqrt{2\pi}\sigma^3} \exp(-0.5 \frac{x^2}{\sigma^2} )
 * /   = -\frac{x}{\sigma^2} g
 * g''= (\frac{x^2}{\sigma^2} - 1) \frac{1}{\sigma^2} g
 */
template<typename T>
CImg<T> gauss_filter (T sigma=1, T deriv=0) {
    T width = 3*sigma;               // may be less width?
    T sigma2 = sigma*sigma;
    CImg<T> filter;
    filter.assign(floor(2*width)+1);

    int i=0;
    for (T x=-width; x<=width; x+=1.0f) {
        T g = exp(-0.5*x*x/sigma2) / sqrt(2*M_PI) / sigma;
        if (deriv==1) g *= -x/sigma2;
        if (deriv==2) g *= (x*x/sigma2 - 1)/sigma2;
        filter[i] = g ;
        //printf ("i=%f -> %f\n", x, filter[i]);
        i++;
    }
    return filter;
}
///****************************************
///ROBERTS
///****************************************
///idea de gradiente en dos direcciones
template<typename T>
CImg<T> roberts(CImg<T> img){
    CImg<T> Gx(3, 3), Gy(3,3);
    Gx(0,0)=0; Gx(0,1)=0; Gx(0,2)=0;
    Gx(1,0)=0; Gx(1,1)=-1; Gx(1,2)=0;
    Gx(2,0)=0; Gx(2,1)=0; Gx(2,2)=1;

    Gy(0,0)=0; Gy(1,0)=0; Gy(2,0)=0;
    Gy(0,1)=0; Gy(1,1)=0; Gy(2,1)=-1;
    Gy(0,2)=0; Gy(1,2)=1; Gy(2,2)=0;

    return (img.get_convolve(Gx)+img.get_convolve(Gy)).abs().normalize(0,255);
}
///****************************************
///SOBEL
///****************************************
///idea de gradiente en dos direcciones
template<typename T>
CImg<T> Sobel(CImg<T> img,int tipo = 0){

    //Sobel con valor de entrada 0 con tendencia vertical y horizontal
    //Cuando el valor de entrada es 1 tiene tendencia diagonal
    CImg<T> Gx(3, 3),Gy(3, 3);

    if(tipo ==0){
        Gx(0,0)=-1; Gx(0,1)=-2; Gx(0,2)=-1;
        Gx(1,0)=0; Gx(1,1)=0; Gx(1,2)=0;
        Gx(2,0)=1; Gx(2,1)=2; Gx(2,2)=1;

        Gy(0,0)=-1; Gy(1,0)=0; Gy(2,0)=1;
        Gy(0,1)=-2; Gy(1,1)=0; Gy(2,1)=2;
        Gy(0,2)=-1; Gy(1,2)=0; Gy(2,2)=1;

    }

    else{

        Gx(0,0)=0; Gx(0,1)=1; Gx(0,2)=2;
        Gx(1,0)=-1; Gx(1,1)=0; Gx(1,2)=1;
        Gx(2,0)=-2; Gx(2,1)=-1; Gx(2,2)=0;

        Gy(0,0)=-2; Gy(1,0)=-1; Gy(2,0)=0;
        Gy(0,1)=-1; Gy(1,1)=0; Gy(2,1)=1;
        Gy(0,2)=0; Gy(1,2)=1; Gy(2,2)=2;
    }


    return (img.get_convolve(Gx)+img.get_convolve(Gy)).abs().normalize(0,255);
}
///****************************************
///PREWITT
///****************************************
///idea de gradiente en dos direcciones
template<typename T>
CImg<T> prewitt(CImg<T> img,int tipo=0){
    //Prewitt con valor de entrada 0 con tendencia vertical y horizontal
    //Prewitt el valor de entrada es 1 tiene tendencia diagonal
    CImg<T> Gx(3, 3),Gy(3, 3);

    if(tipo ==0){
        Gx(0,0)=-1; Gx(0,1)=-1; Gx(0,2)=-1;
        Gx(1,0)=0; Gx(1,1)=0; Gx(1,2)=0;
        Gx(2,0)=1; Gx(2,1)=1; Gx(2,2)=1;

        Gy(0,0)=-1; Gy(1,0)=0; Gy(2,0)=1;
        Gy(0,1)=-1; Gy(1,1)=0; Gy(2,1)=1;
        Gy(0,2)=-1; Gy(1,2)=0; Gy(2,2)=1;

    }

    else{

        Gx(0,0)=0; Gx(0,1)=1; Gx(0,2)=1;
        Gx(1,0)=-1; Gx(1,1)=0; Gx(1,2)=1;
        Gx(2,0)=-1; Gx(2,1)=-1; Gx(2,2)=0;

        Gy(0,0)=-1; Gy(1,0)=-1; Gy(2,0)=0;
        Gy(0,1)=-1; Gy(1,1)=0; Gy(2,1)=1;
        Gy(0,2)=0; Gy(1,2)=1; Gy(2,2)=1;
    }


    return (img.get_convolve(Gx)+img.get_convolve(Gy)).abs().normalize(0,255);
}

///****************************************
///LAPLACIANO
///****************************************
///idea de gradiente en dos direcciones
template<typename T>
CImg<T> laplaciano(CImg<T> img,int tipo =0){
    //LAPLACIANO con valor de entrada 0 valido para aproximación con N4 --> G = 4z5-(z2+z4+z6+z8)
    //LAPLACIANO el valor de entrada 1, valido para aproximación con N8 --> G = 8z5-(z1+z2+z3+z4+z6+z7+z8+z9)
    CImg<T> G(5,5);

    if(tipo ==0){
        G(0,0)=0; G(0,1)=-1; G(0,2)=0;
        G(1,0)=-1; G(1,1)=4; G(1,2)=-1;
        G(2,0)=0; G(2,1)=-1; G(2,2)=0;

    }

    else{

        G(0,0)=-1; G(0,1)=-1; G(0,2)=-1;
        G(1,0)=-1; G(1,1)=8; G(1,2)=-1;
        G(2,0)=-1; G(2,1)=-1; G(2,2)=-1;
    }


    return img.get_convolve(G).abs().normalize(0,255);
}

///****************************************
///LoG (Laplaciano del gaussiano)
///****************************************
///idea de gradiente en dos direcciones
template<typename T>
CImg<T> LoG(CImg<T> img){
    CImg<T> G(5,5);

    G(0,0)=0; G(0,1)=0; G(0,2)=-1;G(0,3)=0; G(0,4)=0;
    G(1,0)=0; G(1,1)=-1; G(1,2)=-2;G(1,3)=-1; G(1,4)=0;
    G(2,0)=-1; G(2,1)=-2; G(2,2)=16;G(2,3)=-2; G(2,4)=-1;
    G(3,0)=0; G(3,1)=-1; G(3,2)=-2;G(3,3)=-1; G(3,4)=0;
    G(4,0)=0; G(4,1)=0; G(4,2)=-1;G(4,3)=0; G(4,4)=0;

    return img.get_convolve(G).abs().normalize(0,255);
}

///****************************************
///FILTRO DE ALTA POTENCIA 1 CANAL
///****************************************
//Recibe la imagen el kernel pasa bajo y la amplificacion
template<typename T>
CImg<T> filtroAP(CImg<T> img,CImg<T> kernel,float amp=0.0){
    //auxiliares
    CImg<T> pb,dif,sum;

    //Aplico filtro
    pb=img.get_convolve(kernel);

    //Se lo resto a lo original para generar el pasa alto
    dif=DifImg(img,pb);

    //Lo sumo a la original para generar alta potencia
    sum=sumaImg((1-amp)*img,dif);

    return sum;
}

///****************************************
///FILTRO DE ALTA POTENCIA 3 CANALES
///****************************************
//Recibe la imagen el kernel pasa bajo y la amplificacion
template<typename T>
CImg<T> filtroAP3(CImg<T> img,CImg<T> kernel, float amp=0.0){

    CImg<T> c1,c2,c3;

    //Aplico alta potencia a cada canal por separado
    c1 = filtroAP(img.get_channel(0),kernel,amp);
    c2 = filtroAP(img.get_channel(1),kernel,amp);
    c3 = filtroAP(img.get_channel(2),kernel,amp);

    //Devuelvo la suma de canales
    return c1+c2+c3;
}

///PROMEDIO HISTOGRAMA
template<typename T>
CImg<T> promedio_histograma(CImgList<T> lista){

    CImg <T> img,promedio(256,1);
    promedio = lista(0);

    for (int p=1;p<lista.size();p++){
        img = lista(p);
        for (int i=0;i<promedio.width();i++){
            promedio(i)+=img(i);
            promedio(i)*=0.5;
        }
    }

    return promedio;

}
///****************************************
///HISTOGRAMA
///****************************************
/// Construye un histograma y lo devuelve en una imagen
template <class T>
CImg<T> draw_histogram(CImg<T> &img){
    CImg<T> histograma;
    histograma.assign(img.width(), img.height(), 1, img.spectrum());
    histograma.fill(0);
    if(img.spectrum()==1){ // es de 1 solo canal
        T white[]={255};
        histograma.draw_graph(img.get_histogram(256, 0, 255), white, 1 , 3, 0);
    }else{
        T red[]={255,0,0};
        T green[]={0,255,0};
        T blue[]={0,0,255};
        CImg<T> r, g, b;
        r=img.get_channel(0);
        g=img.get_channel(1);
        b=img.get_channel(2);
        histograma.draw_graph(r.get_histogram(256, 0, 255), red, 1 , 1, 0);
        histograma.draw_graph(g.get_histogram(256, 0, 255), green, 1 , 1, 0);
        histograma.draw_graph(b.get_histogram(256, 0, 255), blue, 1 , 1, 0);
    }
    return histograma;
}

///****************************************
///ECUALIZACION LOCAL DE HISTOGRAMA
///****************************************
// ecualizacion /de histograma local para unsigned char descartando los bordes
template<typename T>
void LocalHistoEq(CImg<T> &img, T windowSize){
    //CImg<T> histo=img.get_histogram(256, 0, 255);
    CImg<T> ret(img.width()-windowSize*2-1, img.height()-windowSize*2-1);
    T i, j, m, l, ii, jj, mm, ll;
    ii=img.width()-windowSize-1; //img original menos el borde
    jj=img.height()-windowSize-1;
    T pixelValue;//valor actual de pixel
    T newPixelValue;	//nuevo valor de pixel
    // recorre los pixeles de la imagen original
    for(i=windowSize; i<ii; i++){
        for(j=windowSize; j<jj; j++){
            // calcula el nuevo valor del pixel segun la ecualizacion de histograma local
            pixelValue=img(i,j);
            mm=i+windowSize;
            ll=j+windowSize;
            // recorre la ventana
            newPixelValue=0;
            //ahora recorro solamente la porcion determinada por la ventana
            for(m=i-windowSize; m<mm; m++){
                for(l=j-windowSize; l<ll; l++){
                    if(img(m,l)<=pixelValue) //si el valor de imagen es menor al valor actual de pixel, sumar hasta que sea mayor
                        newPixelValue++;     //newPixelValue es el valor cumulado
                }
            }
            // el valor que le corresponde al nuevo pixel segun la probabilidad acumulada
            newPixelValue*=255.0/float((windowSize*2+1)*(windowSize*2+1)); // normaliza la imagen y saca probabilidad acumulada
            ret(i-windowSize,j-windowSize)=newPixelValue;
        }
    }
    img=ret;
}



///****************************************
/// CARGAR PALETA
///****************************************
//uso esta funcion como funcion auxiliar dentro de "Aplicar Paleta"
//cargo paleta de color desde archivo del disco
//paleta= se guardan los valores leidos desde archivo
//nombre= ruta completa al archivo en disco de la paleta
int cargar_paleta(vector< vector< float> > &paleta, string nombre) {
    vector< float > plinea;
    string cad, rS, gS, bS;
    float r, g, b;
    int pos, nval;

    ifstream arch(nombre.c_str());
    if (arch.fail()) {
        cerr << "ERROR al abrir el archivo " << nombre << endl;
        return 1;
    } else {
        paleta.clear();
        // lee cuantos valores por fila
        getline(arch,cad);
        nval=0;
        pos=cad.find_first_of(".");
        while (pos>0) {
            cad.erase(0,pos+1);
            pos=cad.find_first_of(".");
            nval++;
        }
        arch.close();
        arch.open(nombre.c_str());
        plinea.resize(nval);
        while (getline(arch,cad)) {
            pos=cad.find_first_not_of(" ");
            cad.erase(0,pos);
            pos=cad.find_first_of(" ");
            rS=cad.substr(0,pos);
            r=atof(rS.c_str());
            plinea[0]=r;
            cad.erase(0,pos);
            pos=cad.find_first_not_of(" ");
            cad.erase(0,pos);
            pos=cad.find_first_of(" ");
            gS=cad.substr(0,pos);
            g=atof(gS.c_str());
            plinea[1]=g;
            cad.erase(0,pos);
            pos=cad.find_first_not_of(" ");
            cad.erase(0,pos);
            pos=cad.find_first_of(" ");
            bS=cad.substr(0,pos);
            b=atof(bS.c_str());
            plinea[2]=b;
            paleta.push_back(plinea);
        }
        arch.close();
        return 0;
    }
}

///****************************************
///APLICAR PALETA
///****************************************
// parametros:
//Imagen a la que se la aplica la paleta,
//Rruta completa de la ubicacion de archivo de paleta
CImg<float> AplicarPaleta(CImg<float> img, string nomPaleta){
    CImg<float> ret(img.width(), img.height(), 1, 3);

    vector< vector<float> > paleta;
    cargar_paleta(paleta, nomPaleta);

    img.normalize(0, 255);
    for(unsigned i=0; i<img.width(); i++){
        for(unsigned j=0; j<img.height(); j++){
            ret(i,j,0,0)=paleta[unsigned(img(i,j))][0];
            ret(i,j,0,1)=paleta[unsigned(img(i,j))][1];
            ret(i,j,0,2)=paleta[unsigned(img(i,j))][2];
        }
    }

    return ret;
}

///****************************************
///COMPLEMENTO COLOR
///****************************************
///la idea es girar el H 180 grados en todos sus puntos(como decia Rena)
///  de la circunfencia del plato de color
/// y e invertir la intensidad
template <class T>
CImg<T> complemento_color(CImg<T> img){
    img.RGBtoHSI();
    cimg_forXY(img,i,j){
        img(i,j,0,0)+=180;
        if(img(i,j,0,0) > 360)
            img(i,j,0,0)=(img(i,j,0,0)-360);
        img(i,j,0,2)=1-img(i,j,0,2);//Hace el negativo de la intensidad
    }
    img.HSItoRGB();
    return img;
}



template <class T>
CImg<T>  ComposeHSI(CImg<T> h, CImg<T> s, CImg<T> I){
    CImg<T> img;
    int ww=h.width(), hh=h.height(), i, j;
    img.assign(ww, hh, 1, 3);
    img.RGBtoHSI();
    for(i=0; i<ww; i++){
        for(j=0; j<hh; j++){
            img(i,j,0,0)=h(i,j);
            img(i,j,0,1)=s(i,j);
            img(i,j,0,2)=I(i,j);
        }
    }
    img.HSItoRGB();
    return img;
}

///****************************************
/// Color Slicing RGB
///****************************************
template <class T>
CImg<T> ColorMaskRGB(CImg<T> img, unsigned char x, unsigned char y, float radio){
    unsigned ww=img.width();
    unsigned hh=img.height();
    T r0, g0, b0;
    r0=img(x, y, 0, 0);
    g0=img(x, y, 0, 1);
    b0=img(x, y, 0, 2);
    float dist;
    for(unsigned i=0; i<ww; i++){
        for(unsigned j=0; j<hh; j++){
            dist=sqrt(powf(r0-img(i,j,0,0),2)+powf(g0-img(i,j,0,1),2)+powf(b0-img(i,j,0,2),2));
            if(dist>radio){
                img(i,j,0,0)=0;
                img(i,j,0,1)=0;
                img(i,j,0,2)=0;
            }
        }
    }
    return img;
}
///****************************************
/// Color Slicing RGB a partir de un pixel (Sobrecarga)
///****************************************
template <class T>
CImg<T> ColorMaskRGB(CImg<T> img,CImg<T> color, float radio){
    unsigned ww=img.width();
    unsigned hh=img.height();
    T r0, g0, b0;
    r0=color(0, 0, 0, 0);
    g0=color(0, 0, 0, 1);
    b0=color(0, 0, 0, 2);
    float dist;
    for(unsigned i=0; i<ww; i++){
        for(unsigned j=0; j<hh; j++){
            dist=sqrt(powf(r0-img(i,j,0,0),2)+powf(g0-img(i,j,0,1),2)+powf(b0-img(i,j,0,2),2));
            if(dist>radio){
                img(i,j,0,0)=0;
                img(i,j,0,1)=0;
                img(i,j,0,2)=0;
            }
        }
    }
    return img;
}


///****************************************
/// Color Slicing HSI
///****************************************
template <class T>
CImg<T> ColorMaskHSI(CImg<T> img, unsigned mx, unsigned my, float radio){
    img.RGBtoHSI();
    T h=img(mx, my, 0, 0);//h
    T s=img(mx, my, 0, 1);//s
    T i=img(mx, my, 0, 2);//I
    unsigned ww=img.width();
    unsigned hh=img.height();
    float dist;
    for(unsigned i=0; i<ww; i++){
        for(unsigned j=0; j<hh; j++){
            dist=sqrt(powf(h-img(i,j,0,0),2)+powf(s-img(i,j,0,1),2));
            if(dist>radio){
                img(i,j,0,0)=0;//h
                img(i,j,0,1)=0;//s
                img(i,j,0,2)=0;//I
            }
        }
    }
    return img.HSItoRGB();
}

///****************************************
/// Color Slicing HSI a partir de un pixel (Sobrecarga)
///****************************************
template <class T>
CImg<T> ColorMaskHSI(CImg<T> img,CImg<T> color, float radio){
    img.RGBtoHSI();
    color.RGBtoHSI();
    T h=color(0, 0, 0, 0);//h
    T s=color(0, 0, 0, 1);//s
    T i=color(0, 0, 0, 2);//I
    unsigned ww=img.width();
    unsigned hh=img.height();
    float dist;
    for(unsigned i=0; i<ww; i++){
        for(unsigned j=0; j<hh; j++){
            dist=sqrt(powf(h-img(i,j,0,0),2)+powf(s-img(i,j,0,1),2));
            if(dist>radio){
                img(i,j,0,0)=0;//h
                img(i,j,0,1)=0;//s
                img(i,j,0,2)=0;//I
            }
        }
    }
    return img.HSItoRGB();
}

///****************************************
/// GreySlicing
///****************************************
template <class T>
CImg<T> GreyMask(CImg<T> img, unsigned char x, unsigned char y, float radio){
    unsigned ww=img.width();
    unsigned hh=img.height();
    T g0;
    g0=img(x, y, 0, 0);
    float dist;
    for(unsigned i=0; i<ww; i++){
        for(unsigned j=0; j<hh; j++){
            dist=sqrt(powf(g0-img(i,j,0,0),2));
            if(dist>radio){
                img(i,j,0,0)=0;
            }
        }
    }
    return img;
}

///****************************************
///COLOURSLICING INTERACTIVO
///****************************************
//Devuelve una imagen binaria
template<typename T>
CImg<T> colourslicing(CImg<T> imagen,int radio=10){
    CImg<T> final(imagen.width(),imagen.height(),1,1,0);
    CImgDisplay v1(imagen,"Presione sobre el gris deseado"),v2(final,"Resultado");

    while(!v1.is_closed() || !v2.is_closed()){
        v1.wait();
        if(v1.button()==1){

            int mx=v1.mouse_x();
            int my=v1.mouse_y();

            final = ColorMaskHSI(imagen,mx,my,radio);

            v2.render(final);
            v2.paint();
        }
    }

    return final;
}

///****************************************
///BALANCE DE COLOR RGB
///****************************************
template<typename T>
CImg<T> balancecolorRGB(CImg<T> img_orig,int canal,int aumento){
    cimg_forXYZ(img_orig,i,j,k)
            img_orig(i,j,k,canal)+=aumento;
    return img_orig;
}

///****************************************
///BALANCE DE COLOR RGB CONDICIONAL (AUMENTO UN UNICO CANAL)
///****************************************
template<typename T>
CImg<T> balancecolorRGB_condicional(CImg<T> img_orig,int canal,int aumento){
    //Banderas para prueba
    int izq,der;
    if(canal == 0){
        //canales restantes (R,G o B)
        izq = 1;
        der = 2;
    }
    if(canal == 1){
        //canales restantes (R,G o B)
        izq = 0;
        der = 2;
    }
    if(canal == 2){
        //canales restantes (R,G o B)
        izq = 0;
        der = 1;
    }

    cimg_forXYZ(img_orig,i,j,k){
        if(img_orig(i,j,k,canal)>=img_orig(i,j,k,izq) && img_orig(i,j,k,canal)>=img_orig(i,j,k,der)){
            img_orig(i,j,k,canal)+=aumento;
            img_orig(i,j,k,izq)-=aumento/2;
            img_orig(i,j,k,der)-=aumento/2;}
        else{
            //img_orig(i,j,k,izq)+=aumento/2;
            //img_orig(i,j,k,der)+=aumento/2;
        }
    }
    return img_orig;
}

///****************************************
///COPIA CANAL A CANAL
///****************************************
//Sirve para no tener que escribir los ciclos en el main
template<typename T>
CImg<T> copia_canal(CImg<T> img_orig,int canal,CImg<T> img_a_copiar,int canal_a_copiar){
    cimg_forXYZ(img_orig,i,j,k)
            img_orig(i,j,k,canal) = img_a_copiar(i,j,k,canal_a_copiar);
    return img_orig;
}


///----------------FILTRADO FRECUENCIAL------------------------

///****************************************
///TRANSFORMADA DE FOURIER
///****************************************
//devuelve la magnitud la fase la magnitud con escala logaritmica con shift y la fase con escala logaritmica con shift
template<typename T>
CImgList<T> fourier(CImg<T> img){

    CImgList<T>TDF_img=img.get_FFT();
    CImg<T>TDF_real=TDF_img[0];
    CImg<T>TDF_imaginaria=TDF_img[1];

    complex<T>I(0.0,1.0);

    CImg<T>magnitud(img.width(),img.height(),1,1);
    CImg<T>fase(img.width(),img.height(),1,1);

    CImg<T>magnitud_log(img.width(),img.height(),1,1);
    CImg<T>fase_log(img.width(),img.height(),1,1);

    for (int i=0;i<img.width();i++){
        for (int j=0;j<img.height();j++){
            magnitud(i,j)=sqrt(pow(TDF_real(i,j),2)+pow(TDF_imaginaria(i,j),2));
            complex<T>complejo=TDF_real(i,j)+I*TDF_imaginaria(i,j);
            fase(i,j)=arg(complejo);

            //Calculo la magnitud y fase para mostrar en el display()
            magnitud_log(i,j) = log(magnitud(i,j) + 1);
            fase_log(i,j) = log(fase(i,j) + 1);

        }
    }


    magnitud_log = magnitud_log.get_shift(img.width()/2,img.height()/2,0,0,2);
    fase_log = fase_log.get_shift(img.width()/2,img.height()/2,0,0,2);


    CImgList<T> lista(magnitud,fase,magnitud_log,fase_log);

    return lista;
}

///****************************************
///TRANSFORMADA DE FOURIER INVERSA
///****************************************
//devuelve la magnitud la fase la magnitud con escala logaritmica con shift y la fase con escala logaritmica con shift
template<typename T>
CImg<T> fourier_inv(CImg<T> magnitud,CImg<T> fase){

    CImg<T> realfft(magnitud.width(),magnitud.height(),1,1);
    CImg<T> imaginariafft(fase.width(),fase.height(),1,1);

    complex<T>I(0.0,1.0);
    for (int i=0;i<magnitud.width();i++){
        for (int j=0;j<magnitud.height();j++){
            realfft(i,j)=real(magnitud(i,j)*exp(I*fase(i,j)));
            imaginariafft(i,j)=imag(magnitud(i,j)*exp(I*fase(i,j)));

        }
    }

    CImgList<T> orig(magnitud,fase);
    orig[0] = realfft;
    orig[1] = imaginariafft;
    return orig.get_FFT(true)[0];
}

///****************************************
///FILTRADO EN FRECUENCIA - APLICAR FILTRO
///****************************************
//Filtra en frecuencia a partir de una imagen y un filtro
template<typename T>
CImg<T> filtrar(CImg<T> img,CImg<T> filt){
    CImgList<T> img_tr = fourier(img);

    //Acomodo la mascara--> SHIFT de respuesta en frecuencia del filtro antes de multiplicar con la FFT de la img
    filt.shift(filt.width()/2,filt.height()/2,0,0,2);

    return fourier_inv(img_tr.at(0).mul(filt),img_tr.at(1));
}

//Para RGB
template<typename T>
CImg<T> filtrar3(CImg<T> img,CImg<T> filt){
    CImg<T> r,g,b,final(img.width(),img.height());

    r=filtrar(img.get_channel(0),filt);
    g=filtrar(img.get_channel(1),filt);
    b=filtrar(img.get_channel(2),filt);

    ComposeRGB(final,r,g,b);

    return final;
}

///****************************************
///FILTRO IDEAL
///****************************************
template <class T>
CImg<T> ideal_mask(CImg<T> &img,T d, bool highpass=false){
    int i, j, w=img.width(), h=img.height(), w_2=w/2, h_2=h/2;
    CImg<T> mask(w, h, 1, 1);
    d=d*d;

    for(i=0; i<w; i++){
        for(j=0; j<h; j++){
            if((powf(i-w_2,2)+powf(j-h_2,2))<=d){
                mask(i, j)=!highpass;
            }else{
                mask(i, j)=highpass;
            }

        }
    }
    return mask;
}

///****************************************
///FILTROS BUTTER
///****************************************
template <class T>
CImg<T> butterworth_mask(CImg<T> &img, T d,unsigned o, bool highpass=false){
    int i, j, w=img.width(), h=img.height(), w_2=w/2, h_2=h/2;
    CImg<T> mask(w, h, 1, 1);
    float dist;

    for(i=0; i<w; i++){
        for(j=0; j<h; j++){
            dist=sqrt(powf(i-w_2,2)+powf(j-h_2,2));
            mask(i,j)=1/(1+powf(highpass?d/dist:dist/d,2*o));

        }
    }
    return mask;
}

///****************************************
///FILTROS GAUSSIANOS
///****************************************
template <class T>
CImg<T> gaussian_mask(CImg<T> img, T d, bool highpass=false){
    int i, j, w=img.width(), h=img.height(), w_2=w/2, h_2=h/2;
    CImg<T> mask(w, h, 1, 1);
    float dist;

    for(i=0; i<w; i++){
        for(j=0; j<h; j++){
            dist=sqrt(powf(i-w_2,2)+powf(j-h_2,2));
            mask(i,j)=exp(-powf(dist,2)/(2*powf(d,2)));
            if(highpass) mask(i,j)=1-mask(i,j);
        }
    }
    return mask;
}

///****************************************
///FILTROS PASABANDA IDEAL
///****************************************
template <class T>
CImg<T> ideal_band(CImg<T> &img,T d,T W,bool pass=false){
    //Preproceso d para que la frecuencia sea la centrada en W/2
    d=d-W/2;

    if(pass){
        return multiplicacion(ideal_mask(img,d+W),ideal_mask(img,d,true));//pasabanda
    }
    else{
        return sumaImg(ideal_mask(img,d),ideal_mask(img,d+W,true));//rechazabanda
    }
}

///****************************************
///FILTROS PASABANDA BUTTER
///****************************************
template <class T>
CImg<T> butter_band(CImg<T> &img,T d,T W,unsigned o,bool pass=false){
    //corro de para que este en un borde y no en el centro de la frecuencia del filtro
    d=d-W/2;

    if(pass){
        return multiplicacion(butterworth_mask(img,d+W,o),butterworth_mask(img,d,o,true));//pasabanda
    }
    else{
        return sumaImg(butterworth_mask(img,d,o),butterworth_mask(img,d+W,o,true));//rechazabanda
    }
}

///****************************************
///FILTROS PASABANDA GAUSSIANOS
///****************************************
template <class T>
CImg<T> gaussian_band(CImg<T> &img,T d,T W,bool pass=false){
    //corro de para que este en un borde y no en el centro de la frecuencia del filtro
    d=d-W/2;

    if(pass){
        return multiplicacion(gaussian_mask(img,d+W),gaussian_mask(img,d,true));//pasabanda
    }
    else{
        return sumaImg(gaussian_mask(img,d),gaussian_mask(img,d+W,true));//rechazabanda
    }
}

///****************************************
///FILTROS NOTCH IDEAL
///*********NOTimg(*******************************
template <class T>
CImg<T> ideal_notch(CImg<T> &img,T d,T u0,T v0,bool notch=false){
    int i, j, w=img.width(), h=img.height(), w_2=w/2, h_2=h/2;
    CImg<T> mask(w, h, 1, 1);

    for(i=0; i<w; i++){
        for(j=0; j<h; j++){
            if(sqrt(powf(i-w_2-u0,2)+powf(j-h_2-v0,2))<=d || sqrt(powf(i-w_2+u0,2)+powf(j-h_2+v0,2))<=d){
                mask(i, j)=!notch;
            }else{
                mask(i, j)=notch;
            }

        }
    }
    return mask;
}

///****************************************
///FILTROS NOTCH BUTTER
///****************************************
template <class T>
CImg<T> butter_notch(CImg<T> &img,T d,unsigned o,T u0,T v0,bool notch=false){
    int i, j, w=img.width(), h=img.height(), w_2=w/2, h_2=h/2;
    CImg<T> mask(w, h, 1, 1);
    float dist1,dist2;

    for(i=0; i<w; i++){
        for(j=0; j<h; j++){
            dist1=sqrt(powf(i-w_2-u0,2)+powf(j-h_2-v0,2));
            dist2=sqrt(powf(i-w_2+u0,2)+powf(j-h_2+v0,2));
            mask(i,j)=1/(1+powf(notch?(powf(d,2)/(dist1*dist2)):((dist1*dist2)/powf(d,2)),o));

        }
    }
    return mask;
}

///****************************************
///FILTROS NOTCH GAUSSIANOS
///****************************************
template <class T>
CImg<T> gaussian_notch(CImg<T> &img,T d,T u0,T v0,bool notch=false){
    int i, j, w=img.width(), h=img.height(), w_2=w/2, h_2=h/2;
    CImg<T> mask(w, h, 1, 1);
    float dist1,dist2;

    for(i=0; i<w; i++){
        for(j=0; j<h; j++){
            dist1=sqrt(powf(i-w_2-u0,2)+powf(j-h_2-v0,2));
            dist2=sqrt(powf(i-w_2+u0,2)+powf(j-h_2+v0,2));
            mask(i,j)=exp(-(dist1*dist2)/(2*powf(d,2)));
            if(notch) mask(i,j)=1-mask(i,j);
        }
    }
    return mask;
}


///****************************************
///FILTROS LAPLACIANOS
///****************************************
template <class T>
CImg<T> laplacian_mask(CImg<T> &img){
    int i, j, w=img.width(), h=img.height(), w_2=w/2, h_2=h/2;
    CImg<T> mask(w, h, 1, 1);

    for(i=0; i<w; i++){
        for(j=0; j<h; j++){
            mask(i,j)=-(powf(i-w_2,2)+powf(j-h_2,2));
        }
    }
    return mask;
}


///****************************************
/// FILTRO HOMOMORFICO
//@ Genera un filtro homomorfico
///****************************************
template <class T>
CImgList<T> filtroHomomorfico(unsigned int w, unsigned int h, unsigned int frec_corte, T gamma_l, T gamma_h, T c) {
    CImgList<T> img(2, w, h, 1, 1, 0);

    unsigned int centro_x = w/2;
    unsigned int centro_y = h/2;
    unsigned int fcp = pow(frec_corte,2);

    cimg_forXY(img[0],x,y) {
        double distancia = ( (x - centro_x)*(x - centro_x) + (y - centro_y)*(y - centro_y));
        double argumento = -c * distancia/fcp;

        img[0](x,y) = (gamma_h - gamma_l) * (1 - exp (argumento)) + gamma_l;
    }
    img[0].display(); //display para ver H
    return img;
}



//Toma una imagen color retorna si imagen en gris
CImg<double> colorToBW(CImg<double> img) {
    if (img.spectrum() == 1) return img;
    else return img.get_RGBtoHSI().get_channel(2);
}

//DEFINICION DE FILTRO HOMOMORFICO
//float gammaH=0.98,
//	gammaL=0.9;
//float D0=0.7;
//float c=1;

//_frec_corte = 150;
// gamma_l = 0.9;
//gamma_h = 1.1
// c = 1.0
///****************************************
/// WRAPPER FILTRO HOMOMORFICO
//est wrapper aplica los pasos nesesarios en el filtrado homomorfico , ademas de la definicion del filtro
//que se define en la funcion anterior
///****************************************
CImg<double> filtradoHomomorficoCompleto(CImg<double> img, unsigned int frec_corte, float gamma_l, float gamma_h, float c) {
    int w=img.width(), h=img.height();
    CImgList<double> filtro_homomorfico = filtroHomomorfico(w, h,frec_corte,gamma_l,gamma_h,c);
    //filtro_homomorfico[0].display();
    //Calculamos el logaritmo de la imagen
    CImg<double> log_img(w, h, 1, 1, 0);

    cimg_forXY(img,x,y) {
        if (img(x,y) < 1.0) //Si el logaritmo es menor a 1, explota por lo tanto controlo eso y asigno cero si se da el caso
            log_img(x,y) = 0.0;
        else
            log_img(x,y) = log(img(x,y));
    }
    //Filtramos (devuelve el resultado antitransformado)
    CImg<float> resultado_filtrado = filtrar(log_img,filtro_homomorfico.at(0));
    //Exponenciamos
    CImg<double> resultado_exp = resultado_filtrado.get_normalize(0,1).get_exp().get_normalize(0,255);
    //equlize
    CImg<double> resultado_filtrado_equ = resultado_exp.get_equalize(256);
    return resultado_filtrado_equ;

}

///****************************************
///FILTRO DE ENFASIS DE ALTA FRECUENCIA
///****************************************
//Recibe los parametros a = (A-1) y b. 
// Si b = 1 el resultado es un filtro de alta potencia.
// si b> 1 con b>a, entonces obtengo como resultado el filtro de enfasis en alta potencia. 
template <class T>
CImg<T> filtroAP_frecuencia(CImg<T> img,T alpha,T b) {

    //img, frecuencia de corte (D0) y bandera = true [Highpass filter]
    //En este caso uso el gaussiano pero puede ser tambien ideal, butter o laplaciano
    T frec_corte = img.width()/4.0; //nyquest / 2---> (w/2)/2

    CImg<T> filtro_PA;

    filtro_PA = gaussian_mask(img,frec_corte,true);

    CImg<T> Resultado,filtro_frec;

    //Calculo el filtro AP o EAF en funcion de los parametros alpha y b
    filtro_frec = alpha + b*filtro_PA;

    //Filtro y obtengo el resultado
    Resultado = filtrar(img,filtro_frec);

    return Resultado;

}

template <class T>
CImg<T> filtroAP3_frecuencia(CImg<T> img,T alpha,T b) {
    CImg<T> c1,c2,c3;

    //Aplico alta potencia a cada canal por separado
    c1 = img.get_RGBtoHSI().channel(0);
    c2 = img.get_RGBtoHSI().channel(1);
    c3 = filtroAP_frecuencia(img.get_RGBtoHSI().channel(2),alpha,b);

    //Devuelvo la suma de canales
    return ComposeHSI(c1,c2,c3);
}


///****************************************
///FILTRO DE FILTRADO DE RUIDO (DENOISE)
///****************************************

//media geometrica
template <class T>
T media_geometrica(CImg<T> window){
    T val=1.0;
    cimg_forXY(window,x,y){
        val*=window(x,y);
    }
    val=pow(val,T(1)/(window.width()*window.height()));
    return val;
}

//media contraarmonica
template <class T>
T media_carmonica(CImg<T> window,int Q){
    T val1=0.0;
    T val2=0.0;
    cimg_forXY(window,x,y){
        val1+=pow(window(x,y),Q+1);
        val2+=pow(window(x,y),Q);
    }

    return val1*1.0/(val2*1.0);
}

//mediana
template <class T>
T mediana(CImg<T> window){

    vector<T> v;
    cimg_forXY(window,x,y)
            v.push_back(window(x,y));
    sort(v.begin(),v.end());//ordeno
    if(v.size()%2!=0) // si el numero de elementos  de la ventana es impar
        return v.at((v.size()+1)/2);
    else //si el numero de elementos de la ventana es par...
        //es nesesario este caso o la ventana sera siempre de cantidad impar???
        return v.at(v.size()/2);
}

//auxiliar para moda
template <class T>
T most_appeared(vector<T> v){
    int contador = 1;
    int max = 0;
    int valor;
    for (int i = 1; i = v.size()-1;i++){
        if(v[i] == v[i-1]){
            contador+=1;//Si el numero actual es igual al anterior aumento el contador
        }
        else
        {
            contador = 1;//Si el numero actual es otro reseteo el contador
        }

        if(contador > max){
            max = contador;
            valor = v[i];
        }
    }

    return valor;
}

//moda
template <class T>
T moda(CImg<T> window){
    vector<T> v;
    cimg_forXY(window,x,y)
            v.push_back(window(x,y));
    sort(v.begin(),v.end());//ordeno
    //Cuento cual es el valor que mas aparece
    return most_appeared(v);

}

//maximo
template <class T>
T maximo(CImg<T> window){
    return window.max();
}

//minimo
template <class T>
T minimo(CImg<T> window){
    return window.min();
}

//punto medio
template <class T>
T punto_medio(CImg<T> window){
    return 	(window.min()+window.max())/2;
}

//media alfarecortado
template <class T>
T media_alfarecortado(CImg<T> window,int d){
    vector<T> v;
    cimg_forXY(window,x,y){
        v.push_back(window(x,y));
    }
    //Tiene que poder borrar desde arriba y abajo
    sort(v.begin(),v.end());
    for (int i=0;i<d/2;i++){
        if (!v.empty()){v.erase(v.begin());}
        v.pop_back();
    }

    T val=0.0;
    for(int i=0;i<v.size();i++){
        val+=v[i];
    }

    return val/(T)v.size();

}

//ecualizacion local
template <class T>
T equalize_local(CImg<T> window){
    int N=window.width(),M=window.height();
    window.equalize(256);
    return window(N/2,M/2);

}

//filtro adaptativo
template <class T>
T filtro_adaptativo(T g,CImg<T> window,double varianza,double tolerancia){
    if(fabs(window.variance()-varianza) <= tolerancia){
        //cout<<"La diferencia de varianza es: "<<fabs(window.variance()-varianza)<<endl;
        return window.mean();
    }else{
        return (g-((varianza/window.variance())*(g-window.mean())));
    }
}

double distancia(double v1,double v2,int norma){
    switch(norma){
    case 1:
        return fabs(v1-v2);
        break;
    case 2:
        return powf(powf(v1,2)-powf(v2,2),1/2);//Norma 2
        break;
    }
}

double filtro_distancias(CImg<double> window,int d){
    CImg<double> R(window.width(),window.height());

    //Armo la matriz R de distancias
    cimg_forXY(window,i,j){
        //Uso la norma 2 como calculo de distancia
        cimg_forXY(window,s,t){
            R(i,j)+=distancia(window(i,j),window(s,t),1);
        }
    }
    //Veo cual es la menor distancia y devuelvo el valor de la ventana en esas coordenadas
    cimg_forXY(R,i,j){
        if(R(i,j)==R.min()){
            return window(i,j);//Devuelvo el valor de la ventana con minima distancia
        }
    }
}


//int tipofiltro : 1 MEDIA GEOMETRICA , 2 MEDIA CONTRAARMONICA
//                 3 MEDIANA, 4 PUNTO MEDIO ,5 PUNTO MEDIO RECORTADO,6 MAX, 7 MIN



///media armonica= para ruido sal (malo para pimienta), bueno para gaussiano
///1.media geometrica= bueno ruido gaussiano
///2.media contra armonica
//el parametro Q para MEDIA CONTRAARMONICA:
//Q=-1 media armonica
//Q=0 media aritmetica
//Q>0 = elimina pimienta
//Q<0 =elimina sal
///3.mediana= ruido impulsivo (sin desenfoque)
///moda = ruido impulsivo (malo para otro tipo de ruido)
///4.punto medio = ruido gaussiano o uniforme
///5.media alfa recortado= Comportamiento situado entre la media aritmética y la mediana,dependiendo del valor de d
///6.max = ruido sal
///7.minimo = ruido pimienta
///9.moda = ruido impulsivo
///10. filtro adaptativo = usa d como tolerancia de la varianza (valor pequeño) y pide la seleccion de un area homogenea de la imagen
///11. distancias = para imagenes de color
///12. alfa recortado = para imagenes de color

template <class T>
CImg<T> denoise(CImg<T> img,int sizew,int tipofiltro,int Q=0,int d=0){

    int N=img.height(),M=img.width();
    int medio=sizew/2;//posicion del medio de la ventana
    CImg<T> imgout,window(sizew,sizew);

    //Para cuando se usa la funcion distancia en imagenes a color
    if(tipofiltro == 11 || tipofiltro == 12){
        //Es color declaro multicanal
        imgout.assign(M,N,1,3);
    }else{
        imgout.assign(M,N);
    }

    //Parametros para el adaptativo
    double varianza;
    if(tipofiltro == 10){
        //Capturar una zona homogenea para calcular la media y varianza del ruido
        CImgDisplay v1(img,"Presione dos puntos que formen un rectangulo en una zona homogenea");

        int mx1=0,my1=0,mx2=0,my2=0;

        while(!v1.is_closed()){
            v1.wait();
            if(v1.button()==1){

                mx2=mx1;
                my2=my1;

                mx1=v1.mouse_x();
                my1=v1.mouse_y();

            }
        }
        //Calculo la varianza del ruido
        varianza = img.get_crop(mx1,my1,mx2,my2).variance();
        cout<<"La varianza del ruido es: "<<varianza<<endl;
        if (varianza == 0.0){return img;}//Caso trivial
    }


    for (int x=medio;x<M-medio;x++){
        for(int y=medio;y<N-medio;y++){
            //asigno datos a mi ventana
            window=img.get_crop(x-medio,y-medio,x+medio,y+medio);
            switch(tipofiltro){
            case 1:
                imgout(x,y)=media_geometrica(window);
                break;
            case 2:
                imgout(x,y)=media_carmonica(window,Q);
                break;
            case 3:
                imgout(x,y)=mediana(window);
                break;
            case 4:
                imgout(x,y)=punto_medio(window);
                break;
            case 5:
                imgout(x,y)=media_alfarecortado(window,d);
                break;
            case 6:
                imgout(x,y)=maximo(window);
                break;
            case 7:
                imgout(x,y)=minimo(window);
                break;
            case 8:
                imgout(x,y)=equalize_local(window);
                break;
            case 9:
                imgout(x,y)=moda(window);
                break;
            case 10:
                imgout(x,y)=filtro_adaptativo(img(x,y),window,varianza,d);
                break;
            case 11:
                imgout(x,y,0,0)=filtro_distancias(window.get_channel(0),d);
                imgout(x,y,0,1)=filtro_distancias(window.get_channel(1),d);
                imgout(x,y,0,2)=filtro_distancias(window.get_channel(2),d);
                break;
            case 12:
                imgout(x,y,0,0)=media_alfarecortado(window.get_channel(0),d);
                imgout(x,y,0,1)=media_alfarecortado(window.get_channel(1),d);
                imgout(x,y,0,2)=media_alfarecortado(window.get_channel(2),d);
                break;
            }

        }
    }

    imgout.crop(medio,medio,M-medio-1,N-medio-1);
    imgout.resize(M,N);
    return imgout;
}

template<class T>
CImg<T> filtrado_pseudoinverso(CImg<T> imagen,CImg<T> filtro,double tolerancia){
    //Creo el R a partir del filtro y el radio
    //Es como aplicar un pasabajos ideal al filtro de entrada
    CImg<T> R(filtro.width(),filtro.height());
    cimg_forXY(filtro,i,j){
        if(filtro(i,j) > tolerancia){
            R(i,j)=1/filtro(i,j);
        }else{
            R(i,j)=0;
        }
    }

    return filtrar(imagen,R);
}

template<class T>
CImg<T> filtrado_pseudoinverso3(CImg<T> imagen,CImg<T> filtro,double tolerancia){
    CImg<T> r,g,b,final(imagen.width(),imagen.height());

    r=filtrado_pseudoinverso(imagen.get_channel(0),filtro,tolerancia);
    g=filtrado_pseudoinverso(imagen.get_channel(1),filtro,tolerancia);
    b=filtrado_pseudoinverso(imagen.get_channel(2),filtro,tolerancia);

    ComposeRGB(final,r,g,b);

    return final;
}

///-----------------------SEGMENTACION-----------------------

///****************************************
///TRANSFORMADA DE HOUGH
///****************************************
Pixel MaximoP( CImg<double>& img ) {
    int xmax=0, ymax=0;
    double maximo = img(0,0);
    cimg_forXY(img, i, j) {
        if( img(i,j) > maximo ) {
            maximo = img(i,j);
            xmax = i;
            ymax = j;
        }
    }
    return Pixel(xmax, ymax, &img);
}
Pixel MinimoP( CImg<double>& img ) {
    int xmin=0, ymin=0;
    double minimo = img(0,0);
    cimg_forXY(img, i, j) {
        if( img(i,j) < minimo ) {
            minimo = img(i,j);
            xmin = i;
            ymin = j;
        }
    }
    return Pixel(xmin, ymin, &img);
}
///INVERSA (modificada)
///se pasa una transformada y numero de lineas acumulado maximo
CImg<double> InversaHough(CImg<double> img, int nLineas) {
    const unsigned M = img.width(),N = img.height();
    CImg<double> iHough(M, N, 1, 1, 0.0);
    double maxRho = sqrt(float(pow(N-1,2)+pow(M-1,2))), stepRho = 2.*maxRho/(N-1), stepTheta = M_PI/(M-1), rho, theta;
    const double blanco[1] = {255.f};
    float x0, x1, y0, y1;
    for( int nl=0; nl < nLineas; nl++ ) {
        Pixel p = MaximoP( img );
        int t = p.x, r = p.y;
        theta=t*stepTheta-M_PI/2;
        rho=r*stepRho-maxRho;
        if (theta>-M_PI/2 && theta<M_PI/2){
            y0=0; y1=M-1;
            x0=rho/cos(theta);
            x1=rho/cos(theta)-(M-1)*tan(theta);
        }
        else {
            x0=0; x1=N-1;
            y0=rho/sin(theta);
            y1=rho/sin(theta)-(N-1)/tan(theta);
        }
        iHough.draw_line(y0,x0,y1,x1,blanco);
        img(t,r) = 0;
    }
    return iHough;
}

///INVERSA (modificada)
///sobrecarga que devuelve el maximo de los acumuladores pero puede ser
///selectivo es decir max_nro indica si se quiere el segundo o el tercer
///maximo etc. Comenzando por el maximo nro 1

CImg<double> InversaHough_nmax(CImg<double> img, int max_nro=1) {
    if(max_nro < 1){exit(1);}//Error
    const unsigned M = img.width(),N = img.height();
    CImg<double> iHough(M, N, 1, 1, 0.0);
    double maxRho = sqrt(float(pow(N-1,2)+pow(M-1,2))), stepRho = 2.*maxRho/(N-1), stepTheta = M_PI/(M-1), rho, theta;
    const double blanco[1] = {255.f};
    float x0, x1, y0, y1;

    //Hace 0 los maximos que no corresponden
    for( int nl=0; nl < max_nro-1; nl++ ) {
        Pixel p = MaximoP( img );
        int t = p.x, r = p.y;
        img(t,r) = 0;
    }

    Pixel p = MaximoP( img );
    int t = p.x, r = p.y;
    theta=t*stepTheta-M_PI/2;
    rho=r*stepRho-maxRho;
    if (theta>-M_PI/2 && theta<M_PI/2){
        y0=0; y1=M-1;
        x0=rho/cos(theta);
        x1=rho/cos(theta)-(M-1)*tan(theta);
    }
    else {
        x0=0; x1=N-1;
        y0=rho/sin(theta);
        y1=rho/sin(theta)-(N-1)/tan(theta);
    }
    iHough.draw_line(y0,x0,y1,x1,blanco);

    return iHough;
}


////para detectar circulos
template <class T>
CImg<T> HoughCirculos(CImg<T> &img, int r) {
    int M = img.width(), N = img.height();
    CImg<double> iHough(M, N, 1, 1, 0.0);
    cimg_forXY(img, i, j){
        if (img(i, j)) {
            for (int a=0; a<M; ++a){
                double det = r*r-(i-a)*(i-a);
                if (det>0){
                    double b=round(j-sqrt(det));
                    if (b>0 && b<N)
                        iHough(a,b)++;
                }
            }
        }
    }
    return iHough;
}



template <typename T>
CImg<T> splitHough(CImg<T> Hough,double angulo = -180, double row_hough = -1,int ang_tol = 0,int rho_tol = 0,bool just_max = false) {
    // Si le pasamos -180 (por defecto), me devuelve toda la columna (sin importar ang_tol)
    if (angulo == -180) {
        angulo = 0;
        // si es menor o igual a 0, no tiene sentido guardarlo
        ang_tol = Hough.width() / 2;
    }
    double col_hough = (angulo + 90) / 180 * Hough.width();
    if (row_hough == -1) {
        row_hough = Hough.height() / 2;
        rho_tol = Hough.height() / 2;
    }
    // Guardo el maximo pico
    double max_pico = 0;
    double x_max_pico = 0;
    double y_max_pico = 0;
    CImg<T> auxiliar(Hough.width(), Hough.height(), 1, 1, 0);
    for (int i = -ang_tol ; i <= ang_tol ; i++) {
        for (int j = -rho_tol ; j <= rho_tol ; j++) {
            if (col_hough + i < Hough.width() && col_hough + i >= 0 &&
                    row_hough + j < Hough.height()&& row_hough + j >= 0) {
                x_max_pico = col_hough + i;
                y_max_pico = row_hough + j;
                if (just_max) {
                    if (Hough(col_hough + i, row_hough + j) > max_pico) {
                        max_pico = Hough(x_max_pico, y_max_pico);
                    }
                } else {
                    if (Hough(x_max_pico, y_max_pico) > 0) {
                        auxiliar(x_max_pico, y_max_pico) = Hough(x_max_pico, y_max_pico);
                        std::cout << auxiliar(x_max_pico, y_max_pico) << "(" << x_max_pico << "," << y_max_pico << ")"<< std::endl;
                    }
                }
            }
        }
    }
    if (just_max) {
        auxiliar(x_max_pico, y_max_pico) = Hough(x_max_pico, y_max_pico);
    } else { // Aplico un umbral
        auxiliar.threshold(0.3 * auxiliar.mean() + 0.7 * auxiliar.max());
    }
    return auxiliar;
}


/////  coordenada y al eje, me retorna el valor que correspende la transformada Hough en tita y rho
//// en grados para tita (t) entre [-90 ; 90] y entre [-sqrt(2)M;sqrt(2)M] el rho (p)
template <typename T>
double coordenadaXY_a_rho_tita(CImg<T> hough, int coord, unsigned char axis) {
    unsigned int M = hough.width();
    unsigned int N = hough.height();
    // Maximos valores absolutos de theta y de rho
    double max_theta = 90;
    double max_rho = pow(pow(M, 2.0) + pow(N, 2.0), 0.5);
    double valor;
    if (axis == 't') { // tita
        valor = (2.0 * coord / M - 1.0) * max_theta;
    } else if (axis == 'p') { //rho
        valor = (2.0 * coord / N - 1.0) * max_rho;
    }
    return valor;
}




///****************************************
///CRECIMIENTO DE REGIONES
///****************************************

///unsigned char la hice en unsigned char por que la funcion que dieron los profesores esta asi
/// supongo que es suficiente??
///
/// podria ponerla dentro de   autom_seg_region_growed y listo.
template <class T>
CImg<T> wrapper_region_growed(CImg<T> img,int x,int y,int delta,int etiqueta,CImg<T>&region_crecida, CImg<T>& segmentacion) {
    cout<<"x_rand:"<<x<<"y_rand:"<<y<<endl;
    CImg<T> aux;
    aux = region_growing(img, x, y, delta, etiqueta); //aux 3 channel

    region_crecida = img - aux;
    CImg<double> compuesta(img.width(),img.height(),1,3,0);
    CImg<double> aux_flood=region_crecida.get_channel(0).normalize(0,1);
    // region_crecida.get_channel(0).display("flooddddddddd");
    //aca tengo que pintar cada nueva zona de un color distinto.
    compuesta.RGBtoHSI();
    int color=rand()%360;
    cimg_forXY(compuesta,i,j){
        if(region_crecida(i,j)!=0)
            compuesta(i,j,0,0)=color;//h
        compuesta(i,j,0,1)=1;//s
        compuesta(i,j,0,2)=aux_flood(i,j);//I

    }
    compuesta.HSItoRGB();
    segmentacion= compuesta+segmentacion;
    // (img, region_crecida, segmentacion).display("imagen, Region fooldd, Seleccion", 0);


    img -= region_crecida;

    return img;
}


/// segmentacion automatica con region_growed
template <class T>
CImg<T> autom_seg_region_growed(CImg<T> img, int delta, int etiqueta, const int max_segm) {
    //if(img.spectrum()!=1) img=img.get_RGBtoHSI().get_channel(2);
    CImg<T> img_auto(img);
    CImg<T> region_crecida;//(img.width(), img.height(), 1, 3,0);
    CImg<T> segmentacion(img.width(), img.height(), 1, 3,0);
    //segmentacion.fill(0);
    //region_crecida.fill(0);
    int x_rand, y_rand;
    int segmentaciones = 0;
    /* initialize ransegmentacionesdom seed: */
    srand(time(0));
    while (img_auto.max() > 0 && segmentaciones < max_segm) {
        x_rand = rand() % img_auto.width();
        y_rand = rand() % img_auto.height();
        if (img_auto(x_rand,y_rand) != 0) {    //semilla en x,y en cada pasada Tiene que ser distinta de cero tiene que ser distinto de cero
            img_auto = wrapper_region_growed(img_auto, x_rand, y_rand, delta, etiqueta, region_crecida, segmentacion);
            segmentaciones++;
            cout<<segmentaciones<<endl;
        }
    }
    return segmentacion;
}

//segmentacion a partir de coordenadas de una mascara booleana- Va a dejar solo la region que evuelve las coordenadas
CImg<bool> segmenta_coord(CImg<bool> img,int x,int y){
    CImg<bool> final(img.width(),img.height(),1,1,0);
    final = region_growing(img,x,y,0,2);
    ///ToDo----------No anda!!!
    return final;
}

///Inundar
///Retorna una mascara con las regiones inundadas de acuerdo a los paramatros
//CImg<double> img : imagen en griz o bool
//umbral : recibe ubral de corte normalizado a rango [0,1]
// x, y posicion inicial para la semilla en img
//NOTA: al pasar los parametros tener en cuenta que el valor del pixel de semilla en img
// debe ser menor al umbral que se puso.
//si no es asi, va retornar una imagen vacia en false.
// img_cres= InundarInverso(img_nueva, 0.50,297,173 );
CImg<bool> Inundar( CImg<double> img, double umbral, int x, int y,int &cant_pixel) {
    img.normalize(0.0, 1.0);
    CImg<bool> mask(img.width(), img.height(), 1, 1, false );
    CImg<bool> procesado(img.width(), img.height(), 1, 1, false );
    deque<Pixel> cola;
    cola.push_back(Pixel(x, y, &img));
    //cola.push_back( MinimoP( img ) );
    procesado(cola.front().x, cola.front().y) = true;
    // img.display("Inundar");
    while( !cola.empty() ) {
        Pixel p = cola.front(); cola.pop_front();
        double a=p.value();
        if( p.value() > umbral ) {
            mask(p.x,p.y) = true;
            Pixel pix = p;
            pix.x++;
            if( pix.AdentroImagen() && !procesado(pix.x, pix.y) ) {
                cola.push_back( pix );
                procesado(pix.x, pix.y) = true;
                cant_pixel++;
            }
            pix.x -= 2;
            if( pix.AdentroImagen() && !procesado(pix.x, pix.y) ) {
                cola.push_back( pix );
                procesado(pix.x, pix.y) = true;
                cant_pixel++;
            }
            pix.x++; pix.y++;
            if( pix.AdentroImagen() && !procesado(pix.x, pix.y) ) {
                cola.push_back( pix );
                procesado(pix.x, pix.y) = true;
                cant_pixel++;
            }
            pix.y -= 2;
            if( pix.AdentroImagen() && !procesado(pix.x, pix.y) ) {
                cola.push_back( pix );
                procesado(pix.x, pix.y) = true;
                cant_pixel++;
            }
        }
    }
    return mask;
}


CImg<bool> MaximosLocales( CImg<double>& img, double limite ) {
    CImg<bool> maximo(img.width(), img.height(),1,1,true);
    cimg_forXY(img,i,j) {
        for(int dx=-1; dx<=1 && maximo(i,j); dx++) {
            for(int dy=-1; dy<=1 && maximo(i,j); dy++) {
                if(dx==0 && dy==0) continue;
                if( i+dx < 0 || i+dx >= img.width() || j+dy < 0 || j+dy >= img.height() ) continue;
                maximo(i,j) = maximo(i,j) && ( img(i,j) >= img(i+dx,j+dy) ) && img(i,j) > limite;
            }
        }
    }
    return maximo;
}


///****************************************
///MORFOLOGIA
///****************************************
/// TENER EN CUENTA QUE:
/// ANDimg es equivalente a la interseccion de conjuntos
/// ORimg es equivalente a la union de conjuntos
/// NOTimg es equivalente al complemento de un conjunto



///---------------------------------------
///EROSION n veces
///---------------------------------------
CImg<bool> nerode(CImg<bool> img,CImg<bool> ventana,int n){
    for(int i=0;i<n;i++){img.erode(ventana);}//Erosionamos
    return img;
}

///---------------------------------------
///APERTURA
///---------------------------------------
//A • B = (A ⊕ B) ⊖ B
CImg<bool> apertura(CImg<bool> img,CImg<bool> ventana){
    img.erode(ventana);//Erosionamos
    img.dilate(ventana);//Dilatamos
    return img;
}

///---------------------------------------
///CIERRE
///---------------------------------------
//A ◦ B = (A ⊖ B) ⊕ B
CImg<bool> cierre(CImg<bool> img,CImg<bool> ventana){
    img.dilate(ventana);//Dilatamos
    img.erode(ventana);//Erosionamos
    return img;
}

///---------------------------------------
///HIT OR MISS - LOCALIZACION DE CONJUNTO
///---------------------------------------
// A ⊛ B = (A ⊖ D) ∩ [A^c ⊖ (W − D)]
CImg<bool> HitorMiss(CImg<bool> A,CImg<bool> B,CImg<bool> X){
    CImg<bool> W=X.get_dilate(3);//Una ventana que encierra a X
    CImg<bool> D=DIFERENCIAimg(X,W);//Contorno que se formaria entre X y W
    CImg<bool> p1=A.erode(D);
    CImg<bool> p2=NOTimg(A).erode(DIFERENCIAimg(W,D));
    return ANDimg(p1,p2);
}

///---------------------------------------
///HIT OR MISS - LOCALIZACION DE CONJUNTO - Sobrecarga cuando no se requiere el fondo
///---------------------------------------
CImg<bool> HitorMiss(CImg<bool> A,CImg<bool> B){
    return A.get_dilate(B);//Es una simple erosion
}

///---------------------------------------
///EXTRACCION DE CONTORNOS
///---------------------------------------
CImg<bool> extraccion_de_contornos(CImg<bool> img,CImg<bool> ventana){
    //(A ⊕ B) − (A ⊖ B) = dilate | erode^c
    return DIFERENCIAimg(img.get_dilate(ventana),img.get_erode(ventana));
}

///---------------------------------------
///RELLENO SEMIAUTOMATICO INTERACTIVO
///---------------------------------------
//X_k = (X_k−1 ⊕ B) ∩ A c para  k = 1, 2, 3, . . .
CImg<bool> relleno_semiautomatico(CImg<bool> A,bool continuo=false){
    CImg<bool> final(A.width(),A.height(),1,1,0),B(3,3);
    CImgDisplay v1(A,"Presione sobre el lugar a rellenar"),v2(A,"Resultado");
    B.fill(0,1,0,1,1,1,0,1,0);

    while(!v1.is_closed() || !v2.is_closed()){
        v1.wait();
        if(v1.button()==1){

            int mx=v1.mouse_x();
            int my=v1.mouse_y();

            CImg<bool> X(A.width(),A.height(),1,1,0),X_Ant(A.width(),A.height(),1,1,0);
            X(mx,my)=1;

            X_Ant=X;
            X=DIFERENCIAimg(X.get_dilate(B),A);

            while(X_Ant != X){
                X_Ant=X;
                X=DIFERENCIAimg(X.get_dilate(B),A);
            }
            if(continuo){
                final = ORimg(final,ORimg(X,A));//Acumulo en final
            }else{
                final = ORimg(X,A);//Solo se rellena lo que pica el usuario y se reemplaza cada vez que se pica en otro lado
            }

            v2.render(final);
            v2.paint();
        }
    }
    return final;
}

///---------------------------------------
///COMPONENTES CONECTADAS INTERACTIVO
///---------------------------------------
//X_k = (X_k−1 ⊕ B) ∩ A   para k = 1, 2, 3, . . .
CImg<bool> componentes_conectadas(CImg<bool> A){
    CImg<bool> final,B(3,3);
    CImgDisplay v1(A,"Presione sobre el lugar a rellenar"),v2(A,"Resultado");
    B.fill(1,1,1,1,1,1,1,1,1);

    while(!v1.is_closed() || !v2.is_closed()){
        v1.wait();
        if(v1.button()==1){

            int mx=v1.mouse_x();
            int my=v1.mouse_y();

            CImg<bool> X(A.width(),A.height(),1,1,0),X_Ant;
            X(mx,my)=1;//seria el p inicial picado por el usuario
            X_Ant=X;
            X=ANDimg(X.get_dilate(B),A);

            while(X_Ant != X){
                X_Ant=X;
                X=ANDimg(X.get_dilate(B),A);
            }
            final = X;

            v2.render(final);
            v2.paint();
        }
    }
    return final;
}



///---------------------------------------
///CONVEXHULL
///---------------------------------------
//Retorna el convexhull de una imagen binaria (con posibilidad de limites)
//IMPORTANTE Esta programado para que haga el convexhull para blancos de una mascara
//si se desea hacer a partir de los negros hay que poner false en blanco
CImg<bool> ConvexHull(CImg<bool> A,bool blanco=true,bool limitar=false){
    CImg<bool> X,X_Ant,CHull(A.width(),A.height(),1,1,0);
    CImg<bool> B(3,3);
    vector< CImg<bool> > D;
    B.fill(0,1,1,0,0,1,0,1,1);
    if(!blanco){A=NOTimg(A);}
    if(limitar==false){
        for(int j = 0;j<4;j++){//Avanza sobre los B - SUPUESTAMENTE EL SIZE DE B DEBERIA SER 4 (Cada una de las orientaciones)
            //Inicializo el vector X para hacer los calculos del B teactual
            X_Ant=X=A;
            X=HitorMiss(X,B);
            while(X != X_Ant){
                X_Ant=X;
                X=HitorMiss(X,B);
            }
            D.push_back(ORimg(X,A));
            B.rotate(90);//roto B
        }
    }
    else{//No va a funcionar asi!!
        //Si no tengo que sobrepasar los limites
        B=NOTimg(B);
        for(int j = 0;j<4;j++){//Avanza sobre los B - SUPUESTAMENTE EL SIZE DE B DEBERIA SER 4 (Cada una de las orientaciones)
            //Inicializo el vector X para hacer los calculos del B actual
            X_Ant=X=A;
            X=HitorMiss(X,B);
            while(X != X_Ant){//Tengo en cuenta que no se pase de los limites
                X_Ant=X;
                X=HitorMiss(X,B);
            }
            D.push_back(ORimg(X,A));
            B.rotate(90);//roto B
        }
    }
    for(int j = 0;j<4;j++){
        CHull=ORimg(CHull,NOTimg(D[j]));
    }

    if(blanco){
        return NOTimg(CHull);
    }else{
        return CHull;
    }
}

///---------------------------------------
/// THINNING
///---------------------------------------
//Adelgaza la mascara
//A ⊗ B = A − (A ⊛ B) = A ∩ (A ⊛ B) c
CImg<bool> Thinning(CImg<bool> A){
    //A ⊗ B = A − (A ⊛ B) = A ∩ (A ⊛ B)^c
    CImg<bool> p1=A,p2,B(3,3);
    B.fill(0,0,0,0,1,0,1,1,1);

    for(int i = 0; i<8;i++){
        p2 = HitorMiss(p1,B);
        p1 = DIFERENCIAimg(p1,p2);
        B.rotate(45);
    }
}

///---------------------------------------
/// THICKENING
///---------------------------------------
//Engrosa la mascara
//A ⊙ B = A ∪ (A ⊛ B)
CImg<bool> Thickening(CImg<bool> A){
    //A ⊗ B = A − (A ⊛ B) = A ∩ (A ⊛ B)^c
    CImg<bool> p1=A,p2,B(3,3);
    B.fill(0,0,0,0,1,0,1,1,1);

    for(int i = 0; i<8;i++){
        p2 = HitorMiss(p1,B);
        p1 = ORimg(p1,p2);
        B.rotate(45);
    }
}

///---------------------------------------
///DILATACION GEODESICA con n ciclos (Reconstrucción morfológica)
///---------------------------------------
// dilatacion geodesica geodesica n ciclos
CImg<bool> dilatacion_geodesica(CImg<bool> G,int n){
    CImg<bool> final,B(3,3);
    CImgDisplay v1(G,"Presione sobre el lugar a rellenar"),v2(G,"Resultado");
    B.fill(1);

    while(!v1.is_closed() || !v2.is_closed()){
        v1.wait();
        if(v1.button()==1){

            int mx=v1.mouse_x();
            int my=v1.mouse_y();

            CImg<bool> F(G.width(),G.height(),1,1,0);
            F(mx,my)=1;//seria el p inicial picado por el usuario
            for(int j = 0;j<n;j++){
                F=ANDimg(F.get_dilate(B),G);
            }
            final = F;

            v2.render(final);
            v2.paint();
        }
    }
    return final;
}

///---------------------------------------
///EROSION GEODESICA con n ciclos (Reconstrucción morfológica)
///---------------------------------------
//Erosion Geodesica con n ciclos
CImg<bool> erosion_geodesica(CImg<bool> G,CImg<bool> F,int n){//F seria la mascara donde sintetiso a partir de A
    CImg<bool> final,B(3,3);
    B.fill(1);

    for(int j = 0;j<n;j++){
        F=ORimg(F.get_erode(B),G);
    }
    final = F;

    return final;
}

///---------------------------------------
///RECONSTRUCCION POR DILATACION (Reconstrucción morfológica)
///---------------------------------------
//Reconstruccion por dilatacion - No necesito mandarle el F porque lo creo adentro!!!
CImg<bool> reconstruccion_dilatacion(CImg<bool> G){
    CImg<bool> final,B(3,3);
    CImgDisplay v1(G,"Presione sobre el lugar a rellenar"),v2(G,"Resultado");
    B.fill(1);

    while(!v1.is_closed() || !v2.is_closed()){
        v1.wait();
        if(v1.button()==1){

            int mx=v1.mouse_x();
            int my=v1.mouse_y();

            CImg<bool> F(G.width(),G.height(),1,1,0),F_Ant;
            F(mx,my)=1;//seria el p inicial picado por el usuario
            F_Ant=F;
            F=ANDimg(F.get_dilate(B),G);
            while(F_Ant != F){
                F_Ant=F;
                F=ANDimg(F.get_dilate(B),G);
            }
            final = F;

            v2.render(final);
            v2.paint();
        }
    }
    return final;
}

//Reconstruccion por dilatacion - Sobrecarga (no es interactiva)
CImg<bool> reconstruccion_dilatacion(CImg<bool> G,CImg<bool> F){
    CImg<bool> final,B(3,3);
    B.fill(1);

    CImg<bool> F_Ant;
    F_Ant=F;
    F=ANDimg(F.get_dilate(B),G);
    while(F_Ant != F){
        F_Ant=F;
        F=ANDimg(F.get_dilate(B),G);
    }
    final = F;

    return final;
}

///---------------------------------------
///RECONSTRUCCION POR EROSION (Reconstrucción morfológica)
///---------------------------------------
//Reconstruccion por erosion
CImg<bool> reconstruccion_erosion(CImg<bool> G,CImg<bool> F){//F seria la mascara donde sintetiso a partir de A
    CImg<bool> final,B(3,3);
    B.fill(1);

    CImg<bool> F_Ant;
    F_Ant=F;
    F=ORimg(F.get_erode(B),G);
    while(F_Ant != F){
        F_Ant=F;
        F=ORimg(F.get_erode(B),G);
    }
    final = F;

    return final;
}

///---------------------------------------
///APERTURA POR RECONSTUCCION
///---------------------------------------
CImg<bool> apertura_reconstruccion(CImg<bool> G,CImg<bool> B,int n){
    CImg<bool> final;

    CImg<bool> F(G),F_Ant;
    for(int i=0;i<n;i++){
        F.get_erode(B);
    }
    F_Ant=F;
    F=ANDimg(F.get_dilate(B),G);
    while(F_Ant != F){
        F_Ant=F;
        F=ANDimg(F.get_dilate(B),G  );
    }
    final = F;

    return final;
}

///---------------------------------------
/// RELLENO AUTOMATICO
///---------------------------------------
//Devuelve la imagen binaria con los interiores rellenos (Relleno de agujeros automático)
CImg<bool> relleno_automatico(CImg<bool> I){
    CImg<bool> f(I.width(),I.height(),1,1,0),final;

    //Construyo f con los bordes de I
    cimg_forY(I,j){
        f(0,j)=1-I(0,j);//Arriba
        f(I.width()-1,j)=1-I(I.width()-1,j);//Abajo
    }
    cimg_forX(I,i){
        f(i,0)=1-I(i,0);//Izquierda
        f(i,I.height()-1)=1-I(i,I.height()-1);//Derecha
    }
    //Uso la reconstruccion por dilatacion (uso la sobrecarga)
    final = NOTimg(reconstruccion_dilatacion(NOTimg(I),f));

    return final;
}

///---------------------------------------
/// LIMPIEZA DE BORDES
///---------------------------------------
//Devuelve la imagen binaria con los bordes limpios de objetos cortados por los bordes de la imagens (Limpieza de objetos en el borde)
CImg<bool> limpieza_bordes(CImg<bool> I){
    CImg<bool> f(I.width(),I.height(),1,1,0),final;

    //Construyo f con los bordes de I
    cimg_forY(I,j){
        f(0,j)=I(0,j);//Arriba
        f(I.width()-1,j)=I(I.width()-1,j);//Abajo
    }
    cimg_forX(I,i){
        f(i,0)=I(i,0);//Izquierda
        f(i,I.height()-1)=I(i,I.height()-1);//Derecha
    }
    //Uso la reconstruccion por dilatacion (uso la sobrecarga)
    final = DIFERENCIAimg(I,reconstruccion_dilatacion(I,f));

    return final;
}


//Devuelve true si una mascara binaria tiene todos false
bool mascara_vacia(CImg<bool> mascara){
    cimg_forXY(mascara,i,j){
        if(mascara(i,j)==true){
            return false;
        }
    }
    return true;
}

///---------------------------------------
///ESQUELETO (SKELETOR)
///---------------------------------------
//skeletor definido como diapositivas de teoria sin usar libreria skeletor.h
//(usando la funciones de la libreria el skeletor.h queda mejor, ver ejemplo de uso de la libreria dentro de la carpeta)
//pdi-fich2015/Pruebas Parcial/use_skeleton

//Devuelve el esqueleto de una mascara binaria
CImg<bool> esqueleto(CImg<bool> A,CImg<bool> B){
    CImg<bool> esq(A.width(),A.height(),1,1,0),aux;
    int k=0;
    aux=nerode(A,B,k);
    while(!mascara_vacia(aux)){
        esq=ORimg(esq,DIFERENCIAimg(aux,apertura(aux,B)));
        k++;

        aux=nerode(A,B,k);
    }
    return esq;
}

//Aplicar una mascara booleana a una imagen en escala de grises
CImg<int> mul_mb_g(CImg<int> imagen,CImg<bool> mascara){
    cimg_forXY(imagen,i,j){
        if(mascara(i,j)==false)
            imagen(i,j)*=0;
    }
    return imagen;
}

//Aplicar una mascara booleana a una imagen en escala de grises
template <class T>
CImg<T> mul_mb_c(CImg<T> imagen,CImg<bool> mascara){
    cimg_forXY(imagen,i,j){
        if(mascara(i,j)==false){
            imagen(i,j,0,0)*=0;
            imagen(i,j,0,1)*=0;
            imagen(i,j,0,2)*=0;
        }
    }
    return imagen;
}



///Limpieza de objetos en los bordes
//Otra version
//recibe una imagen maskara y retorna la misma sin los objetos en el borde
// por defecto b=true : retorna solo elementos del borde de img el borde
// b=false : retorna img sin los elementos del borde
//NOTA: si no recorba bien los bordes controlar el umbral en la imagen CImg<bool> img que se le da como parametro de entrada
CImg<bool> bordes(CImg<bool> img, bool b=true){
    CImg<bool> mask(img.width()+2,img.height()+2,1,1,0);
    for(int i=1;i<mask.width()-1;i++)
        for(int j=1;j<mask.height()-1;j++)
            mask(i,j)=img(i-1,j-1);
    //Inundar inunda zonas true por eso hago NOTimg(mask) ->(por que el borde lo escribi en false, lo invierto a true)
    int cant_pixel;
    mask=Inundar(NOTimg(mask),0.5,0,0,cant_pixel);
    //quito el borde agregado
    mask.crop(1,1,mask.width()-2,mask.height()-2);
    mask = reconstruccion_dilatacion(mask,img);//Reconstruccion es la posta
    mask.display("maskara reco dilar");
    if (b)
        return NOTimg(mask);
    else
        return reconstruccion_dilatacion(ORimg(img,mask),img);
}


///****************************************
///EQUALIZACION DE HISTOGRAMA (TP FINAL)
///****************************************


///ECUALIZACION ESTANDAR
///
//! Equalize histogram of pixel values.
/**
   \param nb_levels Number of histogram levels used for the equalization.
   \param min_value Minimum pixel value considered for the histogram computation. All pixel values lower than \p min_value will not be counted.
   \param max_value Maximum pixel value considered for the histogram computation. All pixel values higher than \p max_value will not be counted.
   \note
   - If \p min_value==max_value==0 (default behavior), the function first estimates the whole range of pixel values
   then uses it to equalize the histogram.
   \par Example
   \code
   const CImg<float> img("reference.jpg"), res = img.get_equalize(256);
   (img,res).display();
   \endcode
   \image html ref_equalize.jpg
**/
template <class T>
CImg<T> ecualizar_comun(CImg<T> img,int niveles,T min=(T)0,T max=(T)0) {
    if(img.is_empty()){return img;}
    T vmin = min, vmax = max;
    if (vmin==vmax && vmin==0) vmin = img.min_max(vmax);
    if (vmin<vmax) {
        CImg<T> hist = img.get_histogram(niveles,vmin,vmax);
        unsigned long cumul = 0;
        cimg_forX(hist,pos) { cumul+=hist[pos]; hist[pos] = cumul; }
        cimg_for(img,ptrd,T) {
            const int pos = (int)((*ptrd-vmin)*(niveles-1)/(vmax-vmin));
            if (pos>=0 && pos<(int)niveles) *ptrd = (T)(vmin + (vmax-vmin)*hist[pos]/img.size());
        }
    }
    return img;
}


///ECUALIZACION BBHE
template <class T>
CImg<T> ecualizar_bbhe(CImg<T> img, int rango=256){
    double o_mean=floor(img.mean());
    CImg<T> histogram_lower(rango),histogram_upper(rango),nhistogram_lower(rango),nhistogram_upper(rango),histogram_lower_cdf(rango),histogram_upper_cdf(rango);
    histogram_lower.fill(0);histogram_upper.fill(0);nhistogram_lower.fill(0);nhistogram_upper.fill(0);histogram_lower_cdf.fill(0);histogram_upper_cdf.fill(0);

    //Calculo cada uno de las ecualizaciones
    cimg_forXY(img,i,j){
        if(img(i,j)<=o_mean){
            histogram_lower(img(i,j)+1)=histogram_lower(img(i,j)+1)+1;
        }else{
            histogram_upper(img(i,j)+1)=histogram_upper(img(i,j)+1)+1;
        }
    }

    //Armo el cdf de cada uno
    for(int i=0;i<nhistogram_lower.size();i++){
        nhistogram_lower(i)=histogram_lower(i)/(histogram_lower.sum()); //es un promedio
    }
    for(int i=0;i<nhistogram_upper.size();i++){
        nhistogram_upper(i)=histogram_upper(i)/(histogram_upper.sum()); //es un promedio
    }

    //Acumulado
    histogram_lower_cdf(0) = nhistogram_lower(0);
    histogram_upper_cdf(0) = nhistogram_upper(0);

    for (int k = 1;k<nhistogram_lower.size();k++){
        histogram_lower_cdf(k) =  histogram_lower_cdf(k-1) + nhistogram_lower(k);
        histogram_upper_cdf(k) =  histogram_upper_cdf(k-1) + nhistogram_upper(k);
    }

    //Armo la imagen final
    CImg<T> imagen_ecualizada(img.width(),img.height(),1,1,0);
    double rango_lower[2] = {0,o_mean};
    double rango_upper[2] = {(o_mean+1),rango-1};

    cimg_forXY(img,i,j){

        if(img(i,j)<=o_mean){
            imagen_ecualizada(i,j) = rango_lower[0] + round(((rango_lower[1]-rango_lower[0])*histogram_lower_cdf(img(i,j)+1)));
        }else{
            imagen_ecualizada(i,j) = rango_upper[0] + round(((rango_upper[1]-rango_upper[0])*histogram_upper_cdf(img(i,j)+1)));
        }
    }

    return imagen_ecualizada;
}


///ECUALIZACION CLAHE
template <class T>
CImg<T> clahe_histogram(CImg<T> img,vector<int> &nivel,int n_niveles,T tam_nivel,int posx,int posy,int tam_vent,T niv_clip) {

    int ini_vent_x = posx - (tam_vent / 2);
    int ini_vent_y = posy - (tam_vent / 2);

    T val_pix;
    int a, b, nuevo_niv;

    //Logica para armar las ventanas y los valores de los niveles
    //El histograma se reinicia cada vez que se arranca una fila nueva
    if (posy==0) { //Si esta en la primera columna
        //Llena de ceros el vector de niveles correspondiente al hist de la ventana
        for (int i = 0; i < n_niveles; i++){nivel[i] = 0;}

        //Arma la ventana
        for (int i = 0; i < tam_vent; i++) {
            for (int j = 0; j < tam_vent; j++) {
                a = ini_vent_x + i;
                b = ini_vent_y + j;

                if ((a >= 0 && a < img.width()) && (b >= 0 && b < img.height())) {
                    val_pix = img(a,b);
                }
                else {
                    val_pix = 0; // si esta fuera de los limites de la imagen (Condicion Dirichlet)
                }

                //Calculo el nuevo nivel
                nuevo_niv = (int)(val_pix / tam_nivel);

                //Para acomodar el maximo valor en el rango
                if (nuevo_niv == n_niveles)
                    nuevo_niv--;

                nivel[nuevo_niv] += 1;
            }
        }
    }
    else {//Si esta en las restantes filas
        //elimina la fila vieja
        b = ini_vent_y - 1;//Mueve la ventana como un overlap
        for (int j = 0; j < tam_vent; j++) {
            a = ini_vent_x + j;

            if ((a >= 0 && a < img.width()) && (b >= 0 && b < img.height())) {
                val_pix = img(a, b);
            }
            else {
                val_pix = 0;// si esta fuera de los limites de la imagen
            }

            nuevo_niv = (int)(val_pix / tam_nivel);

            // to accomodate the max value in the range
            if (nuevo_niv == n_niveles)
                nuevo_niv--;

            nivel[nuevo_niv] -= 1;
        }

        //agrega la fila nueva
        b = ini_vent_y + tam_vent - 1;
        for (int j = 0; j < tam_vent; j++) {
            a = ini_vent_x + j;

            if ((a >= 0 && a < img.width()) && (b >= 0 && b < img.height())) {
                val_pix = img(a, b);
            }
            else {
                val_pix = 0;// si esta fuera de los limites de la imagen
            }

            nuevo_niv = (int)(val_pix / tam_nivel);

            // to accomodate the max value in the range
            if (nuevo_niv == n_niveles)
                nuevo_niv--;

            nivel[nuevo_niv] += 1;
        }
    }

    //Armo el histograma final y controlo si hay clipping
    CImg<T> hist(n_niveles);
    T sobrante_niveles = 0;

    // calculating the relative frequency
    // and the total extra amount > clip level
    for (int i = 0; i < n_niveles; i++) {
        hist(i) = (T)nivel[i]/(T)(tam_vent*tam_vent);// estaba dividiendo por (T)cont
        //Aca limito o clipeo el nivel para que no supere cierto contraste
        if (hist(i) > niv_clip) {
            sobrante_niveles += (hist(i) - niv_clip);
            hist(i) = niv_clip;
        }
    }

    //ajusto en base al clipping calculado
    if (sobrante_niveles > 0) {
        T sobrante_por_nivel = T(sobrante_niveles /(T)n_niveles);
        for (int i = 0; i < n_niveles; i++) {
            hist(i) += sobrante_por_nivel;
        }
    }

    return hist;
}
template <class T>
CImg<T> ecualizar_clahe(CImg<T> img,int rango,int min,int max,int tam_vent,T niv_clip) {
    CImg<T> final(img.width(),img.height());
    final.fill(0);

    //Numero y tamaño de niveles segun parametros
    int n_niveles=max-min;
    T tam_niv = ceil(rango/n_niveles);

    //Creo el vector de histograma
    vector<int> nivel;
    nivel.reserve(n_niveles);

    //Si el valor de ventana no esta seteado
    if (tam_vent == -1) {
        tam_vent = (int)round(((img.width() < img.height()) ? img.width() : img.height()) / 25);
    }


    for (int i=0;i<img.width();i++) {
        for (int j=0;j<img.height();j++) {
            T val_pix=img(i,j);

            //Creo el histograma
            CImg<T> hist=clahe_histogram(img,nivel,n_niveles,tam_niv,i,j,tam_vent,niv_clip);

            //En que nivel cae segun el rango
            int pix_nuevo_rango=(int)(val_pix/tam_niv);

            //Para acomodar el maximo valor dentro del rango posible
            if (pix_nuevo_rango == n_niveles)
                pix_nuevo_rango--;

            //Calculo la PMF para la ventana procesada
            T pmf = 0;
            for (int k=0;k<=pix_nuevo_rango;k++) {
                pmf += hist(k);
            }

            T val_pix_nuevo = round(pmf*(T)255);//255 seria max en realidad

            final(i,j)= val_pix_nuevo;//Multiplico por 255 para escalarlo (no seria el max?)
        }
    }

    return final;
}


///LUT SIGMOIDEA

template<typename T>
CImg<T> lut_sigmoidea(T k,int ni,int ns){
    CImg<int> resultado(256);resultado.fill(0);
    cimg_forX(resultado,i){
        if(i<ns && i > ni){
            resultado(i)=i;
        }else{
            resultado(i) = i + k*(i / (1.0 + exp(-i)));

        }
    }

    resultado.normalize(0,255);
    return resultado;
}

///ECUALIZAR ACEBSF

template<typename T>
CImg<T> ecualizar_acebsf(CImg<T> img,double k1,int ni,int ns,int rango,int min,int max,int tam_vent,T niv_clip){

    CImg<T> resultado;

    //Step 1 - Aplico la funcion sigmoidea modificado
    //resultado = aplicar_lut_sigmoidea(img,k1,k2);
    resultado = transformacion(img,lut_sigmoidea(k1,ni,ns));
    //Step 2 -aplico clahe
    resultado = ecualizar_clahe(resultado,rango,min,max,tam_vent,niv_clip);


    return resultado;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///                             FUNCIONES PARCIAL DE PRACTICA
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

///****************************************
/// TRIM image. Devuelve la imagen recortada a su minimo convexhull
/// Necesario: Imagen con fondo blanco o similar, tiene que ser homogenea
///****************************************
//mascara representa el convelhull que contiene la imagen que se quiere recortar
//de la mascara se van a obtener la maxima y minimas coordenadas para cortar la imagen
template<class T>
CImg<T> trim_image(CImg<T>img,CImg<bool>mascara){

    //Puntos x e y maximos y minimos
    vector<punto> coordenadas;
    punto max,min;
    int maxx,maxy,minx,miny;

    //Calculo las coordenas min y max de la mascara
    coordenadas=maxmin_coord(mascara);
    max=  coordenadas[1];
    min = coordenadas[0];
    maxx=max.x;maxy=max.y;
    minx=min.x;miny=min.y;

    //Recorto la imagen
    return img.get_crop(maxx,maxy,minx,miny);

}

///****************************************
/// TRIM image wrapper. Pasa una mascara de grises y la imagen original
/// se obtiene el listado de subimagenes.
///****************************************
template<class T>
CImgList<T> trim_image_wrapper(CImg<T>img,CImg<T>mascara){

    //Costruyo la mascara booleana del objeto con ese valor de etiqueta
    CImg<T> trim;
    CImg<bool> mascara_proceso(mascara.width(),mascara.height(),1,1);
    CImgList<T> imagenes;

    //Cuanto cuantos valores de etiquetas distintas de 0 y 1 tengo en la mascara
    //Esto equivale a la cantidad de subimagenes dentro de la mascara

    //Busco los grises disponibles en la mascara
    vector<int> grises = grises_disponibles(mascara);


    //Recorro el vector de grises disponibles y calculo las subimagenes
    for(int k=0;k<grises.size();k++){

        //Busco en el vector el gris de ref.
        int etiqueta = grises[k];
        //Inicializo la mascara en cero
        mascara_proceso.fill(0);
        //Busco los valores con ese valor de etiqueta y genero mi mascara para ese objeto
        cimg_forXY(mascara,i,j){
            if(mascara(i,j)==etiqueta)
                mascara_proceso(i,j)=1;
        }
        //Llamo a la funcion que corta la imagen
        trim = trim_image(img,mascara_proceso);
        imagenes.push_back(trim);
    }

    //Devuelvo la lista de subimagenes
    return imagenes;

}

///****************************************
/// COORD_HOUGH_TO_VALUE
///****************************************
/// En base a la coordenada y al eje, me retorna el valor que correspende la transformada Hough
/// en grados para theta (t) entre [-90 ; 90] y entre [-sqrt(2)M;sqrt(2)M] el rho (p)
template <typename T>
double coord_hough_to_value(CImg<T> hough, int coord, unsigned char axis) {

    unsigned int M = hough.width();
    unsigned int N = hough.height();

    // Maximos valores absolutos de theta y de rho
    double max_theta = 90;
    double max_rho = pow(pow(M, 2.0) + pow(N, 2.0), 0.5); // si M = N, max_rho = sqrt(2) * M

    // Y le resto para ir al rango normal [-1.0 ~ 1.0]
    double valor;

    if (axis == 't') { // theta
        valor = (2.0 * coord / M - 1.0) * max_theta;
    } else if (axis == 'p') { //rho
        valor = (2.0 * coord / N - 1.0) * max_rho;
    } //else {
       // assert(axis && 0); // aviso del error
    //}

    return valor;
}

///****************************************
/// GET_MAX_PEAK. maxima colinealidad
///****************************************
/// Busco la posicion de maximia colinealidad en Hough, y retorno por referencia el valor de
/// tetha entre [-90, 90] y el valor en coord de posicion de rho [0, hough.height()]
/// tambien retorno la transformada Hough sin ese maximo pico
template<typename T>
void get_max_peak(CImg<T> hough, T &theta, T &rho_coord, unsigned int difuminacion = 5) {

    unsigned int max_x = 0;
    unsigned int max_y = 0;

    double max_val = hough.max();

    bool go_loop = true;

    for (unsigned int x = 0; x < hough.width() && go_loop; x++) {
        for (unsigned int y = 0; y < hough.height() && go_loop; y++) {
            if (fabs(hough(x, y) - max_val) < EPS) {
                max_x = x;
                max_y = y;
                go_loop = false;
            }
        }
    }


    // Guardo los valores (Giro respecto al eje x)
    theta = coord_hough_to_value(hough, max_x, 't');
    rho_coord = max_y;

    // Guardo los valores (Giro respecto al eje y)
    //rho_coord = coord_hough_to_value(hough, max_y, 'p');
    //theta = max_x;

}

///****************************************
/// ROTATE IMAGE.
///****************************************
//Rota las imagenes en funcion de un angulo que detecta automaticamente y recorta los sobrantes del fondo
//Recibe la imagen que se desea rotar un umbral para el calculo del sobel que va a entrar en el hough
//Un umbral para el calculo de la mascara
//Un limite de giro angular
//Funciona para imagenes a color y para en escala de grises
template<class T>
CImg<T> rotate_image(CImg<T> img,double umbral_sobel,double umbral_mascara,bool correccion_eje=true,bool color=true){

    CImg<T>greyscale;
    CImg<bool> mascara;
    CImg<T> aux_hough,resultado;
    double tolerancia=2.0;

    //Si la imagen es color la hago en escala de grises
    if(color){
        greyscale = img.get_RGBtoHSI().get_channel(2).get_normalize(0,255);
    }
    else{
        greyscale = img.get_normalize(0,255);
    }

    //Sobel para deteccion de bordes
    aux_hough = Sobel(greyscale,0)+Sobel(greyscale,1);
    aux_hough.threshold(umbral_sobel);
    //aux_hough.display("sobel");

    // Y ahora aplicamos hough
    aux_hough = hough(aux_hough);

    //buscamos el maximo pico
    double max_theta = 0;
    double max_rho_coord = 0;

    get_max_peak<double>(aux_hough, max_theta, max_rho_coord);

    // Ahora que se donde esta el maximo pico, se cual es su inclinacion, por lo que deberia
    // rotarlo hacia 90 o hacia 0, depende cual este mas cerca

    // Si theta es negativo, tomo consideracion especial
    max_theta = (max_theta < 0) ? 180 + max_theta : max_theta;

    // Me va dar 0 (<45), 90 (< 135), 180 (< 225), 270 (<315), o 360
    double degree_to_go = round(max_theta / 90) * 90;

    //Voy a rotar siempre y cuando el angulo que se encuentre el objeto sea mayor a la tolerancia
    if(max_theta <tolerancia && correccion_eje){
        //En caso de que no haya necesidad de rotar
        resultado = img;
    }
    else
    {   // Y lo rotamos
        resultado = img.get_rotate(max_theta - degree_to_go);
    }

    //Hago el blur para homogeneizar el objeto y obtener una mejor mascara con el threshold
    //greyscale.blur(3);
    //greyscale.display("blur");

    //calculamos la mascara para recortar los sobrantes
    mascara = greyscale.get_threshold(umbral_mascara);

    //Morfologia - apertura
    mascara = apertura(mascara,mask(3));

    //roto la mascara segun corresponda. Si hay correccion resp. al eje y ang. theta es<tol, entonces roto.
    if(max_theta > tolerancia && correccion_eje){
        mascara.rotate(max_theta - degree_to_go);
    }

    //Recortamos la imagen
    resultado = trim_image(resultado,mascara);

    return resultado;
}

///****************************************
/// detectar_lineas
///****************************************
/// Detecta las lineas con maxima coolinealidad de una imagen y las devuelve
/// en forma de mascara. La cantidad de lineas que detecte dependera de
/// cant_max que son la cantidad de maximos que filtraria en Hough

template<class T>
CImg<bool> detectar_lineas(CImg<T> img,int umbral_bordes,int cant_max){
    //
    CImg<bool> mascara(img.width(),img.height()),final(img.width(),img.height());
    mascara.fill(0);final.fill(0);

    //Detecto los bordes de la imagen
    CImg<bool> bordes = Sobel(img).normalize(0,255).threshold(umbral_bordes);

    //Calculo la transformada de Hough
    CImg<T> img_hough = hough(bordes).normalize(0,1);

    for(int k=1;k<cant_max+1;k++){
        //Calculo el k maximo de la inversa de hough
        mascara = InversaHough_nmax(img_hough,k).normalize(0,1);

        //Corto las lineas de la antitransformada de hough segun la mascara y
        //dist_maxima_puntos
        CImg<bool> aux(mascara.width(),mascara.height());
        aux.fill(0);
        cimg_forXY(mascara,i,j){
            if(mascara(i,j)==1){
                //Si estoy sobre una linea de la mascara y la mascara
                //es igual al borde guardo los pixeles de bordes en aux
                aux(i,j)=bordes(i,j);
            }
        }
        //Dibujo la linea entre los puntos que quedan en la linea
        punto A,B;
        B.x=-1;B.y=-1;A.x=-1;A.y=-1;
        cimg_forXY(aux,i,j){
            if(aux(i,j)==1){
                B.x=A.x;
                B.y=A.y;
                A.x=i;
                A.y=j;
                if(A.x != -1 && B.x != -1)
                    final.draw_line(A.x,A.y,0,B.x,B.y,0,white);
            }
        }
    }
    return final;
}

///****************************************
/// detectar_linea nro cant_max
///****************************************
/// Detecta la linea con maxima coolinealidad de una imagen y la devuelve
/// en forma de mascara. La linea que detecte dependera de
/// cant_max que es el maximo nro cant_max que filtre Hough

template<class T>
CImg<bool> detectar_linea_nro(CImg<T> img,int umbral_bordes,int max=1){
    //
    CImg<bool> mascara(img.width(),img.height()),final(img.width(),img.height());
    mascara.fill(0);final.fill(0);

    //Detecto los bordes de la imagen
    CImg<bool> bordes = Sobel(img).normalize(0,255).threshold(umbral_bordes);

    //Calculo la transformada de Hough
    CImg<T> img_hough = hough(bordes).normalize(0,1);


    //Calculo el k maximo de la inversa de hough
    mascara = InversaHough_nmax(img_hough,max).normalize(0,1);

    //Corto las lineas de la antitransformada de hough segun la mascara y
    //dist_maxima_puntos
    CImg<bool> aux(mascara.width(),mascara.height());
    aux.fill(0);
    cimg_forXY(mascara,i,j){
        if(mascara(i,j)==1){
            //Si estoy sobre una linea de la mascara y la mascara
            //es igual al borde guardo los pixeles de bordes en aux
            aux(i,j)=bordes(i,j);
        }
    }
    //Dibujo la linea entre los puntos que quedan en la linea
    punto A,B;
    B.x=-1;B.y=-1;A.x=-1;A.y=-1;
    cimg_forXY(aux,i,j){
        if(aux(i,j)==1){
            B.x=A.x;
            B.y=A.y;
            A.x=i;
            A.y=j;
            if(A.x != -1 && B.x != -1)
                final.draw_line(A.x,A.y,0,B.x,B.y,0,white);
        }
    }

    return final;
}



#endif // FUNCIONES

