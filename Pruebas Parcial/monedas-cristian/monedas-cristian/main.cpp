#include "funciones.h"
#define UMBRAL 253

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

int main()
{
    CImg<float> img,img_intensidad;
    CImg<float> etiqueta;
    CImg<bool> mascara,se1(3,3);
    se1.fill(1);
    const char* path = "../../../../Parcial de Practica/Monedas/02_Monedas.jpg";
    img.load(path);
    //img.display("monedas");
    mascara.assign(img.width(),img.height(),1,1);
    //Proceso la mascara para dejarla binaria
    img_intensidad=img.get_RGBtoHSI().get_channel(2).get_normalize(0,255);
    mascara = NOTimg(img_intensidad.threshold(UMBRAL));
    (img,img_intensidad,mascara).display("");
    //Operacion de etiquetado
    vector<int> contadordepixeles;
    etiqueta=InundarEtiquetar(mascara,contadordepixeles);
    etiqueta.display("Etiquetado");

     int cant_monedas = cant_grises(etiqueta);
     cout<<"La cantidad total de monedas es: "<<cant_monedas<<endl;

    //sumar la cantidad de dinero
// puede existir en alguna suma del dinero debido a los rango de cantidad de pixeles estimados para cada tamaño
// de moneda
    float suma=0;
    for(int i=1;i<contadordepixeles.size();i++){
        if(contadordepixeles.at(i)>=3360 && contadordepixeles.at(i)<=3780) suma+=0.01;
        if(contadordepixeles.at(i)>=4350 && contadordepixeles.at(i)<=4560) suma+=0.02;
        if(contadordepixeles.at(i)>=5620 && contadordepixeles.at(i)<=5830) suma+=0.05;
        if(contadordepixeles.at(i)>=4800 && contadordepixeles.at(i)<=5020) suma+=0.1;
        if(contadordepixeles.at(i)>=6070 && contadordepixeles.at(i)<=6296) suma+=0.2;
        if(contadordepixeles.at(i)>=7220 && contadordepixeles.at(i)<=7440) suma+=0.5;
        if(contadordepixeles.at(i)>=6550 && contadordepixeles.at(i)<=6760) suma+=1;
        if(contadordepixeles.at(i)>=7970 && contadordepixeles.at(i)<=8200) suma+=2;

    }

    cout<<"La cantidad de dinero es "<<suma<<endl;

    //Chequeo las monedas
    CImgList<float> miedo = trim_image_wrapper(img,etiqueta);
    miedo.display("Monetaaaa!");

    return 0;
}

