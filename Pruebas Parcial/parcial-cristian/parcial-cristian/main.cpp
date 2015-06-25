#include "funciones.h"

//ejemplo como sacar un maximo
//std::vector<int> v{3, 1, 4, 1, 5, 9};
//  std::vector<int>::iterator result = std::max_element(std::begin(v), std::end(v));
//  std::cout << "Maximo en la posicion: " << std::distance(std::begin(v), result);


//inunda e etiqueta
CImg<float> InundarEtiquetar( CImg<bool> img,vector<int> &cant_pixel_por_etiqueta) {
    CImg<bool> aux;
    CImg<float> aux2(img.width(),img.height(),1,1,0),mascara_etiqueta,final(img.width(),img.height(),1,1,0);
    int etiqueta=2;
    //img.display("Mascara Inundar Etiqueta");
int cant_pixel=0;
    cimg_forXY(img,i,j){
        if(img(i,j)==true && final(i,j)==0){
            aux= Inundar(img, 0.5, i, j,cant_pixel);
            cant_pixel_por_etiqueta.push_back(cant_pixel);
            cant_pixel=0;
            //aux.display("inundar");
            aux2.fill(etiqueta);
            mascara_etiqueta=aux2.get_mul(aux);
            //mascara_etiqueta.display("mascara etiqeuta");
            final+=mascara_etiqueta;
            //final.display("final");
            etiqueta++;
            mascara_etiqueta.fill(0);
        }
    }
    return final;

}

//retorna la cantidad de etiquetas que tienen entre una cantidad de pixel por parametro
//sirve para sergmentar por tamaño de objetos
//(tambien retorna a que numero de etiqueta pertenece)
CImg<float> descartar_mayores_menores(CImg<float>etiqueta,vector<int> cant_pixel_por_etiqueta,int umbral_sup,int umbral_inf){

    for(int i=0;i<cant_pixel_por_etiqueta.size();i++){
        if(cant_pixel_por_etiqueta.at(i)>umbral_sup || cant_pixel_por_etiqueta.at(i)<umbral_inf){ // si cantidad de pixel(tamano) mayor a umbral(tamano umbral) lo debo descartar
            cimg_forXY(etiqueta,m,j)
                    if(etiqueta(m,j)==i+2) //i me indica en que etiqueta estoy(+2 por que inindar etiqeuta arranca en dos)
                    etiqueta(m,j)=0; //borro ese elemento irrelevante de la mascara
        }
    }
    return etiqueta; //retorno la nueva mascara sin la etiqueta irrelevantes
}

#define RADIO 10
#define UMBRAL 180
#define UMBRAL_ESTACIONAMIENTO 0.022

//1. Obtengo mascara amarillo (colo slicing RGB)para determina la zona estacionamiento, esta zona pertenece a discapacitados
//2. (aplico una erocion a mascara amarillo para limpiar imperfecciones)--> cuanto los pixeles true de la mascara_amarillo
//cuento la relacion con la cantidad de pixeles totales de la mascara en relacion a los pixeles false
//obtengo un porcentage, luego coloco un punto de corte(UMBRAL_ESTACIONAMIENTO )
//si la cantidad de pixeles true de la imagen
//total es muy baja, el estacionamiento esta ocupado, sino libre.
//3. Determino la linea del auto -- violeta(auto 208,109,161) es a tres
//imagenes de referencia esta linea tiene el mismo valor
//4. Aplico houg a la mascara obtenida en el punto 3, obtengo la trayectoria del auto
//el resultado lo paso a HoughInversa y me quedo con el prime maximo (el maximo maa maximo)
//retorno ese maximo
//5. Tomo la mascara_amarilla del punto numero de 2
//Aplico una segunda erosion
//relleno_automatico
//extraccion_de_contornos-->contorno
//paso  mascara contorno a hough--> InversaHough --> me quedo con el primer Maximo(el maximo mas maximo)
//6. Ya tengo la trayetoria del auto y trayectoria del estacionamiento, compongo los dos junto con la
//imagen color y muestro.

