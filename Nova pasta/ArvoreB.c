//Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ORDEM_PAG 5
#define MAX_KEYS 4
#define TAM_PAG 
#define MIN_KEYS_PAG 3
#define MIN_KEYS_FOLHA 2

enum boolean {
    true = 1, false = 0
};

typedef enum boolean bool;

//Interfaces das Funções
int obterCampos(FILE* arqCat, char* strCampo, int* reg);
short obterRegistro(FILE* arqReg, char* strBuffer);
void receberDados(char* strBuffer);


typedef struct {
	int key;
	short byteoffset;
}chave;

typedef struct {
	short rrn;
	chave keys[MAX_KEYS];
	short child[ORDEM_PAG]; 
	int qtdKeys;
}pagina;

typedef struct {
	short rrn;
	chave keys[MAX_KEYS+1];
	short child[ORDEM_PAG+1];
	int qtdKeys;
}paginaAuxiliar;


//void inicializaPagina(pagina* p);
//bool insereChave(FILE * arqArvore, char* id, short byteoffset);
void criaArvore(FILE* arqReg, FILE* arqArvore);
void ordenaChaves(pagina *p);
void ordenaChavesAux(paginaAuxiliar *p);


short raiz = -2;
short rrnCount = 1;
short rrnraiz = -2;
	
int main(){
	
	//Variáveis comuns
	int opcao = 0; //Opcão do menu
	int campo = 0; //Contador de campos
	int contadorReg = 0; //Contador de registros
	int field_length = 0; //Armazena o tamanho do campo
	short tamanhoRegistro = 0; //Armazena o tamanho do registro
	short rec_length; //Armazena o tamanho do registro
	char filename[20], strCampo[256], strBuffer[512]; // Strings
	char* token; //Armazena parte de string ao utilizar strtok

	//Variáveis para manutenção da LED
	short LED = -1; //LED com valor inicial
	short byteOffset = 0; //Armazena o byte de início do registro removido
	short tamanhoLED; //Armazena o tamanho do registro apontado pela LED
	short tamanhoByte; //Armazena o tamanho do registro apontado pelo byteoffset
	short valorApontado; //Valor do byteoffset visitado
	short sobra; //Armazena a sobra do registro quando inserimos um registro menor no espaço
	short LEDatual;//Armazena o valor da LED que esta sendo verificada 
	short aux;
	

	//Variáveis da Busca Sequencial para procurar o registro a ser removido
	char searchKey[10]; //ID a ser removido
	char* id; //ID do registro a ser comparado com o procurado
	bool matched = false; //Armazena o estado da procura
	
	FILE* arqCat;
	FILE* arqReg;
	FILE* arqArvore;

	do{

		printf("\n\n           ---------------------------------------------------\n");
		printf("           |PROGRAMA PARA GERENCIAMENTO DE CATALOGO DE ALUNOS|\n");
		printf("           ---------------------------------------------------\n");
	    printf("Suas opcoes sao:\n\n");
	    printf("    1. Importar catalogo de alunos\n");
	    printf("    2. Inserir novo registro\n");
	    printf("    3. Buscar Trabalho por ID\n");
	    printf("    4. Listar Arvore-B\n");
	    printf("    5. Terminar o programa\n");
	    printf("\nDigite o numero da sua escolha: ");
	    scanf("%d", &opcao);
	    fflush(stdin);
	    
	    switch(opcao){
	    	/* IMPORTAÇÃO DE REGISTROS */
	        case 1:
	        	strCampo[0] = '\0';
	        	strBuffer[0] = '\0';


	       		printf("\nDigite o nome do catalogo: ");
   				gets(filename);
    			if((arqCat = fopen(filename, "r")) == NULL){
        			printf("*ERRO!");
       				break;
    			}else{
    				printf("*Arquivo Encontrado!*");}


	       		printf("\n\nDigite um nome para o novo Arquivo de Registros: ");
				gets(filename);
				if((arqReg = fopen(filename, "w+")) == NULL){
	    			printf("*ERRO!");
	    			break;
				}else{
					printf("*Arquivo Criado!*");}

				/*LED = -1;
				rewind(arqReg);
    			fwrite(&LED, sizeof(LED), 1, arqReg);*/

				
				
    			/*Faz a importação dos dados do catalogo para o arquivo de registro com os devidos ajustes*/
				field_length = obterCampos(arqCat, strCampo, &contadorReg);
				strcat(strBuffer,strCampo);
    			strcat(strBuffer,"|");

    			while(field_length > 0){
			        campo ++;
			        printf("\nCampo #%i = %s", campo, strCampo);
			        strCampo[0] = '\0';

			        if(!(campo % 5) ){
				        tamanhoRegistro = strlen(strBuffer);
				        fwrite(&tamanhoRegistro, sizeof(tamanhoRegistro), 1, arqReg);
				        fwrite(strBuffer, tamanhoRegistro, 1, arqReg);
				        strBuffer[0] = '\0';
			        }

			        field_length = obterCampos(arqCat, strCampo, &contadorReg);
			        strcat(strBuffer,strCampo);
			        strcat(strBuffer,"|");
				}
				
				printf("\n\nIMPORTACAO REALIZADA COM SUCESSO!");	        			
				
				//Cria um novo Arquivo de índice de árvore-b
				if((arqArvore = fopen("ArvoreB", "w+")) == NULL){
				    printf("*ERRO!");
				    break;
				} 
				criaArvore(arqReg, arqArvore);	
					
			break;
			
			case 4:
				listaArvoreB(arqArvore);
		}
					
	    
	}while(opcao < 5 && opcao > 0);
	
	//encerra os arquivos ao final
	fclose(arqCat);
	fclose(arqReg);
	fclose(arqArvore);
}
	

