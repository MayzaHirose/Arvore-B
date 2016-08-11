//Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ORDEM_PAG 5
#define MAX_KEYS 4

/* Mayza Yuri Hirose da Costa RA 88739 */

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
	short esq;
	short dir;
}chave;

typedef struct {
	short rrn;
	chave keys[MAX_KEYS];
	int qtdKeys;
	short pai;
}pagina;

typedef struct {
	short rrn;
	chave keys[MAX_KEYS+1];
	int qtdKeys;
	short pai;
}paginaAuxiliar;

void criaArvore(FILE* arqReg, FILE* arqArvore);
void ordenaChaves(pagina *p);
void ordenaChavesAux(paginaAuxiliar *p);
short buscaRegistroArvore(FILE* arqArvore, int id, short filho);
void listaRegistro(FILE* arqReg, short offset);

short rrnCount = 1;
short rrnraiz = -2;
short byte = 0;
int promoteKey = 0;
short promoteByte = 0;

int main(){
	
	//Variáveis comuns
	int opcao = 0; //Opcão do menu
	int field_length = 0; //Armazena o tamanho do campo
	int campo = 0; //Contador de campos
	int contadorReg = 0; //Contador de registros
	short tamanhoRegistro = 0; //Armazena o tamanho do registro
	short rec_length; //Armazena o tamanho do registro
	char filename[20], strCampo[256], strBuffer[512]; // Strings
	char* token; //Armazena parte de string ao utilizar strtok
	char id[2];
	int id2;
	
	int searchKey;
	short offset;
	
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
	    scanf("%d", &opcao);
	    fflush(stdin);
	    
	    switch(opcao){
	    	
	    	/*Importação de Registro */
	        case 1:
	        	strCampo[0] = '\0';
	        	strBuffer[0] = '\0';

	       		printf("\nDigite o nome do catalogo a ser importado: ");
   				gets(filename);
    			if((arqCat = fopen(filename, "r")) == NULL){
        			printf("*ERRO!");
       				break;
    			}

	       		printf("Digite um nome para o Arquivo de Registros: ");
				gets(filename);
				if((arqReg = fopen(filename, "w+")) == NULL){
	    			printf("*ERRO!");
	    			break;
				}
								
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
			/*Fim Importação*/
			
			/*Inicio insere registro*/
			case 2:				
				strCampo[0] = '\0';
		        strBuffer[0] = '\0';
	
		        //Recebe o novo registro
		        receberDados(strBuffer);
		        printf("\n\nDados recebidos: %s", strBuffer);
		        
				//Escrevo o tamanho do registro e o registro no final do arquivo
		        fwrite(&tamanhoRegistro, sizeof(tamanhoRegistro), 1, arqReg);
	        	fwrite(strBuffer, tamanhoRegistro, 1, arqReg);	        		
	        		
	        	id[0] = strBuffer[0];
				id[1] = strBuffer[1]; 
				id2 = atoi(id);
			
				insereChave(arqArvore, id2, byte, rrnraiz);
				byte = byte + tamanhoRegistro +2;					        		
	    	break;
	    	/*Fim insere registro*/
			
			/*Busca de Registro*/
			case 3:
	        	printf("\nQual o ID procurado?: ");
	        	scanf("%i", &searchKey);
				
				offset = buscaRegistroArvore(arqArvore, searchKey, rrnraiz);
				listaRegistro(arqReg, offset);
				
				printf("\n\nEnter para continuar ...");
				getch();
				system("cls");
			break;
			/*Fim Busca Registro*/
			
			/*Listar Arvore B*/
			case 4:
				listaArvoreB(arqArvore);
				
				printf("\n\nEnter para continuar ...");
				getch();
				system("cls");
			break;
			/*Fim Listar Arvore B*/
		}
						    
	}while(opcao < 5 && opcao > 0);
	
	//encerra os arquivos ao final
	fclose(arqCat);
	fclose(arqReg);
	fclose(arqArvore);
}


