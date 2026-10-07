// ============================================================
// SGBD.h
// Simulador de SGBD em pequena escala - versao unificada
//
// Este arquivo junta em um unico lugar o que antes estava
// espalhado em TADBancoTotal.h, TADUtilitarios.h e
// TADCompiladorComandos.h, seguindo esta ordem:
//   1) Structs de todos os TADs
//   2) Prototipos de todas as funcoes, separados por area
//   3) Implementacao de todas as funcoes, na mesma ordem
//
// Depende de TADString2.h (tipo String e funcoes inicString,
// salvaString, apagaString, palavraContida, limpaNome, etc),
// que deve ser incluido ANTES deste arquivo.
// ============================================================
// ============================================================
// STRUCTS (todos os TADs unificados em um so lugar)
// ============================================================
#define MAX_TABELAS_JOIN 10

union valores{
	int ValorI;
	float ValorN;
	String *ValorD;
	char ValorC;
	String *ValorT;
};
struct dados{
	union valores v;
	struct dados *prox;
};
typedef struct dados Dados;

struct camp{
	Dados *Patual, *Pdados;
	String *Campo;
	char Tipo, PK, TamMax;
	struct camp *prox, *FK;
};
typedef struct camp Campos;

struct tab{
	String *Tabela;
	Campos *PCampos;
	struct tab *ant, *prox;
};
typedef struct tab Tabelas;

struct banco{
	String *Banco_Dados;
	Tabelas *PTabelas;
	struct banco *prox, *ant;
};
typedef struct banco BD;

// auxiliar de insert: guarda temporariamente os valores lidos de um INSERT
// antes deles serem validados e transferidos para a tabela
struct auxiliarInsert{
	union valores V;
	struct auxiliarInsert *prox;
};
typedef struct auxiliarInsert AuxInsert;

// descritor de combinacao de linhas (usado para representar os resultados de uma
// juncao/JOIN entre uma ou mais tabelas ao avaliar uma clausula WHERE).
// Cada no guarda o nome de UMA tabela e o numero da linha (posicao) daquela tabela
// que faz parte da combinacao encontrada.
// "proxTabela" liga os nos que fazem parte da MESMA combinacao (uma tabela apos a outra).
// "prox" liga o no cabeca de uma combinacao ate o no cabeca da PROXIMA combinacao encontrada.
struct caixaComandos{
	String *nomeTabela;
	int linha;
	struct caixaComandos *proxTabela;
	struct caixaComandos *prox;
};
typedef struct caixaComandos Comandos;

// campos Comandos: lista simples de tokens (usada por separaComandos, entre outras)
struct camposcomandos{
	String *campo;
	struct camposcomandos *prox;
};
typedef struct camposcomandos CCs;

// ============================================================
// PROTOTIPOS DAS FUNCOES (separados por area)
// ============================================================

// --- Banco de dados ---
void inicioBanco(BD ** b);
void criaCaixaBanco(BD ** b,String *nome);
void adicionaBanco(BD ** b, String *nome);
void alocaTabela(BD ** b, Tabelas *T);
void apagaTodosBancos(BD ** b);
char proximoBanco(BD** b);
char bancoAnterior(BD** b);

// --- Tabela ---
void inicioTabela(Tabelas **T);
void criaCaixaTabela(Tabelas **T,String *nome);
void adicionaTabela(Tabelas **T,String *nome);
void apagaTodasTabelas(Tabelas**T);
int quantidadeTabelas(BD * bd);

// --- Campo ---
void inicioCampos(Campos **C);
void adicionaCampo(Campos **C, String *nome, char Tipo, int TamMax);
void adicionaPKCampo(Campos **C, String *nome);
char existeCampoPK(String *campoProc, Tabelas *T);
Campos *procuraCampoTabela(Tabelas *T, String *nomeCampo, String *nomeTabela);
void reiniciaPonteirosDados(Campos *C);
void apagaTodosCampos(Campos ** C);

// --- Dado ---
void adicionaDado(union valores V, Dados **D);
void apagaTodosValoresN(Dados ** D);
void apagaTodosValoresD(Dados ** D);
void apagaTodosValoresS(Dados ** D);
int existeDadoFK(union valores V, char tipo, Dados *D);

// --- Auxiliar de insert ---
void iniciaAuxiliar(AuxInsert **ai);
void adicionaAuxiliar(AuxInsert **ai, union valores V);
void apagaAuxiliar(AuxInsert **ai);

// --- Resultados de combinacao de linhas (join / where) ---
void iniciaResultados(Comandos ** resultados);
Comandos * criaNoTabela(String *nomeTabela, int linha);
void adicionaResultado(Comandos ** resultados, Comandos *combo);
Comandos * procuraTabelaCombo(Comandos *combo, String *nomeTabela);
Comandos * inverterResultados(Comandos *inicio);
void apagaResultados(Comandos ** resultados);

// --- Campos Comandos (tokens) ---
void iniciaComandos(CCs **cc);
void adicionaComandos(CCs **cc,String *s);
void apagaComandos(CCs **cc);
void exibeComandos(CCs *cc);
CCs * separaComandos(char comando[]);

// --- Visualizacao de dados ---
void mostrarDadosSimples(BD*banco);
void listarTabelas(BD *bd);
void listarColunasTabelas(BD *bd);

// --- Compilador de comandos SQL ---
void CreateDataBase(BD **bd,CCs *comandos);
void CreateTable(BD **bd, CCs *comandos);
void alterTable(BD *bd,CCs *comandos);
Tabelas * procuraTabelaPorNome(BD *bd, String *nome);
Campos * resolveCampo(BD *bd, CCs *tabelas, char token[]);
int avaliaCondicaoAtual(BD *bd, CCs *tabelas, CCs *condicao);
void gerarCombinacoes(BD *bd, CCs *tabelas, CCs *condicao,CCs *listaTabs[], Tabelas *tabsReais[], int linhaAtual[],int totalTabelas, Comandos **resultados);
Comandos * posicaoCondicao(BD *bd, CCs *tabelas, CCs *condicao);
void insert(BD *bd, CCs *comandos);
int obterLarguraColuna(BD *bd, CCs *tabelas, String *nomeColuna);
CCs * montaLinhaDivisoria(BD *bd, CCs *tabelas, CCs *colunas);
CCs * montaCabecalho(BD *bd, CCs *tabelas, CCs *colunas);
void selectDados(BD *bd, CCs *comandos);
void Delete(BD **bd, CCs *comandos);
void Update(BD **bd, CCs *comandos);

// --- Leitura de comandos ---
void leituraArquivoPricipal(BD**bd, char caminho[]);
void leituraEscrita(BD**bd);


// ============================================================
// IMPLEMENTACAO DAS FUNCOES (mesma ordem e agrupamento dos prototipos)
// ============================================================

// --- Banco de dados ---

// Inicializa o ponteiro do banco de dados como vazio (lista ainda sem nenhum banco).
void inicioBanco(BD ** b){
	*b = NULL;
}

// Aloca e prepara a caixa (o no) de um novo banco de dados, mas ainda
// nao encaixa ela na lista -- isso e trabalho de quem chamou.
void criaCaixaBanco(BD ** b,String *nome){
	*b=(BD*)malloc(sizeof(BD));
	(*b)->Banco_Dados = nome;
	(*b)->PTabelas = NULL;
	(*b)->prox = NULL;
	(*b)->ant = NULL;
}

// Cria um novo banco de dados (com o nome informado) e o encaixa no
// final da lista de bancos ja existentes.
void adicionaBanco(BD ** b, String *nome){
	BD *novo, *aux;
	criaCaixaBanco(&novo, nome);
	if(*b==NULL){
		*b=novo;
	}else{
		aux = *b;
		while(aux->prox!=NULL){
			aux = aux ->prox;
		}
		novo->ant = aux;
		aux->prox = novo;
	}
}

// Encaixa uma tabela (ja pronta, com seus campos) no final da lista
// de tabelas de um banco de dados.
void alocaTabela(BD ** b, Tabelas *T){
	Tabelas *aux;
	if (b != NULL && *b != NULL) {
		if ((*b)->PTabelas == NULL) {
			(*b)->PTabelas = T;
		} else {
			aux = (*b)->PTabelas;
			while (aux->prox != NULL) {
				aux = aux->prox;
			}
			aux->prox = T;
			T->ant = aux;
		}
	}
}

// Igual a apagaBanco, mas sem precisar de nome: apaga TODOS os bancos
// alocados (e todo o conteudo de cada um), ate a lista ficar vazia.
void apagaTodosBancos(BD ** b){
	BD *aux;
	while(*b != NULL){
		aux = *b;
		apagaTodasTabelas(&aux->PTabelas);
		apagaString(&aux->Banco_Dados);
		*b = (*b)->prox;
		free(aux);
	}
}

// Avanca o ponteiro do banco atual para o proximo da lista, se houver.
// Devolve 1 se conseguiu avancar, ou 0 se ja estava no ultimo.
char proximoBanco(BD** b){
	char foi =0;
	if((*b)->prox!=NULL){
		*b = (*b)->prox;
		foi = 1;
	}
	return foi;
}

// Volta o ponteiro do banco atual para o anterior da lista, se houver.
// Devolve 1 se conseguiu voltar, ou 0 se ja estava no primeiro.
char bancoAnterior(BD** b){
	char foi =0;
	if((*b)->ant!=NULL){
		*b = (*b)->ant;
		foi = 1;
	}
	return foi;
}


// --- Tabela ---

// Inicializa a lista de tabelas como vazia.
void inicioTabela(Tabelas **T){
	*T = NULL;
}

// Aloca e prepara a caixa (o no) de uma nova tabela, ainda sem
// encaixar ela em nenhuma lista.
void criaCaixaTabela(Tabelas **T,String *nome){
	(*T) = (Tabelas*)malloc(sizeof(Tabelas));
	(*T)->Tabela = nome;
	(*T)->PCampos = NULL;
	(*T)->prox = (*T)->ant = NULL;
}

// Cria uma nova tabela e a insere no INICIO da lista de tabelas.
void adicionaTabela(Tabelas **T,String *nome){
	Tabelas * nova;
	criaCaixaTabela(&nova,nome);
	if(*T==NULL){
		*T = nova;
	}else{
		(*T)->ant = nova;
		nova->prox = *T;
		*T = nova;
	}
}

// Percorre e libera todas as tabelas de uma lista, apagando tambem
// todos os campos (e os dados) de cada uma delas.
void apagaTodasTabelas(Tabelas**T){
	Tabelas *aux;
	while(*T!= NULL){
		aux = *T;
		apagaTodosCampos(&(*T)->PCampos);
		*T = (*T)->prox;
		apagaString(&aux->Tabela);
		free(aux);
	}
}

// Conta quantas tabelas estao alocadas em um banco de dados.
int quantidadeTabelas(BD * bd){
	int quant=0;
	Tabelas *aux = bd->PTabelas;
	while(aux!=NULL){
		quant ++;
		aux = aux->prox;
	}
	return quant;
}


// --- Campo ---

// Inicializa a lista de campos como vazia.
void inicioCampos(Campos **C){
	*C=NULL;
}

// Cria um novo campo (com nome, tipo e tamanho maximo) e o insere no
// final da lista de campos de uma tabela.
void adicionaCampo(Campos **C, String *nome, char Tipo, int TamMax){
	Campos *nova = (Campos*)malloc(sizeof(Campos)), *aux;
	nova->Campo=nome;
	nova->Tipo =Tipo;
	nova->PK = 'N';
	nova->TamMax =TamMax;
	nova->Patual = nova->Pdados = NULL;
	nova->FK = nova->prox = NULL;
	if(*C==NULL){
		*C=nova;
	}else{
		aux= *C;
		while(aux->prox!=NULL)
			aux = aux->prox;
		aux->prox = nova;
	}
}

