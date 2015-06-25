#include <funciones.h>
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
#define UMBRAL 30
#define nLineas 50

#define UMBRAL_SOBEL 50
#define UMBRAL_MASCARA 2
int main()
{
    CImg<float> img,etiqueta,billete;
    CImg<bool> se1(3,3);
    se1.fill(1);
    const char* path = "../../../../Parcial de Practica/Billetes/Billetes_mezclados/2.jpg";
    img.load(path);
CImg<bool>mascara(img.width(),img.height()); mascara.fill(0);
            cimg_forXY(img,j,k)
            //todo lo que sea distinto de fondo lo pongo a blanco y el fondo a negro
            if( !(img(j,k,0,0)==37 && img(j,k,0,1)==53 && img(j,k,0,2)==26) )
            mascara(j,k)=1;
            //multiplico la mascara con fondo negro por la imagen
            mascara=apertura(mascara,se1);
            img.mul(mascara);

    //img.display("billetes mesclados")
            img=denoise(img,3,12,0,4); //limpio un poquito el ruido, osilaciones x compresion, etc que existan en la imagen

   //etiqueto para el trim
   vector<int>cant_pixel_por_etiqueta;
   etiqueta=InundarEtiquetar(mascara,cant_pixel_por_etiqueta);
   //recorto lo billetes
   CImgList<double> billetes = trim_image_wrapper(img,etiqueta),billetes_rotados;



     //roto los billetes segun angulo determinado por hough
      for(int indice =0;indice<billetes.size();indice++){
             billete = billetes(indice);
             //Roto el billete (sobel,hough y trim_image)
             billete = rotate_image(billete,UMBRAL_SOBEL,UMBRAL_MASCARA);
             //Si esto sucede, el objeto no fue rotado adecuadamente, quedo vertical
             //lo giro 90 para dejarlo horizontal
             if(billete.width()<billete.height()){
                 billete.rotate(90);
             }
          //billete.display();
          //los billestes recortados por el primer trim pueden tener partes que corresponden a otro billeste
          //debo eliminar estas partes pequeñas, hago una mascara, pongo 1 a todo lo distinto de cero que representa billete
          // (el fondo es negro)
          //etiqueto esa mascara y elimino de essas etiquetas la que tienen pocos pixeles
          //y recorto de nuevo el billete con esa nueva mascara de la cual se eliminaron los objetos pequeños
          CImg<bool>aux(billete.width(),billete.height()); aux.fill(0);
          cimg_forXY(billete,j,k)
          if( !(billete(j,k,0,0)==0 && billete(j,k,0,1)==0 && billete(j,k,0,2)==0) )
          aux(j,k)=1;
          vector<int>cant_pixel_por_etiqueta2;
          etiqueta=InundarEtiquetar(aux,cant_pixel_por_etiqueta2);
          etiqueta=descartar_mayores_menores(etiqueta,cant_pixel_por_etiqueta2,6000000,50000);
          billete=trim_image(billete,etiqueta); //salida billete bien recortado

          //para determinar si el billete esta al reves
          //si esta invertido lo giro 180
         mascara=billete.get_RGBtoHSI().get_channel(2).get_normalize(0,255).get_threshold(127);
        double aux1= mascara.get_crop(mascara.width()*0.0935483871,mascara.height()*0.42635658915,mascara.width()*0.25483870968,mascara.height()*0.85271317829).mean();
          if (aux1<0.97){
           billete.rotate(180);
          }
          billetes_rotados.push_back(billete);
         }
        billetes_rotados.display();


// sumo la cantidad de dinero
float suma=0;

for(int i=0;i<billetes_rotados.size();i++){

    mascara=billetes_rotados.at(i).get_RGBtoHSI().get_channel(2).get_normalize(0,255).get_threshold(127);
    mascara.crop(mascara.width()*0.2084,mascara.height()*0.0278,mascara.width()*0.29,mascara.height()*0.3585);
  //mascara.display();

 cant_pixel_por_etiqueta.clear();
 etiqueta=InundarEtiquetar(NOTimg(mascara),cant_pixel_por_etiqueta);
//Cuenta de la cantidad de rosas (Contar niveles de grises diferentes)
cout<< "cantidad de ronbos"<<cant_pixel_por_etiqueta.size()<<endl;
 etiqueta.display();

if(cant_pixel_por_etiqueta.size()==6)
    suma=suma+2;
if(cant_pixel_por_etiqueta.size()==4)
    suma=suma+10;
if(cant_pixel_por_etiqueta.size()==5)
    suma=suma+5;
if(cant_pixel_por_etiqueta.size()==2)
    suma=suma+50;
if(cant_pixel_por_etiqueta.size()==1)
    suma=suma+100;
if(cant_pixel_por_etiqueta.size()==3)
    suma=suma+20;
}

cout<<"Cantidad de Billetes "<< billetes_rotados.size()<<endl;
cout<<"Cantidad de dinero "<<suma<<endl;
    return 0;
}

