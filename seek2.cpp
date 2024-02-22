#include <iostream>
#include <stdlib.h>
#include <string>

#include "B+SecondaryIndex.hpp"
#include "hash.hpp"

int main(int argc, char *argv[]){
	if(argc != 2){
		cout << "Digite o título do registro a ser procurado.\nEx: seek2 <Titulo>\n" <<endl;
		return 1;
	}
    fstream *hashFile = new fstream(HASH_FILE_NAME,fstream::in|ios::binary);
    fstream *secIdxFile = new fstream(SEC_INDEX_FILE_NAME,fstream::in|ios::binary);

    if (hashFile->is_open() && secIdxFile->is_open()) {
        SearchTitleOnTree(hashFile,secIdxFile, argv[1]);   
    }
    else {
        cout << "Erro ao abrir o arquivo.\n";
    }
    return 0;
	
}