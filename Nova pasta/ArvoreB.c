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

//Interfaces das Fun��es
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


//void inicializaPagina(pagina* p);
//bool insereChave(FILE * arqArvore, char* id, short byteoffset);
void criaArvore(FILE* arqReg, FILE* arqArvore);


short raiz = -2;
short rrnCount = 1;
	
int main(){
	
	//Vari�veis comuns
	int opcao = 0; //Opc�o do menu
	int campo = 0; //Contador de campos
	int contadorReg = 0; //Contador de registros
	int field_length = 0; //Armazena o tamanho do campo
	short tamanhoRegistro = 0; //Armazena o tamanho do registro
	short rec_length; //Armazena o tamanho do registro
	char filename[20], strCampo[256], strBuffer[512]; // Strings
	char* token; //Armazena parte de string ao utilizar strtok

	//Vari�veis para manuten��o da LED
	short LED = -1; //LED com valor inicial
	short byteOffset = 0; //Armazena o byte de in�cio do registro removido
	short tamanhoLED; //Armazena o tamanho do registro apontado pela LED
	short tamanhoByte; //Armazena o tamanho do registro apontado pelo byteoffset
	short valorApontado; //Valor do byteoffset visitado
	short sobra; //Armazena a sobra do registro quando inserimos um registro menor no espa�o
	short LEDatual;//Armazena o valor da LED que esta sendo verificada 
	short aux;
	

	//Vari�veis da Busca Sequencial para procurar o registro a ser removido
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
	    	/* IMPORTA��O DE REGISTROS */
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

				
				
    			/*Faz a importa��o dos dados do catalogo para o arquivo de registro com os devidos ajustes*/
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
				
				//Cria um novo Arquivo de �ndice de �rvore-b
				if((arqArvore = fopen("ArvoreB", "w+")) == NULL){
				    printf("*ERRO!");
				    break;
				} else{
					printf("b-ok");
				}
				criaArvore(arqReg, arqArvore);	
					
			break;
		}
					
	    
	}while(opcao < 5 && opcao > 0);
	
	//encerra os arquivos ao final
	fclose(arqCat);
	fclose(arqReg);
	fclose(arqArvore);
}
	

/*Fun��o para leitura do catalogo caractere por caractere at� terminar um campo.
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

		//este IF � para ele eliminar os espa�os antes de cada campo no cat�logo
        if(c == ';'){
            c = fgetc(arqCat);
        }

        strCampo[i] = '\0';
        return i;
}

/*Fun��o para leitura do tamanho de um registro no arquivo de registros.
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

/*Fun��o que recebe os dados do novo registro e armazena no buffer recebido*/
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



bool insereChave(FILE * arqArvore, int id2, short byteoffset, short aux){
	int i;
	short filhoQuero;
	pagina *p2 = malloc(sizeof(pagina));
	
	if(aux == -2){
			pagina *p = malloc(sizeof(pagina));
						
			//inicializa pagina
			p->rrn = rrnCount;
			p->qtdKeys = 0;
			for(i=0; i<MAX_KEYS; i++){					
				p->keys[i].key = -1;
				p->keys[i].byteoffset = -1;
				p->child[i] = -1;
			}
			p->child[4] = -1;
			
			p->qtdKeys++;
			p->keys[0].key = id2;
			p->keys[0].byteoffset = byteoffset;
			
			rewind(arqArvore);
			fwrite(p, sizeof(pagina), 1, arqArvore);
						
			//rewind(arqArvore);
			//fread(p2, sizeof(pagina), 1, arqArvore);
		rrnCount++;	
		} else{
			//preciso fazer uma busca
			rewind(arqArvore);
			fread(p2, sizeof(pagina), 1, arqArvore);
			//encontrei uma folha
			if(aux == -1){							
				if(p2->qtdKeys != MAX_KEYS){
					for(i=0;i<MAX_KEYS;i++){
						if(p2->keys[i].key == -1){
							p2->qtdKeys++;
							p2->keys[i].key = id2;
							p2->keys[i].byteoffset = byteoffset;
							ordenaChaves(p2);
							rewind(arqArvore);
							fwrite(p2, sizeof(pagina), 1, arqArvore);
							break;
						}
					}
					//tem q dividir a pagina etc
				} else{				
						for(i=0;i<MAX_KEYS;i++){
							if(p2->keys[i].key > id2){
								filhoQuero = p2->child[i];
								break;
							} else{
								filhoQuero = p2->child[i+1];
							}
						}
						//pagina *p3 = malloc(sizeof(pagina));
						insereChave(arqArvore, id2, byteoffset, filhoQuero);		
				}
			} else{
				insereChave(arqArvore, id2, byteoffset, filhoQuero);
			}
		}
		for(i=0;i<MAX_KEYS;i++){
			printf("\n\nrrnpagina = %d, quantdKey = %i, \nkey = %i \nbyteoffset = %d \nfilha numero %i = %d", p2->rrn, p2->qtdKeys, p2->keys[i].key, p2->keys[i].byteoffset,i, p2->child[i]);
		}
	
}

