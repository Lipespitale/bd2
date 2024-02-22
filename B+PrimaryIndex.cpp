#include "B+PrimaryIndex.hpp"

Header *header = NULL;
fstream *indexFile, *dataFile;

void updateHeader(){
	/*Escreve/atualiza o cabeçalho no arquivo
	*/
	//Posiciona cursor no início do arquivo
	indexFile->seekp(0,indexFile->beg);
	//Escreve o cabeçalho no arquivo
	indexFile->write((char*)header, sizeof(Header));
}

void initHeader(){
	/*Aloca e escreve o cabeçalho de dados no arquivo
	*/
	header = (Header *)malloc(sizeof(Header));
	header->rootPos = -1;
	header->nNodes = 0;
	//Escreve cabeçalho no arquivo
	updateHeader();
}

NodePrim* createNodePrim(){
	/*Aloca um novo nó
	*/
	NodePrim *node = (NodePrim *) malloc(sizeof(NodePrim));
	//Inicializa tamanho, posição do nó
	node->size = 0;
	node->position = 0;

	//Inicializa chaves e apontadores
	for(int i = 0; i < N_KEYS_PRIM; i++){
		node->key[i] = 0;
		node->pointer[i] = 0;
	}
	node->pointer[N_POINTERS_PRIM] = 0;
	return node;
}

int insertNodeOnFile(NodePrim *block){
	/*Escreve um nó no arquivo de índice
	*/
	//Itera a quantidade de nós adicionados
	header->nNodes++;
	//Posiciona o cursor na devida posição
	indexFile->seekp(header->nNodes*sizeof(NodePrim), indexFile->beg);
	//Escreve o nó no arquivo de indice
	indexFile->write((char*)block,sizeof(NodePrim));
	//Atualiza o cabeçalho com a nova quantidade de nós	
	updateHeader();
	return header->nNodes;
}

void updateNodeOnFile(NodePrim *block){
	/*Atualiza nó no arquivo de indice
	*/
	//Posiciona o cursor na devida posição
	indexFile->seekp(block->position*sizeof(NodePrim), indexFile->beg);
	//Escreve o nó no arquivo
	indexFile->write((char*)block,sizeof(NodePrim));
}

void InsertKeyOnAvailableLeaf(NodePrim *node, int key, int pointer){
	/*Insere chave em uma página de dados que ainda tem espaço disponível
	*/
	int i,j;
	//Percorre as chaves já existentes do nó
	for(i=0; i<node->size; i++){
		//Se a chave existente na posição i for larger que a chave atual
		if(node->key[i] > key){
			for(j=node->size; j>i; j--){
				//Desloca chave existente
				node->key[j] = node->key[j-1];
				node->pointer[j] = node->pointer[j-1];
			}
			break;
		}
	}
	//Insere chave no nó
	node->key[i] = key;
	node->pointer[i] = pointer;
	node->size++;
	//Atualiza o nó no arquivo
	updateNodeOnFile(node);
}

void InsertKeyOnAvailableNode(NodePrim *node, int key, int pointer){
	/*Insere chave em uma página que ainda tem espaço disponível
	*/
	int i,j;
	//Percorre as chaves já existentes do nó
	for(i=0; i<node->size; i++){
		//Se a chave existente na posição i for larger que a chave atual
		if(node->key[i] > key){
			//Desloca chave existente
			for(j=node->size; j>i; j--){
				node->key[j] = node->key[j-1];
				node->pointer[j+1] = node->pointer[j];
			}
			break;
		}
	}
	//Insere chave no nó
	node->key[i] = key;
	node->pointer[i+1] = -1 * pointer;
	node->size++;
	//Atualiza o nó no arquivo
	updateNodeOnFile(node);
}

