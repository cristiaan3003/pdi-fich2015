#include <funciones.h>


//ejemplo como sacar un maximo
//std::vector<int> v{3, 1, 4, 1, 5, 9};
//  std::vector<int>::iterator result = std::max_element(std::begin(v), std::end(v));
//  std::cout << "Maximo en la posicion: " << std::distance(std::begin(v), result);


/////COLOR SLICING rbg
///
////Defino el color
//CImg<float> color(1,1,1,3);
//color(0,0,0,0)=202;
//color(0,0,0,1)=163;
//color(0,0,0,2)=88;
////Hago colourslicing de la imagen con ese color
//CImg<float> amarillo= ColorMaskRGB(img.get_blur(1.4),color,10);
//CImg<bool>mascara_amarillo=amarillo;
//(amarillo,mascara_amarillo).display("amarrillo y mascara");
//mascara_amarillo.erode(se2);
//mascara_amarillo.dilate(se2);
//mascara_amarillo=relleno_automatico(mascara_amarillo);
//mascara_amarillo.display("relleno auto"); //ya tengo la mascara, se la paso a sobel para detectar borde , linea, etc




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

void ComposeRGB(CImg<double> &img, CImg<double> &r, CImg<double> &g, CImg<double> &b){
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
#define UMBRAL 30
#define nLineas 10
#define nLinea 50

#define UMBRAL_SOBEL 50
#define UMBRAL_MASCARA 2
int main()
{
    CImgList<double>lista_imagenes;
    CImg<float> img,img_denoise,img_gris;
    CImg<bool> se1(3,3);
    se1.fill(1);
    //const char* path = "../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f01.jpg";
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f01.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f02.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f03.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f04.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f05.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f06.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f07.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f08.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f08.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f10.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f11.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f12.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f13.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f14.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f15.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f16.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f17.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f18.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f19.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f20.jpg");
    lista_imagenes.push_back(img);
    img.load("../../../../Parcial de Practica/PARCIAL PDI 2015/TRAIN/f21.jpg");
    lista_imagenes.push_back(img);

    //test
    //for(int i=0;i<lista_imagenes.size();i++)
     //   lista_imagenes.at(i).rotate(60);


    CImg<float>promedio;
    promedio=reducRuido(lista_imagenes);//pasar lista de imagenes con ruido

    promedio.display(""); // en este punto tengo la imagen  limpia supuestamente

    // 45 50 32  --> color del rio
    // gradas -- 83 97 106
    ///COLOR SLICING rbg

    //Defino el color
    CImg<float> color(1,1,1,3);
    color(0,0,0,0)=83;
    color(0,0,0,1)=97;
    color(0,0,0,2)=102;
    //Hago colourslicing de la imagen con ese color
    CImg<float> mascara_color= ColorMaskRGB(promedio.get_blur(1.8),color,10);
    CImg<bool>mascara=mascara_color;
    //(mascara_color,mascara).display("mascara");
    mascara=nerode(mascara,se1,8);
    mascara.display("mascara");


    CImg<float>copia=promedio;
    CImg<bool>contorno=mascara;
    // Para calcular el angulo calculo la transformada de Hough y la multiplico por una imagen para pesar los angulos
      //giro la imagen de peso un poco.
            CImg<double> Hough = hough( contorno);
            //peso imagen un poco girada
            CImg<double> peso(Hough.width(),Hough.height(),1,1,"1 / (1 + 2 * (((x/w)*2)-1) * (((x/w)*2)-1))",false);
            Hough.mul( peso );

            CImgList<double> lista2( Hough.get_div(peso), peso, Hough, promedio-InversaHough( Hough, nLineas ) );
            lista2.display("Calcular angulo - Hough", false);

            // Para calcular el angulo de giro de la imagen tomo los "nLineas" maximos de la imagen y los promedio
            double angulo = 0;
            for(int i=0; i<nLineas; i++) {
                Pixel p = MaximoP( Hough );
                angulo += p.x;
                Hough( p.x, p.y ) = 0;
            }
            angulo /= (double)nLineas;
            angulo -= (double)(Hough.height()/2);
            angulo *= 90.0 / (double)(Hough.height()/2);

           // if(angulo>0 && angulo<90)

            // Si theta es negativo, tomo consideracion especial
            //angulo = (angulo < 0) ? 180 + angulo : angulo;


            // Me va dar 0 (<45), 90 (< 135), 180 (< 225), 270 (<315), o 360
            //angulo = round(angulo / 90) * 90;

            //angulo=-1*angulo;
            cout<<"Angulo de giro: "<<angulo<<endl;
            copia.rotate(angulo);
            mascara.rotate(angulo);
            copia.display("rotada");



    return 0;
}

