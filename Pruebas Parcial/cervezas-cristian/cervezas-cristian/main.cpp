#include "funciones.h"
#include <algorithm>
#include <iostream>
#define UMBRAL 70
#define RADIO_CIRCULOS 30
#define UMBRAL_TIPO_CERVEZA 2030

//inunda e etiqueta
CImg<float> InundarEtiquetar( CImg<bool> img,vector<int> &cant_pixel_por_etiqueta) {
    CImg<bool> aux;                  //recortada.display();
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
CImg<float> descartar_mayores_menores(CImg<float>etiqueta,vector<int> cant_pixel_por_etiqueta,double umbral_sup,double umbral_inf){

    for(int i=0;i<cant_pixel_por_etiqueta.size();i++){
        if(cant_pixel_por_etiqueta.at(i)>umbral_sup || cant_pixel_por_etiqueta.at(i)<umbral_inf){ // si cantidad de pixel(tamano) mayor a umbral(tamano umbral) lo debo descartar
            cimg_forXY(etiqueta,m,j)                  //recortada.display();
                    if(etiqueta(m,j)==i+2) //i me indica en que etiqueta estoy(+2 por que inindar etiqeuta arranca en dos)
                    etiqueta(m,j)=0; //borro ese elemento irrelevante de la mascara
        }
    }
    return etiqueta; //retorno la nueva mascara sin la etiqueta irrelevantes
}


//testea si el color esta en el radio que se le paso
//color imagen de un pixel que lleva el color a testear rgb
bool determinar_color(CImg<float> img,CImg<float> color, float radio){
    img.RGBtoHSI();
    color.RGBtoHSI();
    float h=color(0, 0, 0, 0);//h
    float s=color(0, 0, 0, 1);//s
    float i=color(0, 0, 0, 2);//I

    float dist;
            dist=sqrt(powf(h-img(0,0,0,0),2)+powf(s-img(0,0,0,1),2));
            cout<<"dist: "<<dist<<endl;
            if(dist<=radio)
                return true;
            else
                return false;


}

#define RADIO 6
int main()
{
    CImg<float> img,img_intensidad,aux,recortada;
    CImg<float> etiqueta;
    CImg<bool> mascara,se1(3,3),mascara2;
    se1.fill(1);
    int mx,my;
    const char* path = "../../../../Parcial de Practica/Cervezas/training/31.jpg";
    img.load(path);
    //img.display("cerveza");
    //Proceso la mascara para dejarla binaria
    img_intensidad=img.get_channel(2).get_normalize(0,255);
    CImg <float> hist = img_intensidad.get_histogram(256,0,255);
     mascara = negativo(img_intensidad.get_blur(1.5)).get_threshold(UMBRAL);
     recortada=trim_image(img,mascara);
      recortada.display("recorte");
     //ahora trabajo con recortada
      CImg<float> color(1,1,1,3,0);
      color.normalize(0,255);
      //color a buscar en un radio
      color(0,0,0,0)=237;
      color(0,0,0,1)=211;
      color(0,0,0,2)=101;
      color.display();
              if (determinar_color(recortada.get_crop(recortada.width()*0.5,recortada.height()*0.5,recortada.width()*0.5,recortada.height()*0.5),color,20))
              {cout<<"cerveza rubia true"<<endl;

                  CImg<float> rubia=ColorMaskHSI(recortada, recortada.width()*0.5,recortada.height()*0.5, RADIO);

                  rubia.display();

                  img_intensidad=rubia.get_RGBtoHSI().get_channel(2).get_normalize(0,255);
                   mascara=img_intensidad;
//                  img_intensidad.display();
//                  mascara=img_intensidad.get_blur(1).get_threshold(200);//umbral para cerveza rubia
                   mascara.erode(se1);
                  mascara.display("mascara");
                 // mascara=NOTimg(mascara);
                  //erosiono - elimino imperfecciones
                  mascara2=mascara.get_erode(se1).get_erode(se1).get_erode(se1).get_erode(se1).get_erode(se1).get_erode(se1).get_erode(se1);

                   mascara = reconstruccion_dilatacion(mascara,mascara2);
                  //dilato veces para recuperar proporciones
                  mascara.dilate(se1).dilate(se1).dilate(se1).dilate(se1);
                  (mascara2,mascara).display("---");

              }
              else{
                  cout<<"cerveza negra true"<<endl;
                   img_intensidad=recortada.get_RGBtoHSI().get_channel(2).get_normalize(0,255);
                   mascara=img_intensidad.get_blur(1.4).get_threshold(110);//umbral para cerveza negra
                   mascara.display("mascara");
                   mascara=NOTimg(mascara);
                   //erosiono - elimino imperfecciones
                   mascara.erode(se1).erode(se1).erode(se1).erode(se1).erode(se1).erode(se1).erode(se1).erode(se1);
                   //dilato 8 veces para recuperar proporciones
                   mascara.dilate(se1).dilate(se1).dilate(se1).dilate(se1).dilate(se1).dilate(se1).dilate(se1).dilate(se1);
                   mascara.display("---");
              }
   mascara=relleno_automatico(mascara);
    //calcular proporcion de cerveza
  mascara2=mascara.get_crop(mascara.width()*0.5,0,mascara.width()*0.5+1,mascara.height());
  mascara2.display();
  int limite_inf=0,limite_sup=0;
  for(int i=mascara2.height()-1;i>=0;i--){
      cout<<"adasd"<<endl;
      if(mascara2(0,i)!=false){
          limite_inf=i; // aqui arranca el contenido de cerveza (desde abajo)en el baso
          break;
      }}

      for(int i=0;i<mascara2.height();i++){
          cout<<"adasd"<<endl;
          if(mascara2(0,i)!=false){
              limite_sup=i; // aqui arranca el contenido de cerveza (desde arriba) en el baso
              break;
          }}
      int cerveza=limite_inf-limite_sup;
      //  limite_inf  ---- 100 %
      //  cerveza  ----- X= (cerveza*100)/limite_inf
      float porcentage=(float)(cerveza*100)/limite_inf;
      float porcentageEspuma=(float)(limite_sup*100)/limite_inf;
      cout<<porcentage<<"% de cerveza - "<<porcentageEspuma<<"% de espuma"<<endl;


    return 0;
}
