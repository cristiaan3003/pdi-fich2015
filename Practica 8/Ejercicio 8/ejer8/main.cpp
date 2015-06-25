#include "funciones.h"

#define UMBRAL 243

int main()
{
    //imagen
    CImg<float> original;
    original.load("../../../../images/Morfologia/Cuerpos.jpg");
    CImg<bool> B(8,8),B2(3,3);
    B.fill(1);
    B2.fill(1);

    //Calculo los esqueletos con Hamilton-Jacobi
    const int median = 1;
    const bool invert = true;
    const float thresh = -0.3f;
    const bool curve = true;
    const bool correction = true;
    const float dlt1 = 2;
    const float dlt2 = 1.0f;

    //Tengo que usar la imagen original pero guardar una version binaria (proceso conforme a esi)
    //CImg<unsigned int> mascara = original.get_norm().quantize(2).normalize(0.0f,1.0f);
    CImg<unsigned int> mascara = original.get_RGBtoHSI().channel(2).normalize(0,255).threshold(UMBRAL).get_normalize(0,1);
    if (median) mascara.blur_median(median);
    if (invert) (mascara-=1)*=-1;

    //Segmento para que la mascara sea mejor
    mascara = apertura(mascara,B2);
    mascara = relleno_automatico(mascara);

    //Calculo el mapa de distancia
    CImg<float> distance = mascara.get_distance(0);

    //Calculo el gradiente de la funcion y el flujo o divergencia del gradiente
    const CImgList<float> grad = distance.get_gradient("xyz");
    CImg<float> flux = mascara.get_flux(grad,1,1);

    //Uso la correccion de Torsello del flujo si es necesaria
    if (correction) {
        CImg<float>
                logdensity = mascara.get_logdensity(distance,grad,flux,dlt1),
                nflux = mascara.get_corrected_flux(logdensity,grad,flux,dlt2);
        flux = nflux;
    }

    //Calculo el esqueleto
    const CImg<unsigned int> skel = mascara.get_skeleton(flux,distance,curve,thresh);

//    CImg<unsigned int> mascara = original.get_RGBtoHSI().channel(2).normalize(0,255).threshold(UMBRAL).get_normalize(0,1);
//    //Operaciones morfologicas para que la mascara sea mejor
//    mascara = NOTimg(mascara);
//    mascara = cierre(mascara,B2);
//    mascara = apertura(mascara,B2);

//    skel=esqueleto(mascara,B2);
    (original,mascara,skel).display();


    return 0;
}

