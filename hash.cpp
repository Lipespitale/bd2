#include <iostream>     
#include <stdio.h>
#include <fstream>
#include <string.h>
#include "hash.hpp"


int collision = 0;

void initOutputFile(fstream *f){
    /* Aloca o arquivo de dados
    */
    Block buffer = {0};

    cout << "Alocando arquivo de dados ("<<HASH_FILE_NAME<<")..."<<endl;

    //Preenche os buckets com blocos vazios
    for(int i=0; i<N_BUCKETS; i++) {
        f->write((char*)&buffer,sizeof(Block));
    }
}

int hashing(int id) {
    /* Função hashing modular
    */
    return id % N_BUCKETS;
}


Block getBucketById(fstream *f, int id) {
    /* Busca um bucket no arquivo de dados por ID e
    o retorna.
    */
    Block buffer={0};
    int bucketKey = hashing(id);

    //Busca a posição do bucket no arquivo de dados
    f->seekg (bucketKey*sizeof(Block),ios::beg);

    //Copia o bloco correspondente no buffer
    f->read((char*)&buffer,sizeof(Block));

    return buffer;
}

void printRegister(Article article) {
    /*Imprime um registro do tipo Article
    */
    cout<< "\n*ID: "             << article.id 
        << "\n*Titulo: "         << article.title 
        << "\n*Ano: "            << article.year
        << "\n*Autor: "          << article.author
        << "\n*Citações:"        << article.citations
        << "\n*Atualização: "    << article.update
        << "\n*Snippet: "        << article.snippet 
        << endl << endl;
}


bool insertHashFile(fstream *f, Article article) {
	/*Insere no arquivo de dados o ponteiro que identifica o artigo
	*/
    int i;

    Block buffer= getBucketById(f,article.id); //Pega o bucket correspondente

    Article *v_article = (Article*)&buffer.body;

    //Verifica se há espaço disponível no bloco
    if(buffer.nRegisters < N_REGISTERS) {
        for(i=0; i<buffer.nRegisters; i++) {
            if(article.id < v_article[i].id) {
                //Desloca os artigos armazenados para manter a ordem crescente na nova inserção
                for(int j=buffer.nRegisters; j>i; j--) {
                    memcpy(&v_article[j],(char*)&v_article[i-1], sizeof(Article));
                }
                break;
            }
        }

        //Insere artigo
        memcpy(&v_article[i],(char*)&article, sizeof(Article));

        //Atualiza a quantidade de registros ocupados no bloco
        buffer.nRegisters++;

        //Volta o cursor para o início do bloco        
        f->seekp( - sizeof(Block) , ios::cur);

        //Escreve o bloco no arquivo de dados
        f->write((char*)&buffer,sizeof(Block));

        return true;
    }else {
        collision++;
        return false;   
    }
}

Article findRegisterById(fstream *f,int id) {
	/* Busca no arquivo de dados um registro de acordo com o ID informado.
	Caso o registro exista, são retornados os campos do registro, a quantidade
	de blocos lidos para encontrá-lo e a quantidade total de blocos do arquivo 
	de dados.
	*/

	Block buffer= getBucketById(f,id); //Pega o bloco pelo id
    Article *v_article = (Article*)&buffer.body;

    int begin= 0;
    int end = buffer.nRegisters - 1;

    while (begin <= end) {  //Percorre o corpo do bloco para achar o registro com o Id informado (busca binária)
        int i = (begin + end) / 2;  // Calcula o meio do sub-vetor
        if (v_article[i].id == id) {  // Item encontrado
            printRegister(v_article[i]);
            cout<< "------------------------------------------------" <<
            "\nBlocos lidos: 1" <<                                  //Como assumimos que 1 bucket é igual a 1 bloco, o único bloco lido é o bloco o qual está armazenado o registro informado
            "\nTotal de Blocos alocados: "<<N_BUCKETS<<endl;        //Total de blocos alocados no arquivo de dados 
            return v_article[i];
        }
        if (v_article[i].id < id) {  // Item está no sub-vetor à direita 
            begin = i + 1;
        } else {  // vector[i] > item. Item está no sub-vetor à esquerda
            end = i;
        }
    }

    cout << "Registro não encontrado.\n";
    return {0};
}


Block getBucketByPosition(fstream *f, int position) {
    /* Busca um bucket no arquivo de dados pela posição e
    o retorna.
    */ 
    Block buffer={0};
    //Busca a posição do bucket no arquivo de dados
    f->seekg (position*sizeof(Block),ios::beg);
    //Copia o bloco correspondente no buffer
    f->read((char*)&buffer,sizeof(Block));
    
    return buffer;
}


Article findRegisterByTitle(fstream *f, int position, char title[T_TITLE]){
    /* Busca no arquivo de dados um registro de acordo com o título informado
    */

    Block buffer = getBucketByPosition(f, position); //Pega o bloco pela posição
    Article *v_article = (Article*)&buffer.body;

    for(int i=0; i<buffer.nRegisters; i++) { //Percorre o corpo do bloco para achar o registro com o Título informado (busca sequencial)
        if(strcmp(title, v_article[i].title) == 0){
            printRegister(v_article[i]);
            return v_article[i];
        }
    }

    cout << "Registro não encontrado.\n";
    return {0};
}

