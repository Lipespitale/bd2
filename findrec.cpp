#include <iostream>     
#include "hash.hpp"

int main(int argc, char *argv[])
{
    if(argc<2) {
        cout << "Digite o ID do registro a ser procurado.\nEx: findrec <ID>\n";
        return 1;
    }

    fstream *hashFile = new fstream(HASH_FILE_NAME,fstream::in|ios::binary);

    if (hashFile->is_open()) {
        findRegisterById(hashFile,atoi(argv[1]));   
    }
    else {
        cout << "Erro ao abrir o arquivo.\n";
    }
    return 0;
}