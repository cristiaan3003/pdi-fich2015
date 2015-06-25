#include "funciones.h"

#define UMBRAL 230
#define nLineas 150
#define pesoBlur 12
#define umbralTexto 0.7
#define umbralBinario 0.4

int main()
{
    //imagen
    CImg<float> original;
    original.load("/home/asusn56/Copy cristiaan3003@gmail.com/PDI-2015/pdi-fich2015/Parcial de Practica/Escaner/2.png");


    // Primero elimino las imagenes y objetos grandes que me puedan molestar
            CImg<double> intensidad =  original.get_RGBtoHSI().get_channel(2);
            // Utilizo un pasa bajos para eliminar el texto
            CImg<double> blur = intensidad.get_blur(pesoBlur);//filtro de promedidado de CImg
            // Umbralizo la imagen para detectar donde no hay imagenes
            CImg<bool> textoMask = blur.get_threshold( umbralTexto );
            // En las regiones donde hay imagen ( textoMask es true ) coloco intensidad 1 (color de fondo)
            CImg<double> texto = intensidad;
            // Utilizo un umbral para detectar el texto
            CImg<bool> binarizada(texto.width(), texto.height(), 1, 1, false);

            cimg_forXY(texto,x,y) {
                if( !textoMask(x,y) )
                    texto(x,y)=1;
                binarizada(x,y) = texto(x,y) < umbralBinario;
            }
            CImgList<double> lista1( original, intensidad, blur, textoMask, texto, binarizada);
            lista1.display("Eliminar Imagenes", false);
            binarizada.display("binarizada");


            // Para calcular el angulo calculo la transformada de Hough y la multiplico por una imagen para pesar los angulos
              //giro la imagen de peso un poco girada
                    CImg<double> Hough = hough( binarizada );
                    //degrade : "1 / (1 + 2 * (((x/w)*2)-1) * (((x/w)*2)-1))"
                    CImg<double> peso(Hough.width(),Hough.height(),1,1,"1 / (1 + 2 * (((x/w)*2)-1) * (((x/w)*2)-1))",false);
                   // peso.display("peso");
                    Hough.mul( peso );
                    CImgList<double> lista2( Hough.get_div(peso), peso, Hough, original-InversaHough( Hough, nLineas ) );
                    lista2.display("Calcular angulo - Hough", false);

                    // Para calcular el angulo de giro de la imagen tomo los "nLineas" maximos de la imagen y los promedio
                    double angulo = 0;
                    for(int i=0; i<nLineas; i++) {
                        Pixel p = MaximoP( Hough );
                        angulo += p.x;
                        Hough( p.x, p.y ) = 0;
                    }
                    angulo /= (double)nLineas;
                    angulo -= (double)(Hough.width()/2);
                    angulo *= 90.0 / (double)(Hough.width()/2);
                    cout<<"Angulo de giro: "<<angulo<<endl;

                    // Giro la imagen y aplico un poco de zoom
                    int cx = original.width()/2;
                    int cy = original.height()/2;
                    float zoom = fabs(1 / cos( (M_PI/180.0)*angulo )) + 0.05;
                    CImg<double> final = original.get_rotate(angulo, cx, cy, zoom, 1, 2);
                    CImgList<double> lista3( original, final );
                    lista3.display("Resultado", false);


    return 0;
}

