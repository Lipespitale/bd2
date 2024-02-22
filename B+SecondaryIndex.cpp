#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include "B+SecondaryIndex.hpp"
#include "hash.hpp"

Head *head = NULL;
fstream *hashF, *secF;

//begin Creation and insertion

void updateHead(){
	/*Escreve/atualiza o cabeçalho no arquivo
	*/
	//Posiciona cursor no início do arquivo
	secF->seekp(0,secF->beg);
	//Escreve o cabeçalho no arquivo					
	secF->write((char*)head, sizeof(Head));		
}

void initHead(){
	/*Aloca e escreve o cabeçalho de dados no arquivo
	*/
	head = (Head *)malloc(sizeof(Head));
	head->rootPos = -1;
	head->nNodes = 0;
	//Escreve cabeçalho no arquivo
	updateHead(); 						
}

Node* createNode(){
	/*Aloca um novo nó
	*/
	Node *node = (Node *) malloc(sizeof(Node));
	//Inicializa tamanho, posição do nó
	node->size = 0;					
	node->position = 0;

	//Inicializa chaves e apontadores
	for(int i = 0; i < N_KEYS - 1; i++){
		strcpy(node->key[i],"\0");
		node->pointer[i] = 0;
	}
	node->pointer[N_POINTERS - 1] = 0;

	return node;
}

int insertNodeOnFile(Node *node){
	/*Escreve um nó no arquivo de índice
	*/
	//Itera a quantidade de nós adicionados
	head->nNodes++;
	//Posiciona o cursor na devida posição											
	secF->seekp(head->nNodes*sizeof(Node), secF->beg);
	//Escreve o nó no arquivo de indice	   
	secF->write((char*)node,sizeof(Node));
	//Atualiza o cabeçalho com a nova quantidade de nós				   
	updateHead();

	return head->nNodes;
}

void updateNodeOnFile(Node *node){
	/*Atualiza nó no arquivo de indice
	*/
	//Posiciona o cursor na devida posição
	secF->seekp(node->position*sizeof(Node), secF->beg);
	//Escreve o nó no arquivo
	secF->write((char*)node,sizeof(Node));
}

void InsertKeyOnAvailableLeaf(Node *node, char key[T_TITLE], int pointer){
	/*Insere chave em uma página de dados que ainda tem espaço disponível
	*/
	int i;
	//Percorre as chaves já existentes do nó
	for(i=0; i< node->size; i++){
		//Se a chave existente na posição i for maior que a chave atual
		if(strcmp(node->key[i], key) > 0){
			for(int j=node->size; j>i; j--){
				//Desloca chave existente
				strcpy(node->key[j],node->key[j-1]);
				node->pointer[j] = node->pointer[j-1];
			}
			break;
		}
	}
	//Insere chave no nó
	strcpy(node->key[i],key);
	node->pointer[i] = pointer;
	node->size++;
	//Atualiza o nó no arquivo
	updateNodeOnFile(node);
}

void InsertKeyOnAvailableNode(Node *node, char key[T_TITLE], int pointer){
	/*Insere chave em uma página que ainda tem espaço disponível
	*/

	int i;
	//Percorre as chaves já existentes do nó
	for(i=0; i < node->size; i++){
		//Se a chave existente na posição i for maior que a chave atual
		if(strcmp(node->key[i],key) > 0){
			//Desloca chave existente
			for(int j=node->size; j>i; j--){
                strcpy(node->key[j],node->key[j-1]);
				node->pointer[j+1] = node->pointer[j];
			}
			break;
		}
	}
	//Insere chave no nó
	strcpy(node->key[i],key);
	node->pointer[i+1] = -1 * pointer;
	node->size++;
	//Atualiza o nó no arquivo
	updateNodeOnFile(node);
}