// Marca, pelo nome, um campo ja existente na lista como chave primaria (PK).
void adicionaPKCampo(Campos **C, String *nome){
	Campos *aux;
	if(stricmp((*C)->Campo->l,nome->l)==0){
		(*C)->PK = 'S';
	}else{
		aux=(*C)->prox;
		while(aux!=NULL&&stricmp(aux->Campo->l,nome->l)!=0)
			aux = aux->prox;
		if(aux!=NULL){
			aux->PK = 'S';
		}
	}
}

// Verifica se um determinado nome de campo e chave primaria em
// alguma das tabelas da lista informada.
char existeCampoPK(String *campoProc, Tabelas *T){
	Campos *auxC;
	int achou = 0;
	while(T!=NULL && achou == 0){
		auxC = T->PCampos;
		while(auxC!=NULL && achou ==0){
			if(strcmp(auxC->Campo->l, campoProc->l)==0 &&auxC->PK=='S'){
				achou++;
			}
			auxC = auxC->prox;
		}
		T = T->prox;
	}
	return achou;
}

// Procura um campo especifico dentro de uma tabela especifica,
// combinando o nome da tabela com o nome do campo.
Campos *procuraCampoTabela(Tabelas *T, String *nomeCampo, String *nomeTabela){
	Campos *Volta = NULL;
	if(T!=NULL){
		while(T!=NULL && strcmp(T->Tabela->l,nomeTabela->l)!=0){
			T=T->prox;
		}
		if(T!=NULL){
			Volta = T->PCampos;
			while(Volta!=NULL && strcmp(Volta->Campo->l, nomeCampo->l)!=0){
				Volta= Volta->prox;
			}
		}
	}
	return Volta;
}

// Reposiciona o ponteiro "Patual" de cada campo de volta para o
// inicio dos dados, preparando a tabela para uma nova leitura.
void reiniciaPonteirosDados(Campos *C){
	while(C!=NULL){
		C->Patual = C->Pdados;
		C = C->prox;
	}
}

// Libera todos os campos de uma tabela, apagando tambem todos os
// dados guardados em cada um deles.
void apagaTodosCampos(Campos ** C){
	Campos *aux;
	while(*C!=NULL){
		aux = *C;
		*C = (*C)->prox;
		if(aux->Tipo=='T')
			apagaTodosValoresS(&aux->Pdados);
		else if(aux->Tipo=='D')
			apagaTodosValoresD(&aux->Pdados);
		else
			apagaTodosValoresN(&aux->Pdados);
		apagaString(&aux->Campo);
		free(aux);
	}
}


// --- Dado ---

// Adiciona um novo valor no final da lista de dados de um campo.
void adicionaDado(union valores V, Dados **D){
	Dados *aux,*novo;
	novo = (Dados*)malloc(sizeof(Dados));
	novo->v = V;
	novo->prox = NULL;
	if(*D==NULL){
		*D = novo;
	}else{
		aux = *D;
		while(aux->prox!=NULL){
			aux = aux->prox;
		}
		aux->prox = novo;
	}
}

// Libera uma lista de dados numericos/simples (inteiro, float ou
// char), que nao guardam nenhuma String internamente.
void apagaTodosValoresN(Dados ** D){
	Dados *aux;
	while(*D!= NULL){
		aux= *D;
		*D = (*D)->prox;
		free(aux);
	}
}

// Libera uma lista de dados do tipo data, apagando tambem a String
// guardada dentro de cada valor.
void apagaTodosValoresD(Dados ** D){
	Dados *aux;
	while(*D!= NULL){
		aux= *D;
		*D = (*D)->prox;
		apagaString(&aux->v.ValorD);
		free(aux);
	}
}

// Libera uma lista de dados do tipo texto, apagando tambem a String
// guardada dentro de cada valor.
void apagaTodosValoresS(Dados ** D){
	Dados *aux;
	while(*D!= NULL){
		aux= *D;
		*D = (*D)->prox;
		apagaString(&aux->v.ValorT);
		free(aux);
	}
}

// Verifica se um determinado valor ja existe na lista de dados de um campo
// (usado para validar chaves estrangeiras - FK).
int existeDadoFK(union valores V, char tipo, Dados *D) {
	Dados *aux = D;
	int achou = 0;
	while (aux != NULL && achou == 0) {
		if (tipo == 'I') {
			if (aux->v.ValorI == V.ValorI) {
				achou = 1;
			}
		} 
		else if (tipo == 'N') {
			if (aux->v.ValorN == V.ValorN) {
				achou = 1;
			}
		} 
		else if (tipo == 'C') {
			if (aux->v.ValorC == V.ValorC) {
				achou = 1;
			}
		} 
		else if (tipo == 'T') {
			if (aux->v.ValorT != NULL && V.ValorT != NULL) {
				if (strcmp(aux->v.ValorT->l, V.ValorT->l) == 0) {
					achou = 1;
				}
			}
		} 
		else if (tipo == 'D') {
			if (aux->v.ValorD != NULL && V.ValorD != NULL) {
				if (strcmp(aux->v.ValorD->l, V.ValorD->l) == 0) {
					achou = 1;
				}
			}
		}
		aux = aux->prox;
	}
	return achou;
}


// --- Auxiliar de insert ---

// Inicializa a lista auxiliar de insert como vazia.
void iniciaAuxiliar(AuxInsert **ai){
	*ai = NULL;
}

// Adiciona um valor na lista auxiliar usada para guardar
// temporariamente os valores lidos de um comando INSERT.
void adicionaAuxiliar(AuxInsert **ai, union valores V){
	AuxInsert *novo, *aux;
	novo =(AuxInsert *)malloc(sizeof(AuxInsert));
	novo->V = V;
	novo->prox=NULL;
	if(*ai==NULL){
		*ai = novo;
	}else{
		aux = *ai;
		while(aux->prox!=NULL)
			aux = aux->prox;
		aux->prox=novo;
	}
}

// Libera todos os nos da lista auxiliar de insert. Nao mexe no
// conteudo da union -- isso e responsabilidade de quem usou os dados antes.
void apagaAuxiliar(AuxInsert **ai) {
    AuxInsert *aux = NULL;
    while (*ai != NULL) {
    	aux = *ai;
    	*ai = (*ai)->prox;
        free(aux);
    }
}


// --- Resultados de combinacao de linhas (join / where) ---

// Inicializa a lista de resultados (combinacoes de linhas) como vazia.
void iniciaResultados(Comandos ** resultados){
	*resultados = NULL;
}

//Cria um unico no (uma tabela + uma linha). SEMPRE copia a String recebida
//(nunca guarda o ponteiro original) -- assim nunca corremos o risco de, ao apagar
//esta estrutura depois, apagar por tabela o nome real de uma tabela do banco.
Comandos * criaNoTabela(String *nomeTabela, int linha){
	Comandos *novo;
	novo = (Comandos*)malloc(sizeof(Comandos));
	inicString(&novo->nomeTabela);
	salvaString(&novo->nomeTabela, nomeTabela->tam, nomeTabela->l);
	novo->linha = linha;
	novo->proxTabela = NULL;
	novo->prox = NULL;
	return novo;
}

//Adiciona uma combinacao inteira (ja pronta) na lista de resultados (encadeia por prox)
void adicionaResultado(Comandos ** resultados, Comandos *combo){
	Comandos *aux;
	if(*resultados == NULL){
		*resultados = combo;
	}else{
		aux = *resultados;
		while(aux->prox != NULL)
			aux = aux->prox;
		aux->prox = combo;
	}
}

//Procura, DENTRO de uma combinacao, o no que corresponde a uma determinada tabela
Comandos * procuraTabelaCombo(Comandos *combo, String *nomeTabela){
	while(combo != NULL && stricmp(combo->nomeTabela->l, nomeTabela->l) != 0)
		combo = combo->proxTabela;
	return combo;
}

//Inverte a lista de resultados (usada pelo DELETE, para apagar do fim para o
//inicio e nao perder a posicao das linhas que ainda faltam remover)
Comandos * inverterResultados(Comandos *inicio){
	Comandos *anterior = NULL, *atual = inicio, *proximo;
	while(atual != NULL){
		proximo = atual->prox;
		atual->prox = anterior;
		anterior = atual;
		atual = proximo;
	}
	return anterior;
}

//Libera TODA a estrutura: cada combinacao (nivel externo, por "prox") e, dentro
//de cada uma, cada tabela participante (nivel interno, por "proxTabela"). Como
//toda String guardada aqui e sempre uma COPIA (ver criaNoTabela), e sempre seguro
//apagar -- nunca vamos apagar o nome real de uma tabela do banco de dados.
void apagaResultados(Comandos ** resultados){
	Comandos *comboAtual, *proxCombo, *noAtual, *proxNo;
	comboAtual = *resultados;
	while(comboAtual != NULL){
		proxCombo = comboAtual->prox;
		noAtual = comboAtual;
		while(noAtual != NULL){
			proxNo = noAtual->proxTabela;
			apagaString(&noAtual->nomeTabela);
			free(noAtual);
			noAtual = proxNo;
		}
		comboAtual = proxCombo;
	}
	*resultados = NULL;
}


// --- Campos Comandos (tokens) ---

// Inicializa uma lista de comandos (tokens) como vazia.
void iniciaComandos(CCs **cc){
	*cc=NULL;
}

// Adiciona um novo token (campo) no final de uma lista de comandos.
void adicionaComandos(CCs **cc,String *s){
	CCs *aux, *novo;
	novo =(CCs*)malloc(sizeof(CCs));
	novo->campo = s;
	novo->prox = NULL;
	if(*cc == NULL){
		*cc=novo;
	}else{
		aux = *cc;
		while(aux->prox!=NULL)
			aux=aux->prox;
		aux->prox = novo;
	}
}

// Libera todos os nos de uma lista de comandos, apagando tambem a
// String guardada em cada um.
void apagaComandos(CCs **cc){
	CCs * aux;
	while(*cc!=NULL){
		aux = *cc;
		*cc= (*cc)->prox;
		apagaString(&aux->campo);
		free(aux);
	}
}

// Imprime na tela, um por linha, o conteudo de cada no da lista de comandos.
void exibeComandos(CCs *cc){
	while(cc!=NULL){
		printf("%s\n",cc->campo->l);
		cc=cc->prox;
	}
}

// Quebra uma linha de comando SQL em uma lista de tokens (palavras,
// operadores, virgulas, etc), respeitando o que esta entre aspas.
CCs * separaComandos(char comando[]){
	CCs *cmds;
	String * cmd;
	char separado[100];
	int i=0, j=0, opLen;
	int dentroAspas = 0; 	
	iniciaComandos(&cmds);	
	while(comando[i] != ';' && comando[i] != '\0'){
		if (comando[i] == '\'') {
			dentroAspas = !dentroAspas;
			separado[j++] = comando[i];
		}else if (!dentroAspas && (
                    (comando[i] == '>' && comando[i+1] == '=') ||
                    (comando[i] == '<' && comando[i+1] == '=') ||
                    (comando[i] == '<' && comando[i+1] == '>') ||
                    (comando[i] == '!' && comando[i+1] == '=') ||
                    (comando[i] == '=' && comando[i+1] == '=') ||
                    comando[i] == '>' || comando[i] == '<' || comando[i] == '=')) {

            // 1. Se havia algum texto sendo acumulado antes do operador (ex: "idade"), salva ele
            if (j > 0) {
                separado[j] = '\0';
                inicString(&cmd);
                salvaString(&cmd, j, separado);
                adicionaComandos(&cmds, cmd);
                j = 0;
            }

            // 2. Determina se e um operador de 2 caracteres ou de 1 caractere
            if ((comando[i] == '>' && comando[i+1] == '=') ||
                (comando[i] == '<' && comando[i+1] == '=') ||
                (comando[i] == '<' && comando[i+1] == '>') ||
                (comando[i] == '!' && comando[i+1] == '=') ||
                (comando[i] == '=' && comando[i+1] == '=')) {
                opLen = 2;
            } else {
                opLen = 1;
            }

            // 3. Monta e salva o operador como um token
            separado[0] = comando[i];
            if (opLen == 2) {
                separado[1] = comando[i+1];
                separado[2] = '\0';
                i++; // Avanca o caractere extra por causa do operador duplo
            } else {
                separado[1] = '\0';
            }

            inicString(&cmd);
            salvaString(&cmd, opLen, separado);
            adicionaComandos(&cmds, cmd);
            j = 0;

        }else if ((comando[i] == ' ' || comando[i] == '\n' || comando[i] == '\t' || comando[i] == '\r') && !dentroAspas) {
			if (j>0) {
				separado[j] = '\0';
				inicString(&cmd);
				salvaString(&cmd, j, separado);
				adicionaComandos(&cmds, cmd);
				j = 0;
			}
		}else {
			separado[j++] = comando[i];
		}
		i++;
	}
	
	if (j > 0) {
		separado[j] = '\0';
		inicString(&cmd);
		salvaString(&cmd, j, separado);
		adicionaComandos(&cmds, cmd);
	}
	
	return cmds;
}


