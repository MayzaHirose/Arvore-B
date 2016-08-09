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
bool buscaChave(short pagAtual, short key, short* foundRRN, short* foundPos);



struct paginaArvore {
	short rrnPagina;
	short key_count; //quantidade de chaves que estao na pagina
	short keys[MAX_KEYS];//chaves da pagina
	short offsets[MAX_KEYS];//o byte offset da chave no arquivo de registro
	short child[ORDEM_PAG];//rrns dos filhos
};

typedef struct paginaArvore pagina;


//Ponteiros para os arquivos
	FILE* arqCat;
	FILE* arqReg;
	FILE* arqArvore;
	
void inicializaPagina(pagina* p){
	int i;
	(*p).key_count = 0;
	for(i=0; i<MAX_KEYS; i++){					
		(*p).keys[i] = -1;
		(*p).offsets[i] = -1;
		(*p).child[i] = -1;
	}
	(*p).child[4] = -1;					
}

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
	
	short byteoffset = 0;
	short pagCount = 1;
	
	short pagAtual;
	
	char key2[2];
	short key;
	
	short foundRRN;
	short foundPos;
	
	
	/*pagina p;
	raiz r;
	p.child[0] = 1;
	p.keys[0] = 1;
	p.key_count = 1;
	p.offsets[0] = 1;	
	
	printf("CHILD = %d", p.child[0]);*/

	
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
	        	pagCount = 1;
	        	byteoffset = 0;
	        	
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
					printf("*Arquivo de Registro Criado!*");}
					
				//Cria um novo Arquivo de índice de árvore-b
				if((arqArvore = fopen("ArvoreB", "w+")) == NULL){
	    			printf("*ERRO!");
	    			break;
				}else{
					printf("\n\n*Arvore-B criada!*");}
				
				pagina p;	
				p.rrnPagina = pagCount;
				
				inicializaPagina(&p);
					
				rewind(arqArvore);
				fwrite(&p, sizeof(pagina), 1, arqArvore);
				rewind(arqArvore);
				fread(&pagAtual, sizeof(pagAtual), 1, arqArvore);							
				rewind(arqArvore);							
				
				/*Faz a importação dos dados do catalogo para o arquivo de registro com os devidos ajustes*/
				field_length = obterCampos(arqCat, strCampo, &contadorReg);
				strcat(strBuffer,strCampo);
    			strcat(strBuffer,"|");
			
    			while(field_length > 0){
			        campo ++;
			        //printf("\nCampo #%i = %s", campo, strCampo); 
			        strCampo[0] = '\0';

			        if(!(campo % 5) ){
			        	key2[0] = strBuffer[0];
						key2[1] = strBuffer[1];
						key = atoi(key2);
						buscaChave(pagAtual, key, &foundRRN, &foundPos);
						//insereChave(&rrnAtual, &key, )
						
						rewind(arqArvore);
						fseek(arqArvore, (sizeof(pagina)*(foundRRN-1))+5+foundPos, SEEK_SET);
						fwrite(&key, sizeof(key), 1, arqArvore);
						rewind(arqArvore);
						fseek(arqArvore, (sizeof(pagina)*(foundRRN-1))+(2*MAX_KEYS+1)+foundPos, SEEK_SET);
						fwrite(&byteoffset, sizeof(byteoffset), 1, arqArvore);
						
						
						
						
						
				        tamanhoRegistro = strlen(strBuffer);
				        
				        fwrite(&tamanhoRegistro, sizeof(tamanhoRegistro), 1, arqReg);
				        fwrite(strBuffer, tamanhoRegistro, 1, arqReg);
				        strBuffer[0] = '\0';
			        }

			        field_length = obterCampos(arqCat, strCampo, &contadorReg);
			        strcat(strBuffer,strCampo);
			        strcat(strBuffer,"|");
			        
			        byteoffset = tamanhoRegistro;
				}
				
				printf("\n\nIMPORTACAO REALIZADA COM SUCESSO!");	
					
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

bool buscaChave(short pagAtual, short key, short* foundRRN, short* foundPos){
	int i;
	pagina page;
	short pos;
	bool match;
	short n = -1;
	
	if(pagAtual == n){
		return false;
	}
	else{
		fread(&page.rrnPagina, sizeof(page.rrnPagina), 1, arqArvore);
		fread(&page.key_count, sizeof(page.key_count), 1, arqArvore);
		
		for(i=0; i<MAX_KEYS; i++){
			fread(&page.keys[i], sizeof(page.keys[i]), 1, arqArvore);
		}
		for(i=0; i<MAX_KEYS; i++){
			fread(&page.offsets[i], sizeof(page.offsets[i]), 1, arqArvore);
		}
		for(i=0; i<ORDEM_PAG; i++){		
			fread(&page.child[i], sizeof(page.child[i]), 1, arqArvore);
		}
		
		for(i=0; i<MAX_KEYS; i++){
			if(page.keys[i] == key|| page.keys[i] == -1){
				pos = i;											
				(*foundRRN)= pagAtual;
				(*foundPos) = pos;
				return true;
			} else if(page.keys[i] > key){
				pos = i;
			} else if(page.keys[i] < key){
				pos = i+1;
			}
			
		}
		return buscaChave(page.child[pos], key, foundRRN, foundPos);	
	}
	
}

/*bool insereChave(short *rrn, short *key, short *promo_key, short *promo_r_child){
	pagina page;
	
	if(*current_rrn == -1){
		(*promo_key)= (*key);
		(*promo_r_child) = -1;
		return true;
	} else{
		fread(&page.rrnPagina, sizeof(page.rrnPagina), 1, arqArvore);
		fread(&page.key_count, sizeof(page.key_count), 1, arqArvore);
		
		for(i=0, i<MAX_KEYS, i++){
			fread(&page.keys[i], sizeof(page.keys[i]), 1, arqArvore);
		}
		for(i=0, i<MAX_KEYS, i++){
			fread(&page.offsets[i], sizeof(page.offsets[i]), 1, arqArvore);
		}
		for(i=0, i<ORDEM_PAGINA, i++){		
			fread(&page.child[i], sizeof(page.child[i]), 1, arqArvore);
		}
		for(i=0, i<MAX_KEYS, i++){
			if(page.keys[i] == key){
				pos = i;											
				foundRRN = rrnAtual;
				foundPos = pos;
				return true;
			} else if(page.keys[i] > key){
				pos = i;
			} else if(page.keys[i] < key){
				pos = i+1;
			}
			
		}
	}
	
}*/