NodeAux* insertKeyOnFullLeaf(Node *node, char key[T_TITLE], int pointer){
	/* Trata a inserção da chave para o caso em que a página de dados já está cheia
	*/
	int added=0;
    char pivot[T_TITLE];
	Node *newNode = createNode();
	NodeAux *newParent = (NodeAux *) malloc(sizeof(NodeAux));
	strcpy(pivot,node->key[M]);

	if(strcmp(key,pivot) < 0){
		int i,j=0;
		for(i=M; i<=N_KEYS - 1; i++){
			strcpy(newNode->key[j],node->key[i]);
			newNode->pointer[j] = node->pointer[i];
			newNode->size++;
			node->pointer[i] = 0;
			strcpy(node->key[i],"\0");
			node->size--;
			j++;
		}
		for(i=M-1; i>=0; i--){
			if(strcmp(key,node->key[i]) > 0){
				break;
			}else{
				strcpy(node->key[i+1], node->key[i]);
				node->pointer[i+1] = node->pointer[i];
			}
		}
		strcpy(node->key[i+1],key);
		node->pointer[i+1] = pointer;
		node->size++;
	}else{
		int i,j=0;
		for(i=M; i<=N_KEYS - 1; i++){
			if(added == 0 && strcmp(key,node->key[i]) < 0){
				strcpy(newNode->key[j],key);
				newNode->pointer[j] = pointer;
				newNode->size++;
				j++;
				added = 1;
			}
			strcpy(newNode->key[j],node->key[i]);
			newNode->pointer[j] = node->pointer[i];
			newNode->size++;
			strcpy(node->key[i],"\0");
			node->pointer[i] = 0;
			node->size--;
			j++;
		}
		if(added == 0){
			strcpy(newNode->key[j],key);
			newNode->pointer[j] = pointer;
			newNode->size++;
		}
	}
	newNode->pointer[N_POINTERS - 1] = node->pointer[N_POINTERS - 1];
	newNode->position = insertNodeOnFile(newNode);
	node->pointer[N_POINTERS - 1] = -1 * newNode->position;

	updateNodeOnFile(node);
	updateNodeOnFile(newNode);

	strcpy(newParent->key, pivot);
	newParent->leftPointer = -1 * node->position;
	newParent->rightPointer = -1 * newNode->position;

	free(newNode);
	return newParent;
}

NodeAux* insertKeyOnFullNode(Node *node, char key[T_TITLE], int pointer){
	/* Trata a inserção da chave para o caso em que a página já está cheia
	*/
	int added=0;
    char pivot[T_TITLE];
	Node *newNode = createNode();
	NodeAux *newParent = (NodeAux *) malloc(sizeof(NodeAux));
	strcpy(pivot,node->key[M]);
	strcpy(node->key[M],"\0");

	node->size--;

	if(strcmp(key,pivot) < 0){
		int i,j=0;
		newNode->pointer[0] = node->pointer[M+1];
		for(i=M+1; i<=N_KEYS - 1; i++){
			strcpy(newNode->key[j],node->key[i]);
			newNode->pointer[j+1] = node->pointer[i+1];
			newNode->size++;
			strcpy(node->key[i],"\0");
			node->pointer[i+1] = 0;
			node->size--;
			j++;
		}
		for(i=M; i>0; i--){
			if(strcmp(key,node->key[i]) > 0){
				break;
			}else{
				strcpy(node->key[i],node->key[i-1]);
				node->pointer[i+1] = node->pointer[i];
			}
		}
		strcpy(node->key[i],key);
		node->pointer[i+1] = -1 * pointer;
		node->size++;
	}

	else{
		int i,j=0;
		newNode->pointer[0] = node->pointer[M+1];
		for(i=M+1; i<=N_KEYS - 1; i++){
			if(added == 0 && strcmp(key,node->key[i]) < 0){
				strcpy(newNode->key[j],key);
				newNode->pointer[j+1] = -1 * pointer;
				newNode->size++;
				j++;
				added = 1;
			}
			strcpy(newNode->key[j],node->key[i]);
			newNode->pointer[j+1] = node->pointer[i+1];
			newNode->size++;
			node->pointer[i+1] = 0;
			strcpy(node->key[i],"\0");
			node->size--;
			j++;
		}
		if(added == 0){
			strcpy(newNode->key[j], key);
			newNode->pointer[j+1] = -1 * pointer;
			newNode->size++;
		}
	}
	
	newNode->position = insertNodeOnFile(newNode);
	updateNodeOnFile(newNode);
	updateNodeOnFile(node);

	strcpy(newParent->key, pivot);
	newParent->leftPointer = -1 * node->position;
	newParent->rightPointer = -1 * newNode->position;
	free(newNode);
	return newParent;
}