void ordenaChaves(pagina *p){
	int i, j, aux, auxkey;
	short auxbyteoffset, auxfilhodireito;
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
	
	//short filhoQuero;
	
	int i;

	rewind(arqArvore);
	fwrite(&raiz, sizeof(raiz), 1, arqArvore);
	rewind(arqReg);
	rec_length = obterRegistro(arqReg, strBuffer);

	while(rec_length > 0){
		id[0] = strBuffer[0];
		id[1] = strBuffer[1]; 
		id2 = atoi(id);

		rewind(arqArvore);
		fread(&aux, sizeof(raiz) ,1, arqArvore);
		insereChave(arqArvore, id2, byteoffset, aux);
		/*if(aux == -1){
			pagina *p = malloc(sizeof(pagina));
						
			//inicializa pagina
			p->rrn = rrnCount;
			p->qtdKeys = 0;
			for(i=0; i<MAX_KEYS; i++){					
				p->keys[i].key = -1;
				p->keys[i].byteoffset = -1;
				p->child[i] = -1;
			}
			p->child[4] = -1;
			
			p->qtdKeys++;
			p->keys[0].key = id2;
			p->keys[0].byteoffset = byteoffset;
			
			rewind(arqArvore);
			fwrite(p, sizeof(pagina), 1, arqArvore);
						
			rewind(arqArvore);
			fread(p2, sizeof(pagina), 1, arqArvore);
			
		} else{
			rewind(arqArvore);
			fread(p2, sizeof(pagina), 1, arqArvore);
			if(p2->qtdKeys != MAX_KEYS){
				for(i=0;i<MAX_KEYS;i++){
					if(p2->keys[i].key == -1){
						p2->qtdKeys++;
						p2->keys[i].key = id2;
						p2->keys[i].byteoffset = byteoffset;
						ordenaChaves(p2);
						rewind(arqArvore);
						fwrite(p2, sizeof(pagina), 1, arqArvore);
						break;
					}
				}
				//tem q dividir a pagina etc
			} else{
				
					for(i=0;i<MAX_KEYS;i++){
						if(p2->keys[i].key > id2){
							filhoQuero = p2->child[i];
							break;
						} else{
							filhoQuero = p2->child[i+1];
						}
					}
		
			}
			
		}
		for(i=0;i<MAX_KEYS;i++){
			printf("\n\nrrnpagina = %d, quantdKey = %i, \nkey = %i \nbyteoffset = %d \nfilha numero %i = %d", p2->rrn, p2->qtdKeys, p2->keys[i].key, p2->keys[i].byteoffset,i, p2->child[i]);
		}*/
			
		rec_length = obterRegistro(arqReg, strBuffer);
		byteoffset = byteoffset + rec_length;	
	}
		
	
}

