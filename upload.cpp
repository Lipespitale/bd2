#include "hash.hpp"
#include "B+PrimaryIndex.hpp"
#include "B+SecondaryIndex.hpp"
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>

void removePastFiles(){
    /* Remove arquivos anteriormente gerados no diretório
    */
    remove(HASH_FILE_NAME); //Remove arquivo hashing
    remove(PRIM_INDEX_FILE_NAME);
    remove(SEC_INDEX_FILE_NAME);
}
void copyStringToStr(char* a, string b, int size) {
    /*Copia uma string a para b
    */
    strncpy(a, b.c_str(), size);
    a[size-1] = 0;
}

Article parser(string line){
    /* Faz o parser de uma linha do arquivo de entrada e
    armazena as informações dos campos extraídos na estrutura
    Article.
    */
	Article article;
	string delimiter = "\";"; //O delimitador utilizado foi ";
	size_t pos = 0;
	string token;
	int column = 0;

	while ((pos = line.find(delimiter)) != string::npos) {
       
       token = line.substr(1, pos-1);
       
    
	    if (column == 0){
	    	article.id= atoi(token.c_str()); //Extrai id
	    }
	    else if(column == 1){
	    	copyStringToStr(article.title, token.c_str(), T_TITLE); //Extrai título
	    }
	    else if(column == 2){
	    	article.year = atoi(token.c_str()); //Extrai Ano
	    }
	    else if(column == 3){

            copyStringToStr(article.author, token.c_str(), T_AUTHOR); //Extrai autor
	    }
	    else if(column == 4){
	    	article.citations = atoi(token.c_str()); //Extrai Citações
	    }
        else if(column == 5){
            copyStringToStr(article.update, token.c_str(), T_UPDATE); //Extrai atualização
        }
	    line.erase(0, pos + delimiter.length());   
	    column ++;
	}

	if(line.length() > 0){
        token = line.substr(0, line.length()-1);

        if(strcmp(token.c_str(), "NULL") != 0){
            token = line.substr(1, line.length()-3); //Trata o caso em que o Snippet é NULL
        }

        copyStringToStr(article.snippet, token.c_str(), T_SNIPPET); //Extrai Snippet
	}

    return article;
}


int main(int argc, char *argv[]){
	if(argc<2) {
        cout << "Digite o nome do arquivo de entrada.\nEx: upload <file>\n";
        return 1;
    }

    ifstream file(argv[1]);
    string line;

    if (!file){
        printf("Arquivo não encontrado.\n");
        exit(1);
    }

    removePastFiles();                       // Remove arquivos antigos gerados, caso existam

    
    fstream *hashFile = new fstream(HASH_FILE_NAME,fstream::in|fstream::out|fstream::trunc|ios::binary); //Cria arquivo de dados
    initOutputFile(hashFile);                       // Aloca o arquivo de dados

    fstream *secondaryIndexFile = new fstream(SEC_INDEX_FILE_NAME,fstream::in|fstream::out|fstream::trunc|ios::binary); //Cria arquivo de indice secundário
    fstream *primaryIndexFile = new fstream(PRIM_INDEX_FILE_NAME,fstream::in|fstream::out|fstream::trunc|ios::binary); //Cria arquivo de indice primário

    cout << "Carregando os registros no arquivo de dados..."<<endl;

    while (getline(file, line)){
        Article article = parser(line); //Faz o parser de cada linha do arquivo de entrada
        insertHashFile(hashFile, article); 	//Insere artigo no arquivo de dados
    }

    cout << "Carregando os dados no arquivo de índice primário..." << endl;
    insertPrimaryIndexFile(hashFile, primaryIndexFile);

    cout << "Carregando os dados no arquivo de índice secundário..." << endl;
    insertSecondaryIndexFile(hashFile, secondaryIndexFile); //Carrega os dados no arquivo de índice secundário

    cout << "Pronto!" << endl;
    hashFile->close();
    primaryIndexFile->close();
    secondaryIndexFile->close();
    
}