// --- Visualizacao de dados ---

// Mostra na tela informacoes basicas do banco de dados atual (nome e
// quantidade de tabelas).
void mostrarDadosSimples(BD*banco){
	if(banco !=NULL){
		printf("Usando: banco %s\n", banco->Banco_Dados->l);
		if(banco->PTabelas!=NULL){
			printf("Quantidade de tabelas: %d\n\n", quantidadeTabelas(banco));
		}else{
			printf("Sem tabelas alocadas\n\n");
		}
	}else{
		printf("Banco de dados nao alocado\n");
	}
}

// Lista o nome de todas as tabelas alocadas em todos os bancos de dados
// existentes no programa no momento (percorre "prox" de BD e "prox" de Tabelas).
void listarTabelas(BD *bd) {
	BD *auxBD;
	Tabelas *auxTab;
	int existeTabela;

	existeTabela = 0;
	auxBD = bd;
	while (auxBD != NULL) {
		auxTab = auxBD->PTabelas;
		while (auxTab != NULL) {
			printf(" - %s (banco: %s)\n", auxTab->Tabela->l, auxBD->Banco_Dados->l);
			existeTabela = 1;
			auxTab = auxTab->prox;
		}
		auxBD = auxBD->prox;
	}
	if (!existeTabela) {
		printf("\nNenhuma tabela alocada no momento.\n");
	}
}

// Mostra todas as tabelas de todos os bancos existentes, e para cada uma
// exibe suas colunas (nome, tipo, tamanho maximo e se e chave primaria).
void listarColunasTabelas(BD *bd) {
	BD *auxBD;
	Tabelas *auxTab;
	Campos *auxCamp;
	int existeTabela;
	char tipoDescricao[15];

	existeTabela = 0;
	auxBD = bd;
	while (auxBD != NULL) {
		auxTab = auxBD->PTabelas;
		while (auxTab != NULL) {
			existeTabela = 1;
			printf("\nTabela: %s (banco: %s)\n", auxTab->Tabela->l, auxBD->Banco_Dados->l);
			printf("+----------------------+---------------+-------+----+\n");
			printf("| Coluna               | Tipo          | Tam.  | PK |\n");
			printf("+----------------------+---------------+-------+----+\n");

			auxCamp = auxTab->PCampos;
			while (auxCamp != NULL) {
				if (auxCamp->Tipo == 'I') {
					strcpy(tipoDescricao, "INTEGER");
				} else if (auxCamp->Tipo == 'N') {
					strcpy(tipoDescricao, "NUMERIC");
				} else if (auxCamp->Tipo == 'C') {
					strcpy(tipoDescricao, "CHARACTER(1)");
				} else if (auxCamp->Tipo == 'D') {
					strcpy(tipoDescricao, "DATE");
				} else {
					strcpy(tipoDescricao, "CHARACTER");
				}

				printf("| %-20s | %-13s | %5d | %2c |\n",
					auxCamp->Campo->l, tipoDescricao, (int)auxCamp->TamMax, auxCamp->PK);

				auxCamp = auxCamp->prox;
			}
			printf("+----------------------+---------------+-------+----+\n");

			auxTab = auxTab->prox;
		}
		auxBD = auxBD->prox;
	}
	if (!existeTabela) {
		printf("\nNenhuma tabela alocada no momento.\n");
	}
}


// --- Compilador de comandos SQL ---

// Interpreta um comando CREATE DATABASE e cria um novo banco de
// dados com o nome informado.
//Exemplo de uso:
//CREATE DATABASE db_locadora;
void CreateDataBase(BD **bd,CCs *comandos){
	String *nome;
	while (comandos != NULL && (stricmp(comandos->campo->l, "CREATE") == 0 || stricmp(comandos->campo->l, "DATABASE") == 0))
		comandos = comandos->prox;
	if (comandos != NULL) {
		limpaNome(comandos->campo, &nome);
		adicionaBanco(&*bd, nome);
	}
}

// Interpreta um comando CREATE TABLE e monta a tabela com todos os
// seus campos, tipos e a chave primaria (PK).
//Exemplo de uso:
//CREATE TABLE cliente (
// id_cliente INTEGER NOT NULL,
// nome CHARACTER(20) ,
// cpf CHARACTER(20) ,
// celular CHARACTER(20) ,
// CONSTRAINT PK_cliente PRIMARY KEY (id_cliente)
//);
void CreateTable(BD **bd, CCs *comandos) {
	Tabelas *tab;
	Campos *c, *aux;
	String *nome;
	int dentroParentesis = 0, fimPK = 0;
	inicioTabela(&tab);
	inicioCampos(&c);
	while (comandos != NULL) {
		if (dentroParentesis == 0) {
			if (stricmp(comandos->campo->l, "CREATE") != 0 && stricmp(comandos->campo->l, "TABLE") != 0) {
				limpaNome(comandos->campo, &nome);
				adicionaTabela(&tab, nome);
				dentroParentesis++;
			}
		} else {
			if (stricmp(comandos->campo->l, "CONSTRAINT") == 0) {
				while (comandos != NULL && stricmp(comandos->campo->l, "KEY") != 0) {
					comandos = comandos->prox;
				}
				if (comandos != NULL) {
					comandos = comandos->prox;
					fimPK = 0;
					while (comandos != NULL && fimPK == 0) {
						aux = c;
						while (aux != NULL) {
							if (palavraContida(comandos->campo, aux->Campo)) {
								adicionaPKCampo(&aux, aux->Campo);
							}
							aux = aux->prox;
						}
						
						if (palavraContidaChar(comandos->campo, ")")) {
							fimPK = 1;
						}
						
						if (fimPK == 0) {
							comandos = comandos->prox;
						}
					}
				}
			} else if (comandos->prox != NULL) {
				if (palavraContidaChar(comandos->prox->campo, "INTEGER")) {
					limpaNome(comandos->campo, &nome);
					adicionaCampo(&c, nome, 'I', 10);
				} else if (palavraContidaChar(comandos->prox->campo, "DATE")) {
					limpaNome(comandos->campo, &nome);
					adicionaCampo(&c, nome, 'D', 10);
				} else if (palavraContidaChar(comandos->prox->campo, "CHARACTER(20)")) {
					limpaNome(comandos->campo, &nome);
					adicionaCampo(&c, nome, 'T', 20);
				} else if (palavraContidaChar(comandos->prox->campo, "CHARACTER(1)")) {
					limpaNome(comandos->campo, &nome);
					adicionaCampo(&c, nome, 'C', 1);
				} else if (palavraContidaChar(comandos->prox->campo, "NUMERIC")) {
					limpaNome(comandos->campo, &nome);
					adicionaCampo(&c, nome, 'N', 10);
				}
			}
		}
		
		if (comandos != NULL)
			comandos = comandos->prox;
	}
	tab->PCampos = c;
	alocaTabela(&(*bd), tab);
}

// Interpreta um comando ALTER TABLE ... ADD CONSTRAINT ... FOREIGN KEY
// e liga um campo de uma tabela (FK) ao campo correspondente (PK) de outra tabela.
//Exemplo de uso:
//ALTER TABLE aluguel ADD CONSTRAINT cliente_aluguel
// FOREIGN KEY (id_cliente) REFERENCES cliente (id_cliente);
void alterTable(BD *bd,CCs *comandos){
	String *nome, *campoOr, *campoFim;
	Tabelas *auxT;
	Campos *auxC1, *auxC2;
	while(comandos != NULL && (stricmp(comandos->campo->l, "ALTER") == 0 || stricmp(comandos->campo->l, "TABLE") == 0))
		comandos = comandos->prox;
	if(comandos!=NULL){
		limpaNome(comandos->campo,&campoFim);
		while(comandos != NULL && stricmp(comandos->campo->l, "KEY") != 0)
			comandos = comandos->prox;
		if(comandos !=NULL && comandos->prox !=NULL){
			limpaNome(comandos->prox->campo,&nome);
			while(comandos != NULL && stricmp(comandos->campo->l, "REFERENCES") != 0)
				comandos = comandos->prox;
			if(comandos !=NULL && comandos->prox !=NULL){
				limpaNome(comandos->prox->campo,&campoOr);
				if(existeCampoPK(nome,bd->PTabelas)){
					auxT = bd->PTabelas;
					while(auxT != NULL && strcmp(auxT->Tabela->l,campoFim->l)!=0)
						auxT= auxT->prox;
					if(auxT!=NULL){
						auxC1=auxT->PCampos;
						while(auxC1 != NULL && strcmp(nome->l, auxC1->Campo->l) != 0)
							auxC1=auxC1->prox;
						if(auxC1!=NULL){
							auxT = bd->PTabelas;
							while(auxT != NULL && strcmp(auxT->Tabela->l,campoOr->l)!=0)
								auxT= auxT->prox;
							if(auxT!=NULL){
								auxC2=auxT->PCampos;
								while(auxC2 != NULL && strcmp(nome->l, auxC2->Campo->l) != 0)
									auxC2=auxC2->prox;
								if(auxC2!=NULL){
									auxC1->FK = auxC2;
								}
							}
						}
					}
				}
				// "nome", "campoOr" e "campoFim" foram criados so para comparar (strcmp/existeCampoPK).
				// O FK guardado acima e um PONTEIRO para o Campos* que ja existe na tabela --
				// nao precisamos (nem devemos) manter essas copias de String depois disso.
				apagaString(&campoOr);
			}
			apagaString(&nome);
		}
		apagaString(&campoFim);
	}
}

//Localiza, dentro do banco, a tabela cujo nome bate com a String informada
Tabelas * procuraTabelaPorNome(BD *bd, String *nome) {
    Tabelas *t;
    t = bd->PTabelas;
    while (t != NULL && stricmp(t->Tabela->l, nome->l) != 0)
        t = t->prox;
    return t;
}

