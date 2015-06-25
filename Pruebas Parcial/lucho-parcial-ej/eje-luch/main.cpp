#include "funciones.h"

#define PATH "../../../../Parcial de Practica/parcial/parking4.png"
#define CP_R 202
#define CP_G 163
#define CP_B 88
#define RADIO_CS 50
#define TOL_PIX 8000
#define CPA_R 208
#define CPA_G 109
#define CPA_B 161
#define RADIO_CSA 50
#define ANCHO_EST 1000

vector<punto> localizar(CImg<bool> mascara,int siguientes,int umbral,int tol){
    //LOCALIZACION EN X

    //Vector para averiguar la posicion en x
    int cont_aux=0;
    vector<int> sumatoria_x;
    for(int i=0;i<mascara.width();i++){
        for(int j=0;j<mascara.height();j++){
             if(mascara(i,j)==1){
                 cont_aux++;
             }
        }
        sumatoria_x.push_back(cont_aux);
        cont_aux=0;

    }

    //Detecto las coordenadas maximas y minimas en x
    int x_min=0,x_max=0;
    bool encontro = false;
    //Recorro de adeante para atras
    for(int i=0;i<mascara.width();i++){
        //Cuento las siguentes 20 para ver si realmente es el lugar
        int sig_20=0;
        for(int j=i;j<i+siguientes;j++){
            if(j<mascara.width()){
                sig_20+=sumatoria_x[j];
            }
        }

        if(sumatoria_x[i]>umbral && sig_20>tol && encontro == false){
            x_min=i;
            encontro=true;
        }
    }

    //Recorro de atras para adelante
    encontro=false;
    for(int i=0;i<mascara.width();i++){
        //Cuento las siguentes 20 para ver si realmente es el lugar
        int sig_20=0;
        for(int j=i;j<i+siguientes;j++){
            if(j<mascara.width()){
                sig_20+=sumatoria_x[mascara.width()-j];
            }
        }

        if(sumatoria_x[mascara.width()-i]>umbral && sig_20>tol && encontro == false){
            x_max=mascara.width() - i;
            encontro=true;
        }
    }

    //cout<<x_min<<" "<<x_max<<endl;

    //LOCALIZACION EN Y

    //Vector para averiguar la posicion en y
    cont_aux=0;
    vector<int> sumatoria_y;
    for(int j=0;j<mascara.height();j++){
        for(int i=0;i<mascara.width();i++){
             if(mascara(i,j)==1){
                 cont_aux++;
             }
        }
        sumatoria_y.push_back(cont_aux);
        cont_aux=0;

    }

    //Detecto las coordenadas maximas y minimas en x
    int y_min=0,y_max=0;
    encontro = false;
    //Recorro de adeante para atras
    for(int i=0;i<mascara.height();i++){
        //Cuento las siguentes 20 para ver si realmente es el lugar
        int sig_20=0;
        for(int j=i;j<i+siguientes;j++){
            if(j<mascara.height()){
                sig_20+=sumatoria_y[j];
            }
        }

        if(sumatoria_y[i]>umbral && sig_20>tol && encontro == false){
            y_min=i;
            encontro=true;
        }
    }

    //Recorro de atras para adelante
    encontro=false;
    for(int i=0;i<mascara.height();i++){
        //Cuento las siguentes 20 para ver si realmente es el lugar
        int sig_20=0;
        for(int j=i;j<i+siguientes;j++){
            if(j<mascara.height()){
                sig_20+=sumatoria_y[mascara.height()-j];
            }
        }

        if(sumatoria_y[mascara.height()-i]>umbral && sig_20>tol && encontro == false){
            y_max=mascara.height() - i;
            encontro=true;
        }
    }

    //cout<<y_min<<" "<<y_max<<endl;

    //Armo el vector final
    punto a,b;
    a.x=x_min;a.y=y_min;
    b.x=x_max;b.y=y_max;
    vector<punto> final;
    final.push_back(a);
    final.push_back(b);

    return final;

}