AuxNode* insertKeyOnFullLeaf(NodePrim *node, int key, int pointer){
	/* Trata a inserção da chave para o caso em que a página de dados já está cheia
	*/
	int i,j, pivot, inserted=0;
	NodePrim *newNodeReturn = NULL;
	AuxNode *fatherNode = NULL;
	newNodeReturn = createNodePrim();
	fatherNode = (AuxNode *) malloc(sizeof(AuxNode));
	pivot = node->key[ORDER_M];
	node->key[ORDER_M] = 0;
	node->size--;
	if(key < pivot){
		j=0;
		newNodeReturn->pointer[0] = node->pointer[ORDER_M+1];
		for(i=ORDER_M+1; i<=N_KEYS_PRIM; i++){
			newNodeReturn->key[j] = node->key[i];
			newNodeReturn->pointer[j+1] = node->pointer[i+1];
			newNodeReturn->size++;
			node->key[i] = 0;
			node->pointer[i+1] = 0;
			node->size--;
			j++;
		}
		for(i=ORDER_M; i>0; i--){
			if(key > node->key[i]){
				break;
			}else{
				node->key[i] = node->key[i-1];
				node->pointer[i+1] = node->pointer[i];
			}
		}
		node->key[i] = key;
		node->pointer[i+1] = -1 * pointer;
		node->size++;
	}else{
		j=0;
		newNodeReturn->pointer[0] = node->pointer[ORDER_M+1];
		for(i=ORDER_M+1; i<=N_KEYS_PRIM; i++){
			if(inserted == 0 && key < node->key[i]){
				newNodeReturn->key[j] = key;
				newNodeReturn->pointer[j+1] = -1 * pointer;
				newNodeReturn->size++;
				j++;
				inserted = 1;
			}
			newNodeReturn->key[j] = node->key[i];
			newNodeReturn->pointer[j+1] = node->pointer[i+1];
			newNodeReturn->size++;
			node->key[i] = 0;
			node->pointer[i+1] = 0;
			node->size--;
			j++;
		}
		if(inserted == 0){
			newNodeReturn->key[j] = key;
			newNodeReturn->pointer[j+1] = -1 * pointer;
			newNodeReturn->size++;
		}
	}
	newNodeReturn->position = insertNodeOnFile(newNodeReturn);
	updateNodeOnFile(newNodeReturn);
	updateNodeOnFile(node);
	fatherNode->key = pivot;
	fatherNode->leftPointer = -1 * node->position;
	fatherNode->rightPointer = -1 * newNodeReturn->position;
	free(newNodeReturn);
	return fatherNode;
}

AuxNode* insertKeyOnFullNode(NodePrim *node, int key, int pointer){
	/* Trata a inserção da chave para o caso em que a página já está cheia
	*/
	int i,j, pivot, inserted=0;
	NodePrim *newNodeReturn = NULL;
	AuxNode *fatherNode = NULL;
	newNodeReturn = createNodePrim();
	fatherNode = (AuxNode *) malloc(sizeof(AuxNode));
	pivot = node->key[ORDER_M];
	if(key < pivot){
		j=0;
		for(i=ORDER_M; i<= N_KEYS_PRIM; i++){
			newNodeReturn->key[j] = node->key[i];
			newNodeReturn->pointer[j] = node->pointer[i];
			newNodeReturn->size++;
			node->key[i] = 0;
			node->pointer[i] = 0;
			node->size--;
			j++;
		}
		for(i=ORDER_M-1; i>=0; i--){
			if(key > node->key[i]){
				break;
			}else{
				node->key[i+1] = node->key[i];
				node->pointer[i+1] = node->pointer[i];
			}
		}
		node->key[i+1] = key;
		node->pointer[i+1] = pointer;
		node->size++;
	}else{
		j=0;
		for(i=ORDER_M; i<=N_KEYS_PRIM; i++){
			if(inserted == 0 && key < node->key[i]){
				newNodeReturn->key[j] = key;
				newNodeReturn->pointer[j] = pointer;
				newNodeReturn->size++;
				j++;
				inserted = 1;
			}
			newNodeReturn->key[j] = node->key[i];
			newNodeReturn->pointer[j] = node->pointer[i];
			newNodeReturn->size++;
			node->key[i] = 0;
			node->pointer[i] = 0;
			node->size--;
			j++;
		}
		if(inserted == 0){
			newNodeReturn->key[j] = key;
			newNodeReturn->pointer[j] = pointer;
			newNodeReturn->size++;
		}
	}
	newNodeReturn->pointer[N_POINTERS_PRIM] = node->pointer[N_POINTERS_PRIM];
	newNodeReturn->position = insertNodeOnFile(newNodeReturn);
	node->pointer[N_POINTERS_PRIM] = -1 * newNodeReturn->position;
	updateNodeOnFile(node);
	updateNodeOnFile(newNodeReturn);
	fatherNode->key = pivot;
	fatherNode->leftPointer = -1 * node->position;
	fatherNode->rightPointer = -1 * newNodeReturn->position;
	free(newNodeReturn);
	return fatherNode;
}

NodePrim* getNodePrimFromFile(int position){
	/*Retorna um nó do arquivo de indice de acordo
	com a posição informada.
	*/
	if(position > 0){
		cout << "Endereço errado" << endl;
		return NULL;
	}
	NodePrim *node = createNodePrim();
	//Posiciona o cursor na posição informada
	indexFile->seekg(-1 * position * sizeof(NodePrim),indexFile->beg);
	//Copia o nó 
	indexFile->read((char *)node, sizeof(NodePrim));
	return node;
}