//Resolve um token ("campo" ou "tabela.campo") para o Campos* correspondente,
//procurando em TODAS as tabelas envolvidas no FROM (lista "tabelas").
//Devolve NULL quando o token nao e um campo conhecido (ou seja, e um valor literal).
Campos * resolveCampo(BD *bd, CCs *tabelas, char token[]) {
    char nomeTab[50];
    char nomeCampo[50];
    int pontoIndex, i, j;
    CCs *auxTabLista;
    Tabelas *t;
    Campos *c, *achou;

    pontoIndex = -1;
    for (i = 0; token[i] != '\0'; i++) {
        if (token[i] == '.') pontoIndex = i;
    }

    if (pontoIndex != -1) {
        for (i = 0; i < pontoIndex; i++) nomeTab[i] = token[i];
        nomeTab[pontoIndex] = '\0';
        for (i = pontoIndex + 1, j = 0; token[i] != '\0'; i++) nomeCampo[j++] = token[i];
        nomeCampo[j] = '\0';
    } else {
        nomeTab[0] = '\0';
        strcpy(nomeCampo, token);
    }

    achou = NULL;
    auxTabLista = tabelas;
    while (auxTabLista != NULL && achou == NULL) {
        if (nomeTab[0] == '\0' || stricmp(auxTabLista->campo->l, nomeTab) == 0) {
            t = procuraTabelaPorNome(bd, auxTabLista->campo);
            if (t != NULL) {
                c = t->PCampos;
                while (c != NULL && achou == NULL) {
                    if (stricmp(c->Campo->l, nomeCampo) == 0) {
                        achou = c;
                    }
                    c = c->prox;
                }
            }
        }
        auxTabLista = auxTabLista->prox;
    }
    return achou;
}

//Avalia a cadeia de condicoes ligadas por AND para a combinacao ATUAL de linhas
//(uma linha por tabela). Assume que os ponteiros Patual de TODOS os campos das
//tabelas envolvidas ja estao posicionados na linha certa dessa combinacao --
//quem garante isso e quem chama esta funcao (gerarCombinacoes).
int avaliaCondicaoAtual(BD *bd, CCs *tabelas, CCs *condicao) {
    CCs *auxCond;
    char op[20];
    char val1[100];
    char val2[100];
    char tokenEsq[100];
    char tokenDir1[100];
    char tokenDir2[100];
    Campos *campoEsq, *campoDir;
    union valores valor, valormin, valormax;
    int atendeTodas, atendeCondicao;
    int k, i, res;
    float diferenca, targetVal;
    float eps = 0.00001f;

    atendeTodas = 1;
    auxCond = condicao;

    while (auxCond != NULL && atendeTodas) {
        if (stricmp(auxCond->campo->l, "AND") == 0) {
            auxCond = auxCond->prox;
        }

        if (auxCond != NULL) {
            // Extrai Token Esquerda
            for (k = 0; auxCond->campo->l[k] != '\0' && auxCond->campo->l[k] != ';'; k++) {
                tokenEsq[k] = auxCond->campo->l[k];
            }
            tokenEsq[k] = '\0';
            auxCond = auxCond->prox;

            // Extrai Operador
            if (auxCond != NULL) {
                for (k = 0; auxCond->campo->l[k] != '\0' && auxCond->campo->l[k] != ';'; k++) {
                    op[k] = auxCond->campo->l[k];
                }
                op[k] = '\0';
                auxCond = auxCond->prox;
            } else {
                op[0] = '\0';
            }

            // Extrai Token Direita 1 (removendo aspas)
            tokenDir1[0] = '\0';
            if (auxCond != NULL) {
                for (k = 0, i = 0; auxCond->campo->l[k] != '\0' && auxCond->campo->l[k] != ';'; k++) {
                    if (auxCond->campo->l[k] != '\'') {
                        tokenDir1[i++] = auxCond->campo->l[k];
                    }
                }
                tokenDir1[i] = '\0';
                auxCond = auxCond->prox;
            }

            // Extrai Token Direita 2 (BETWEEN)
            tokenDir2[0] = '\0';
            if (stricmp(op, "BETWEEN") == 0) {
                if (auxCond != NULL && stricmp(auxCond->campo->l, "AND") == 0) {
                    auxCond = auxCond->prox;
                }
                if (auxCond != NULL) {
                    for (k = 0, i = 0; auxCond->campo->l[k] != '\0' && auxCond->campo->l[k] != ';'; k++) {
                        if (auxCond->campo->l[k] != '\'') {
                            tokenDir2[i++] = auxCond->campo->l[k];
                        }
                    }
                    tokenDir2[i] = '\0';
                    auxCond = auxCond->prox;
                }
            }

            // Resolve os dois lados: o campo "de verdade" pode estar tanto na
            // esquerda ("id_veiculo = 5") quanto na direita ("5 = id_veiculo"),
            // e tambem pode ser uma comparacao entre campos de duas tabelas
            // ("ordem_servico.id_veiculo = veiculo.id_veiculo").
            campoEsq = resolveCampo(bd, tabelas, tokenEsq);
            campoDir = resolveCampo(bd, tabelas, tokenDir1);

            atendeCondicao = 0;

            if (campoEsq == NULL && campoDir != NULL) {
                // O campo esta do lado direito -> troca de lado e inverte o
                // operador relacional para manter o mesmo sentido logico
                // (ex.: "5 > id_veiculo" equivale a "id_veiculo < 5").
                campoEsq = campoDir;
                strcpy(val1, tokenEsq);
                campoDir = NULL;
                if (strcmp(op, "<") == 0) strcpy(op, ">");
                else if (strcmp(op, ">") == 0) strcpy(op, "<");
                else if (strcmp(op, "<=") == 0) strcpy(op, ">=");
                else if (strcmp(op, ">=") == 0) strcpy(op, "<=");
            } else {
                strcpy(val1, tokenDir1);
            }
            strcpy(val2, tokenDir2);

            if (campoEsq != NULL && campoEsq->Patual != NULL) {
                valor = campoEsq->Patual->v;

                // Se o "valor alvo" tambem for um campo (comparacao entre duas
                // colunas), formata o valor ATUAL dele como string.
                if (campoDir != NULL && campoDir->Patual != NULL) {
                    if (campoDir->Tipo == 'I') {
                        itoa(campoDir->Patual->v.ValorI, val1, 10);
                    } else if (campoDir->Tipo == 'N') {
                        sprintf(val1, "%.2f", campoDir->Patual->v.ValorN);
                    } else if (campoDir->Tipo == 'C') {
                        val1[0] = campoDir->Patual->v.ValorC;
                        val1[1] = '\0';
                    } else if ((campoDir->Tipo == 'T' || campoDir->Tipo == 'D') && campoDir->Patual->v.ValorT != NULL) {
                        strcpy(val1, campoDir->Patual->v.ValorT->l);
                    }
                }

                // --- COMPARACAO DE INTEIROS ---
                if (campoEsq->Tipo == 'I') {
                    int vCampo = valor.ValorI;
                    int vTarget = atoi(val1);

                    if ((strcmp(op, "=") == 0 || strcmp(op, "==") == 0) && vCampo == vTarget) atendeCondicao = 1;
                    else if ((strcmp(op, "!=") == 0 || strcmp(op, "<>") == 0) && vCampo != vTarget) atendeCondicao = 1;
                    else if (strcmp(op, "<") == 0 && vCampo < vTarget) atendeCondicao = 1;
                    else if (strcmp(op, "<=") == 0 && vCampo <= vTarget) atendeCondicao = 1;
                    else if (strcmp(op, ">") == 0 && vCampo > vTarget) atendeCondicao = 1;
                    else if (strcmp(op, ">=") == 0 && vCampo >= vTarget) atendeCondicao = 1;

                    if (stricmp(op, "BETWEEN") == 0) {
                        valormin.ValorI = atoi(val1);
                        valormax.ValorI = atoi(val2);
                        if (vCampo >= valormin.ValorI && vCampo <= valormax.ValorI) atendeCondicao = 1;
                    }
                }
                // --- COMPARACAO DE FLOATS ---
                else if (campoEsq->Tipo == 'N') {
                    targetVal = (float)atof(val1);
                    diferenca = valor.ValorN - targetVal;

                    if (strcmp(op, "=") == 0 || strcmp(op, "==") == 0) {
                        if (diferenca >= -eps && diferenca <= eps) atendeCondicao = 1;
                    } else if (strcmp(op, "!=") == 0 || strcmp(op, "<>") == 0) {
                        if (diferenca < -eps || diferenca > eps) atendeCondicao = 1;
                    } else if (strcmp(op, "<") == 0 && valor.ValorN < targetVal) atendeCondicao = 1;
                    else if (strcmp(op, "<=") == 0 && valor.ValorN <= targetVal) atendeCondicao = 1;
                    else if (strcmp(op, ">") == 0 && valor.ValorN > targetVal) atendeCondicao = 1;
                    else if (strcmp(op, ">=") == 0 && valor.ValorN >= targetVal) atendeCondicao = 1;

                    if (stricmp(op, "BETWEEN") == 0) {
                        valormin.ValorN = (float)atof(val1);
                        valormax.ValorN = (float)atof(val2);
                        if (valor.ValorN >= valormin.ValorN && valor.ValorN <= valormax.ValorN) atendeCondicao = 1;
                    }
                }
                // --- COMPARACAO DE CHAR ---
                else if (campoEsq->Tipo == 'C') {
                    char cCampo = valor.ValorC;
                    char cTarget = val1[0];

                    if ((strcmp(op, "=") == 0 || strcmp(op, "==") == 0) && cCampo == cTarget) atendeCondicao = 1;
                    else if ((strcmp(op, "!=") == 0 || strcmp(op, "<>") == 0) && cCampo != cTarget) atendeCondicao = 1;
                    else if (strcmp(op, "<") == 0 && cCampo < cTarget) atendeCondicao = 1;
                    else if (strcmp(op, "<=") == 0 && cCampo <= cTarget) atendeCondicao = 1;
                    else if (strcmp(op, ">") == 0 && cCampo > cTarget) atendeCondicao = 1;
                    else if (strcmp(op, ">=") == 0 && cCampo >= cTarget) atendeCondicao = 1;

                    if (stricmp(op, "BETWEEN") == 0 && cCampo >= val1[0] && cCampo <= val2[0]) atendeCondicao = 1;
                }
                // --- COMPARACAO DE STRINGS / DATAS ---
                else if ((campoEsq->Tipo == 'T' || campoEsq->Tipo == 'D') && valor.ValorT != NULL && valor.ValorT->l != NULL) {
                    res = strcmp(valor.ValorT->l, val1);

                    if ((strcmp(op, "=") == 0 || strcmp(op, "==") == 0) && res == 0) atendeCondicao = 1;
                    else if ((strcmp(op, "!=") == 0 || strcmp(op, "<>") == 0) && res != 0) atendeCondicao = 1;
                    else if (strcmp(op, ">") == 0 && res > 0) atendeCondicao = 1;
                    else if (strcmp(op, ">=") == 0 && res >= 0) atendeCondicao = 1;
                    else if (strcmp(op, "<") == 0 && res < 0) atendeCondicao = 1;
                    else if (strcmp(op, "<=") == 0 && res <= 0) atendeCondicao = 1;

                    if (stricmp(op, "BETWEEN") == 0 && strcmp(valor.ValorT->l, val1) >= 0 && strcmp(valor.ValorT->l, val2) <= 0) {
                        atendeCondicao = 1;
                    }
                }

                if (!atendeCondicao) {
                    atendeTodas = 0;
                }
            } else {
                // Nenhum dos dois lados e um campo valido -> condicao falha
                atendeTodas = 0;
            }
        }
    }

    return atendeTodas;
}

