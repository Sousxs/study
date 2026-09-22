/* gcc -O2 -o grafos grafos.c && ./grafos 
 *
 * Formato de entrada.txt: sequencia de comandos, cada um seguido dos seus dados.
 *
 *   1 n <matriz n x n>                                 matriz de adjacencia, nao dirigido
 *   2 n m <matriz n x m>                               matriz de incidencia, nao dirigido (1 = incide)
 *   3 n m <matriz n x m>                               matriz de incidencia, dirigido (+1 origem, -1 destino)
 *   4 n <vizinhos de v1> 0 ... <vizinhos de vn> 0      lista de adjacencia
 *   5                                                  mostra lista, matriz e graus
 *   6                                                  classifica: ciclo, completo, roda, euleriano, bipartido, conexo, cliques
 *   7 n m                                              desafio: constroi um grafo euleriano com n vertices e m arestas
 *   0                                                  encerra
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


static void limparGrafo(int vertices, int ehDirigido) {
    n = vertices;
    dirigido = ehDirigido;

    memset(adj, 0, sizeof adj);
}


static void adicionarAresta(int a, int b) {
    adj[a][b]++;

    if (!dirigido) {
        adj[b][a]++;
    }
}


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

static int grau(int i) {
    int soma = 0;

    for (int j = 0; j < n; j++) {
        soma += adj[i][j];
    }

    return soma;
}


static int grauEntrada(int i) {
    int soma = 0;

    for (int j = 0; j < n; j++) {
        soma += adj[j][i];
    }

    return soma;
}


// sem lacos e sem arestas paralelas 
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

// retorna 1 se todos os vertices (exceto "ignorado") tiverem grau g
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

// grafo subjacente (ignora direcao), sem o vertice "ignorado"; guarda so 0/1
static void montarSubjacente(Matriz A, int ignorado) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = i != ignorado && j != ignorado && (adj[i][j] || adj[j][i]);
        }
    }
}


// C = A x B guardando so 0/1: interessa se existe caminho, nao quantos (evita overflow)
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


// P = P x A
static void elevar(Matriz P, Matriz A) {
    static Matriz T;

    multiplicar(P, A, T);
    memcpy(P, T, sizeof(Matriz));
}


// conexo: S = A + A^2 + ... + A^(n-1) nao pode ter zero fora da diagonal
static bool ehConexo(int ignorado) {
    static Matriz A;
    static Matriz P;
    static Matriz S;

    montarSubjacente(A, ignorado);
    memcpy(P, A, sizeof(Matriz));
    memcpy(S, A, sizeof(Matriz));

    for (int r = 2; r < n; r++) {
        elevar(P, A);

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                S[i][j] = S[i][j] || P[i][j];
            }
        }
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

// ciclo: todos com grau 2, simples e conexo
static bool ehCiclo() {
    return n >= 3 && ehSimples() && todosComGrau(2, -1) && ehConexo(-1);
}

// completo: todos com grau n-1, simples
static bool ehCompleto() {
    return ehSimples() && todosComGrau(n - 1, -1);
}

// roda: um centro com grau n-1, todos os  outros com grau 3, simples e conexo
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

// euleriano: conexo, todos com grau par (nao dirigido) ou um com saida - entrada = +1 e outro com saida - entrada = -1 (dirigido)
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

// bipartido: nao tem ciclo impar, ou seja, a diagonal de A^r e zero para todo r impar
static bool ehBipartido() {
    static Matriz A;
    static Matriz P;

    montarSubjacente(A, -1);
    memcpy(P, A, sizeof(Matriz));

    for (int r = 1; r <= n; r++) {
        if (r % 2 == 1) {
            for (int i = 0; i < n; i++) {
                if (P[i][i]) {
                    return 0;
                }
            }
        }

        elevar(P, A);
    }

    return 1;
}


// clique: escolhidos[0..qtd) ja e um clique; tenta completar ate k vertices olhando so vertices >= inicio
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


static bool temClique(int k) {
    int escolhidos[MAXN];

    return completarClique(k, 0, escolhidos, 0);
}


// ==================== desafio: construir grafo euleriano ====================

// procura um ciclo de comprimento L usando so pares que ainda nao tem aresta; ciclo[0..qtd) ja esta escolhido
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


static void adicionarCiclo(int L, int ciclo[]) {
    for (int i = 0; i < L; i++) {
        adicionarAresta(ciclo[i], ciclo[(i + 1) % L]);
    }
}


// Todo grafo euleriano e uma uniao de ciclos sem arestas em comum.
// Comeca com um ciclo passando por todos os vertices e vai somando ciclos ate chegar em m arestas.
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


/* ==================== principal ==================== */

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