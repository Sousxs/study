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
 *   6                         classifica: ciclo, completo, roda, euleriano, bipartido, clique
 *   0                         encerra
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXN 1000
#define MAXM 1000

static FILE *entrada;

static int n;                       
static int dirigido;                
static int adj[MAXN][MAXN];         
static int linha;                    

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

    /* lista assimetrica: tratado como dirigido */
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

// retorna 1 se todos os vertices tiverem grau g
static int todosComGrau(int g, int ignorado) {
    for (int i = 0; i < n; i++) {
        if (i != ignorado && grau(i) != g) {
            return 0;
        }
    }

    return 1;
}


// Busca em largura a partir do inicio no grafo subjacente, ignorando o vertice
// Pinta cor[] com 0/1 (cor -1 = nao visitado).
// Retorna 0 se dois vizinhos ficaram com a mesma cor (componente nao bipartida).
static int buscaLargura(int inicio, int ignorado, int cor[]) {
    int fila[MAXN];
    int frente = 0;
    int tras = 0;
    int bipartido = 1;

    cor[inicio] = 0;
    fila[tras++] = inicio;

    while (frente < tras) {
        int u = fila[frente++];

        for (int v = 0; v < n; v++) {
            if (v == ignorado || (adj[u][v] == 0 && adj[v][u] == 0)) {
                continue;
            }

            if (cor[v] < 0) {
                cor[v] = !cor[u];
                fila[tras++] = v;
            } else if (cor[v] == cor[u]) {
                bipartido = 0;     
            }
        }
    }

    return bipartido;
}


static int ehConexo(int ignorado) {
    int cor[MAXN];

    memset(cor, -1, sizeof cor);
    buscaLargura(ignorado == 0, ignorado, cor);

    for (int v = 0; v < n; v++) {
        if (v != ignorado && cor[v] < 0) {
            return 0;
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

// bipartido: busca em largura, pintando cor 0/1, retorna 0 se dois vizinhos ficarem com a mesma cor
static bool ehBipartido() {
    int cor[MAXN];

    memset(cor, -1, sizeof cor);

    for (int inicio = 0; inicio < n; inicio++) {
        if (cor[inicio] < 0 && !buscaLargura(inicio, -1, cor)) {
            return 0;
        }
    }

    return 1;
}

// clique: conjunto de vertices todos ligados entre si
static bool temClique() {
    for (int a = 0; a < n; a++) {
        for (int b = a + 1; b < n; b++) {
            for (int c = b + 1; c < n; c++) {
                // verifica se a, b e c formam um clique (K3)
                if (adj[a][b] && adj[b][a] && adj[b][c] && adj[c][b] && adj[a][c] && adj[c][a]) {
                    return 1;
                }
            }
        }
    }

    return 0;
}


// ==================== saida ==================== 

static bool mostrarGrafo() {
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
    printf("Clique: %s\n", temClique() ? "sim" : "nao");
}


// ==================== principal ====================

int main() {
    entrada = fopen("entrada.txt", "r");

    if (entrada == NULL) {
        printf("entrada.txt nao encontrado, lendo do teclado\n");
        entrada = stdin;
    }

    int comando;

    while ((comando = lerInteiro(0, 6)) != 0) {
        if (comando == 1) 
            lerMatrizAdjacencia();
        if (comando == 2) 
            lerMatrizIncidencia(0);
        if (comando == 3) 
            lerMatrizIncidencia(1);
        if (comando == 4) 
            lerListaAdjacencia();
        if (comando <= 5) 
            mostrarGrafo();
        if (comando == 6) 
            classificarGrafo();
    }
}