//Gera, DE FORMA ITERATIVA (sem nenhuma chamada recursiva), todas as
//combinacoes possiveis de linhas entre as tabelas do FROM (produto
//cartesiano) e, para cada combinacao COMPLETA (uma linha escolhida de cada
//tabela), testa a condicao do WHERE.
//Em vez de uma funcao que chama a si mesma para cada nivel (tabela) do
//produto cartesiano, mantemos um vetor "iniciado[]" e um indice "nivel" que
//sobe e desce manualmente -- e a mesma ideia da recursao, so que simulada
//com uma pilha (os vetores) e um laco while, sem nenhum novo quadro de
//chamada de funcao sendo empilhado.
void gerarCombinacoes(BD *bd, CCs *tabelas, CCs *condicao,
                       CCs *listaTabs[], Tabelas *tabsReais[], int linhaAtual[],
                       int totalTabelas, Comandos **resultados) {
    Dados *dadoAtual[MAX_TABELAS_JOIN];
    int iniciado[MAX_TABELAS_JOIN];
    Campos *c;
    Comandos *combo, *ultimo;
    int k, nivel;

    for (k = 0; k < MAX_TABELAS_JOIN; k++) {
        iniciado[k] = 0;
    }
    nivel = 0;

    while (nivel >= 0) {
        if (nivel == totalTabelas) {
            // Combinacao completa (uma linha escolhida de cada tabela): testa o WHERE
            if (avaliaCondicaoAtual(bd, tabelas, condicao)) {
                combo = NULL;
                ultimo = NULL;
                for (k = 0; k < totalTabelas; k++) {
                    if (combo == NULL) {
                        combo = criaNoTabela(listaTabs[k]->campo, linhaAtual[k]);
                        ultimo = combo;
                    } else {
                        ultimo->proxTabela = criaNoTabela(listaTabs[k]->campo, linhaAtual[k]);
                        ultimo = ultimo->proxTabela;
                    }
                }
                adicionaResultado(resultados, combo);
            }
            // "Retorna" para o nivel anterior, para que ele avance sua propria linha
            nivel--;
        } else {
            if (!iniciado[nivel]) {
                // Primeira vez que entramos neste nivel: posiciona na primeira linha
                if (tabsReais[nivel] != NULL && tabsReais[nivel]->PCampos != NULL) {
                    // Sempre reinicia esta tabela do zero: este nivel e percorrido
                    // por completo uma vez para CADA combinacao escolhida nos niveis
                    // anteriores (as tabelas de fora do laco).
                    reiniciaPonteirosDados(tabsReais[nivel]->PCampos);
                    dadoAtual[nivel] = tabsReais[nivel]->PCampos->Pdados;
                    linhaAtual[nivel] = 1;
                } else {
                    dadoAtual[nivel] = NULL;
                }
                iniciado[nivel] = 1;
            } else {
                // Estamos voltando de um nivel mais interno: avanca esta linha
                if (dadoAtual[nivel] != NULL) {
                    c = tabsReais[nivel]->PCampos;
                    while (c != NULL) {
                        if (c->Patual != NULL) {
                            c->Patual = c->Patual->prox;
                        }
                        c = c->prox;
                    }
                    dadoAtual[nivel] = dadoAtual[nivel]->prox;
                    linhaAtual[nivel]++;
                }
            }

            if (dadoAtual[nivel] != NULL) {
                // Ainda ha linha valida neste nivel: desce para o proximo nivel
                nivel++;
            } else {
                // Nivel esgotado: reseta para uma futura reentrada e sobe um nivel
                iniciado[nivel] = 0;
                nivel--;
            }
        }
    }
}

//Ponto de entrada: recebe a lista de tabelas do FROM e a condicao do WHERE,
//e devolve a lista de combinacoes (uma linha por tabela) que satisfazem a
//condicao -- um produto cartesiano de verdade, e nao mais um simples
//pareamento por posicao entre as tabelas.
Comandos * posicaoCondicao(BD *bd, CCs *tabelas, CCs *condicao) {
    Comandos *resultados;
    CCs *listaTabs[MAX_TABELAS_JOIN];
    Tabelas *tabsReais[MAX_TABELAS_JOIN];
    int linhaAtual[MAX_TABELAS_JOIN];
    int totalTabelas, i;
    CCs *auxTabLista;

    iniciaResultados(&resultados);

    if (bd != NULL && tabelas != NULL && condicao != NULL) {
        totalTabelas = 0;
        auxTabLista = tabelas;
        while (auxTabLista != NULL && totalTabelas < MAX_TABELAS_JOIN) {
            listaTabs[totalTabelas] = auxTabLista;
            tabsReais[totalTabelas] = procuraTabelaPorNome(bd, auxTabLista->campo);
            totalTabelas++;
            auxTabLista = auxTabLista->prox;
        }

        gerarCombinacoes(bd, tabelas, condicao, listaTabs, tabsReais, linhaAtual, totalTabelas, &resultados);

        // Reinicia os ponteiros de dados de todas as tabelas ao finalizar,
        // para nao deixar nada "no meio" da lista para quem usar essas
        // tabelas depois.
        for (i = 0; i < totalTabelas; i++) {
            if (tabsReais[i] != NULL) {
                reiniciaPonteirosDados(tabsReais[i]->PCampos);
            }
        }
    }

    return resultados;
}

// Interpreta um comando INSERT INTO e adiciona uma nova linha de
// dados na tabela, validando a chave primaria e as chaves estrangeiras.
//Exemplo de uso:
//INSERT INTO tabela (coluna_1, coluna_2, coluna_3, ...)
//VALUES (valor_1, 'valor_2', valor_3, ...);
void insert(BD *bd, CCs *comandos) {
	Tabelas *tab;
	Campos *auxCamp;
	CCs *ordem = NULL, *auxOrdem;
	AuxInsert *valores = NULL, *auxVal;
	union valores val;
	String *tempNome = NULL;
	int encontrouColuna, fkValida;
	while (comandos != NULL && (stricmp(comandos->campo->l, "INSERT") == 0 || stricmp(comandos->campo->l, "INTO") == 0))
		comandos = comandos->prox;
	if (comandos != NULL && bd != NULL) {
		tab = bd->PTabelas;
		while (tab != NULL && strcmp(comandos->campo->l, tab->Tabela->l) != 0)
			tab = tab->prox;
		if (tab != NULL) {
			comandos = comandos->prox;
			while (comandos != NULL && stricmp(comandos->campo->l, "VALUES") != 0) {
				limpaNome(comandos->campo, &tempNome);
				if (tempNome->l[0] != '\0' && strcmp(tempNome->l, "(") != 0 && strcmp(tempNome->l, ")") != 0 && strcmp(tempNome->l, ",") != 0) {
					adicionaComandos(&ordem, tempNome);
				} else {
					// token descartado ( "(" , ")" ou "," ) -> a String alocada por
					// limpaNome nao vai para lugar nenhum, entao precisa ser liberada aqui
					apagaString(&tempNome);
				}
				comandos = comandos->prox;
			}
			if (comandos != NULL && stricmp(comandos->campo->l, "VALUES") == 0) {
				comandos = comandos->prox;
				auxOrdem = ordem;

				while (comandos != NULL) {
					limpaNome(comandos->campo, &tempNome);
					if (tempNome->l[0] != '\0' && strcmp(tempNome->l, "(") != 0 && strcmp(tempNome->l, ")") != 0 && strcmp(tempNome->l, ",") != 0) {
						if (auxOrdem != NULL) {
							auxCamp = tab->PCampos;
							while (auxCamp != NULL && strcmp(auxCamp->Campo->l, auxOrdem->campo->l) != 0)
								auxCamp = auxCamp->prox;
							if (auxCamp != NULL) {
								if (auxCamp->Tipo == 'I') {
									val.ValorI = atoi(tempNome->l);
									// tipos I/N/C nao guardam a String em lugar nenhum, entao
									// depois de extrair o valor ela precisa ser liberada
									apagaString(&tempNome);
								} else if (auxCamp->Tipo == 'N') {
									val.ValorN = (float)atof(tempNome->l);
									apagaString(&tempNome);
								} else if (auxCamp->Tipo == 'C') {
									val.ValorC = tempNome->l[0];
									apagaString(&tempNome);
								} else if (auxCamp->Tipo == 'T') {
									val.ValorT = tempNome; // posse transferida para "val" (e depois para a tabela)
								} else {
									val.ValorD = tempNome; // posse transferida para "val" (e depois para a tabela)
								}
								adicionaAuxiliar(&valores, val);
							} else {
								// coluna informada no INSERT nao existe na tabela -> tempNome nao e usado
								apagaString(&tempNome);
							}
							auxOrdem = auxOrdem->prox;
						} else {
							// mais valores do que colunas em "ordem" -> tempNome nao e usado
							apagaString(&tempNome);
						}
					} else {
						// token descartado ( "(" , ")" ou "," )
						apagaString(&tempNome);
					}
					comandos = comandos->prox;
				}
			}
			auxCamp = tab->PCampos;
			fkValida = 1;

			while (auxCamp != NULL && fkValida) {
				auxOrdem = ordem;
				auxVal = valores;
				encontrouColuna = 0;

				while (auxOrdem != NULL && !encontrouColuna) {
					if (strcmp(auxCamp->Campo->l, auxOrdem->campo->l) == 0) {
						encontrouColuna = 1;
						val = auxVal->V;
					} else {
						auxOrdem = auxOrdem->prox;
						if (auxVal != NULL) auxVal = auxVal->prox;
					}
				}

				if (!encontrouColuna) {
					if (auxCamp->Tipo == 'I') val.ValorI = 0;
					else if (auxCamp->Tipo == 'N') val.ValorN = 0.0f;
					else if (auxCamp->Tipo == 'C') val.ValorC = '\0';
					else val.ValorT = NULL;
				}
				if (auxCamp->PK == 'S') {
					if (!encontrouColuna || val.ValorT == NULL) {
						fkValida = 0;
					}
				}
				if (auxCamp->FK != NULL) {
					if (!encontrouColuna || val.ValorT == NULL) {
						fkValida = 0;
					} else if (!existeDadoFK(val, auxCamp->Tipo, auxCamp->FK->Pdados)) {
						fkValida = 0;
					}
				}

				auxCamp = auxCamp->prox;
			}
			if (fkValida) {
				auxCamp = tab->PCampos;

				while (auxCamp != NULL) {
					auxOrdem = ordem;
					auxVal = valores;
					encontrouColuna = 0;

					while (auxOrdem != NULL && !encontrouColuna) {
						if (strcmp(auxCamp->Campo->l, auxOrdem->campo->l) == 0) {
							encontrouColuna = 1;
							val = auxVal->V;
						} else {
							auxOrdem = auxOrdem->prox;
							if (auxVal != NULL) auxVal = auxVal->prox;
						}
					}

					if (!encontrouColuna) {
						if (auxCamp->Tipo == 'I') val.ValorI = 0;
						else if (auxCamp->Tipo == 'N') val.ValorN = 0.0f;
						else if (auxCamp->Tipo == 'C') val.ValorC = '\0';
						else val.ValorT = NULL;
					}
					adicionaDado(val, &(auxCamp->Pdados));

					auxCamp = auxCamp->prox;
				}
			} else {
				// A validacao de PK/FK falhou: nenhum valor foi transferido para a
				// tabela (adicionaDado nao rodou). Os campos do tipo 'T'/'D' guardados
				// em "valores" continuam com uma String alocada que ninguem mais vai
				// liberar -- por isso apagamos aqui, ANTES de apagaAuxiliar (que so
				// libera os nos da lista, nunca o conteudo da union).
				auxCamp = tab->PCampos;
				while (auxCamp != NULL) {
					auxOrdem = ordem;
					auxVal = valores;
					encontrouColuna = 0;
					while (auxOrdem != NULL && !encontrouColuna) {
						if (strcmp(auxCamp->Campo->l, auxOrdem->campo->l) == 0) {
							encontrouColuna = 1;
							if (auxCamp->Tipo == 'T' || auxCamp->Tipo == 'D') {
								apagaString(&auxVal->V.ValorT);
							}
						} else {
							auxOrdem = auxOrdem->prox;
							if (auxVal != NULL) auxVal = auxVal->prox;
						}
					}
					auxCamp = auxCamp->prox;
				}
			}
			apagaAuxiliar(&valores);
			// "ordem" guarda copias dos nomes das colunas (feitas por limpaNome) --
			// precisam ser liberadas sempre, o INSERT tenha dado certo ou nao.
			apagaComandos(&ordem);
		}
	}
}