/*Função para leitura do catalogo caractere por caractere até terminar um campo.
  Quando encontra um caractere ';' ou '\n', retorna a quantidade de caracteres lidos.*/
int obterCampos(FILE* arqCat, char* strCampo, int* reg){
        int i = 0;
        char c;

        c = fgetc(arqCat);

        while(feof(arqCat) == 0 && (c != ';' && c != '\n')){      	
            strCampo[i] = c;
            i++;
            c = fgetc(arqCat);
        }

        //contador de registros encontrados
        if(c == '\n'){
            (*reg)++;
        }

		//este IF é para ele eliminar os espaços antes de cada campo no catálogo
        if(c == ';'){
            c = fgetc(arqCat);
        }

        strCampo[i] = '\0';
        return i;
}

/*Função para leitura do tamanho de um registro no arquivo de registros.
  Faz a leitura da quantidade de caracteres do registro e armazena estes em um buffer.
  Retorna o tamanho do registro lido*/
short obterRegistro(FILE *arqReg, char *strBuffer){
        short rec_length;

        if (fread(&rec_length, sizeof(rec_length), 1, arqReg) == 0){
            return 0;
        }
        rec_length = fread(strBuffer, 1 ,rec_length, arqReg);

        strBuffer[rec_length] = '\0';
        return rec_length;
}

/*Função que recebe os dados do novo registro e armazena no buffer recebido*/
void receberDados(char* strBuffer){
	char strCampo[256];
	strBuffer[0] = '\0';
	printf("\n*Insira os dados do novo Registro*");
	printf("\nID do Aluno: ");
    gets(strCampo);
    strcat(strBuffer,strCampo);
    strcat(strBuffer,"|");

    printf("Nome do Autor: ");
    gets(strCampo);
    strcat(strBuffer,strCampo);
    strcat(strBuffer,"|");

    printf("Titulo do Trabalho: ");
    gets(strCampo);
    strcat(strBuffer,strCampo);
    strcat(strBuffer,"|");

    printf("Nome do Curso: ");
    gets(strCampo);
    strcat(strBuffer,strCampo);
    strcat(strBuffer,"|");

    printf("Tipo do Trabalho: ");
    gets(strCampo);
    strcat(strBuffer,strCampo);
    strcat(strBuffer,"|");
}

/*void inicializaPagina(pagina* p){
	printf("inicializando pag");
	int i;
	(*p).rrn = rrnCount;
	for(i=0; i<MAX_KEYS; i++){					
		(*p).keys[i].key[0] = -1;
		(*p).keys[i].byteoffset = -1;
		(*p).child[i] = -1;
	}
	(*p).child[4] = -1;					
}*/



