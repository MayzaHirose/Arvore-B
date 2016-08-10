//Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ORDEM_PAG 5
#define MAX_KEYS 4

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
	short pai;
}pagina;

typedef struct {
	short rrn;
	chave keys[MAX_KEYS+1];
	short child[ORDEM_PAG+1];
	int qtdKeys;
	short pai;
}paginaAuxiliar;

void criaArvore(FILE* arqReg, FILE* arqArvore);
void ordenaChaves(pagina *p);
void ordenaChavesAux(paginaAuxiliar *p);
short buscaRegistro(FILE* arqArvore, int id, short filho);
void listaRegistro(FILE* arqReg, short offset);

short raiz = -2;
short rrnCount = 1;
short rrnraiz = -2;
int promoteKey = 0;
short promoteByte = 0;

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

	//Variáveis da Busca Sequencial para procurar o registro a ser removido
	char searchKey[2]; //ID a ser removido
	int searchKey2;
	short offset;
	//char* id; //ID do registro a ser comparado com o procurado
	
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
	    //printf("%i\n",(int)sizeof(pagina));
	    //printf("teste = %i", teste);
	    scanf("%d", &opcao);
	    fflush(stdin);
	    
	    switch(opcao){
	    	/* IMPORTAÇÃO DE REGISTROS */
	        case 1:
	        	strCampo[0] = '\0';
	        	strBuffer[0] = '\0';


	       		printf("\nDigite o nome do catalogo a ser importado: ");
   				gets(filename);
    			if((arqCat = fopen(filename, "r")) == NULL){
        			printf("*ERRO!");
       				break;
    			}/*else{
    				printf("*Arquivo Encontrado!*");}*/


	       		printf("Digite um nome para o Arquivo de Registros: ");
				gets(filename);
				if((arqReg = fopen(filename, "w+")) == NULL){
	    			printf("*ERRO!");
	    			break;
				}/*else{
					printf("*Arquivo Criado!*");}*/

				/*LED = -1;
				rewind(arqReg);
    			fwrite(&LED, sizeof(LED), 1, arqReg);*/

				
				
    			/*Faz a importação dos dados do catalogo para o arquivo de registro com os devidos ajustes*/
				field_length = obterCampos(arqCat, strCampo, &contadorReg);
				strcat(strBuffer,strCampo);
    			strcat(strBuffer,"|");

    			while(field_length > 0){
			        campo ++;
			        //printf("\nCampo #%i = %s", campo, strCampo);
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
				
				//printf("\n\nIMPORTACAO REALIZADA COM SUCESSO!");	        			
				
				//Cria um novo Arquivo de índice de árvore-b
				if((arqArvore = fopen("ArvoreB", "w+")) == NULL){
				    printf("*ERRO!");
				    break;
				} 
				criaArvore(arqReg, arqArvore);
				printf("\nArquivos de Arvore e Registro Criados com Sucesso!");
				printf("\n\nEnter para continuar ...");
				getch();
				system("cls");	
					
			break;
			
			case 3:
	        	printf("\nQual o ID procurado?: ");
	        	//gets(searchKey);
	        	scanf("%i", &searchKey2);
	        	//searchKey2 = atoi(searchKey);
				
				offset = buscaRegistro(arqArvore, searchKey2, rrnraiz);
				//printf("\n\nBYTEOFFSET = %d ", offset);
				listaRegistro(arqReg, offset);
				printf("\n\nEnter para continuar ...");
				getch();
				system("cls");
			break;
			
			case 4:
				listaArvoreB(arqArvore);
				printf("\n\nEnter para continuar ...");
				getch();
				system("cls");
			break;
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
	bool divisao=false;
	bool promocao=false;
	short pagAtual = 0;
	short pagVim = 0;
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
			p->pai = -2;
			
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
			//printf("FILHO QUERO = %d", filhoQuero);
			//getch();
			fseek(arqArvore, (filhoQuero-1)*(int)sizeof(pagina), SEEK_SET);
			fread(p, sizeof(pagina), 1, arqArvore);
			
			/*printf("\nRRN: %d", p->rrn);
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
			getch();*/
			
			//ja pego qual a pagina estamos para qdo retornar ele aber onde colocar a chave
			pagAtual = p->rrn;
			
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
			//printf("ultimo filho quero = %d ", filhoQuero);
			//getch();
				//verifico se a pagina tem lugar sobrando			
				if(p->qtdKeys != MAX_KEYS){
					for(i=0;i<MAX_KEYS;i++){
						//encontro lugar em branco
						if(p->keys[i].key == -1){
							p->qtdKeys++;
							p->keys[i].key = id2;
							p->keys[i].byteoffset = byteoffset;
							ordenaChaves(p);
							
							/*printf("\nRRN: %d", p->rrn);
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
							getch();*/
							
							rewind(arqArvore);
							fseek(arqArvore, (p->rrn-1)*sizeof(pagina), SEEK_SET);
							fwrite(p, sizeof(pagina), 1, arqArvore);
							//break;
							return;
						}
					}
				//se não tenho q dividir a pagina etc
				} else{	
					//pego e junto a pagina com a chave q eu preciso e vejo se tem q romover ou criar nova raiz
					promocao = true;
					
					while(promocao){
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
						aux->pai = -2;
						for(i=0; i<ORDEM_PAG; i++){	
							aux->child[i] = -1;
						}
						ordenaChavesAux(aux);
						
						pagina *nova = malloc(sizeof(pagina));
						nova->rrn = rrnCount;						
						nova->qtdKeys = 0;	
						nova->pai = -2;
						rrnCount++;	
						
						for(i=0; i<MAX_KEYS; i++){					
							nova->keys[i].key = -1;
							nova->keys[i].byteoffset = -1;
							nova->child[i] = -1;
						}
						nova->child[4] = -1;
						
						p->qtdKeys = 2;
						
						p->keys[2].key = -1;
						p->keys[3].key = -1;
						p->keys[2].byteoffset = -1;
						p->keys[3].byteoffset = -1;
						
						p->keys[0].key = aux->keys[0].key;
						p->keys[1].key = aux->keys[1].key;
						p->keys[0].byteoffset = aux->keys[0].byteoffset;
						p->keys[1].byteoffset = aux->keys[1].byteoffset;
						
												
						nova->qtdKeys = 2;
						nova->keys[0].key = aux->keys[3].key;
						nova->keys[1].key = aux->keys[4].key;
						nova->keys[0].byteoffset = aux->keys[3].byteoffset;
						nova->keys[1].byteoffset = aux->keys[4].byteoffset;
						
						promoteKey = aux->keys[2].key;
						promoteByte = aux->keys[2].byteoffset;
						
						//se for uma destas opções, preciso criar nova raiz
						//if(pagAtual == 1 || pagAtual == rrnraiz){
						//se o pai nao existe, crio um novo pai
						if(p->pai == -2 ){
							//if(p->rrn == rrnraiz)				
							pagina *novaRaiz = malloc(sizeof(pagina));
							//paginaAuxiliar *aux = malloc(sizeof(paginaAuxiliar));
							//rewind(arqArvore);
							//fseek(arqArvore, ((filhoQuero-1)*sizeof(pagina))+, SEEK_SET);
							//fread(aux, sizeof(paginaAuxiliar), 1, arqArvore);
							
							//inicializa	
							novaRaiz->rrn = rrnCount;
							novaRaiz->qtdKeys = 0;	
							novaRaiz->pai = -2;
									
							for(i=0; i<MAX_KEYS; i++){					
								novaRaiz->keys[i].key = -1;
								novaRaiz->keys[i].byteoffset = -1;
								novaRaiz->child[i] = -1;
								
							}
							novaRaiz->child[4] = -1;
							//fim inicializa
							
							//seta valores						
							novaRaiz->qtdKeys++;
							novaRaiz->keys[0].key = promoteKey;
							novaRaiz->keys[0].byteoffset = promoteByte; 
							
							novaRaiz->child[0] = p->rrn;
							novaRaiz->child[1] = nova->rrn;
							
							p->pai = novaRaiz->rrn;
							nova->pai = novaRaiz->rrn;
							
							rewind(arqArvore);
							fwrite(p, sizeof(pagina), 1, arqArvore);
							fwrite(nova, sizeof(pagina), 1, arqArvore);
							fwrite(novaRaiz, sizeof(pagina), 1, arqArvore);
						
							rrnCount++;	
							rrnraiz=novaRaiz->rrn;
							promocao = false;
						} else {
							printf("TA ENTRANDO NA PROMOCAO PRO PAI");
							rewind(arqArvore);
							fseek(arqArvore, (p->pai-1)*sizeof(pagina), SEEK_SET);
							pagina* auxPai = malloc(sizeof(pagina));
							fread(auxPai, sizeof(pagina), 1, arqArvore);
							if(auxPai->qtdKeys != MAX_KEYS){
								for(i=0;i<MAX_KEYS;i++){
									//encontro lugar em branco
									if(auxPai->keys[i].key == -1){
										auxPai->qtdKeys++;
										auxPai->keys[i].key = promoteKey;
										auxPai->keys[i].byteoffset = promoteByte;
										auxPai->child[i] = p->rrn;
										auxPai->child[i+1] = nova->rrn;
										ordenaChaves(auxPai);
										
										rewind(arqArvore);
										fseek(arqArvore, (p->pai-1)*sizeof(pagina), SEEK_SET);
										fwrite(auxPai, sizeof(pagina), 1, arqArvore);
										
										p->pai = auxPai->rrn;
										nova->pai = auxPai->rrn;
										
										rewind(arqArvore);
										fseek(arqArvore, (p->rrn-1)*sizeof(pagina), SEEK_SET);										
										fwrite(p, sizeof(pagina), 1, arqArvore);
										fseek(arqArvore, 0, SEEK_END);
										fwrite(nova, sizeof(pagina), 1, arqArvore);
										break;
										//return;
									}
								}
								promocao = false;
							}
							
						}						
					}//fim while
				}//fim else
				
				
			} else{
				insereChave(arqArvore, id2, byteoffset, filhoQuero);				
			}
	}

	//for(i=0;i<MAX_KEYS;i++){
		//printf("\n\nrrnpagina = %d, quantdKey = %i, \nkey = %i \nbyteoffset = %d \nfilha numero %i = %d", p->rrn, p->qtdKeys, p->keys[i].key, p->keys[i].byteoffset,i, p->child[i]);
		//getch();
	//}
	
}