void inicializaPagina(pagina* p){
	int i;
	p->rrn = 0;
	p->qtdKeys = 0;
	p->pai = -2;
	for(i=0; i<MAX_KEYS; i++){					
		p->keys[i].key = -1;
		p->keys[i].byteoffset = -1;				
		p->keys[i].esq = -1;
		p->keys[i].dir = -1;
	}					
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

void insereChave(FILE * arqArvore, int id, short byteoffset, short filhoQuero){
	int i;
	bool promocao = false;
	
	pagina *p = malloc(sizeof(pagina));
	
	//Primeiro caso, não existe pagina, raiz -2
	if(filhoQuero == -2){					
			
			inicializaPagina(p);
			
			p->rrn = rrnCount;		
			p->qtdKeys++;
			p->keys[0].key = id;
			p->keys[0].byteoffset = byteoffset;
			
			rewind(arqArvore);
			fwrite(p, sizeof(pagina), 1, arqArvore);
			rewind(arqArvore);
			fread(&rrnraiz, sizeof(rrnraiz) ,1, arqArvore);

			rrnCount++;	
			
	//Já tem página. Preciso buscar a página que eu quero inserir
	} else{
			rewind(arqArvore);
			fseek(arqArvore, (filhoQuero-1)*sizeof(pagina), SEEK_SET);
			fread(p, sizeof(pagina), 1, arqArvore);
			
			//Pego a posição q a chave deveria estar
			for(i=0;i<p->qtdKeys;i++){
				if(p->keys[i].key > id){					
					filhoQuero = p->keys[i].esq;					
					break;
				} else{				
					filhoQuero = p->keys[i].dir;					
				}
			}
			
			//Se o lugar onde eu deveria estar aponta pra -1
			if(filhoQuero == -1){	
			
				//Verifico se a pagina tem lugar sobrando			
				if(p->qtdKeys != MAX_KEYS){
					for(i=0;i<MAX_KEYS;i++){
						if(p->keys[i].key == -1){
							p->qtdKeys++;
							p->keys[i].key = id;
							p->keys[i].byteoffset = byteoffset;
							ordenaChaves(p);
							
							rewind(arqArvore);
							fseek(arqArvore, (p->rrn-1)*sizeof(pagina), SEEK_SET);
							fwrite(p, sizeof(pagina), 1, arqArvore);							
							return;
						}
					}
					
				//Se não tiver lugar, tenho q dividir a pagina.
				} else{	
				
					promocao = true;
					while(promocao){
						//Crio uma página auxiliar para colocar todas as chaves, inclusive a nova
						paginaAuxiliar *aux = malloc(sizeof(paginaAuxiliar));
						
						//Faço a auxiliar receber as chaves de p e a nova chave
						for(i=0;i<MAX_KEYS;i++){
							aux->keys[i].key = p->keys[i].key;												
							aux->keys[i].byteoffset = p->keys[i].byteoffset;
							aux->keys[i].esq = p->keys[i].esq;
							aux->keys[i].dir = p->keys[i].dir;
						}
						aux->keys[4].key = id;
						aux->keys[4].byteoffset = byteoffset;
						aux->keys[4].esq = -1;
						aux->keys[4].dir = -1;						
						aux->rrn = 0;
						aux->qtdKeys = 5;
						aux->pai = -2;					
						ordenaChavesAux(aux);
						
						pagina *nova = malloc(sizeof(pagina));						
						inicializaPagina(nova);
						
						nova->rrn = rrnCount;						
						rrnCount++;	
						
						//Agora retiro as chaves de 'p' que nao fazem mais parte dela e pego as novas de aux que ja foram ordenadas										
						p->qtdKeys = 2;
						
						//retiro as chaves
						for(i=2;i<4;i++){
							p->keys[i].key = -1;						
							p->keys[i].byteoffset = -1;
							p->keys[i].esq = -1;
							p->keys[i].dir = -1;
						}
						//Coloco novos valores no 'p'
						for(i=0;i<2;i++){
							p->keys[i].key = aux->keys[i].key;						
							p->keys[i].byteoffset = aux->keys[i].byteoffset;
						}
																		
						nova->qtdKeys = 2;						
						nova->keys[0].key = aux->keys[3].key;
						nova->keys[1].key = aux->keys[4].key;
						nova->keys[0].byteoffset = aux->keys[3].byteoffset;
						nova->keys[1].byteoffset = aux->keys[4].byteoffset;
						
						//A chave para promoção será a que está na posição 2
						promoteKey = aux->keys[2].key;
						promoteByte = aux->keys[2].byteoffset;
						
						//Verifico se o pai da página possui lugar disponível para promocao						
						promo:
						//se nao existe pai, crio uma nova raiz e seto os valores
						if(p->pai == -2 ){			
							pagina *novaRaiz = malloc(sizeof(pagina));
							
							inicializaPagina(novaRaiz);								
							novaRaiz->rrn = rrnCount;
							rrnCount++;

							//seta valores						
							novaRaiz->qtdKeys++;
							novaRaiz->keys[0].key = promoteKey;
							novaRaiz->keys[0].byteoffset = promoteByte; 
														
							novaRaiz->keys[0].esq = p->rrn;
							novaRaiz->keys[0].dir = nova->rrn;
							
							//As paginas que foram divididas recebem a nova raiz como pai
							p->pai = novaRaiz->rrn;
							nova->pai = novaRaiz->rrn;
																												
							//Gravo as páginas utilizadas																		
							rewind(arqArvore);
							fseek(arqArvore, (p->rrn-1)*sizeof(pagina), SEEK_SET);
							fwrite(p, sizeof(pagina), 1, arqArvore);
							
							rewind(arqArvore);
							fseek(arqArvore, (nova->rrn-1)*sizeof(pagina), SEEK_SET);
							fwrite(nova, sizeof(pagina), 1, arqArvore);

							fseek(arqArvore, 0, SEEK_END);
							fwrite(novaRaiz, sizeof(pagina), 1, arqArvore);
							
							//agora a raiz é a nova pagina criada							
							rrnraiz=novaRaiz->rrn;
							promocao = false;
							
						//Se o pai existe
						} else {
							
							rewind(arqArvore);
							fseek(arqArvore, (p->pai-1)*sizeof(pagina), SEEK_SET);
							pagina* auxPai = malloc(sizeof(pagina));
							fread(auxPai, sizeof(pagina), 1, arqArvore);
							
							//Se o pai tem lugar sobrando
							if(auxPai->qtdKeys != MAX_KEYS){
								for(i=0;i<MAX_KEYS;i++){
									//encontro lugar em branco
									if(auxPai->keys[i].key == -1){
										auxPai->qtdKeys++;
										auxPai->keys[i].key = promoteKey;
										auxPai->keys[i].byteoffset = promoteByte;																				
										auxPai->keys[i].esq = p->rrn;
										auxPai->keys[i].dir = nova->rrn;
										ordenaChaves(auxPai);
										
										//arrumo a ordenação
										for(i=0;i<MAX_KEYS;i++){
											if(auxPai->keys[i].key > promoteKey){
												auxPai->keys[i].esq = nova->rrn;
												break;
											} 
										}
										
										rewind(arqArvore);
										fseek(arqArvore, (p->pai-1)*sizeof(pagina), SEEK_SET);
										fwrite(auxPai, sizeof(pagina), 1, arqArvore);
										
										p->pai = auxPai->rrn;
										nova->pai = auxPai->rrn;
										
										rewind(arqArvore);
										fseek(arqArvore, (p->rrn-1)*sizeof(pagina), SEEK_SET);										
										fwrite(p, sizeof(pagina), 1, arqArvore);
										
										rewind(arqArvore);
										fseek(arqArvore, (nova->rrn-1)*sizeof(pagina), SEEK_SET);
										fwrite(nova, sizeof(pagina), 1, arqArvore);
										break;
									}
								}
								promocao = false;
								
							//Se o pai não tem lugar, preciso criar nova raiz...
							} else {	
									//Pagina auxiliar para ordenar a pagina							
									paginaAuxiliar *aux2 = malloc(sizeof(paginaAuxiliar));
									
									for(i=0;i<MAX_KEYS;i++){
										aux2->keys[i].key = auxPai->keys[i].key;
										aux2->keys[i].byteoffset = auxPai->keys[i].byteoffset;
										aux2->keys[i].esq = auxPai->keys[i].esq;
										aux2->keys[i].dir = auxPai->keys[i].dir;
									}									
									aux2->keys[4].key = promoteKey;																		
									aux2->keys[4].byteoffset = promoteByte;
									aux2->keys[4].esq = -1;
									aux2->keys[4].dir = -1;
									aux2->qtdKeys = 5;
									
									aux2->rrn = 0;									
									aux2->pai = -2;
																		
									//antes de ordenar ja seto onde o promote key inicial deveria apontar
									aux2->keys[4].esq = p->rrn;
									aux2->keys[4].dir = nova->rrn;
									
									ordenaChavesAux(aux2);
									
									for(i=0;i<MAX_KEYS+1;i++){
										if(aux2->keys[i].key > promoteKey){
											aux2->keys[i].esq = nova->rrn;
											break;
										} 
									}
									
									//Crio uma nova pagina que sera a q divide o pai que nao tinha espaço
									pagina *nova2 = malloc(sizeof(pagina));
																		
									inicializaPagina(nova2);
									nova2->rrn = rrnCount;						
									rrnCount++;	
									
									//Retiro chaves que nao fazem mais parte da pagina pai (que era a raiz que nao tinha espaço)
									auxPai->qtdKeys = 2;									
									for(i=2;i<4;i++){
										auxPai->keys[i].key = -1;									
										auxPai->keys[i].byteoffset = -1;									
										auxPai->keys[i].esq = -1;
										auxPai->keys[i].dir = -1;
									}
									
									//seto novos valores
									for(i=0;i<2;i++){
										auxPai->keys[i].key = aux2->keys[i].key;
										auxPai->keys[i].byteoffset = aux2->keys[i].byteoffset;
										auxPai->keys[i].esq = aux2->keys[i].esq;
										auxPai->keys[i].dir = aux2->keys[i].dir;
									}
									
									//nova pagina 2 recebe novas chaves						
									nova2->qtdKeys = 2;
									nova2->keys[0].key = aux2->keys[3].key;
									nova2->keys[1].key = aux2->keys[4].key;
									nova2->keys[0].byteoffset = aux2->keys[3].byteoffset;
									nova2->keys[1].byteoffset = aux2->keys[4].byteoffset;
									nova2->keys[0].esq = aux2->keys[3].esq;
									nova2->keys[0].dir = aux2->keys[3].dir;
									nova2->keys[1].esq = aux2->keys[4].esq;
									nova2->keys[1].dir = aux2->keys[4].dir;
									
									//Se a chave que estava sendo promovida é igual a chave que sera promovida,
									//preciso verificar para setar os valores dos pais e filhos 			
									if(promoteKey!=aux2->keys[2].key){
										if(auxPai->keys[0].key == promoteKey){
											p->pai = auxPai -> rrn;
											nova->pai = auxPai->rrn;							
											auxPai->keys[0].esq = p->rrn;
											auxPai->keys[0].dir = nova->rrn;
											auxPai->keys[1].esq = nova->rrn;
											
										} else if(auxPai->keys[1].key == promoteKey){
											p->pai = auxPai -> rrn;
											nova->pai = auxPai->rrn;
											auxPai->keys[1].esq = p->rrn;
											auxPai->keys[1].dir = nova->rrn;
											auxPai->keys[0].dir = p->rrn;
										} else if(nova2->keys[0].key == promoteKey){
											p->pai = nova2->rrn;
											nova->pai = nova2->rrn;
											nova2->keys[0].esq = p->rrn;
											nova2->keys[0].dir = nova->rrn;
											nova2->keys[1].esq = nova->rrn;
											
										} else if(nova2->keys[1].key == promoteKey){
											p->pai = nova2->rrn;
											nova->pai = nova2->rrn;
											nova2->keys[1].esq = p->rrn;
											nova2->keys[1].dir = nova->rrn;
											nova2->keys[0].dir = p->rrn;
										}
									} else {
										p->pai = auxPai->rrn;
										nova->pai = nova2->rrn;
										auxPai->keys[1].dir = p->rrn;
										nova2->keys[0].esq = nova->rrn;
									}

									promoteKey = aux2->keys[2].key;
									promoteByte = aux2->keys[2].byteoffset;

									rewind(arqArvore);
									fseek(arqArvore, (p->rrn-1)*sizeof(pagina), SEEK_SET);										
									fwrite(p, sizeof(pagina), 1, arqArvore);
									
									rewind(arqArvore);
									fseek(arqArvore, (nova->rrn-1)*sizeof(pagina), SEEK_END);
									fwrite(nova, sizeof(pagina), 1, arqArvore);

									for(i=0;i<MAX_KEYS;i++){
										p->keys[i].key = auxPai->keys[i].key;
										p->keys[i].byteoffset = auxPai->keys[i].byteoffset;
										p->keys[i].esq = auxPai->keys[i].esq;
										p->keys[i].dir = auxPai->keys[i].dir;
									}
									p->pai = -2;
									p->rrn = auxPai->rrn;
									p->qtdKeys = auxPai->qtdKeys;
									
									for(i=0;i<MAX_KEYS;i++){
										nova->keys[i].key = nova2->keys[i].key;
										nova->keys[i].byteoffset = nova2->keys[i].byteoffset;
										nova->keys[i].esq = nova2->keys[i].esq;
										nova->keys[i].dir = nova2->keys[i].dir;
									}
									nova->pai = -2;
									nova->rrn = nova2->rrn;
									nova->qtdKeys = nova2->qtdKeys;
									
									goto promo;
							}							
						}						
					}//fim while
				}//fim else
			//Desce enquanto tem filho		
			} else{
				insereChave(arqArvore, id, byteoffset, filhoQuero);				
			}
	}

}

void ordenaChaves(pagina *p){
	int i, j, aux;
	short auxbyteoffset, auxEsq, auxDir;
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
                                  
                  auxEsq = p->keys[j].esq;
                  p->keys[j].esq = p->keys[j-1].esq;
                  p->keys[j-1].esq = auxEsq;
                  
                  auxDir = p->keys[j].dir;
                  p->keys[j].dir = p->keys[j-1].dir;
                  p->keys[j-1].dir = auxDir;
                  
                  j--;
                  
            }
      }
      
    for(i=0;i<p->qtdKeys;i++){
		p->keys[i].key = valores[i];
	}
}