AuxNode* addKeyOnTree(NodePrim *node, int key, int pointer){
	/* Função de inserção da chave na árvore
	*/
	int i,j;
	AuxNode *nodeReturn = NULL;
	//Caso que é página de índice
	if(node->pointer[0] < 0){
		int smaller=0,larger=node->size-1,position;
		//Acha a devida posição do apontador
		while(smaller <= larger){
			position = (smaller + larger) / 2;
			//Quando a chave atual é smaller que uma chave já existente na posiçaõ
			if(key < node->key[position]){
				larger = position - 1;
			}
			//Quando a chave atual é larger ou igual a uma chave já existente na posição
			else{
				smaller = position + 1;
			}
		}
		//O apontador deve estar na primera posição
		if(larger < 0){
			larger = 0;
		}
		//O apontador está na posição posterior à chave já existente que é smaller ou igual a chave atual
		else if(key >= node->key[larger]){
			larger++;
		}
		position = larger;
		nodeReturn = addKeyOnTree(getNodePrimFromFile(node->pointer[position]),key,pointer);
		if(nodeReturn != NULL){
			//Se a página ainda tiver espaço
			if(node->key[N_KEYS_PRIM] == 0){
				InsertKeyOnAvailableNode(node, nodeReturn->key, -1 * nodeReturn->rightPointer);
				free(nodeReturn);
				nodeReturn = NULL;
			}
			//Se a página não tiver mais espaço
			else{
				AuxNode *newNodeReturn = NULL;
				newNodeReturn = insertKeyOnFullNode(node, nodeReturn->key, -1 * nodeReturn->rightPointer);
				free(nodeReturn);
				nodeReturn = newNodeReturn;
			}
		}
	}
	//Caso que é página de dados(folha)
	else{
		//Se a página ainda tiver espaço
		if(node->key[N_KEYS_PRIM] == 0){
			InsertKeyOnAvailableLeaf(node,key,pointer);
		}
		//Se a página não tiver mais espaço
		else{
			nodeReturn = insertKeyOnFullLeaf(node,key,pointer);
		}
	}
	free(node);
	return nodeReturn;
}

void insertIntoTree(int key, int pointer){
	/* Insere uma chave na arvore e a salva no arquivo de indice primario
	*/
	AuxNode *nodeReturn;
	nodeReturn = addKeyOnTree(getNodePrimFromFile(header->rootPos),key,pointer);
	if(nodeReturn != NULL){
		NodePrim *newNodeReturn;
		int pos;
		newNodeReturn = createNodePrim();
		newNodeReturn->key[0] = nodeReturn->key;
		newNodeReturn->pointer[0] = nodeReturn->leftPointer;
		newNodeReturn->pointer[1] = nodeReturn->rightPointer;
		newNodeReturn->size++;
		pos = insertNodeOnFile(newNodeReturn);
		newNodeReturn->position = pos;
		updateNodeOnFile(newNodeReturn);
		header->rootPos = -1 * pos;
		updateHeader();
		free(newNodeReturn);
		free(nodeReturn);
	}
}

void populatePrimIndexFile(){
	/* Pega o id do primeiro elemento de cada bloco do arquivo de dados e insere na arvore
	*/
	Block buffer = {0};

	//Posiciona cursor no início do arquivo de dados
	dataFile->clear();
	dataFile->seekg(0,dataFile->beg);

	//insere o id inicial de cada bloco na arvore
	for (int i = 0; i < N_BUCKETS; i++) {
		//faz a leitura de um bloco
        dataFile->read((char*)&buffer,sizeof(Block));
		//verifica se o bloco nao esta vazio
        if(buffer.nRegisters > 0) {
            Article *vet;
            vet=(Article*)&buffer.body;
			//insere o id do primeiro registro do bloco na arvore
            insertIntoTree(vet[0].id, i);
        }
    }
}

void insertPrimaryIndexFile(fstream *hashFile, fstream *primIdxFile){
	/*Faz upload dos dados no arquivo de índice secundário
	*/
	indexFile = primIdxFile;
	dataFile = hashFile;

	//Inicializa cabeçalho no arquivo
	initHeader();

	//Aloca nó raiz 
	NodePrim *root = createNodePrim();
	//Escreve nó raiz no arquivo
	root->position = insertNodeOnFile(root);
	//Atualiza a posição do nó raiz no arquivo
	updateNodeOnFile(root);
	free(root);

	//Popula o arquivo de indices
	populatePrimIndexFile();
}

//end creation and insertion

void getHeader(){
	/* Pega a estrutura cabeçalho do arquivo de indice
	*/
	//Se cabeçalho não existir, aloca
	if(header == NULL){
		header = (Header *)malloc(sizeof(Header));
	}
	//Posiciona o cursor para o início do arquivo
	indexFile->seekg(0,indexFile->beg);
	//Copia o cabeçalho do arquivo para header
	indexFile->read((char *)header, sizeof(Header));
}