Node* getNodeFromFile(int position){
	/*Retorna um nó do arquivo de indice de acordo
	com a posição informada.
	*/
	if(position > 0){
		cout << "Endereço errado" << endl;
		return NULL;
	}

	Node *node = createNode();
	//Posiciona o cursor na posição informada
	secF->seekg(-1 * position * sizeof(Node),secF->beg);
	//Copia o nó 
	secF->read((char *)node, sizeof(Node));

	return node;
}

NodeAux* addKeyOnTree(Node *node, char key[T_TITLE], int pointer){
	/* Função de inserção da chave na árvore
	*/
	NodeAux *index = NULL;

	//Caso que é página de índice
	if(node->pointer[0] < 0){
		int position;
		int minPos = 0;
		int maxPos=node->size-1;

		//Acha a devida posição do apontador 		
		while(minPos <= maxPos){
			position = (minPos + maxPos) / 2;
            //Quando a chave atual é menor que uma chave já existente na posiçaõ
			if(strcmp(key,node->key[position]) < 0){
				maxPos = position - 1;
			}
			//Quando a chave atual é maior ou igual a uma chave já existente na posição
			else{
				minPos = position + 1;
			}
		}
		//O apontador deve estar na primera posição
		if(maxPos < 0){
			maxPos = 0;
		}
		//O apontador está na posição posterior à chave já existente que é menor ou igual a chave atual
		else if(strcmp(key,node->key[maxPos]) >= 0){
			maxPos++;
		}
		position = maxPos;

		index = addKeyOnTree(getNodeFromFile(node->pointer[position]),key,pointer);

		if(index!= NULL){
			//Se a página ainda tiver espaço
			if(strcmp(node->key[N_KEYS - 1], "\0") == 0){
				InsertKeyOnAvailableNode(node, index->key, -1 * index->rightPointer);
				free(index);
				index = NULL;
			}
			//Se a página não tiver mais espaço
			else{
				NodeAux *novoRetorno = insertKeyOnFullNode(node, index->key, -1 * index->rightPointer);
				free(index);
				index = novoRetorno;
			}
		}
	}
	//Caso que é página de dados(folha)
	else{
		//Se a página ainda tiver espaço
		if(strcmp(node->key[N_KEYS - 1],"\0") == 0){
			InsertKeyOnAvailableLeaf(node,key,pointer);
		}
		//Se a página não tiver mais espaço
		else{
			index = insertKeyOnFullLeaf(node,key,pointer);
		}
	}
	free(node);
	return index;
}

void populateFile(){
	/*Percorre todos os blocos do arquivo de dados e 
	insere o título de cada registro na árvore
	*/
	Block buffer = {0};
	int pos;

	//Posiciona cursor no início do arquivo de dados
	hashF->clear();
	hashF->seekg(0,hashF->beg);

	//Insere o título de todos os registros do arquivo de dados na árvore
	for (int i = 0; i <N_BUCKETS; i++) {
		//Copia o bucket para o buffer
        hashF->read((char*)&buffer,sizeof(Block));
		//Se o bucket não for vazio
        if(buffer.nRegisters > 0) {
            Article *v_article = (Article*)&buffer.body;
			//Percorre os registros do bloco
            for(int j=0; j<buffer.nRegisters; j++){
				//Insere o título (chave) do registro na árvore
				NodeAux *page = addKeyOnTree(getNodeFromFile(head->rootPos),v_article[j].title,i);
				
				if(page != NULL){
					//Aloca um novo nó
					Node *node = createNode();
					//O novo nó recebe as informações do nó criado na árvore
					strcpy(node->key[0],page->key);
					node->pointer[0] = page->leftPointer;
					node->pointer[1] = page->rightPointer;
					node->size++;
					//Insere o nó no arquivo
					int pos = insertNodeOnFile(node);
					node->position = pos;
					updateNodeOnFile(node);
					//Atualiza posição da raiz
					head->rootPos = -1 * pos;
					updateHead();

					free(node);
					free(page);
				}
			}
        }
    }
}

