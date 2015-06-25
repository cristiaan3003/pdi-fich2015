#include "CImg.h"
#include <iostream>
#include <string>
using namespace std;

void LeeDir(string Archi[], string dire){
    string comando = "/bin/ls " + dire;
    FILE *proc = popen(comando.c_str(),"r");
    char buf[1024];
    int idx=0;
    while ( !feof(proc) && fgets(buf,sizeof(buf),proc) ){
	Archi[idx] = string(buf);
	string::size_type pos = Archi[idx].find_last_not_of("\n \t");
	if(pos != string::npos){
	    Archi[idx] = dire + "/" + Archi[idx].substr(0, pos+1); 
	} 
	idx++;
    }
}

int main(int argc,char **argv) {
    // Cantidad de imagenes y directorio donde estan guardadas
    string Archi[21];
    string DirIN="img";
    LeeDir(Archi, DirIN);
    cout << Archi[0]<<endl;
}