void openIndexFile(const char *pathIndexFile){
	//abre o arquivo de indice
	indexFile = new fstream(pathIndexFile, fstream::in | fstream::out | ios::binary);
	if(!indexFile){
		cout << "O arquivo de indice não pode ser aberto. Cancelando as operacoes" << endl;
		exit(EXIT_FAILURE);
	}
}

void openDataFile(const char *pathDataFile){
	//abre o arquivo de dados
	dataFile = new fstream(pathDataFile, fstream::in | ios::binary);
	if(!indexFile){
		cout << "O arquivo de dados não pode ser aberto. Cancelando as operacoes" << endl;
		exit(EXIT_FAILURE);
	}
}

void closeFiles(){
	//fecha os arquivos abertos
	indexFile->close();
	dataFile->close();
	free(indexFile);
	free(dataFile);
	free(header);
}

int searchInTree(const char *pathDataFile,const char *pathIndexFile, int key){
	/* pesquisa uma chave no arquivo de indice primario
	*/
	int pointer, pos, smaller, larger, posPointer, i, numReadBlocks=0;
	NodePrim *node,*aux;

	openIndexFile(pathIndexFile);
	openDataFile(pathDataFile);
	getHeader();

	numReadBlocks++;
	pos = header->rootPos;
	while(1){
		node = getNodePrimFromFile(pos);
		numReadBlocks++;
		if(node != NULL){
			if(node->pointer[0] < 0){
				smaller=0;
				larger=node->size-1;
				while(smaller <= larger){
					posPointer = (smaller + larger) / 2;
					if(key < node->key[posPointer]){
						larger = posPointer - 1;
					}else{
						smaller = posPointer + 1;
					}
				}
				if(larger < 0){
					larger = 0;
				}else if(key >= node->key[larger]){
					larger++;
				}
				posPointer = larger;
				pos = node->pointer[posPointer];
			}else{
				break;
			}
		}else{
			cout << "Erro: nó não foi carregado na função searchInTree" << endl;
			return -1;
		}
	}
	cout << "Quantidade de Blocks lidos: " << numReadBlocks << endl;
	smaller=0;
	larger=node->size-1;
	while(smaller <= larger){
		posPointer = (smaller + larger) / 2;
		if(key == node->key[posPointer]){
			return node->pointer[posPointer];
		}else if(key < node->key[posPointer]){
			larger = posPointer - 1;
		}else{
			smaller = posPointer + 1;
		}
	}
	closeFiles();
	return -1;
}

Block * readBlock(fstream *arq, int position) {
	/* Faz a leitura de um bloco do arquivo de dados
	*/
    Block *buffer= NULL;
    int i,j;

    buffer = (Block*) malloc(sizeof(Block));

    //Encontrando a posição do bucket no hashfile
    arq->seekg(position*sizeof(Block),ios::beg);

    //Leitura de Block
    arq->read((char*)buffer,sizeof(Block));
    return buffer;
}

void copyArticle(Article *target, Article *source){
	/* copia os dados de source para target
	*/
    target->id = source->id;
    strcpy(target->title,source->title);
    target->year = source->year;
    strcpy(target->author,source->author);
    target->citations = source->citations;
    strcpy(target->update,source->update);
    strcpy(target->snippet,source->snippet);
}

Article* getArticleByPositionID(fstream *arq, int position, int id){
	/* Busca os dados de um registro pelo id dentro de um bloco do arquivo de dados
	*/
    Block *bucket;
    Article *article, *result=NULL;
    int larger, smaller, posPointer;

    result = (Article *) malloc(sizeof(Article));

    bucket = readBlock(arq, position);
    article = (Article*)&bucket->body;

    smaller=0;
    larger=bucket->nRegisters-1;

	//faz uma busca binaria no bloco pelo id
    while(smaller <= larger){
        posPointer = (smaller + larger) / 2;

        if(id == article[posPointer].id){
            copyArticle(result, &article[posPointer]);
            break;
        }else if(id < article[posPointer].id){
            larger = posPointer - 1;
        }else{
            smaller = posPointer + 1;
        }
    }

    free(bucket);
    return result;
}

void seek1(const char *pathDataFile,const  char *pathIndexFile, int key){
	/* Busca uma chave por id no arquivo de indice primario e imprime seus dados
	*/
	int position = 0;
	Article *article;
	position = searchInTree(pathDataFile,pathIndexFile,key);
	//verifica se não encontrou a chave
	if(position < 0){
		cout << "Registro com o id " << key << " nao encontrado" << endl;
	}
	//se encontrou a chave busca ela no arquivo de dados e imprime
	else{
		openDataFile(pathDataFile);
		article = getArticleByPositionID(dataFile,position, key);
		printRegister(*article);
		free(article);
		dataFile->close();
	}
}