void insertSecondaryIndexFile(fstream *hashFile, fstream *secIdxFile){
	/*Faz upload dos dados no arquivo de índice secundário
	*/
	hashF =  hashFile;
	secF = secIdxFile;

	//Inicializa cabeçalho no arquivo
	initHead();										

	//Aloca nó raiz 
	Node *root = createNode();
	//Escreve nó raiz no arquivo						
	root->position = insertNodeOnFile(root);
	//Atualiza a posição do nó raiz no arquivo
	updateNodeOnFile(root);
	free(root);

	//Popula o arquivo de indices
	populateFile();
}

//end Creation and Insertion




//begin Search
void getHead(){
	/* Pega a estrutura cabeçalho do arquivo de indice
	*/
	//Se cabeçalho não existir, aloca
	if(head == NULL){
		head = (Head *)malloc(sizeof(Head));
	}
	//Posiciona o cursor para o início do arquivo
	secF->seekg(0,secF->beg);
	//Copia o cabeçalho do arquivo para head
	secF->read((char *)head, sizeof(Head));
}

int SearchTitleOnTree(fstream *caminhoArquivoDados,fstream *caminhoArquivoIndice, char key[T_TITLE]){
	hashF = caminhoArquivoDados;
	secF = caminhoArquivoIndice;

	int ponteiro, pos, minPos, maxPos, pPosition, i, blocks =0;
	Node *node,*aux;

	//Carrega cabeçalho
	getHead();
	
	pos = head->rootPos;

	while(1){
		//Pega o nó no arquivo
		node = getNodeFromFile(pos);
		blocks++;

		//Busca posição do apontador
		if(node != NULL){
			if(node->pointer[0] < 0){
				minPos = 0;
				maxPos=node->size-1;
				//Acha a devida posição do apontador
				while(minPos <= maxPos){
					pPosition = (minPos + maxPos) / 2;
					//Quando a chave atual é menor que uma chave já existente na posiçaõ
					if(strcmp(key,node->key[pPosition]) < 0){
						maxPos = pPosition - 1;
					}
					//Quando a chave atual é maior ou igual a uma chave já existente na posição
					else{
						minPos = pPosition + 1;
					}
				}
				//O apontador deve estar na primera posição
				if(maxPos < 0){
					maxPos = 0;
				}
				//O apontador está na posição posterior à chave já existente que é menor ou igual a chave atual
				else if(strcmp(key, node->key[maxPos]) >= 0){
					maxPos++;
				}
				pPosition = maxPos;
				pos = node->pointer[pPosition];
			}else{
				break;
			}
		}
		else{
			cout << "Erro: nó não foi carregado na função buscaNaArvore" << endl;
			cout << "Registro com o id " << key << " nao encontrado" << endl;
			return -1;
		}
	}

	
	minPos=0;
	maxPos=node->size-1;

	while(minPos <= maxPos){
		pPosition = (minPos + maxPos) / 2;
		if(strcmp(key,node->key[pPosition]) == 0){
			break;
		}else if(strcmp(key,node->key[pPosition]) < 0){
			maxPos = pPosition - 1;
		}else{
			minPos = pPosition + 1;
		}
	}

	//Consulta o arquivo de dados de acordo com a posição do apontador para o registro apenas para imprimir a estrutura
	findRegisterByTitle(hashF,node->pointer[pPosition], key);
	cout<< "------------------------------------------------" << endl;
	cout << endl << "Total de blocos de indice armazenados: " << head->nNodes << endl;
	cout << "Quantidade de blocos lidos: " << blocks << endl;

	
	secF->close();
	hashF->close();
	free(secF);
	free(hashF);
	free(head);

	return -1;
}


//end Search


