// Problemas
//1.
// Deberia refinar mejor la mascara estacionamiento, para que quede mas homogenea y al hacer houg directa y luego
//inversa y me retorne linea vertical como corresponde a la parking2.png y no esa linea inclinada

//2.
//Debo pintarlas lineas imagen original, en la imagen color, la imagen no tiene que quedar en grices

//3.
//....

int main()
{
    CImg<float> img,img_intensidad;
    CImg<bool> mascara,se1(3,3),se2(1,6),se3(1,3),se4(2,2);
    se1.fill(1);
    se2.fill(1);
    se3.fill(1);
    se4.fill(1);
    const char* path = "../../../../Parcial de Practica/parcial/parking4.png";
    img.load(path);
    img.display();
    //Proceso la mascara para dejarla binaria
    img_intensidad=img.get_RGBtoHSI().get_channel(2).get_normalize(0,255);


    ///COLOR SLICING
    //ZONA AMARRILLA

    //Defino el color del lugar para discapacitados
    CImg<float> color(1,1,1,3);
    color(0,0,0,0)=202;
    color(0,0,0,1)=163;
    color(0,0,0,2)=88;
    //Hago colourslicing de la imagen con ese color
    CImg<float> amarillo= ColorMaskRGB(img.get_blur(1.4),color,10);
    CImg<bool>mascara_amarillo=amarillo;
    (amarillo,mascara_amarillo).display("amarrillo y mascara");
    mascara_amarillo.erode(se2);
    mascara_amarillo.dilate(se2);
    mascara_amarillo=relleno_automatico(mascara_amarillo);
    mascara_amarillo.display("relleno auto"); //ya tengo la mascara, se la paso a sobel para detectar borde , linea, etc

    ///COLOR SLICING
    //AUTO

    //Defino el color del lugar para discapacitados
    //CImg<float> color(1,1,1,3);
    color(0,0,0,0)=208;
    color(0,0,0,1)=109;
    color(0,0,0,2)=161;
    //Hago colourslicing de la imagen con ese color
    CImg<float> autod= ColorMaskRGB(img,color,2);
    CImg<bool>mascara_linea=autod;
    (autod,mascara_linea).display("auto y mascara");


    int total_pixel_img=mascara_amarillo.width()*mascara_amarillo.height();
    //hago porcetange de pixeles amarrillo que quedan visible es el esacionamiento segun tamañode imagen
    int cont=0;
    cimg_forXY(mascara_amarillo,m,n){
        if(mascara_amarillo(m,n)==true)
            cont++;
    }
    float porcenta=(float)cont/total_pixel_img;
    cout<<total_pixel_img<<endl;
    cout<<cont<<endl;
    cout<<"porcenta: "<<porcenta<<endl;   //0.0195604
    if(porcenta<UMBRAL_ESTACIONAMIENTO) //umbral_estacionamiento
    {cout<<"estacionamiento ocupado"<<endl;
    return 0;
    }
    else
        cout<<"estacionamiento libre"<<endl;


    CImg<bool>B2(3,3);
    B2.fill(0,0,0,1,1,1,0,0,0);
    CImg<float>skel=esqueleto(mascara_amarillo,B2);
    skel.display();
    mascara_amarillo=skel;



   // linea_hough_auto.draw_line();
    // aplico huugh
      //linea auto
      CImg<double> Hough_auto = hough(mascara_linea);
      Hough_auto.display("hough auto");
      CImg<double> linea_hough_auto = InversaHough(Hough_auto,1);
      linea_hough_auto.normalize(0,255);
      linea_hough_auto.display("linea  auto hough");

      //linea estacionamiento
      CImg<double> Hough_est = hough(mascara_amarillo);
      Hough_est.display("hough auto");
      CImg<double> linea_hough_est = InversaHough(Hough_est,1);
      linea_hough_est.normalize(0,255);
      linea_hough_est.display("linea  estacionamiento hough");





      //pinto las lineas que obtume en la imagen
      CImg<double> resultado=(linea_hough_auto.normalize(0,255)+ linea_hough_est.normalize(0,255)+ img.normalize(0,255))/2;
      resultado.display("resultado");









    return 0;
}