void ordenaChaves(pagina *p){
	int i, j, aux;
	short auxbyteoffset, auxChild;
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
                  
                  auxChild = p->child[j];
                  p->child[j] = p->child[j - 1];
                  p->child[j -1] = auxChild;
                  
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
	short rec_length = 0;
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

		//rewind(arqArvore);
		//fread(&aux, sizeof(raiz) ,1, arqArvore);
		
		//insereChave(arqArvore, id2, byteoffset, aux);
		insereChave(arqArvore, id2, byteoffset, rrnraiz);
		byteoffset = byteoffset + rec_length +2;	
		rec_length = obterRegistro(arqReg, strBuffer);
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
		printf("\nRRN: %d | Pai: %d ", p->rrn, p->pai);
		printf("\nChaves: ");
		for(i=0;i<MAX_KEYS;i++){
			printf("%i | ", p->keys[i].key);
		}
		printf("\nOffsets: ");
		for(i=0;i<MAX_KEYS;i++){
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

short buscaRegistro(FILE* arqArvore, int id, short filho){
	//short filho;
	int i;
	rewind(arqArvore);
	fseek(arqArvore, (filho-1)*sizeof(pagina), SEEK_SET);
	pagina *p = malloc(sizeof(pagina));
	fread(p, sizeof(pagina), 1, arqArvore);
	for(i=0;i<p->qtdKeys;i++){
		//encontro lugar em branco
		if(p->keys[i].key == id){
			printf("\n----------------------------------------------------------------");
			printf("\nEsta na pagina: %d ", p->rrn);						
			return p->keys[i].byteoffset;
		}
	}
	for(i=0;i<p->qtdKeys;i++){
		if(p->keys[i].key > id){
			filho = p->child[i];
			//rrnChild = i;
			return buscaRegistro(arqArvore, id, filho);
		} else{
			filho = p->child[i+1];
			if(i==p->qtdKeys-1)
				return buscaRegistro(arqArvore, id, filho);
		}
	}
	return;
}

void listaRegistro(FILE* arqReg, short offset){
	//pagina *p = malloc(sizeof(pagina));
	short tam;
	char reg[512];
	char *token;
	int campo = 1;
	rewind(arqReg);
	fseek(arqReg, offset, SEEK_SET);
	fread(&tam, sizeof(tam), 1, arqReg);
    fread(reg, 1 ,tam, arqReg);
	token = strtok(reg, "|");
	printf("| Byteoffset: %d | Tamanho do Registro : %i\n", offset, tam);
	while(token != NULL){
		printf("\nCampo #%i: %s", campo, token);
		token = strtok(NULL, "|");
		campo++;
	}
	printf("\n----------------------------------------------------------------");
}