void insereChave(FILE * arqArvore, int id2, short byteoffset, short filhoQuero){
	int i;
	short rrnChild;
	bool divisao=true;
	bool promocao=false;
	//short filhoQuero;
	pagina *p = malloc(sizeof(pagina));
	
	if(filhoQuero == -2){
			//pagina *p = malloc(sizeof(pagina));
						
			//inicializa pagina
			p->rrn = rrnCount;
			p->qtdKeys = 0;
			for(i=0; i<MAX_KEYS; i++){					
				p->keys[i].key = -1;
				p->keys[i].byteoffset = -1;
				p->child[i] = -1;
			}
			p->child[4] = -1;
			//fim inicializacao
			
			p->qtdKeys++;
			p->keys[0].key = id2;
			p->keys[0].byteoffset = byteoffset;
			
			rewind(arqArvore);
			fwrite(p, sizeof(pagina), 1, arqArvore);
			rewind(arqArvore);
			fread(&rrnraiz, sizeof(raiz) ,1, arqArvore);
						
			//rewind(arqArvore);
			//fread(p2, sizeof(pagina), 1, arqArvore);
		rrnCount++;	
	} else{
			//preciso fazer uma busca
			rewind(arqArvore);
			printf("FILHO QUERO = %d", filhoQuero);
			getch();
			fseek(arqArvore, (filhoQuero-1)*sizeof(pagina), SEEK_SET);
			fread(p, sizeof(pagina), 1, arqArvore);
			
			printf("\nRRN: %d", p->rrn);
			printf("\nChaves: ");
			for(i=0;i<p->qtdKeys;i++){
				printf("%i | ", p->keys[i].key);
			}
			printf("\nOffsets: ");
			for(i=0;i<p->qtdKeys;i++){
				printf("%d | ", p->keys[i].byteoffset);
			}
			printf("\nVetor de Filhos: ");
			for(i=0;i<ORDEM_PAG;i++){
				printf("%d | ", p->child[i]);
			}
			printf("\n");
			getch();
			
			//pego a posição q a chave deveria estar
			for(i=0;i<p->qtdKeys;i++){
							if(p->keys[i].key > id2){
								filhoQuero = p->child[i];
								rrnChild = i;
								break;
							} else{
								filhoQuero = p->child[i+1];
							}
			}
			
			//se o lugar onde eu deveria estar ainda nao ta apontando
			if(filhoQuero == -1){	
				//verifico se a pagina tem lugar sobrando			
				if(p->qtdKeys != MAX_KEYS){
					for(i=0;i<MAX_KEYS;i++){
						//encontro lugar em branco
						if(p->keys[i].key == -1){
							p->qtdKeys++;
							p->keys[i].key = id2;
							p->keys[i].byteoffset = byteoffset;
							ordenaChaves(p);
							rewind(arqArvore);
							fseek(arqArvore, (p->rrn-1)*sizeof(pagina), SEEK_SET);
							fwrite(p, sizeof(pagina), 1, arqArvore);
							break;
							//return;
						}
					}
				//se não tenho q dividir a pagina etc
				} else{	
					//enquanto divisao
					//while(divisao){	
					//gambi basica
						paginaAuxiliar *aux = malloc(sizeof(paginaAuxiliar));
						aux->keys[0].key = p->keys[0].key;
						aux->keys[1].key = p->keys[1].key;
						aux->keys[2].key = p->keys[2].key;
						aux->keys[3].key = p->keys[3].key;
						aux->keys[4].key = id2;
						
						aux->keys[0].byteoffset = p->keys[0].byteoffset;
						aux->keys[1].byteoffset = p->keys[1].byteoffset;
						aux->keys[2].byteoffset = p->keys[2].byteoffset;
						aux->keys[3].byteoffset = p->keys[3].byteoffset;
						aux->keys[4].byteoffset = byteoffset;
						
						aux->rrn = 0;
						aux->qtdKeys = 5;
						for(i=0; i<MAX_KEYS+1; i++){	
							aux->child[i] = -1;
						}
						ordenaChavesAux(aux);
						
						pagina *nova = malloc(sizeof(pagina));
						pagina *novaRaiz = malloc(sizeof(pagina));
						//paginaAuxiliar *aux = malloc(sizeof(paginaAuxiliar));
						//rewind(arqArvore);
						//fseek(arqArvore, ((filhoQuero-1)*sizeof(pagina))+, SEEK_SET);
						//fread(aux, sizeof(paginaAuxiliar), 1, arqArvore);
						
						//inicializa
						nova->rrn = rrnCount;						
						nova->qtdKeys = 0;	
						rrnCount++;	
						novaRaiz->rrn = rrnCount;
						novaRaiz->qtdKeys = 0;	
								
						for(i=0; i<MAX_KEYS; i++){					
							nova->keys[i].key = -1;
							nova->keys[i].byteoffset = -1;
							nova->child[i] = -1;

							novaRaiz->keys[i].key = -1;
							novaRaiz->keys[i].byteoffset = -1;
							novaRaiz->child[i] = -1;
							
						}
						nova->child[4] = -1;
						novaRaiz->child[4] = -1;
						//fim inicializa
						
						//seta valores
						p->qtdKeys--;
						p->qtdKeys--;
						
						p->keys[2].key = -1;
						p->keys[3].key = -1;
						p->keys[2].byteoffset = -1;
						p->keys[3].byteoffset = -1;
						
						p->keys[0].key = aux->keys[0].key;
						p->keys[1].key = aux->keys[1].key;
						p->keys[0].byteoffset = aux->keys[0].byteoffset;
						p->keys[1].byteoffset = aux->keys[1].byteoffset;
						
												
						nova->qtdKeys++;
						nova->qtdKeys++;
						nova->keys[0].key = aux->keys[3].key;
						nova->keys[1].key = aux->keys[4].key;
						nova->keys[0].byteoffset = aux->keys[3].byteoffset;
						nova->keys[1].byteoffset = aux->keys[4].byteoffset;
						
						novaRaiz->qtdKeys++;
						novaRaiz->keys[0].key = aux->keys[2].key;
						novaRaiz->keys[0].byteoffset = aux->keys[2].byteoffset; 
						
						novaRaiz->child[0] = p->rrn;
						novaRaiz->child[1] = nova->rrn;
						
						rewind(arqArvore);
						fwrite(p, sizeof(pagina), 1, arqArvore);
						fwrite(nova, sizeof(pagina), 1, arqArvore);
						fwrite(novaRaiz, sizeof(pagina), 1, arqArvore);
						
						//p->child[rrnChild] = nova->rrn;
					//}
					rrnCount++;	
					rrnraiz=novaRaiz->rrn;				
				}
				
			} else{
				insereChave(arqArvore, id2, byteoffset, filhoQuero);
			}
	}
	//for(i=0;i<MAX_KEYS;i++){
		//printf("\n\nrrnpagina = %d, quantdKey = %i, \nkey = %i \nbyteoffset = %d \nfilha numero %i = %d", p->rrn, p->qtdKeys, p->keys[i].key, p->keys[i].byteoffset,i, p->child[i]);
		//getch();
	//}
	return;
	
}

