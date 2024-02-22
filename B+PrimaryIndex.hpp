#ifndef PRIMARY_INDEX_HPP
#define PRIMARY_INDEX_HPP

#define PRIM_INDEX_FILE_NAME "primaryIndexFile"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include "hash.hpp"
using namespace std;

//Define a ordem da árvore
#define ORDER_M 1000

//Define a quantidade de apontadores para cada nó
#define N_POINTERS_PRIM 2 * ORDER_M

//Define a quantidade de chaves para cada nó
#define N_KEYS_PRIM N_POINTERS_PRIM - 1

typedef struct NodePrim{
	int size;
	int position;
	int key[2*ORDER_M];
	int pointer[2*ORDER_M+1];
} NodePrim;

typedef struct AuxNode{
	int key;
	int leftPointer;
	int rightPointer;
} AuxNode;

typedef struct Header{
	int rootPos;
	int nNodes;
} Header;

void insertPrimaryIndexFile(fstream *hashFile, fstream *primIdxFile);
void seek1(const char *caminhoArquivoDados, const char *caminhoArquivoIndice, int chave);

#endif