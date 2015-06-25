#include "funciones.h"
#define RADIO 8
#define RADIO2 15
#define UMBRAL1 200 //umbral borde
#define UMBRAL2 20 //controlar este umbral
#define UMBRAL3 127
#define RADIO_CIRCULOS 30

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
    CImg<float> img,img_intensidad,aux,aux2;
    CImg<float> etiqueta1,etiqueta2,etiqueta3;
    CImg<bool> mascara,mascara1,mascara2,mascara3,se1(3,3),se2(5,5),se3(3,3);
    se1.fill(1);//leno 3x3
    se2.fill(1);//lleno 5x5
    se3.fill(0,1,0,1,1,1,0,1,0);//cruz
    int mx,my;
    const char* path = "../../../../Parcial de Practica/Moscas/Platos04.jpg";
    img.load(path);
    //img.display();
    aux=img.get_blur(2);
    aux2=img.get_blur(8);
   // aux2.display();
    bool delacasa=false;

    CImgDisplay v1(img,"Presione sobre la sopa");

    while(!v1.is_closed()){
        v1.wait();
        if(v1.button()==1){
//coor de color(sopa)
            mx=v1.mouse_x();
            my=v1.mouse_y();

            float r0=aux2(mx, my, 0, 0);
            float g0=aux2(mx, my, 0, 1);
            float b0=aux2(mx, my, 0, 2);

            float dist=sqrt(powf(r0-221,2)+powf(g0-84,2)+powf(b0-2,2));
            if(dist<RADIO){ //es el color que busco
                delacasa=true;
                cout<<"DE LA CASA"<<endl;
            }
            else{etiqueta1=InundarEtiquetar(mascara1,cant_pixel_por_etiqueta1);
                etiqueta1=descartar_mayores_menores(etiqueta1,cant_pixel_por_etiqueta1,10000,50);
                etiqueta1.display("Etiquetado moscas borde");
                delacasa=false;
                cout<<"zapallo"<<endl;
            }

        }
    }




    //Proceso la mascara para dejarla binaria
    img_intensidad=img.get_channel(2).get_normalize(0,255);
    //img_intensidad.display("img intensidad");
    //get_histogram (nbins,mi,max)
    CImg <int> hist = img.get_histogram(256,0,255);
    //hist.display_graph();

   //moscas el el borde del plato
    mascara1 = NOTimg(img_intensidad.get_blur(1.5).get_threshold(UMBRAL1));
    //mascara1.display("borde plato");

   //todas las moscas de la imagen
    mascara2 = NOTimg(img_intensidad.get_blur(1.5).get_threshold(UMBRAL2));
    (img_intensidad,mascara2).display("todas las moscas");

    //moscas fuera del plato
    //segmento aca por que estoy apurado y no se como carajo hacerlo de otra forma
    float r0=210;
    float g0=40;
    float b0=47;
    float dist;
    for(unsigned i=0; i<aux.width(); i++){
        for(unsigned j=0; j<aux.height(); j++){
            dist=sqrt(powf(r0-aux(i,j,0,0),2)+powf(g0-aux(i,j,0,1),2)+powf(b0-aux(i,j,0,2),2));
            if(dist>RADIO2){
                aux(i,j,0,0)=0;
                aux(i,j,0,1)=0;
                aux(i,j,0,2)=0;
            }
        }
    }

    //aux.display("moscas fuera del plato color");
    mascara3=aux.get_RGBtoHSI().get_channel(2).get_normalize(0,255).get_threshold(UMBRAL3);
            mascara3=NOTimg(mascara3);
            //mascara3.erode(se2).erode(se1);
            mascara3.display("moscas fuera del plato");



    //Operacion de etiquetado
//LABEL CC cuenta mal cuando tiene cierta pendiente que no se muy bien como o por que
            //asi que me implemente mi propio etiquetado con inundar
            //anda  sin importar que tan retorcida la figura.
 //RECORDAR: InundarEtiquetar -->etiqueta arranca en 2, por lo que el valor de la etiqueta 2,3,4...n
            //va desfazado en de unidades con la posicion de si cantidad de pixeles en cant_pixel_por_etiqueta
            //osea la cant de pixeles de etiqueta 2, va estar en la posicion 0 de cant_pixel_por_etiqueta
            // cant pixeles etiqueta 3 en la posicon 1 de cant_pixel_por_etiqueta y asi...
   vector<int> cant_pixel_por_etiqueta1;
    vector<int> cant_pixel_por_etiqueta2;
     vector<int> cant_pixel_por_etiqueta3;
    etiqueta1=InundarEtiquetar(mascara1,cant_pixel_por_etiqueta1);
    etiqueta1=descartar_mayores_menores(etiqueta1,cant_pixel_por_etiqueta1,10000,50);
    etiqueta1.display("Etiquetado moscas borde");

    etiqueta2 = InundarEtiquetar(mascara2,cant_pixel_por_etiqueta2);
    etiqueta2=descartar_mayores_menores(etiqueta2,cant_pixel_por_etiqueta2,10000,50);
    etiqueta2.display("Etiquetado tosdas moscas");

    etiqueta3 = InundarEtiquetar(mascara3,cant_pixel_por_etiqueta3);
    etiqueta3=descartar_mayores_menores(etiqueta3,cant_pixel_por_etiqueta3,10000,100);
    etiqueta3.display("Etiquetado moscas fuera");

    //Cuenta (Contar niveles de grises diferentes)
    int moscas_borde_plato = cant_grises(etiqueta1);// en el borde del plato
    int total_moscas = cant_grises(etiqueta2); //todas las moscas
    int moscas_fuera_plato = cant_grises(etiqueta3);//fuera del plato

           int moscas_centro_plato = total_moscas-moscas_borde_plato-moscas_fuera_plato;
           int moscas_en_plato= moscas_borde_plato+moscas_centro_plato;

      cout<<"en el borde: "<<moscas_borde_plato<<endl<<" total moscas: "<< total_moscas<<endl<<" moscas fuera: "<<moscas_fuera_plato<<endl;
 cout<<"moscas en el plato: "<<moscas_en_plato<<endl<<" moscas en sopa: "<<moscas_centro_plato<<endl;

 if(delacasa)
     cout<<"plato de la casa"<<endl;
 else
     cout<<"plato de zapallo"<<endl;
 if(    (!delacasa && (moscas_centro_plato<=3)) || (delacasa && (moscas_centro_plato<=4))){
     cout<<"buen plato"<<endl;

 }else
 {
     cout<<"no sirve el plato"<<endl;
 }


    return 0;
}
