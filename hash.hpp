#ifndef HASH_H
#define HASH_H

using namespace std;
#include <fstream>

//Define o tamanho dos campos de tipo alfa
#define T_TITLE 300
#define T_AUTHOR 150
#define T_UPDATE 20
#define T_SNIPPET 1024

/*Define a quantidade de buckets e a quantidade
máxima de registros em um bloco*/
#define N_BUCKETS 270973
#define N_REGISTERS 7

//Define o tamanho do corpo do bloco
#define T_BODY N_REGISTERS * (12 + T_TITLE +  T_AUTHOR +  T_UPDATE + T_SNIPPET)

//Define o nome do arquivo hashing
#define HASH_FILE_NAME "hashFile"

struct Article{
    unsigned int id;
    char title[T_TITLE];
    unsigned int year;
    char author[T_AUTHOR];
    unsigned int citations;
    char update[T_UPDATE];
    char snippet[T_SNIPPET];
}; 

struct Block{
    unsigned int nRegisters; //Guarda informação sobre a quantidade de registros armazenados no bloco
    char body[T_BODY];       //Vetor de artigos
};

void initOutputFile(fstream *f);
bool insertHashFile(fstream *f, Article article);
Article findRegisterById(fstream *f,int id);
Article findRegisterByTitle(fstream *f, int position, char title[T_TITLE]);
void printRegister(Article article);

#endif