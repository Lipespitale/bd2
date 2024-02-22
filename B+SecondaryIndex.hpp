#ifndef SEC_INDEX_H
#define SEC_INDEX_H

using namespace std;
#include <fstream>
#include "hash.hpp"

//Define a ordem da árvore
#define M 50

//Define a quantidade de apontadores para cada nó
#define N_POINTERS 2 * M + 1

//Define a quantidade de chaves para cada nó
#define N_KEYS 2 * M

//Define o nome do arquivo de índice secundário
#define SEC_INDEX_FILE_NAME "secondaryIndexFile"

struct Node{
	int size;               //Quantidade de chaves ocupadas no nó
	int position;           //Posicao do nó no arquivo de índice.
	char key[N_KEYS][T_TITLE]; //Valores de busca (chaves) do nó. A chave é o título.
	int pointer[N_POINTERS];    // Apontadores do nó.    
};

struct NodeAux{
	char key[T_TITLE];      //Chave do nó. A chave é o título.
	int leftPointer;       //Ponteiro da esquerda
	int rightPointer;      //Ponteiro da direita
};

struct Head{
	int rootPos;             //Posição da raiz
	int nNodes;             //Quantidade de nós
};



void insertSecondaryIndexFile(fstream *hashFile, fstream *secIdxFile);
int SearchTitleOnTree(fstream *caminhoArquivoDados, fstream *caminhoArquivoIndice, char chave[300]);

#endif