// Descobre a largura (numero de caracteres) necessaria para imprimir
// uma coluna, comparando o tamanho do nome do cabecalho com o tamanho
// de todos os valores ja guardados nela.
int obterLarguraColuna(BD *bd, CCs *tabelas, String *nomeColuna) {
    int largura = nomeColuna->tam; // Largura minima inicial e o tamanho do nome do cabecalho
    CCs *cond = tabelas;
    Tabelas *auxTab;
    Campos *auxCamp;
    Dados *dadoAtual; // Ajuste para o nome do tipo do seu no de dados se for diferente
    char buffer[50];
    int tamDado, achou = 0;

    while (cond != NULL && !achou) {
        auxTab = bd->PTabelas;
        while (auxTab != NULL && strcmp(auxTab->Tabela->l, cond->campo->l) != 0) {
            auxTab = auxTab->prox;
        }

        if (auxTab != NULL) {
            auxCamp = auxTab->PCampos;
            while (auxCamp != NULL && !achou) {
                if (strcmp(auxCamp->Campo->l, nomeColuna->l) == 0) {
                    achou = 1; // Coluna encontrada
                    // Percorre todos os registros salvos nesta coluna
                    dadoAtual = auxCamp->Pdados; // <-- Subsitua 'PDados' pelo ponteiro inicial da sua lista de dados
                    while (dadoAtual != NULL) {
                        tamDado = 0;

                        if (auxCamp->Tipo == 'I') {
                            itoa(dadoAtual->v.ValorI, buffer, 10);
                            tamDado = strlen(buffer);
                        } else if (auxCamp->Tipo == 'N') {
                            sprintf(buffer, "%.2f", dadoAtual->v.ValorN);
                            tamDado = strlen(buffer);
                        } else if (auxCamp->Tipo == 'C') {
                            tamDado = 1;
                        } else if (auxCamp->Tipo == 'T' && dadoAtual->v.ValorT != NULL) {
                            tamDado = dadoAtual->v.ValorT->tam;
                        } else if (auxCamp->Tipo == 'D' && dadoAtual->v.ValorD != NULL) {
                            tamDado = dadoAtual->v.ValorD->tam;
                        }

                        if (tamDado > largura) {
                            largura = tamDado;
                        }

                        dadoAtual = dadoAtual->prox;
                    }
                } else {
                    auxCamp = auxCamp->prox;
                }
            }
        }
        cond = cond->prox;
    }

    return largura;
}

// Monta a linha de separacao ("+-----+-----+") usada acima e abaixo
// do cabecalho da tabela impressa.
CCs * montaLinhaDivisoria(BD *bd, CCs *tabelas, CCs *colunas) {
    char linha[1000];
    int i = 0, j, quant, largura;
    CCs *aux;
    String *temporario;
    CCs *saida;
    iniciaComandos(&saida);

    linha[i++] = '+';
    aux = colunas;

    while (aux != NULL && i < 990) {
        largura = obterLarguraColuna(bd, tabelas, aux->campo);
        quant = largura + 2; // +2 por conta dos espacos de margem

        for (j = 0; j < quant; j++, i++) {
            linha[i] = '-';
        }
        linha[i++] = '+';
        aux = aux->prox;
    }
    linha[i] = '\0';

    salvaString(&temporario, i, linha);
    adicionaComandos(&saida, temporario);

    return saida;
}

// Monta o cabecalho completo da tabela impressa: linha divisoria,
// nomes das colunas e outra linha divisoria.
CCs * montaCabecalho(BD *bd, CCs *tabelas, CCs *colunas){
	char linha[1000];
	int i = 0, j, quant, largura;
	CCs *aux, *saida, *linhaDivisoria;
	String *temporario;
	iniciaComandos(&saida);

	// 1. Linha superior (+-----+-----+)
    linhaDivisoria = montaLinhaDivisoria(bd, tabelas, colunas);
    adicionaComandos(&saida, linhaDivisoria->campo); // Junta na lista de saida
    // a String ja foi "adotada" por "saida"; so falta liberar o no CCs que a envolvia
    // (nao usar apagaComandos aqui, pois ele apagaria a String tambem, e ela ainda esta em uso)
    free(linhaDivisoria);


	// --- LINHA DO TEXTO DAS COLUNAS (| id_veiculo | marca |) ---
	i = 0;
	linha[i++] = '|';
	aux = colunas;
	while(aux != NULL && i < 990){
		linha[i++] = ' ';
		largura = obterLarguraColuna(bd, tabelas, aux->campo);
		
		// Copia o nome do campo
		for(j = 0; j < aux->campo->tam && j < largura; j++, i++){
			linha[i] = aux->campo->l[j];
		}
		// Preenche com espacos se o nome for menor que a largura da coluna
		while(j < largura){
			linha[i++] = ' ';
			j++;
		}

		linha[i++] = ' ';
		linha[i++] = '|';
		aux = aux->prox;
	}
	linha[i] = '\0';
	salvaString(&temporario, i, linha);
	adicionaComandos(&saida, temporario);

	// 3. Linha inferior (+-----+-----+)
    linhaDivisoria = montaLinhaDivisoria(bd, tabelas, colunas);
    adicionaComandos(&saida, linhaDivisoria->campo);
    free(linhaDivisoria);

	return saida;
}