int main()
{
    //Carga de imagen a procesar
    CImg<float> original;
    original.load(PATH);

    //En primera instancia detecto si el estacionamiento para discapacitado esta ocupado o no
    bool ocupado=false;

    //Defino el color del lugar para discapacitados
    CImg<float> color(1,1,1,3);
    color(0,0,0,0)=CP_R;
    color(0,0,0,1)=CP_G;
    color(0,0,0,2)=CP_B;

    //Hago colourslicing de la imagen con ese color
    CImg<float> img_cs = ColorMaskRGB(original,color,RADIO_CS);

    //Cuento la cantidad de pixeles distintos de negro
    int cant_pixeles = 0;
    cimg_forXY(img_cs,i,j){
        if(img_cs(i,j,0,0)==CP_R && img_cs(i,j,0,1)==CP_G && img_cs(i,j,0,2)==CP_B){
            cant_pixeles++;
        }
    }

    //cout<<cant_pixeles<<endl;

    //Si la cantidad de pixeles es menor a 8000 el lugar esta ocupado
    if(cant_pixeles < TOL_PIX){
        ocupado = true;
        cout<<"El lugar para discapacitados esta OCUPADO"<<endl;
    }else{
        cout<<"El lugar para discapacitados esta LIBRE"<<endl;
    }

    //En el caso de que este libre calculo la trayectoria a seguir
    if(ocupado == false){
        //Calculo la posicion y dimensiones del lugar para discapacitados

        CImg<bool> mascara = img_cs.get_RGBtoHSI().channel(0).normalize(0,1).threshold(0.5);
        mascara = apertura(mascara,mask(3));
        mascara = relleno_automatico(mascara);
        mascara.display();
        vector<punto> loc_es = localizar(mascara,20,50,1500);

        //Detecto la posicion del auto
        //Defino el color del lugar para discapacitados
        CImg<float> colora(1,1,1,3);
        colora(0,0,0,0)=CPA_R;
        colora(0,0,0,1)=CPA_G;
        colora(0,0,0,2)=CPA_B;

        //Hago colourslicing de la imagen con ese color
        CImg<float> aux = original.get_crop(0,original.height()/2,original.width(),original.height());
        CImg<float> color_auto = ColorMaskRGB(aux,colora,RADIO_CSA);

        CImg<bool> mascara_a = color_auto.get_RGBtoHSI().channel(0).normalize(0,1).threshold(0.5);
        mascara_a = apertura(mascara_a,mask(3));
        mascara_a = relleno_automatico(mascara_a);

        //Averiguo las cordenadas del centro del auto en base al color rosa
        vector<punto> loc_au=localizar(mascara_a,3,1,1);
        loc_au[0].y = loc_au.at(0).y + original.height()/2;
        loc_au[1].y = loc_au.at(1).y + original.height()/2;


        //Dibujo las lineas con las coordenadas averiguadas

        //Linea desde el auto hasta el frente del estacionamiento
        CImg<bool> indicaciones(original.width(),original.height(),1,1,0);

        //Punto en el capot del auto
        int x_la=(loc_au[0].x+loc_au[1].x)/2;
        int y_la=(loc_au[0].y+loc_au[1].y)/2;

        //Punto enfrente del estacionamiento
        int x_fe=0,y_fe=0;
        if(loc_es[0].x+loc_es[1].x < ANCHO_EST){//Caso que no es diagonal
            x_fe=(loc_es[0].x+loc_es[1].x)/2;
            y_fe=(loc_au[0].y+loc_au[1].y)/2;
        }else{//Caso que es diagonal
            x_fe=(loc_es[0].x+loc_es[1].x)/2 - (loc_es[1].x-loc_es[0].x);
            y_fe=(loc_au[0].y+loc_au[1].y)/2;
        }

        //Punto centrado en el lugar para estacionar
        int x_le=(loc_es[0].x+loc_es[1].x)/2;
        int y_le=(loc_es[0].y+loc_es[1].y)/2;

        indicaciones.draw_line(x_la,y_la,x_fe,y_fe,white);
        indicaciones.draw_line(x_fe,y_fe,x_le,y_le,white);

        //Dibujo las indicaciones sobre la imagen original
        cimg_forXY(original,i,j){
            if(indicaciones(i,j)==1){
                original(i,j,0,0)=255;
                original(i,j,0,1)=0;
                original(i,j,0,2)=0;
            }
        }

        original.display();

    }

    //(original,img_cs).display();

    return 0;
}

