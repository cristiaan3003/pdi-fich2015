#ifndef FUNCIONES
#define FUNCIONES

#include <iostream>
#include "CImg.h"
#include <vector>
#include <algorithm>
#include <math.h>
#include <string>
#include <complex>
#include <PDI_functions.h>
#include <fstream>
#include <cstdio>
#include <ctime>

using namespace cimg_library;
using namespace std;

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
}punto;

//Sort de Vector con puntos
bool myfunction (punto i,punto j) { return (i.normaManhattan()<j.normaManhattan()); }

//Para usar sort con vectores de puntos
bool compara_puntos(const punto &a, const punto &b)
{
    return a.x < b.x;
}

vector<punto> ordenarCoordenadas(vector<punto> puntos){
    // using object as comp
    std::sort (puntos.begin(), puntos.end(),myfunction);
    return puntos;
}

//Colores auxiliares

const unsigned char red[] = { 255,0,0 },
                    green[] = { 0,255,0 },
                    blue[] = { 0,0,255 },
                    white[] = { 255,255,255 },
                    black[] = { 0,0,0 },
                    yellow[] = { 255,255,0 };



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
CImg<T> generar_lut_logb(T c,bool positiva){
    CImg<T> lut(1,256,1,1);
    T s;
    for (int i=0;i<256;i++){
        //Aca mapeamos el rango total resultante para que despues se traduzca en un nivel de gris
        s=floor(c*(255*((log(1+i)/log(1+255)))));// normalizo -.- mmmm
//        if(positiva){
//            s=c*log(1+i);
//        }
//        else
//        {
//            s=c*pow(10,1+i);//MAL!!!!
//        }

        lut(i)=s;
    }
    //lut.normalize(0,255);

    return lut;

}



///LUT EXPONECIAL
// gamma es positivo
// 0 < gamma < 1 Aclara la imagen
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
template<typename T>
CImg<T> sumaImg(CImg<T> img1, CImg<T> img2, T corrimiento=0 ){
    CImg<T> resultado(img1.width(),img1.height(),1,1);
    cimg_forXY(img1,i,j){
        if((i+corrimiento) >= 0 && (j+corrimiento)>=0){
            resultado(i,j)=(img1(i,j)+ img2(i+corrimiento,j+corrimiento))/2;
        }else
        {
            resultado(i,j)=(img1(i,j)+ img2(i,j))/2;
        }
    }
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
template<typename T>
CImg<T> reducRuido(CImgList<T>img){
    CImg<T> suma(img[0].width(),img[0].height(),1,1,0);
    for(int i=0;i<img.size();i++)
        suma=sumaImg(suma,img[i],0); //sumo
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
CImg<T> umbral_por_tramos(CImg<T> &img, T p1,T p2){
    CImg<T> resultado(img.width(),img.height(),1,1);
    cimg_forXY(img,i,j){
        if (img(i,j)<=p1 || img(i,j)>=p2)
            resultado(i,j)=255;
        else
            resultado(i,j)=0;
    }
    return resultado;
}
///****************************************
///OR
///****************************************
//Or entre imagen binaria y una mascara binaria
template<typename T>
CImg<T> ORimg(CImg<T> img, CImg<T> masc){
    CImg<T> resultado(img.width(),img.height(),1,1,0);
    cimg_forXY(img,i,j){
        if ((int)*img.data(i,j,0,0)==0 & (int)*masc.data(i,j,0,0)==0)
            resultado(i,j)=0;
        else
            resultado(i,j)=1;
    }
    return resultado;
}
///****************************************
///AND
///****************************************
//And entre imagen binaria y una mascara binaria
template<typename T>
CImg<T> ANDimg(CImg<T> &img, CImg<T> &masc){
    CImg<T> resultado(img.width(),img.height(),1,1);
    cimg_forXY(img,i,j)
            resultado(i,j)=(img(i,j)*masc(i,j));
    return resultado;
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

///PROMEDIO KERNEL
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
///SOBEL
///****************************************
///idea de gradiente en dos direcciones
template<typename T>
CImg<T> Sobel(CImg<T> img){
    CImg<T> Gx(3, 3), Gy(3,3);
    Gx(0,0)=-1; Gx(0,1)=-2; Gx(0,2)=-1;
    Gx(1,0)=0; Gx(1,1)=0; Gx(1,2)=0;
    Gx(2,0)=1; Gx(2,1)=2; Gx(2,2)=1;

    Gy(0,0)=-1; Gy(1,0)=0; Gy(2,0)=1;
    Gy(0,1)=-2; Gy(1,1)=0; Gy(2,1)=2;
    Gy(0,2)=-1; Gy(1,2)=0; Gy(2,2)=1;

    return (img.get_convolve(Gx)+img.get_convolve(Gy)).abs().normalize(0,255);
}
///****************************************
///FILTRO DE ALTA POTENCIA 1 CANAL
///****************************************
//Recibe la imagen el kernel pasa bajo y la amplificacion
template<typename T>
CImg<T> filtroAP(CImg<T> img,CImg<T> kernel,T amp){
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
CImg<T> filtroAP3(CImg<T> img,CImg<T> kernel, T amp){

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
        //img(i,j,0,2)=1-img(i,j,0,2);
	}
    img.HSItoRGB();
    return img;
}

template <class T>
void ComposeRGB(CImg<T> &img, CImg<T> &r, CImg<T> &g, CImg<T> &b){
    int ww=r.width(), hh=r.height(), i, j;
    img.assign(ww, hh, 1, 3);
    for(i=0; i<ww; i++){
        for(j=0; j<hh; j++){
            img(i,j,0,0)=r(i,j);
            img(i,j,0,1)=g(i,j);
            img(i,j,0,2)=b(i,j);
        }
    }
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



///COPIA CANAL A CANAL
//Sirve para no tener que escribir los ciclos en el main
template<typename T>
CImg<T> copia_canal(CImg<T> img_orig,int canal,CImg<T> img_a_copiar,int canal_a_copiar){
    cimg_forXYZ(img_orig,i,j,k)
            img_orig(i,j,k,canal) = img_a_copiar(i,j,k,canal_a_copiar);
    return img_orig;
}

///TRANSFORMADA DE FOURIER
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


///TRANSFORMADA DE FOURIER INVERSA
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

///PASABAJOS IDEAL
// Genera una mascara pasabajos ideal a partir de un tamanio de imagen y un radio
template<typename T>
CImg<T> pasabajos_ideal(T radio,T ancho,T alto){
    CImg<T> mascara(ancho,alto,1,1);
    mascara.fill(0);

    mascara.draw_circle(0,0,radio,white,1);
    mascara.draw_circle(ancho,0,radio,white,1);
    mascara.draw_circle(0,alto,radio,white,1);
    mascara.draw_circle(ancho,alto,radio,white,1);

    mascara.normalize(0,1);

    mascara.display();

    return mascara;

}

///PASAALTOS IDEAL
// Genera una mascara pasabajos ideal a partir de un tamanio de imagen y un radio
template<typename T>
CImg<T> pasaalto_ideal(T radio,T ancho,T alto){
    return negativo(pasabajos_ideal(radio,ancho,alto));
}

///FILTRADO EN FRECUENCIA
//Filtra en frecuencia a partir de una imagen y un filtro
template<typename T>
CImg<T> filtrar(CImg<T> img,CImg<T> filt){
    CImgList<float> img_tr = fourier(img);
    return fourier_inv(multiplicacion(img_tr.at(0),filt),img_tr.at(1));
}

#endif // FUNCIONES