// Interpreta um comando SELECT (com FROM, WHERE e juncao de tabelas)
// e imprime na tela as linhas que atendem a condicao pedida.
//Exemplos de uso:
//SELECT coluna_1, coluna_2, coluna_3, ...
//FROM tabela;
//SELECT *
//FROM tabela;
//SELECT coluna1, coluna2, ...
//FROM tabela
//WHERE coluna_2 BETWEEN valor_inicial AND valor_final;
//SELECT tabela_1.coluna_1, tabela_1.coluna_2, tabela_2.coluna_1, tabela_2.coluna_2, ...
//FROM tabela_1, tabela_2
//WHERE tabela_1.chave_primaria = tabela_2.chave_estrangeira;
void selectDados(BD *bd, CCs *comandos){
	CCs *colunas, *tabelas, *condicao, *exibir, *auxTabLista, *linhaDivisoria;
	Campos *auxCamp, *c;
	Tabelas *auxTab;
	AuxInsert *valores;
	String *nomeTabela, *nomeTemp;
	char linha[1000];
	Dados *dadoAtual, *dadoControle;
	int j, k;
	char numeros[32];
	int i, tamanho, larguraColuna, contadorAux;
	Comandos *linhasEncontradas, *auxCombo, *noTabela;
	char nomeTabTok[50], nomeCampoTok[50];
	int pontoIdx;
	iniciaResultados(&linhasEncontradas);
	iniciaComandos(&colunas);
	iniciaComandos(&tabelas);
	iniciaComandos(&condicao);
	iniciaAuxiliar(&valores);
	//colunas = campos
	while(comandos!=NULL && stricmp(comandos->campo->l, "SELECT") == 0)
		comandos = comandos->prox;
	if(comandos != NULL){
		while(comandos!=NULL && stricmp(comandos->campo->l, "FROM") != 0){
			limpaNome(comandos->campo,&nomeTemp);
			adicionaComandos(&colunas,nomeTemp);
			comandos= comandos->prox;
		}
		if(comandos!=NULL && stricmp(comandos->campo->l, "FROM") == 0){
			comandos = comandos->prox;
			while(comandos !=NULL&& stricmp(comandos->campo->l, "WHERE") != 0){
				limpaNome(comandos->campo,&nomeTemp);
				adicionaComandos(&tabelas,nomeTemp);
				comandos =comandos->prox;
			}
			if (comandos == NULL) {
			    if (stricmp(colunas->campo->l, "*") == 0) {
			        apagaComandos(&colunas);
			        iniciaComandos(&colunas);
			        auxTabLista = tabelas;
			        while (auxTabLista != NULL) {
			            auxTab = bd->PTabelas;
			            while (auxTab != NULL && stricmp(auxTab->Tabela->l, auxTabLista->campo->l) != 0) {
			                auxTab = auxTab->prox;
			            }
			            if (auxTab != NULL) {
			                auxCamp = auxTab->PCampos;
			                while (auxCamp != NULL) {
			                	limpaNome(auxCamp->Campo, &nomeTemp);
			                    adicionaComandos(&colunas, nomeTemp);
			                    auxCamp = auxCamp->prox;
			                }
			            }
			            auxTabLista = auxTabLista->prox;
			        }
			    }
			    
			    exibir = montaCabecalho(bd, tabelas, colunas);
			
			    // Pega a tabela principal para controlar o laco de linhas
			    auxTab = bd->PTabelas;
			    while (auxTab != NULL && stricmp(auxTab->Tabela->l, tabelas->campo->l) != 0)
			        auxTab = auxTab->prox;
			
			    if (auxTab != NULL && auxTab->PCampos != NULL) {
			        // Reinicia Patual para todos os campos da tabela antes de imprimir
			        reiniciaPonteirosDados(auxTab->PCampos);
			        
			        dadoControle = auxTab->PCampos->Pdados;
			
			        while (dadoControle != NULL) {
			            i = 0;
			            linha[i++] = '|';
			
			            auxTabLista = colunas; // Percorre as colunas selecionadas no SELECT
			            while (auxTabLista != NULL) {
			                linha[i++] = ' ';
			
			                // Procura de qual tabela do FROM veio este campo. O token do
			                // SELECT pode vir qualificado ("tabela.campo") ou nao ("campo") --
			                // por isso separamos o nome da tabela (se houver) do nome do
			                // campo antes de comparar, em vez de comparar a string inteira
			                // (que poderia vir com o prefixo da tabela) contra o nome puro do campo.
			                pontoIdx = -1;
			                for (k = 0; auxTabLista->campo->l[k] != '\0'; k++) {
			                    if (auxTabLista->campo->l[k] == '.') {
			                        pontoIdx = k;
			                    }
			                }
			                if (pontoIdx != -1) {
			                    for (k = 0; k < pontoIdx; k++) {
			                        nomeTabTok[k] = auxTabLista->campo->l[k];
			                    }
			                    nomeTabTok[pontoIdx] = '\0';
			                    strcpy(nomeCampoTok, auxTabLista->campo->l + pontoIdx + 1);
			                } else {
			                    nomeTabTok[0] = '\0';
			                    strcpy(nomeCampoTok, auxTabLista->campo->l);
			                }
			
			                auxCamp = NULL;
			                condicao = tabelas;
			                while (condicao != NULL && auxCamp == NULL) {
			                    if (nomeTabTok[0] == '\0' || stricmp(condicao->campo->l, nomeTabTok) == 0) {
			                        auxTab = bd->PTabelas;
			                        while (auxTab != NULL && stricmp(auxTab->Tabela->l, condicao->campo->l) != 0)
			                            auxTab = auxTab->prox;
			
			                        if (auxTab != NULL) {
			                            auxCamp = auxTab->PCampos;
			                            while (auxCamp != NULL && stricmp(auxCamp->Campo->l, nomeCampoTok) != 0)
			                                auxCamp = auxCamp->prox;
			                        }
			                    }
			                    condicao = condicao->prox;
			                }
			
			                // Se achou o campo, pega o valor atual
			                if (auxCamp != NULL && auxCamp->Patual != NULL) {
			                    dadoAtual = auxCamp->Patual;
			
			                    if (auxCamp->Tipo == 'I') {
			                        itoa(dadoAtual->v.ValorI, numeros, 10);
			                        tamanho = 0;
			                        while (numeros[tamanho] != '\0') tamanho++;
			                        salvaString(&nomeTabela, tamanho, numeros);
			                    } else if (auxCamp->Tipo == 'N') {
			                        sprintf(numeros, "%.2f", dadoAtual->v.ValorN);
			                        tamanho = 0;
			                        while (numeros[tamanho] != '\0') tamanho++;
			                        salvaString(&nomeTabela, tamanho, numeros);
			                    } else if (auxCamp->Tipo == 'C') {
			                        numeros[0] = dadoAtual->v.ValorC;
			                        numeros[1] = '\0';
			                        salvaString(&nomeTabela, 1, numeros);
			                    } else if (auxCamp->Tipo == 'T' && dadoAtual->v.ValorT != NULL) {
			                        salvaString(&nomeTabela, dadoAtual->v.ValorT->tam, dadoAtual->v.ValorT->l);
			                    } else if (auxCamp->Tipo == 'D' && dadoAtual->v.ValorD != NULL) {
			                        salvaString(&nomeTabela, dadoAtual->v.ValorD->tam, dadoAtual->v.ValorD->l);
			                    } else {
			                        numeros[0] = '\0';
			                        salvaString(&nomeTabela, 0, numeros);
			                    }
			
			                    larguraColuna = obterLarguraColuna(bd, tabelas, auxTabLista->campo);
			
			                    j = 0;
			                    if (nomeTabela != NULL && nomeTabela->l != NULL) {
			                        while (nomeTabela->l[j] != '\0' && j < larguraColuna) {
			                            linha[i++] = nomeTabela->l[j++];
			                        }
			                    }
			
			                    while (j < larguraColuna) {
			                        linha[i++] = ' ';
			                        j++;
			                    }
			                    apagaString(&nomeTabela);
			
			                    // Avanca o ponteiro de dados do campo
			                    auxCamp->Patual = auxCamp->Patual->prox;
			                } else {
			                    // Caso o campo nao seja encontrado nesta tabela
			                    larguraColuna = obterLarguraColuna(bd, tabelas, auxTabLista->campo);
			                    for (j = 0; j < larguraColuna; j++) {
			                        linha[i++] = ' ';
			                    }
			                }
			
			                linha[i++] = ' ';
			                linha[i++] = '|';
			                auxTabLista = auxTabLista->prox;
			            }
			
			            linha[i] = '\0';
			            salvaString(&nomeTemp, i, linha);
			            adicionaComandos(&exibir, nomeTemp);
			
			            dadoControle = dadoControle->prox;
			        }
			
			        linhaDivisoria = montaLinhaDivisoria(bd, tabelas, colunas);
			        adicionaComandos(&exibir, linhaDivisoria->campo);
			        free(linhaDivisoria);
			        exibeComandos(exibir);
			        apagaComandos(&exibir);
			    }
			    apagaComandos(&tabelas);
			}else{
				if(stricmp(comandos->campo->l,"WHERE")==0){
					comandos = comandos->prox;
				}
				
				
				if (comandos != NULL) {
				    condicao = comandos;
				    linhasEncontradas = posicaoCondicao(bd, tabelas, condicao);
				
				    // Expandir '*' caso tenha sido selecionado
				    if (stricmp(colunas->campo->l, "*") == 0) {
				        apagaComandos(&colunas);
				        iniciaComandos(&colunas);
				        auxTabLista = tabelas;
				        while (auxTabLista != NULL) {
				            auxTab = bd->PTabelas;
				            while (auxTab != NULL && strcmp(auxTab->Tabela->l, auxTabLista->campo->l) != 0) {
				                auxTab = auxTab->prox;
				            }
				            if (auxTab != NULL) {
				                auxCamp = auxTab->PCampos;
				                while (auxCamp != NULL) {
				                    limpaNome(auxCamp->Campo, &nomeTemp);
			                    	adicionaComandos(&colunas, nomeTemp);
				                    auxCamp = auxCamp->prox;
				                }
				            }
				            auxTabLista = auxTabLista->prox;
				        }
				    }
				
				    exibir = montaCabecalho(bd, tabelas, colunas);
				
				    // Verifica se ha combinacoes validas
	    if (linhasEncontradas != NULL) {
	        auxCombo = linhasEncontradas;

	        // Varre a lista de combinacoes (uma linha por tabela) encontradas
	        while (auxCombo != NULL) {

	            // MONTAGEM DA LINHA VISUAL (impressao das colunas do SELECT):
	            i = 0;
	            linha[i++] = '|';

	            auxTabLista = colunas;
	            while (auxTabLista != NULL) {
	                linha[i++] = ' ';

	                // Descobre de qual tabela do FROM veio esta coluna. O token do SELECT
	                // pode vir qualificado ("tabela.campo") ou nao ("campo") -- por isso
	                // separamos o nome da tabela (se houver) do nome do campo antes de
	                // comparar, em vez de comparar a string inteira contra o campo puro.
	                pontoIdx = -1;
	                for (k = 0; auxTabLista->campo->l[k] != '\0'; k++) {
	                    if (auxTabLista->campo->l[k] == '.') {
	                        pontoIdx = k;
	                    }
	                }
	                if (pontoIdx != -1) {
	                    for (k = 0; k < pontoIdx; k++) {
	                        nomeTabTok[k] = auxTabLista->campo->l[k];
	                    }
	                    nomeTabTok[pontoIdx] = '\0';
	                    strcpy(nomeCampoTok, auxTabLista->campo->l + pontoIdx + 1);
	                } else {
	                    nomeTabTok[0] = '\0';
	                    strcpy(nomeCampoTok, auxTabLista->campo->l);
	                }

	                auxCamp = NULL;
	                condicao = tabelas;
	                while (condicao != NULL && auxCamp == NULL) {
	                    if (nomeTabTok[0] == '\0' || stricmp(condicao->campo->l, nomeTabTok) == 0) {
	                        auxTab = bd->PTabelas;
	                        while (auxTab != NULL && stricmp(auxTab->Tabela->l, condicao->campo->l) != 0)
	                            auxTab = auxTab->prox;

	                        if (auxTab != NULL) {
	                            c = auxTab->PCampos;
	                            while (c != NULL && stricmp(c->Campo->l, nomeCampoTok) != 0)
	                                c = c->prox;
	                            if (c != NULL) {
	                                auxCamp = c;
	                                // Dentro desta combinacao especifica, procura a linha
	                                // que corresponde a tabela dona deste campo
	                                noTabela = procuraTabelaCombo(auxCombo, condicao->campo);
	                            }
	                        }
	                    }
	                    condicao = condicao->prox;
	                }

	                // Posiciona um Dados* direto na linha certa (sem mexer no Patual
	                // global do campo, ja que cada combinacao pode usar uma linha
	                // diferente para a MESMA tabela)
	                dadoAtual = NULL;
	                if (auxCamp != NULL && noTabela != NULL) {
	                    dadoAtual = auxCamp->Pdados;
	                    contadorAux = 1;
	                    while (dadoAtual != NULL && contadorAux < noTabela->linha) {
	                        dadoAtual = dadoAtual->prox;
	                        contadorAux++;
	                    }
	                }

	                if (auxCamp != NULL && dadoAtual != NULL) {
	                    if (auxCamp->Tipo == 'I') {
	                        itoa(dadoAtual->v.ValorI, numeros, 10);
	                        tamanho = 0;
	                        while (numeros[tamanho] != '\0') tamanho++;
	                        salvaString(&nomeTabela, tamanho, numeros);
	                    } else if (auxCamp->Tipo == 'N') {
	                        sprintf(numeros, "%.2f", dadoAtual->v.ValorN);
	                        tamanho = 0;
	                        while (numeros[tamanho] != '\0') tamanho++;
	                        salvaString(&nomeTabela, tamanho, numeros);
	                    } else if (auxCamp->Tipo == 'C') {
	                        numeros[0] = dadoAtual->v.ValorC;
	                        numeros[1] = '\0';
	                        salvaString(&nomeTabela, 1, numeros);
	                    } else if (auxCamp->Tipo == 'T' && dadoAtual->v.ValorT != NULL) {
	                        salvaString(&nomeTabela, dadoAtual->v.ValorT->tam, dadoAtual->v.ValorT->l);
	                    } else if (auxCamp->Tipo == 'D' && dadoAtual->v.ValorD != NULL) {
	                        salvaString(&nomeTabela, dadoAtual->v.ValorD->tam, dadoAtual->v.ValorD->l);
	                    } else {
	                        numeros[0] = '\0';
	                        salvaString(&nomeTabela, 0, numeros);
	                    }

	                    larguraColuna = obterLarguraColuna(bd, tabelas, auxTabLista->campo);

	                    j = 0;
	                    if (nomeTabela != NULL && nomeTabela->l != NULL) {
	                        while (nomeTabela->l[j] != '\0' && j < larguraColuna) {
	                            linha[i++] = nomeTabela->l[j++];
	                        }
	                    }

	                    while (j < larguraColuna) {
	                        linha[i++] = ' ';
	                        j++;
	                    }
	                    apagaString(&nomeTabela);
	                } else {
	                    // Coluna nao encontrada nesta combinacao
	                    larguraColuna = obterLarguraColuna(bd, tabelas, auxTabLista->campo);
	                    for (j = 0; j < larguraColuna; j++) {
	                        linha[i++] = ' ';
	                    }
	                }

	                linha[i++] = ' ';
	                linha[i++] = '|';
	                auxTabLista = auxTabLista->prox;
	            }

	            linha[i] = '\0';
	            salvaString(&nomeTemp, i, linha);
	            adicionaComandos(&exibir, nomeTemp);

	            // Avanca para a proxima combinacao encontrada
	            auxCombo = auxCombo->prox;
	        }

	        linhaDivisoria = montaLinhaDivisoria(bd, tabelas, colunas);
	        adicionaComandos(&exibir, linhaDivisoria->campo);
	        free(linhaDivisoria);
	        exibeComandos(exibir);
	        apagaComandos(&exibir);
	    }
	    apagaResultados(&linhasEncontradas);
				}
				apagaComandos(&tabelas);
			}
			apagaComandos(&colunas);
		}
		
	}
}

// Interpreta um comando DELETE FROM e remove, de tras para frente,
// todas as linhas que atendem a condicao do WHERE.
void Delete(BD **bd, CCs *comandos) {
    CCs *auxCmd, *tabelas = NULL, *condicao = NULL;
    Tabelas *tab;
    Campos *campoAux;
    Dados *ant, *temp;
    Comandos *linhasEncontradas = NULL, *auxCombo = NULL, *noTabela;
    String *nomeTabCopia;
    int i, posRemover;

    if (bd != NULL && *bd != NULL && comandos != NULL) {
        auxCmd = comandos;
        if (auxCmd != NULL && stricmp(auxCmd->campo->l, "DELETE") == 0) 
            auxCmd = auxCmd->prox;
        if (auxCmd != NULL && stricmp(auxCmd->campo->l, "FROM") == 0) 
            auxCmd = auxCmd->prox;

        if (auxCmd != NULL) {
            tab = (*bd)->PTabelas;
            while (tab != NULL && stricmp(tab->Tabela->l, auxCmd->campo->l) != 0) {
                tab = tab->prox;
            }

            if (tab != NULL) {
                iniciaComandos(&tabelas);
                // IMPORTANTE: nunca guardamos tab->Tabela diretamente aqui. Se
                // fizessemos isso, um apagaComandos(&tabelas) mais tarde apagaria
                // por tabela o proprio nome da tabela dentro do banco de dados!
                // Por isso sempre fazemos uma COPIA independente.
                inicString(&nomeTabCopia);
                salvaString(&nomeTabCopia, tab->Tabela->tam, tab->Tabela->l);
                adicionaComandos(&tabelas, nomeTabCopia);

                while (auxCmd != NULL && stricmp(auxCmd->campo->l, "WHERE") != 0) {
                    auxCmd = auxCmd->prox;
                }

                if (auxCmd != NULL && auxCmd->prox != NULL) {
                    condicao = auxCmd->prox;

                    // 1. Obtem todas as combinacoes de linhas que satisfazem o WHERE
                    // (para DELETE ha so uma tabela em "tabelas", entao cada
                    // combinacao carrega uma unica linha desta tabela)
                    linhasEncontradas = posicaoCondicao(*bd, tabelas, condicao);

                    // 2. INVERTE A LISTA para apagar do FIM para o INICIO
                    linhasEncontradas = inverterResultados(linhasEncontradas);

                    auxCombo = linhasEncontradas;
                    while (auxCombo != NULL) {
                        noTabela = procuraTabelaCombo(auxCombo, tab->Tabela);

                        if (noTabela != NULL) {
                            posRemover = noTabela->linha;

                            // Percorre todas as colunas/campos da tabela para remover a linha
                            campoAux = tab->PCampos;
                            while (campoAux != NULL) {
                                campoAux->Patual = campoAux->Pdados;
                                ant = NULL;
                                i = 1;

                                while (campoAux->Patual != NULL && i < posRemover) {
                                    ant = campoAux->Patual;
                                    campoAux->Patual = campoAux->Patual->prox;
                                    i++;
                                }

                                if (campoAux->Patual != NULL) {
                                    temp = campoAux->Patual;

                                    // Remocao do no da lista de dados
                                    if (ant == NULL) {
                                        campoAux->Pdados = temp->prox;
                                    } else {
                                        ant->prox = temp->prox;
                                    }

                                    // Libera a memoria das strings alocadas dentro da union
                                    if (campoAux->Tipo == 'T') {
                                        apagaString(&temp->v.ValorT);
                                    } else if (campoAux->Tipo == 'D') {
                                        apagaString(&temp->v.ValorD);
                                    }

                                    free(temp);
                                }

                                reiniciaPonteirosDados(campoAux);
                                campoAux = campoAux->prox;
                            }
                        }

                        auxCombo = auxCombo->prox;
                    }
                    apagaResultados(&linhasEncontradas);
                }
                // "tabelas" guarda uma COPIA (ver acima) -> seguro apagar por completo
                apagaComandos(&tabelas);
            }
        }
    }
}