void ordenaChavesAux(paginaAuxiliar *p){
	int i, j, aux;
	short auxbyteoffset, auxEsq, auxDir;
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
                                    
                  auxEsq = p->keys[j].esq;
                  p->keys[j].esq = p->keys[j-1].esq;
                  p->keys[j-1].esq = auxEsq;
                  
                  auxDir = p->keys[j].dir;
                  p->keys[j].dir = p->keys[j-1].dir;
                  p->keys[j-1].dir = auxDir;
                  
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

	rewind(arqReg);
	rec_length = obterRegistro(arqReg, strBuffer);
	
	while(rec_length > 0){
		id[0] = strBuffer[0];
		id[1] = strBuffer[1]; 
		id2 = atoi(id);

		insereChave(arqArvore, id2, byteoffset, rrnraiz);
		byteoffset = byteoffset + rec_length +2;
		byte = byteoffset;	//para insercao externa
		rec_length = obterRegistro(arqReg, strBuffer);
	}			
}

void listaArvoreB(FILE* arqArvore){
	int i;
	pagina *p = malloc(sizeof(pagina));
	rewind(arqArvore);
	fread(p, sizeof(pagina), 1, arqArvore);
	while(feof(arqArvore) == 0){
		if(p->rrn == rrnraiz)
			printf("\n-----Pagina Raiz-----");
		printf("\nRRN: %d | Pai: %d | Qtd chaves: %i", p->rrn, p->pai, p->qtdKeys);
		printf("\nChaves: ");
		for(i=0;i<MAX_KEYS;i++){
			printf("%i | ", p->keys[i].key);
		}
		printf("\nOffsets: ");
		for(i=0;i<MAX_KEYS;i++){
			printf("%d | ", p->keys[i].byteoffset);
		}
		printf("\nValores Apontados pela Pagina: ");
		for(i=0;i<MAX_KEYS;i++){
			if(i==0)
				printf("%d | ", p->keys[i].esq);
			printf("%d | ", p->keys[i].dir);
		}
		printf("\n");
		fread(p, sizeof(pagina), 1, arqArvore);
	}
}

short buscaRegistroArvore(FILE* arqArvore, int id, short filho){
	int i;
	
	rewind(arqArvore);
	fseek(arqArvore, (filho-1)*sizeof(pagina), SEEK_SET);
	pagina *p = malloc(sizeof(pagina));
	fread(p, sizeof(pagina), 1, arqArvore);
	
	for(i=0;i<p->qtdKeys;i++){
		if(p->keys[i].key == id){
			printf("\n----------------------------------------------------------------");
			printf("\nEsta na pagina: %d ", p->rrn);						
			return p->keys[i].byteoffset;
		} 
	}
	for(i=0;i<p->qtdKeys;i++){
		if(p->keys[i].key > id){
			filho = p->keys[i].esq;
			return buscaRegistroArvore(arqArvore, id, filho);
		} else{
			filho = p->keys[i].dir;
			if(i==p->qtdKeys-1)
				return buscaRegistroArvore(arqArvore, id, filho);
		}
	}
	return;
}

void listaRegistro(FILE* arqReg, short offset){
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
