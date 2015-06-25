#include "funciones.h"

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
    CImg<float> img,sin_lineas,zonas_dZB,recortada,img_intensidad,final;
    CImg<float> etiqueta1;
    CImg<bool> se1(3,3);
    se1.fill(1);
    int mx,my;
    const char* path = "../../../../Parcial de Practica/tormentas/1.png";
    img.load(path);
    //Proceso la mascara para dejarla binaria
    img_intensidad=img.get_channel(2).get_normalize(0,255);
    //(img,img_intensidad).display("img -  intensidad");

    //borrar lineas negras que me molestan y recortar imagen a donde estan la nubes (todas la fotos el mismo tamano)
    sin_lineas=img;
    for(unsigned i=0; i<img.width(); i++){
        for(unsigned j=0; j<img.height(); j++){
               if(sin_lineas(i,j,0,0)==0 &&   sin_lineas(i,j,0,1)==0 &&  sin_lineas(i,j,0,2)==0){
                   sin_lineas(i,j,0,0)=255;
                   sin_lineas(i,j,0,1)=255;
                   sin_lineas(i,j,0,2)=255;
               }
        }
    }
    //sin_lineas.display("sin lineas");
    recortada=sin_lineas.get_crop(86,31,428,374);
    //recortada.display("recortada y sin lineas");
    //recortada.displ#define RADIO_CIRCULOS 30ay();


    //obtengo zonas que tiene solo mas de 48dBZ
    //zonas_dZB=recortada;
     CImg<bool> mascara(recortada.width(),recortada.height());
     mascara.fill(0);
    for(unsigned i=0; i<recortada.width(); i++){
        for(unsigned j=0; j<recortada.height(); j++){
               if(recortada(i,j,0,0)>=68 && recortada(i,j,0,1)==0 && recortada(i,j,0,2)>=132){
                   mascara(i,j)=true;//marco zonas que pasan de 48
               }
        }
    }
    mascara=NOTimg(mascara);
    mascara.erode(se1).dilate(se1);
    mascara=NOTimg(mascara);
    //(recortada,mascara,recortada.get_mul(mascara)).display("dZB");

    //cuento la dimension de cara una de las areas detectadas y determino si es grande o pequeña

    // 1px  ----- 0.5km2
    // 0.5km2 ------------ 1px
    // 40km2 ------------- x
    //x=40*0.5=80px
    //si el area es mayor o igual a 80 pixeles se lanza alerta

    //Operacion de etiquetado
//LABEL CC cuenta mal cuando tiene cierta pendiente que no se muy bien como o por que
            //asi que me implemente mi propio etiquetado con inundar
            //anda  sin importar que tan retorcida la figura.
 //RECORDAR: InundarEtiquetar -->etiqueta arranca en 2, por lo que el valor de la etiqueta 2,3,4...n
            //va desfazado en de unidades con la posicion de si cantidad de pixeles en cant_pixel_por_etiqueta
            //osea la cant de pixeles de etiqueta 2, va estar en la posicion 0 de cant_pixel_por_etiqueta
            // cant pixeles etiqueta 3 en la posicon 1 de cant_pixel_por_etiqueta y asi...
   vector<int> cant_pixel_por_etiqueta1;
   etiqueta1=InundarEtiquetar(mascara,cant_pixel_por_etiqueta1);
   etiqueta1=descartar_mayores_menores(etiqueta1,cant_pixel_por_etiqueta1,10000000,48);
   //etiqueta1.display("zonas peligrosas");
   mascara=etiqueta1;//actualizo mi mascara con solo las zonas de alerta
   //mascara.display();
   //reetiqueto
   cant_pixel_por_etiqueta1.clear();
   etiqueta1=InundarEtiquetar(mascara,cant_pixel_por_etiqueta1);
   etiqueta1=descartar_mayores_menores(etiqueta1,cant_pixel_por_etiqueta1,10000000,48);
   //etiqueta1.display("zonas peligrosas");
   for(int i=0;i<cant_pixel_por_etiqueta1.size();i++){
   cout<<"Area en Km2 "<< cant_pixel_por_etiqueta1.at(i)*0.5<<" - corresponde a etiqueta:"<< i+2<<endl;
   }

   //(recortada,recortada.get_mul(mascara)).display("FINAL final");
   zonas_dZB=recortada.get_mul(mascara);
            int maximo=-1,x=0,y=0;
            int minimo=zonas_dZB.max();
   for(unsigned i=0; i<zonas_dZB.width(); i++){
       for(unsigned j=0; j<zonas_dZB.height(); j++){
              if(zonas_dZB(i,j,0,0)>maximo && zonas_dZB(i,j,0,0)!=255 && zonas_dZB(i,j,0,2)<minimo && zonas_dZB(i,j,0,2)!=0){
                  maximo=zonas_dZB(i,j,0,0);//marco zonas que pasan de 48
                   minimo=zonas_dZB(i,j,0,2);
                  x=i;
                  y=j;
              }
       }
   }
   //valores rgb del maximo minimo
   cout<<"maximo"<<maximo<<endl<<"coordenada x:"<<x<<" y: "<<y<<endl<<"minimo"<<minimo<<endl;

   //poner un punto rojo do#define RADIO 15nde estan los maximos
   for(unsigned i=0; i<zonas_dZB.width(); i++){
       for(unsigned j=0; j<zonas_dZB.height(); j++){
           //cout<<zonas_dZB(i,j,0,0)<<endl;
              if(zonas_dZB(i,j,0,0)==maximo && zonas_dZB(i,j,0,1)==0 && zonas_dZB(i,j,0,2)==minimo){
                  img(i+86,j+31,0,0)=255;
                  img(i+86,j+31,0,1)=0;
                  img(i+86,j+31,0,2)=0;
                  cout<<"coordenadas x:"<<i<<"coordenada y:"<<j<<endl;
                  //final.draw_circle(i,j,5,red);
                  //zonas_dZB.display();

                  //Calculo de latitud y longitud del punto maximo
                  float latitud=0,longitud=0;
                  longitud = 59.25 + (fabs(i)*fabs(-61.75+59.25)/(428-86)); // desde 86 a 428 (en x)
                  latitud = 30.8 + (fabs(j)*fabs(-30.8+32.9)/(373-31)); //desde 31 a 373  (en y)
                  cout<<"longitud: "<<longitud<<" latitud: "<<latitud<<endl;


              }
       }
   }

   img.display("fin");
//Extensión: ? Kms; Máximo dBZ: ?; Latitud del máximo: ?;Longitud del máximo ?;

//Datos extra:
   //  Area de cobertura de un píxel: 0,5km²
//  Limites en latitud -32.9(XXX,374px) a -30.79(XXX,31px)
//  Limites en longitud -61.75(86,XXX) a 59.25(428,XXX)
//  Radio de cobertura 120km


    return 0;
}