// Interpreta um comando UPDATE e altera os valores das colunas do
// SET para todas as linhas que atendem a condicao do WHERE.
void Update(BD **bd, CCs *comandos) {
    CCs *auxCmd, *inicioSet, *condicaoWhere, *tabelas = NULL;
    Tabelas *tab;
    Campos *campoSet;
    Dados *noDado;
    Comandos *linhasEncontradas = NULL, *auxCombo = NULL, *noTabela;
    String *tempNome = NULL, *nomeTabCopia;
    char nomeCampoSet[50], novoValorStr[100];
    int i, posLinha;

    if (bd != NULL && *bd != NULL && comandos != NULL) {
        auxCmd = comandos;
        
        // Pula o token "UPDATE"
        if (auxCmd != NULL && stricmp(auxCmd->campo->l, "UPDATE") == 0) 
            auxCmd = auxCmd->prox;

        if (auxCmd != NULL) {
            // 1. Busca a tabela no banco de dados
            tab = (*bd)->PTabelas;
            while (tab != NULL && stricmp(tab->Tabela->l, auxCmd->campo->l) != 0) {
                tab = tab->prox;
            }

            if (tab != NULL) {
                // Avanca ate a clausula SET
                while (auxCmd != NULL && stricmp(auxCmd->campo->l, "SET") != 0) {
                    auxCmd = auxCmd->prox;
                }

                if (auxCmd != NULL && auxCmd->prox != NULL) {
                    inicioSet = auxCmd->prox;

                    // Avanca ate a clausula WHERE
                    while (auxCmd != NULL && stricmp(auxCmd->campo->l, "WHERE") != 0) {
                        auxCmd = auxCmd->prox;
                    }

                    if (auxCmd != NULL && auxCmd->prox != NULL) {
                        condicaoWhere = auxCmd->prox;

                        iniciaComandos(&tabelas);
                        // IMPORTANTE: nunca guardamos tab->Tabela diretamente aqui. Se
                        // fizessemos isso, um apagaComandos(&tabelas) mais tarde apagaria
                        // por tabela o proprio nome da tabela dentro do banco de dados!
                        // Por isso sempre fazemos uma COPIA independente (mesmo cuidado
                        // tomado em Delete).
                        inicString(&nomeTabCopia);
                        salvaString(&nomeTabCopia, tab->Tabela->tam, tab->Tabela->l);
                        adicionaComandos(&tabelas, nomeTabCopia);

                        // 2. Avalia quais combinacoes (linhas) satisfazem a condicao do WHERE
                        // (para UPDATE ha so uma tabela em "tabelas", entao cada
                        // combinacao carrega uma unica linha desta tabela)
                        linhasEncontradas = posicaoCondicao(*bd, tabelas, condicaoWhere);

                        auxCombo = linhasEncontradas;
                        while (auxCombo != NULL) {
                            noTabela = procuraTabelaCombo(auxCombo, tab->Tabela);

                            if (noTabela != NULL) {
                                posLinha = noTabela->linha;

                                auxCmd = inicioSet;

                                // 3. Processa cada atribuicao dentro de SET
                                while (auxCmd != NULL && stricmp(auxCmd->campo->l, "WHERE") != 0) {

                                    // Captura o nome do campo a ser atualizado
                                    limpaNome(auxCmd->campo, &tempNome);
                                    if (tempNome != NULL && tempNome->l != NULL) {
                                        strcpy(nomeCampoSet, tempNome->l);
                                    } else {
                                        strcpy(nomeCampoSet, auxCmd->campo->l);
                                    }
                                    // tempNome e sempre uma copia alocada por limpaNome (mesmo
                                    // quando so serve para preencher nomeCampoSet) -> tem que
                                    // ser liberada aqui, e nao so no ramo "if" como estava antes
                                    apagaString(&tempNome);

                                    // Avanca do Campo -> '='
                                    auxCmd = auxCmd->prox;
                                    if (auxCmd != NULL && (strcmp(auxCmd->campo->l, "=") == 0 || strcmp(auxCmd->campo->l, "==") == 0)) {
                                        // Avanca do '=' -> Valor
                                        auxCmd = auxCmd->prox; 
                                    }

                                    // Se capturou um valor valido antes de chegar no WHERE
                                    if (auxCmd != NULL && stricmp(auxCmd->campo->l, "WHERE") != 0) {
                                        strcpy(novoValorStr, auxCmd->campo->l);

                                        // Localiza a estrutura da coluna na tabela
                                        campoSet = tab->PCampos;
                                        while (campoSet != NULL && stricmp(campoSet->Campo->l, nomeCampoSet) != 0) {
                                            campoSet = campoSet->prox;
                                        }

                                        // Atualiza a celula de dados referente a linha indicada por posLinha
                                        if (campoSet != NULL) {
                                            noDado = campoSet->Pdados;
                                            i = 1;
                                            while (noDado != NULL && i < posLinha) {
                                                noDado = noDado->prox;
                                                i++;
                                            }

                                            if (noDado != NULL) {
                                                if (campoSet->Tipo == 'I') {
                                                    noDado->v.ValorI = atoi(novoValorStr);
                                                } else if (campoSet->Tipo == 'N') {
                                                    noDado->v.ValorN = (float)atof(novoValorStr);
                                                } else if (campoSet->Tipo == 'C') {
                                                    noDado->v.ValorC = novoValorStr[0];
                                                } else if (campoSet->Tipo == 'T' || campoSet->Tipo == 'D') {
                                                    // Se ja havia uma String alocada aqui (valor antigo
                                                    // da celula), salvaString sobrescreveria o ponteiro
                                                    // sem liberar o que havia antes -> vazamento. Por
                                                    // isso apagamos o valor antigo primeiro.
                                                    if (noDado->v.ValorT != NULL) {
                                                        apagaString(&(noDado->v.ValorT));
                                                    }
                                                    salvaString(&(noDado->v.ValorT), strlen(novoValorStr), novoValorStr);
                                                }
                                            }
                                        }
                                    }

                                    // Avanca para o proximo par (ou virgula/WHERE)
                                    if (auxCmd != NULL) {
                                        auxCmd = auxCmd->prox;
                                    }
                                }
                            }

                            auxCombo = auxCombo->prox;
                        }
                        apagaResultados(&linhasEncontradas);
                    }
                }
                // "tabelas" guarda uma COPIA (ver acima) -> seguro apagar por completo
                apagaComandos(&tabelas);
            }
        }
    }
}


// --- Leitura de comandos ---

// Le um arquivo texto com varios comandos SQL (separados por ';') e
// executa cada um deles, um apos o outro, montando o banco de dados.
void leituraArquivoPricipal(BD**bd, char caminho[]){
	FILE *arquivo = fopen(caminho, "r");
	char comando[1000];
	CCs *comandos;
	int i,c;
	iniciaComandos(&comandos);
	if(arquivo!=NULL){
		i=0;
		while((c = fgetc(arquivo))!=EOF){
			comando[i++]=c;
			if(c==';'){
				comando[i]='\0';
				comandos = separaComandos(comando);
				i=0;
				if(comandos !=NULL){
					if(comandos->campo !=NULL && comandos->campo->l!=NULL){
						if(stricmp(comandos->campo->l,"CREATE")==0){
							if(comandos->prox!=NULL && comandos->prox->campo!=NULL){
								if(stricmp(comandos->prox->campo->l,"DATABASE")==0){
									CreateDataBase(&*bd,comandos);
								}else if(stricmp(comandos->prox->campo->l,"TABLE")==0){
									CreateTable(&*bd,comandos);
								}
							}
						}else if(stricmp(comandos->campo->l,"ALTER")==0){
							alterTable(*bd,comandos);
						}else if(stricmp(comandos->campo->l,"INSERT")==0){
							insert(*bd,comandos);
						}else if(stricmp(comandos->campo->l,"DELETE")==0){
							Delete(&*bd,comandos);
						}else if(stricmp(comandos->campo->l,"UPDATE")==0){
							Update(&*bd,comandos);
						}else if(stricmp(comandos->campo->l,"SELECT")==0){
						    selectDados(*bd, comandos);
						}
					}
					apagaComandos(&comandos);
				}
				
			}
		}
		fclose(arquivo);
	}else{
		printf("Arquivo nao encontrado");
	}
}

// Le comandos SQL digitados pelo usuario, um de cada vez, e executa
// cada um deles ate que uma linha em branco seja digitada.
void leituraEscrita(BD**bd){
	char comando[1000];
	CCs *comandos;
	iniciaComandos(&comandos);
	printf("\ndigite o comando SQL que deseja executar: \n");
	fgets(comando,sizeof(comando),stdin);
	while(comando[0]!='\n'){
		comandos = separaComandos(comando);
		if(comandos !=NULL){
			if(comandos->campo !=NULL && comandos->campo->l!=NULL){
				if(stricmp(comandos->campo->l,"CREATE")==0){
					if(comandos->prox!=NULL && comandos->prox->campo!=NULL){
						if(stricmp(comandos->prox->campo->l,"DATABASE")==0){
							CreateDataBase(&*bd,comandos);
						}else if(stricmp(comandos->prox->campo->l,"TABLE")==0){
							CreateTable(&*bd,comandos);
						}
					}
				}else if(stricmp(comandos->campo->l,"ALTER")==0){
					alterTable(*bd,comandos);
				}else if(stricmp(comandos->campo->l,"INSERT")==0){
					insert(*bd,comandos);
				}else if(stricmp(comandos->campo->l,"DELETE")==0){
					Delete(&*bd,comandos);
				}else if(stricmp(comandos->campo->l,"UPDATE")==0){
					Update(&*bd,comandos);
				}else if(stricmp(comandos->campo->l,"SELECT")==0){
					selectDados(*bd,comandos);
				}else{
					printf("\n Comando nao encontrado! \n");
				}
			}
			apagaComandos(&comandos);
		}
		printf("\ndigite o comando SQL que deseja executar: \n");
		fgets(comando,sizeof(comando),stdin);
	}	
}
