/* grafos.c - CMP2119 Algoritmos em Grafos
 *
 * Compilar: gcc -O2 -o grafos grafos.c && ./grafos 
 *
 * Formato de entrada.txt: sequencia de comandos, cada um seguido dos seus dados.
 *
 *   1 n <matriz n x n>        matriz de adjacencia, nao dirigido
 *   2 n m <matriz n x m>      matriz de incidencia, nao dirigido (1 = incide)
 *   3 n m <matriz n x m>      matriz de incidencia, dirigido (+1 origem, -1 destino)
 *   4 n <vizinhos de v1> 0 ... <vizinhos de vn> 0      lista de adjacencia
 *   5                         mostra lista, matriz e graus
 *   6                         classifica: ciclo, completo, roda, euleriano, bipartido, conexo, cliques
 *   7 n m                     desafio: constroi um grafo euleriano com n vertices e m arestas
 *   0                         encerra
 *
 * Conexo e bipartido sao verificados por multiplicacao de matrizes (Grafos_11):
 * A^r[i][j] conta os caminhos de comprimento r entre i e j.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAXN 1000
#define MAXM 1000

static FILE *entrada;

static int n;                       // numero de vertices 
static int dirigido;                // 1 se o grafo e dirigido
static int adj[MAXN][MAXN];         // adj[i][j] = numero de arestas i->j (i-j se nao dirigido)
static int linha = 1;                // numero da linha atual do arquivo de entrada

// ==================== leitura ====================

// Descarta espacos, quebras de linha, comentarios (# ate o fim da linha) e o BOM
// que alguns editores gravam no inicio do arquivo. Sem isso o fscanf("%d") pararia
// no primeiro '#' e o programa terminaria sem ler nada. Tambem conta as linhas
// para as mensagens de erro apontarem onde esta o problema.
static void pularComentarios() {
    int c;
 
    while ((c = fgetc(entrada)) != EOF) {
        if (c == '#') {
            while ((c = fgetc(entrada)) != EOF && c != '\n') {
            }
        }
 
        if (c == '\n') {
            linha++;
        } else if (!isspace(c) && c != 0xEF && c != 0xBB && c != 0xBF) {
            ungetc(c, entrada);
            return;
        }
    }
}

// Unica porta de entrada de dados: le um inteiro e valida o intervalo.
// Centralizar aqui garante que toda leitura passa pelo mesmo tratamento de
// comentarios e de erro. Fim de arquivo e tratado como "fim dos comandos".
static int lerInteiro(int minimo, int maximo) {
    int valor;
 
    pularComentarios();             
 
    if (fscanf(entrada, "%d", &valor) != 1) {
        if (feof(entrada)) {
            exit(0);                
        }
 
        printf("Erro na linha %d: esperado um numero, encontrado o caractere %d\n", linha, fgetc(entrada));
        exit(1);
    }
 
    if (valor < minimo || valor > maximo) {
        printf("Entrada invalida: %d (esperado entre %d e %d)\n", valor, minimo, maximo);
        exit(1);
    }
 
    return valor;
}


// Prepara o grafo para uma nova leitura: define n, se e dirigido e zera a matriz.
// Toda entrada comeca por aqui para nao sobrar aresta do grafo anterior.
static void limparGrafo(int vertices, int ehDirigido) {
    n = vertices;
    dirigido = ehDirigido;

    memset(adj, 0, sizeof adj);
}


// Insere uma aresta respeitando o tipo do grafo: no nao dirigido a matriz e
// simetrica, entao a aresta entra nos dois sentidos. Evita repetir esse cuidado
// em cada funcao de leitura e no construtor de grafo euleriano.
static void adicionarAresta(int a, int b) {
    adj[a][b]++;

    if (!dirigido) {
        adj[b][a]++;
    }
}


// Comando 1: le a matriz de adjacencia de um grafo nao dirigido.
// A matriz ja e a representacao interna, entao so valida a simetria
// (grafo nao dirigido exige adj[i][j] == adj[j][i]).
static void lerMatrizAdjacencia() {
    limparGrafo(lerInteiro(1, MAXN), 0);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            adj[i][j] = lerInteiro(0, 1000);
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (adj[i][j] != adj[j][i]) {
                printf("Matriz nao simetrica em (%d,%d)\n", i + 1, j + 1);
                exit(1);
            }
        }
    }
}


// Comandos 2 e 3: le a matriz de incidencia (linhas = vertices, colunas = arestas)
// e converte para a matriz de adjacencia. Cada coluna precisa ter exatamente dois
// extremos; no dirigido +1 marca a origem e -1 o destino.
static void lerMatrizIncidencia(int ehDirigido) {
    static int inc[MAXN][MAXM];

    int vertices = lerInteiro(1, MAXN);
    int m = lerInteiro(0, MAXM);

    limparGrafo(vertices, ehDirigido);

    for (int i = 0; i < n; i++) {
        for (int e = 0; e < m; e++) {
            inc[i][e] = lerInteiro(-ehDirigido, 1);
        }
    }

    for (int e = 0; e < m; e++) {
        int origem = -1;
        int destino = -1;

        for (int i = 0; i < n; i++) {
            if (inc[i][e] == 0) {
                continue;
            }

            // dirigido: +1 e origem, -1 e destino
            // nao dirigido: primeiro 1 e origem, segundo 1 e destino
            int ehOrigem = dirigido ? inc[i][e] == 1 : origem < 0;
            int *extremo = ehOrigem ? &origem : &destino;

            if (*extremo >= 0) {
                printf("Aresta %d tem mais de dois extremos\n", e + 1);
                exit(1);
            }

            *extremo = i;
        }

        if (origem < 0 || destino < 0) {
            printf("Aresta %d nao tem dois extremos\n", e + 1);
            exit(1);
        }

        adicionarAresta(origem, destino);
    }
}


// Comando 4: le a lista de adjacencia (vizinhos de cada vertice terminados em 0)
// e monta a matriz. Se a lista nao for simetrica, o grafo so pode ser dirigido.
static void lerListaAdjacencia() {
    limparGrafo(lerInteiro(1, MAXN), 0);

    for (int i = 0; i < n; i++) {
        int vizinho;

        while ((vizinho = lerInteiro(0, n)) != 0) {
            adj[i][vizinho - 1] = 1;
        }
    }

    // lista assimetrica: tratado como dirigido
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (adj[i][j] != adj[j][i]) {
                dirigido = 1;
            }
        }
    }
}


// ==================== propriedades basicas ====================

// Grau do vertice = soma da linha i. No dirigido essa soma e o grau de saida.
static int grau(int i) {
    int soma = 0;

    for (int j = 0; j < n; j++) {
        soma += adj[i][j];
    }

    return soma;
}


// Grau de entrada = soma da coluna i. So faz sentido no grafo dirigido.
static int grauEntrada(int i) {
    int soma = 0;

    for (int j = 0; j < n; j++) {
        soma += adj[j][i];
    }

    return soma;
}


// Grafo simples: sem lacos (diagonal zero) e sem arestas paralelas (no maximo 1).
// Ciclo, completo, roda e clique so sao definidos para grafos simples.
static bool ehSimples() {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (adj[i][j] > (i != j)) {
                return 0;
            }
        }
    }

    return 1;
}

// Verifica se todos os vertices tem grau g, opcionalmente pulando um vertice.
// O parametro "ignorado" existe por causa da roda: o centro tem grau diferente
// dos demais. Ciclo e completo passam -1 para nao ignorar ninguem.
static int todosComGrau(int g, int ignorado) {
    for (int i = 0; i < n; i++) {
        if (i != ignorado && grau(i) != g) {
            return 0;
        }
    }

    return 1;
}


// ==================== multiplicacao de matrizes ====================

typedef int Matriz[MAXN][MAXN];

// Monta a matriz 0/1 do grafo subjacente (sem direcao e sem arestas repetidas),
// opcionalmente removendo um vertice. Conexidade e bipartido nao dependem do
// sentido das arestas, e o "ignorado" serve para testar a roda sem o centro.
static void montarSubjacente(Matriz A, int ignorado) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = i != ignorado && j != ignorado && (adj[i][j] || adj[j][i]);
        }
    }
}


// C = A x B. A^r[i][j] conta os caminhos de comprimento r entre i e j (Grafos_11);
// aqui so importa se existe caminho ou nao, entao o resultado e 0/1 e o laco
// interno para no primeiro k que liga i a j. Isso evita overflow em grafos grandes.
static void multiplicar(Matriz A, Matriz B, Matriz C) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;

            for (int k = 0; k < n && !C[i][j]; k++) {
                C[i][j] = A[i][k] && B[k][j];
            }
        }
    }
}


// S = S ou (S x B). Se S guarda "existe caminho de ate r passos", depois da chamada
// guarda "de ate r + (passos de B)". Acumular direto dispensa guardar A^r a parte
// e copiar matrizes entre uma potencia e outra.
static void acumular(Matriz S, Matriz B) {
    static Matriz T;

    multiplicar(S, B, T);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            S[i][j] = S[i][j] || T[i][j];
        }
    }
}


// Conexo pelo metodo das matrizes: S = A + A^2 + ... + A^(n-1). Um caminho simples
// tem no maximo n-1 arestas, entao se S[i][j] == 0 para algum par i != j, nao
// existe caminho entre eles e o grafo e desconexo.
static bool ehConexo(int ignorado) {
    static Matriz A;
    static Matriz S;

    montarSubjacente(A, ignorado);
    montarSubjacente(S, ignorado);

    for (int r = 2; r < n; r++) {
        acumular(S, A);
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j && i != ignorado && j != ignorado && !S[i][j]) {
                return 0;
            }
        }
    }

    return 1;
}


// ==================== classificacao ====================

// Ciclo Cn: grafo simples, conexo, com todos os vertices de grau 2.
// A conexidade e necessaria: dois triangulos separados tambem tem grau 2 em todos.
static bool ehCiclo() {
    return n >= 3 && ehSimples() && todosComGrau(2, -1) && ehConexo(-1);
}

// Completo Kn: grafo simples em que todo vertice liga aos outros n-1.
static bool ehCompleto() {
    return ehSimples() && todosComGrau(n - 1, -1);
}

// Roda Wn: um centro ligado a todos (grau n-1) e os demais formando um ciclo
// (grau 3 = 2 do ciclo + 1 do centro). Testa cada vertice como candidato a centro
// e confere se o resto continua conexo, o que garante que forma um unico ciclo.
static bool ehRoda() {
    if (n < 4 || !ehSimples()) {
        return 0;
    }

    for (int centro = 0; centro < n; centro++) {
        if (grau(centro) == n - 1 && todosComGrau(3, centro) && ehConexo(centro)) {
            return 1;
        }
    }

    return 0;
}

// Euleriano (Grafos_07): precisa ser conexo e, no nao dirigido, ter todos os graus
// pares (ciclo) ou exatamente dois impares (so caminho). No dirigido, saida == entrada
// em todos (ciclo) ou um vertice com +1 e outro com -1 (so caminho). O mesmo laco
// atende os dois casos: "diferenca" e saida-entrada no dirigido e grau%2 no outro.
static const char *verificarEuleriano() {
    if (!ehConexo(-1)) {
        return "nao (desconexo)";
    }

    int excessoSaida = 0;
    int excessoEntrada = 0;

    for (int i = 0; i < n; i++) {
        int diferenca = dirigido ? grau(i) - grauEntrada(i) : grau(i) % 2;

        if (diferenca == 1) {
            excessoSaida++;
        } else if (diferenca == -1) {
            excessoEntrada++;
        } else if (diferenca != 0) {
            return "nao";
        }
    }

    if (excessoSaida == 0 && excessoEntrada == 0) {
        return "sim (ciclo euleriano)";
    }

    if (dirigido ? excessoSaida == 1 && excessoEntrada == 1 : excessoSaida == 2) {
        return "apenas caminho euleriano";
    }

    return "nao";
}

// Bipartido (Grafos_08) pelo metodo das matrizes: um grafo e bipartido se e so se nao
// tem ciclo impar. A diagonal de A^r mostra quem volta a si mesmo em r passos, entao
// soma so as potencias impares (multiplicando por A^2 a cada passo) e exige
// diagonal zero. Um laco cai no caso r = 1.
static bool ehBipartido() {
    static Matriz A;
    static Matriz A2;
    static Matriz S;

    montarSubjacente(A, -1);
    montarSubjacente(S, -1);
    multiplicar(A, A, A2);

    for (int r = 3; r <= n; r += 2) {
        acumular(S, A2);
    }

    for (int i = 0; i < n; i++) {
        if (S[i][i]) {
            return 0;
        }
    }

    return 1;
}


// Backtracking para clique: escolhidos[0..qtd) ja e um clique e a funcao tenta
// acrescentar vertices (sempre em ordem crescente, para nao testar o mesmo conjunto
// duas vezes) ate chegar a k. Um vertice so entra se liga a todos os ja escolhidos.
static bool completarClique(int k, int inicio, int escolhidos[], int qtd) {
    if (qtd == k) {
        return 1;
    }

    for (int v = inicio; v < n; v++) {
        bool ligadoATodos = 1;

        for (int i = 0; i < qtd && ligadoATodos; i++) {
            ligadoATodos = adj[escolhidos[i]][v] && adj[v][escolhidos[i]];
        }

        if (ligadoATodos) {
            escolhidos[qtd] = v;

            if (completarClique(k, v + 1, escolhidos, qtd + 1)) {
                return 1;
            }
        }
    }

    return 0;
}


// Existe clique de tamanho k? Comeca o backtracking com o conjunto vazio.
// O quadro pede k = 3 e k = 5.
static bool temClique(int k) {
    int escolhidos[MAXN];

    return completarClique(k, 0, escolhidos, 0);
}


// ==================== desafio: construir grafo euleriano ====================

// Busca um ciclo de comprimento L usando apenas pares de vertices que ainda nao tem
// aresta (para o grafo continuar simples). ciclo[0..qtd) e o caminho ja montado;
// quando chega em L vertices, confere se o ultimo fecha com o primeiro.
static bool acharCiclo(int L, int ciclo[], int qtd, bool usado[]) {
    int u = ciclo[qtd - 1];

    if (qtd == L) {
        return !adj[u][ciclo[0]];
    }

    for (int v = 0; v < n; v++) {
        if (usado[v] || adj[u][v]) {
            continue;
        }

        usado[v] = 1;
        ciclo[qtd] = v;

        if (acharCiclo(L, ciclo, qtd + 1, usado)) {
            return 1;
        }

        usado[v] = 0;
    }

    return 0;
}


// Insere as L arestas do ciclo encontrado. Somar um ciclo mantem todos os graus
// pares, que e a propriedade que faz o grafo continuar euleriano.
static void adicionarCiclo(int L, int ciclo[]) {
    for (int i = 0; i < L; i++) {
        adicionarAresta(ciclo[i], ciclo[(i + 1) % L]);
    }
}


// Comando 7 (desafio): constroi um grafo euleriano com n vertices e m arestas.
// Ideia: todo grafo euleriano e uma uniao de ciclos sem arestas em comum. Comeca
// com um ciclo por todos os vertices (garante conexo) e soma ciclos de 3, 4 ou 5
// arestas ate completar m. Quando sobram 1 ou 2 arestas nao existe ciclo tao curto,
// entao o ciclo inicial deixa 2 vertices de fora e eles entram num triangulo ou
// num quadrado. O maximo de arestas e o de Kn; se n e par, Kn tem graus impares e
// perde n/2 arestas (um emparelhamento).
static void construirEuleriano() {
    int vertices = lerInteiro(3, MAXN);
    int maximo = vertices * (vertices - 1) / 2 - (vertices % 2 == 0 ? vertices / 2 : 0);
    int m = lerInteiro(vertices, maximo);
    int ciclo[MAXN];
    bool usado[MAXN];

    limparGrafo(vertices, 0);

    int base = n;
    int restante = m - n;

    // sobra 1 ou 2: nao existe ciclo tao curto, entao o ciclo inicial deixa 2 vertices de fora
    // e eles entram num triangulo (sobra 1) ou num quadrado (sobra 2) junto com vertices do ciclo
    if (restante == 1 || restante == 2) {
        base = n - 2;
    }

    for (int i = 0; i < base; i++) {
        adicionarAresta(i, (i + 1) % base);
    }

    if (restante == 1) {
        adicionarCiclo(3, (int[]){0, n - 2, n - 1});
        restante = 0;
    } else if (restante == 2) {
        adicionarCiclo(4, (int[]){0, n - 2, 1, n - 1});
        restante = 0;
    }

    while (restante > 0) {
        int L = restante == 4 ? 4 : restante == 5 ? 5 : 3;
        bool achou = 0;

        memset(usado, 0, sizeof usado);

        for (int inicio = 0; inicio < n && !achou; inicio++) {
            usado[inicio] = 1;
            ciclo[0] = inicio;
            achou = acharCiclo(L, ciclo, 1, usado);
            usado[inicio] = 0;
        }

        if (!achou) {
            printf("Nao consegui construir um grafo euleriano com %d vertices e %d arestas\n", n, m);
            exit(1);
        }

        adicionarCiclo(L, ciclo);
        restante -= L;
    }
}


// ==================== saida ====================

// Imprime as tres visoes pedidas nos exercicios: lista de adjacencia, matriz de
// adjacencia e graus (entrada/saida quando dirigido). Roda depois de toda leitura.
static void mostrarGrafo() {
    printf("%s, %d vertices\n", dirigido ? "Dirigido" : "Nao dirigido", n);

    printf("\nLista de adjacencia:\n");

    for (int i = 0; i < n; i++) {
        printf("%d:", i + 1);

        for (int j = 0; j < n; j++) {
            for (int k = 0; k < adj[i][j]; k++) {
                printf(" %d", j + 1);
            }
        }

        printf("\n");
    }

    printf("\nMatriz de adjacencia:\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%2d", adj[i][j]);
        }

        printf("\n");
    }

    printf("\nGraus:\n");

    for (int i = 0; i < n; i++) {
        if (dirigido) {
            printf("%d: entrada %d, saida %d\n", i + 1, grauEntrada(i), grau(i));
        } else {
            printf("%d: %d\n", i + 1, grau(i));
        }
    }

    printf("\n");
}


// Comando 6: responde cada pergunta dos slides para o grafo carregado.
static void classificarGrafo() {
    printf("Simples: %s\n", ehSimples() ? "sim" : "nao");
    printf("Ciclo: %s\n", ehCiclo() ? "sim" : "nao");
    printf("Completo: %s\n", ehCompleto() ? "sim" : "nao");
    printf("Roda: %s\n", ehRoda() ? "sim" : "nao");
    printf("Euleriano: %s\n", verificarEuleriano());
    printf("Bipartido: %s\n", ehBipartido() ? "sim" : "nao");
    printf("Conexo: %s\n", ehConexo(-1) ? "sim" : "nao");
    printf("Clique de tamanho 3: %s\n", temClique(3) ? "sim" : "nao");
    printf("Clique de tamanho 5: %s\n", temClique(5) ? "sim" : "nao");
}


// ==================== principal ====================

// Le entrada.txt (ou o teclado, se nao existir) e executa os comandos em sequencia.
// Toda leitura e seguida de mostrarGrafo; o desafio tambem classifica para provar
// que o grafo construido e euleriano.
int main() {
    entrada = fopen("entrada.txt", "r");

    if (entrada == NULL) {
        printf("entrada.txt nao encontrado, lendo do teclado\n");
        entrada = stdin;
    }

    int comando;

    while ((comando = lerInteiro(0, 7)) != 0) {
        if (comando == 1) 
            lerMatrizAdjacencia();
        if (comando == 2) 
            lerMatrizIncidencia(0);
        if (comando == 3) 
            lerMatrizIncidencia(1);
        if (comando == 4) 
            lerListaAdjacencia();
        if (comando == 7) 
            construirEuleriano();
        if (comando <= 5 || comando == 7) 
            mostrarGrafo();
        if (comando == 6 || comando == 7) 
            classificarGrafo();
    }
}