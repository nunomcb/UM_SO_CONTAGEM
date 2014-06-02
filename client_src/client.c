#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "contagem.h"

void incrementarController(){
	int tamContador = 0, i = 0, valor = 0;
	char nome[100], **contador;
	

	printf("\nIndique quantos nomes terá o contador: ");
	if(scanf("%d", &tamContador) < 1){
		printf("Erro de leitura\n");
		return;
	}
	contador = (char **) malloc( sizeof(char *) * tamContador);


	printf("Indique os nomes do contador (separados por espaço)\n");
	for(; i < tamContador; i++){
		if(scanf("%s", nome) < 1){
			printf("Erro de leitura\n");
			return;
		}
		contador[i] = malloc(sizeof(char) * (strlen(nome)+1));
		strcpy(contador[i] , nome);
	}


	printf("Indique o valor a ser incrementado: ");
	if(scanf("%d", &valor) < 1){
			printf("Erro de leitura\n");
			return;
	}
	contador[tamContador] = NULL;

	
	incrementar(contador, valor);
}

void agregarController(){
	int tamPrefixo = 0, i = 0, nivel = 0;
	char elem[100], **prefixo;
	

	printf("\nIndique quantos elementos terá o prefixo: ");
	if(scanf("%d", &tamPrefixo) < 1){
		printf("Erro de leitura\n");
		return;
	}

	prefixo = (char **) malloc( sizeof(char *) * tamPrefixo);



	printf("Indique os elementos do prefixo (separados por espaço)\n");
		for(; i < tamPrefixo; i++){
			if(scanf("%s", elem) < 1){
				printf("Erro de leitura\n");
				return;
			}
		prefixo[i] = malloc(sizeof(char) * (strlen(elem)+1));
		strcpy(prefixo[i] , elem);
	}

	printf("Indique o nivel de agregação: ");
		if(scanf("%d", &nivel) < 1){
			printf("Erro de leitura\n");
			return;
		}
	prefixo[tamPrefixo] = NULL;


	printf("\nIndique o nome do ficheiro (onde serão guardados os resultados)");
	if(scanf("%s", elem) < 1){
		printf("Erro de leitura\n");
		return;
	}

	if (agregar(prefixo,nivel,elem))
        printf("ESSA MERDA NÃO EXISTE!\n");
}

int main() {
	int continuar = 1, input = 0;

	printf("\nBem-vindo\n");

	while(continuar){
		printf("\n\nO que pretende fazer?\n1.Registar incremento\n2.Conhecer o total de contagens num grupo de contadores\n3.Sair\n>");
		
		if(scanf("%d", &input) < 1){
			printf("Erro de leitura\n");
			return -1;
		}

		switch(input){
			case 1:
				incrementarController();
				break;
			case 2:
				agregarController();
				break;
			case 3:
				continuar = 0;
				break;
			default:
				printf("Opção Inexistente\n");
				break;
		}
	}

	return 0;
}
