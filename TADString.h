struct letras{
	char *l;
	int tam;
};
typedef struct letras String;

void inicString(String ** palavra){
	*palavra = NULL;
}

void salvaString(String ** palavra, int tamanho, char s[]){
	*palavra= (String*)malloc(sizeof(String));
	(*palavra)->l = (char*)malloc((tamanho+1)*sizeof(char));
	strcpy((*palavra)->l,s);
	(*palavra)->tam = tamanho;
}

// Retorna 1 se 'busca' estiver contida em 'texto', ou 0 caso contrário
int palavraContida(String *texto, String *busca) {
	int i, j, limite, achou;
	achou = 0;
	if(texto != NULL && busca != NULL && texto->l != NULL && busca->l != NULL && busca->tam <= texto->tam){
	    limite = texto->tam - busca->tam;
	
	    for (i = j = 0; i <= limite && j != busca->tam; i++) {
	        j = 0;
	
	        // Compara caractere por caractere a partir da posição 'i'
	        while (j < busca->tam && texto->l[i + j] == busca->l[j]) {
	            j++;
	        }    
	    }
	    // Se 'j' chegou ao tamanho da busca, todos os caracteres bateram
        if (j == busca->tam) {
            achou = 1;
        }
	}
    return achou; // 1 se encontrou, 0 caso contrario
}

int palavraContidaChar(String *texto, char *busca) {
    int i = 0, j = 0, tamBusca = 0, achei = 0;

    if (texto != NULL && texto->l != NULL && busca != NULL) {
        while (busca[tamBusca] != '\0') {
            tamBusca++;
        }
        while (i <= (texto->tam - tamBusca) && !achei) {
            j = 0;
            while (j < tamBusca && texto->l[i + j] == busca[j]) {
                j++;
            }
            if (j == tamBusca) {
                achei = 1;
            }
            i++;
        }
    }

    return achei;
}

void apagaString(String ** palavra){
	if(*palavra !=NULL){
		if((*palavra)->l != NULL)
			free((*palavra)->l);
		free(*palavra);
		*palavra = NULL;
	}
}

void limpaNome(String *origem, String **destino) {
    int tam = 0, i=0;
    char palavra[100];
    char c;
    if (origem != NULL && origem->l != NULL) {
        c = origem->l[i];
        while (c != '\0' && tam < 99) {
        	if(((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'|| c==' '|| c == '*'| c == '.' || c == '-')){
        		palavra[tam] = c;
            	tam++;	
        	}
			i++;
            c = origem->l[i];
        }
        palavra[tam] = '\0'; 
        inicString(&(*destino));
        salvaString(&(*destino), tam, palavra);
    }
}