void ordenaChaves(pagina *p){
	int i, j, aux;
	short auxbyteoffset;
	int valores[p->qtdKeys];
	
	for(i=0;i<p->qtdKeys;i++){
		valores[i] = p->keys[i].key;
	}	
  	
      for (i = 1; i < p->qtdKeys; i++) {
            j = i;
            while (j > 0 && valores[j - 1] > valores[j]) {
                  aux = valores[j];
                  valores[j] = valores[j - 1];
                  valores[j - 1] = aux;
                  auxbyteoffset = p->keys[j].byteoffset;
                  p->keys[j].byteoffset = p->keys[j - 1].byteoffset;
                  p->keys[j -1].byteoffset = auxbyteoffset;
                  
                  j--;
                  
            }
      }
      
    for(i=0;i<p->qtdKeys;i++){
		p->keys[i].key = valores[i];
	}
}

void ordenaChavesAux(paginaAuxiliar *p){
	int i, j, aux;
	short auxbyteoffset;
	int valores[p->qtdKeys];
	
	for(i=0;i<p->qtdKeys;i++){
		valores[i] = p->keys[i].key;
	}	
  	
      for (i = 1; i < p->qtdKeys; i++) {
            j = i;
            while (j > 0 && valores[j - 1] > valores[j]) {
                  aux = valores[j];
                  valores[j] = valores[j - 1];
                  valores[j - 1] = aux;
                  auxbyteoffset = p->keys[j].byteoffset;
                  p->keys[j].byteoffset = p->keys[j - 1].byteoffset;
                  p->keys[j -1].byteoffset = auxbyteoffset;
                  
                  j--;
                  
            }
      }
      
    for(i=0;i<p->qtdKeys;i++){
		p->keys[i].key = valores[i];
	}
}

void criaArvore(FILE* arqReg, FILE* arqArvore){
	short rec_length;
	char strBuffer[512];
	char id[2];
	int id2;
	short byteoffset = 0;
	short aux =0;
	int i;

	//rewind(arqArvore);
	//fwrite(&raiz, sizeof(raiz), 1, arqArvore);
	rewind(arqReg);
	rec_length = obterRegistro(arqReg, strBuffer);

	while(rec_length > 0){
		id[0] = strBuffer[0];
		id[1] = strBuffer[1]; 
		id2 = atoi(id);

		rewind(arqArvore);
		fread(&aux, sizeof(raiz) ,1, arqArvore);
		
		//insereChave(arqArvore, id2, byteoffset, aux);
		insereChave(arqArvore, id2, byteoffset, rrnraiz);
			
		rec_length = obterRegistro(arqReg, strBuffer);
		byteoffset = byteoffset + rec_length;	
	}
		
	
}

void listaArvoreB(FILE* arqArvore){
	int i;
	short rec_length;
	pagina *p = malloc(sizeof(pagina));
	rewind(arqArvore);
	fread(p, sizeof(pagina), 1, arqArvore);
	while(feof(arqArvore) == 0){
		if(p->rrn == rrnraiz)
			printf("\n\n-----Pagina Raiz-----");
		printf("\nRRN: %d", p->rrn);
		printf("\nChaves: ");
		for(i=0;i<p->qtdKeys;i++){
			printf("%i | ", p->keys[i].key);
		}
		printf("\nOffsets: ");
		for(i=0;i<p->qtdKeys;i++){
			printf("%d | ", p->keys[i].byteoffset);
		}
		printf("\nVetor de Filhos: ");
		for(i=0;i<ORDEM_PAG;i++){
			printf("%d | ", p->child[i]);
		}
		printf("\n");
		fread(p, sizeof(pagina), 1, arqArvore);
	}
}
