#include "funciones.h"
void forma1(){
    CImg<double> img,img_denoised,img_nueva;
   //img.load("../../../../images/corrientes_ruidogris.jpg");
   img.load("../../../../images/iguazu_ruidogris.jpg");
   // //denoise(img.get_channel(0),sizew,tipofiltro,Q,d);
    img_denoised = denoise(img,3,3);
    img_denoised = denoise(img,3,5,0,4);//Media Alfa Recortada d = la cantidad de pixeles de la ventana que puede recortar

    img_denoised.display("imagen denoise");
    double A=1.3;//medio
    img_nueva=(A *  img_denoised -  img_denoised.get_convolve(mask(6)));//alta potencia
    img_nueva.display("alta potencia");
    img_nueva.threshold(70);
    img_nueva.display("umbral");
CImg<double> Hough = hough( img_nueva );
Hough.display("hough");
// Busco donde esta su maxima colinearidad
Pixel coordenadas= MaximoP(Hough);
double valor_tita = coordenadaXY_a_rho_tita(Hough, coordenadas.x, 't'); //tita
//splitHough(CImg<T> Hough, T angulo,T rho,int ang_tol = 0,int tolerancia_rho = 0, bool solo_max = false)
CImg<double> linea(hough(splitHough(Hough, valor_tita, (double) coordenadas.y, 1, 1), true));

//(img_denoised * 0.7 + linea * 0.3).display("Final", 0);

CImg<double> resultado=(linea.normalize(0,255)+ img_denoised.normalize(0,255))/2;
CImgList<double> lista( /*original, filtrada,*/ img_nueva, /*Hough,*/  resultado );
lista.display("Pista - forma 1", false);

}

void forma2(){
    CImg<double> img,img_denoised,img_nueva;
   //img.load("../../../../images/corrientes_ruidogris.jpg");
    img.load("../../../../images/iguazu_ruidogris.jpg");
   // //denoise(img.get_channel(0),sizew,tipofiltro,Q,d);
    img_denoised = denoise(img,3,3);
    img_denoised = denoise(img,3,5,0,4);//Media Alfa Recortada d = la cantidad de pixeles de la ventana que puede recortar

    img_denoised.display("imagen denoise");
    double A=1.3;//medio
    img_nueva=(A *  img_denoised -  img_denoised.get_convolve(mask(6)));//alta potencia

    (img_nueva).display("alta potencia");
    img_nueva.threshold(70).erode(3).dilate(3);
    img_nueva.display("umbral");
//    Pixel coordenadas= MaximoP(img_denoised);
//    imgnueva= region_growing(img_denoised,coordenadas.x,coordenadas.y,180,256);
//    coordenadas= MaximoP(imgnueva);

    CImg<double> Hough = hough(img_nueva);
    Hough.display("hough");
    CImg<double> linea = InversaHough(Hough,3);

    linea.normalize(0,255);
    linea.display("linea");
    CImg<double> resultado=sumaImg(linea.normalize(0,255),img_denoised.normalize(0,255));
    CImgList<double> lista( /*original, filtrada,*/ img_nueva, /*Hough,*/  resultado );
    lista.display("Pista", false);


}

int main()
{

    // forma1();
 forma2();


    return 0;
}

