#include "funciones.h"
///CImg<T>& noise	(sigma,noise_type = 0 )
///Add random noise to pixel values.
///Parameters
///sigma	Amplitude of the random additive noise. If sigma<0, it stands for a percentage of the global value range.
///noise_type Type of additive noise (can be 0=gaussian, 1=uniform, 2=Salt and Pepper, 3=Poisson or 4=Rician).
///
int delta=50;
int etiqueta=256;
int max_segm=3;
int main()
{
    unsigned mx, my;
    CImg<unsigned char> img,imgnueva;
    const char* path = "../../../../images/bone.tif";
    img.load(path);

        CImgDisplay v1,v2,v3;
        v1.assign(img);
        v1.set_title("click");
        v2.assign(img);
        v2.set_title("original");
        v3.assign(img);
        v3.set_title("nueva");

            while(!v1.is_closed() && !v2.is_closed() && !v3.is_closed()  ){
                v1.wait();
                if(v1.button()==1){
                    //MODO DE SELECCION MANUAL DE LA SEMILLA
                    mx=v1.mouse_x();
                    my=v1.mouse_y();
                    imgnueva= region_growing(img,mx,my,delta,etiqueta);
                    v3.render(imgnueva);
                    v3.paint();
                }

            }

//MODO SELECCION AUTOMATICA DE SEMILLA
(img, autom_seg_region_growed(img, delta, etiqueta, max_segm)).display("Imagen Original, segmentacion automatica");
        // orig: imagen a procesar
        // x,y: posición de la semilla
        // delta: define el rango de pertenencia como [semilla-delta/2, semilla+delta/2]
        // etiqueta: nro de la etiqueta, no debe pertenecer al rango


    return 0;
}

