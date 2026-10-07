#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio2.h>
#include "TADString.h"
#include "TADTodosComandos.h"

// Trabalho feito por: Jose Miguel Gomes de Oliveira RA: 262515300

void removerQuebraLinha(char str[]) {
	int i = 0;
	while (str[i] != '\0') {
		if (str[i] == '\n' || str[i] == '\r') {
			str[i] = '\0';
		}
		i++;
	}
}

char escolhas() {
	printf("\n=========================================\n");
	printf("   SISTEMA DE GERENCIAMENTO DE BANCO DE DADOS   \n");
	printf("=========================================\n");
	printf("[A] Executar comandos SQL interativamente\n");
	printf("[B] Inserir arquivo de comandos (Script SQL)\n");
	printf("[C] Listar todas as tabelas\n");
	printf("[D] Mostrar todas as tabelas e suas colunas\n");
	printf("[E] Mudar para proximo banco\n");
	printf("[F] Mudar para banco anterior\n");
	printf("[ESC] Encerrar programa\n");
	printf("Opcao: ");
	return toupper(getche());
}

int main() {
	BD *banco;
	char caminho[1000], opcao;

	inicioBanco(&banco);

	printf("=========================================\n");
	printf("   INICIALIZACAO DO SGBD DINAMICO (LISTAS) \n");
	printf("=========================================\n");
	printf("[E] - Inserir arquivo de comandos padrao ('base.txt')\n");
	printf("[I] - Informar caminho do arquivo de comandos\n");
	printf("[Qualquer outra tecla] - Continuar sem carregar arquivo inicial\n");
	printf("Escolha uma opcao: ");

	opcao = toupper(getch());
	printf("%c\n\n", opcao);

	if (opcao == 'E') {
		strcpy(caminho, "base.txt");
		printf("Lendo arquivo '%s'...\n", caminho);
		leituraArquivoPricipal(&banco, caminho);
	} else if (opcao == 'I') {
		printf("Insira o caminho para o arquivo SQL: ");
		fgets(caminho, sizeof(caminho), stdin);

		removerQuebraLinha(caminho);

		printf("Lendo arquivo '%s'...\n", caminho);
		leituraArquivoPricipal(&banco, caminho);
	} else {
		printf("Sistema continuara sem pre-insercao de scripts SQL.\n\n");
	}

	printf("\nDeseja entrar no modo interativo? <S - Sim / ESC - Nao>: ");
	opcao = toupper(getch());
	printf("%c\n", opcao);

	while (opcao != 'S' && opcao != 27) {
		printf("Opcao invalida. Digite S para Sim ou ESC para Nao: ");
		opcao = toupper(getch());
		printf("%c\n", opcao);
	}

	while (opcao != 27) {
		clrscr();
		mostrarDadosSimples(banco);
		opcao = escolhas();
		printf("\n");


		switch (opcao) {
			case 'A':
				printf("\n--- Modo Interativo SQL ---\n");
				printf("Digite os comandos SQL finalizando com ';'. Deixe em branco/Enter para sair.\n");
				leituraEscrita(&banco);
				break;

			case 'B':
				printf("\n--- Leitura de Arquivo SQL ---\n");
				printf("Insira o caminho para o arquivo SQL: ");

				fflush(stdin);
				fgets(caminho, sizeof(caminho), stdin);

				removerQuebraLinha(caminho);

				if (caminho[0] != '\0') {
					leituraArquivoPricipal(&banco, caminho);
				} else {
					printf("Caminho invalido!\n");
				}
				getch();
				break;

			case 'C':
				printf("\n--- Tabelas do Banco de Dados ---\n");
				listarTabelas(banco);
				getch();
				break;

			case 'D':
				printf("\n--- Estrutura das Tabelas ---\n");
				listarColunasTabelas(banco);
				getch();
				break;
				
			case 'E':
				printf("\n--- Verificando possibilidade de troca ---\n");
				if(proximoBanco(&banco)){
					printf("\nFoi possivel trocar de banco de dados\n");
				}else{
					printf("\nNao foi possivel trocar de banco de dados\n");
				}
				getch();
				break;
				
			case 'F':
				printf("\n--- Verificando possibilidade de troca ---\n");
				if(bancoAnterior(&banco)){
					printf("\nFoi possivel trocar de banco de dados\n");
				}else{
					printf("\nNao foi possivel trocar de banco de dados\n");
				}
				getch();
				break;

			case 27:
				printf("\nEncerrando o programa...\n");
				break;

			default:
				printf("\nOpcao invalida! Tente novamente.\n");
				break;
		}
	}

	apagaTodosBancos(&banco);

	printf("\nPrograma finalizado com sucesso.\n");
	return 0;
}
