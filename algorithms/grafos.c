/* grafos.c - CMP2119 Algoritmos em Grafos
 *
 * Compilar e rodar:  gcc -O2 -o grafos grafos.c && ./grafos
 *
 * O programa le comandos do arquivo entrada.txt (ou do teclado, se o arquivo
 * nao existir). Linhas que comecam com '#' sao comentarios.
 *
 *   Comando  Dados                                  O que faz
 *   -------  -------------------------------------  ----------------------------------------------
 *   1        n, matriz n x n                        le matriz de adjacencia (nao dirigido)
 *   2        n m, matriz n x m                      le matriz de incidencia (nao dirigido, 1 = incide)
 *   3        n m, matriz n x m                      le matriz de incidencia (dirigido: +1 origem, -1 destino)
 *   4        n, vizinhos de cada vertice + 0        le lista de adjacencia (cada lista termina em 0)
 *   5        -                                      mostra o grafo atual
 *   6        -                                      classifica o grafo atual
 *   7        n m                                    constroi um grafo euleriano com n vertices e m arestas
 *   0        -                                      encerra
 *
 * Como o grafo e guardado:
 *   adj[i][j] = quantidade de arestas que saem do vertice i e chegam no vertice j.
 *   Em grafo nao dirigido, cada aresta i-j aumenta adj[i][j] E adj[j][i].
 *   Internamente os vertices vao de 0 a n-1; na tela aparecem de 1 a n.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAXN 1000   // maximo de vertices
#define MAXM 1000   // maximo de arestas (matriz de incidencia)

#define NENHUM -1   // usado quando nenhum vertice deve ser ignorado

FILE *entrada;

int n;                  // numero de vertices
bool dirigido;          // true se o grafo for dirigido
int adj[MAXN][MAXN];    // matriz de adjacencia


/* ========================================================================
 *  LEITURA DA ENTRADA
 * ======================================================================== */

// Le o proximo numero da entrada, ignorando comentarios.
// Se o arquivo acabar, encerra o programa.
int lerInteiro() {
    char palavra[64];

    while (fscanf(entrada, "%63s", palavra) == 1) {
        if (palavra[0] == '#') {
            fscanf(entrada, "%*[^\n]");     // pula o resto da linha
        } else {
            return atoi(palavra);
        }
    }

    exit(0);
}

// Comeca um grafo novo, sem nenhuma aresta.
void novoGrafo(int vertices, bool ehDirigido) {
    n = vertices;
    dirigido = ehDirigido;
    memset(adj, 0, sizeof adj);
}

void adicionarAresta(int a, int b) {
    adj[a][b]++;

    if (!dirigido) {
        adj[b][a]++;
    }
}

// Comando 1: a matriz lida ja e a propria matriz de adjacencia.
void lerMatrizAdjacencia() {
    novoGrafo(lerInteiro(), false);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            adj[i][j] = lerInteiro();
        }
    }
}

// Comandos 2 e 3: cada coluna da matriz de incidencia e uma aresta.
// Em cada coluna procuramos os dois vertices que a aresta liga.
void lerMatrizIncidencia(bool ehDirigido) {
    static int incidencia[MAXN][MAXM];

    int vertices = lerInteiro();
    int arestas = lerInteiro();

    novoGrafo(vertices, ehDirigido);

    for (int i = 0; i < n; i++) {
        for (int e = 0; e < arestas; e++) {
            incidencia[i][e] = lerInteiro();
        }
    }

    for (int e = 0; e < arestas; e++) {
        int origem = NENHUM;
        int destino = NENHUM;

        for (int i = 0; i < n; i++) {
            if (incidencia[i][e] == 0) {
                continue;               // vertice i nao toca a aresta e
            }

            // Dirigido: +1 e a origem, -1 e o destino.
            // Nao dirigido: o primeiro 1 vira "origem" e o segundo vira "destino".
            if (incidencia[i][e] == 1 && origem == NENHUM) {
                origem = i;
            } else {
                destino = i;
            }
        }

        adicionarAresta(origem, destino);
    }
}

// Comando 4: para cada vertice, le os vizinhos ate encontrar 0.
// Se alguma ligacao aparecer em um so sentido, o grafo e considerado dirigido.
void lerListaAdjacencia() {
    novoGrafo(lerInteiro(), false);

    for (int i = 0; i < n; i++) {
        int vizinho = lerInteiro();

        while (vizinho != 0) {
            adj[i][vizinho - 1] = 1;
            vizinho = lerInteiro();
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (adj[i][j] != adj[j][i]) {
                dirigido = true;
            }
        }
    }
}


/* ========================================================================
 *  FUNCOES AUXILIARES
 * ======================================================================== */

// Grau de saida (em grafo nao dirigido e o grau normal): soma da linha i.
int grau(int i) {
    int soma = 0;

    for (int j = 0; j < n; j++) {
        soma += adj[i][j];
    }

    return soma;
}

// Grau de entrada: soma da coluna i.
int grauEntrada(int i) {
    int soma = 0;

    for (int j = 0; j < n; j++) {
        soma += adj[j][i];
    }

    return soma;
}

// Existe alguma aresta entre a e b, em qualquer sentido?
bool ligados(int a, int b) {
    return adj[a][b] > 0 || adj[b][a] > 0;
}

// Todos os vertices (menos o "ignorado") tem grau g?
bool todosComGrau(int g, int ignorado) {
    for (int i = 0; i < n; i++) {
        if (i != ignorado && grau(i) != g) {
            return false;
        }
    }

    return true;
}


/* ========================================================================
 *  CLASSIFICACAO DO GRAFO
 * ======================================================================== */

// Simples: sem lacos (aresta de um vertice para ele mesmo) e sem arestas repetidas.
bool ehSimples() {
    for (int i = 0; i < n; i++) {
        if (adj[i][i] != 0) {
            return false;
        }

        for (int j = 0; j < n; j++) {
            if (adj[i][j] > 1) {
                return false;
            }
        }
    }

    return true;
}

// Conexo: a partir de um vertice qualquer, a busca alcanca todos os outros.
// Se "ignorado" for um vertice, testa o grafo como se ele tivesse sido removido
// (isso e usado na verificacao de roda).
// Busca em profundidade com pilha; a direcao das arestas e ignorada.
bool ehConexo(int ignorado) {
    bool visitado[MAXN] = {false};
    int pilha[MAXN];
    int topo = 0;

    int inicio = 0;
    if (ignorado == 0) {
        inicio = 1;
    }

    visitado[inicio] = true;
    pilha[topo++] = inicio;

    while (topo > 0) {
        int v = pilha[--topo];

        for (int u = 0; u < n; u++) {
            if (u != ignorado && !visitado[u] && ligados(v, u)) {
                visitado[u] = true;
                pilha[topo++] = u;
            }
        }
    }

    for (int i = 0; i < n; i++) {
        if (i != ignorado && !visitado[i]) {
            return false;
        }
    }

    return true;
}

// Roda: um vertice central ligado a todos, e os demais formam um ciclo.
// Ou seja: o centro tem grau n-1, os outros tem grau 3 (2 do ciclo + 1 do centro)
// e, tirando o centro, o que sobra continua conexo.
bool ehRoda() {
    if (n < 4 || !ehSimples()) {
        return false;
    }

    for (int centro = 0; centro < n; centro++) {
        if (grau(centro) == n - 1 && todosComGrau(3, centro) && ehConexo(centro)) {
            return true;
        }
    }

    return false;
}

// Nao dirigido:
//   todos os graus pares         -> ciclo euleriano
//   exatamente 2 graus impares   -> so caminho euleriano
// Dirigido (diferenca = saida - entrada):
//   todas as diferencas 0                        -> ciclo euleriano
//   um vertice com +1, um com -1, resto com 0    -> so caminho euleriano
const char *verificarEuleriano() {
    if (!ehConexo(NENHUM)) {
        return "nao (desconexo)";
    }

    if (!dirigido) {
        int impares = 0;

        for (int i = 0; i < n; i++) {
            if (grau(i) % 2 == 1) {
                impares++;
            }
        }

        if (impares == 0) {
            return "sim (ciclo euleriano)";
        }
        if (impares == 2) {
            return "apenas caminho euleriano";
        }
        return "nao";
    }

    int sobraSaida = 0;     // vertices com saida = entrada + 1
    int sobraEntrada = 0;   // vertices com entrada = saida + 1

    for (int i = 0; i < n; i++) {
        int diferenca = grau(i) - grauEntrada(i);

        if (diferenca == 1) {
            sobraSaida++;
        } else if (diferenca == -1) {
            sobraEntrada++;
        } else if (diferenca != 0) {
            return "nao";
        }
    }

    if (sobraSaida == 0 && sobraEntrada == 0) {
        return "sim (ciclo euleriano)";
    }
    if (sobraSaida == 1 && sobraEntrada == 1) {
        return "apenas caminho euleriano";
    }
    return "nao";
}

// Bipartido: da para pintar os vertices com 2 cores sem que vizinhos tenham a mesma cor.
// Cores: 0 = sem cor, 1 e -1 sao as duas cores. Pinta por busca em largura (fila).
bool ehBipartido() {
    int cor[MAXN] = {0};
    int fila[MAXN];

    for (int s = 0; s < n; s++) {
        if (cor[s] != 0) {
            continue;                   // ja pintado por uma busca anterior
        }

        int frente = 0, fim = 0;
        cor[s] = 1;
        fila[fim++] = s;

        while (frente < fim) {
            int v = fila[frente++];

            for (int u = 0; u < n; u++) {
                if (!ligados(v, u)) {
                    continue;
                }

                if (cor[u] == cor[v]) {
                    return false;       // vizinho com a mesma cor: nao e bipartido
                }

                if (cor[u] == 0) {
                    cor[u] = -cor[v];   // cor oposta
                    fila[fim++] = u;
                }
            }
        }
    }

    return true;
}

// Tenta montar um clique de k vertices (todos ligados entre si), escolhendo
// um vertice por vez. "escolhidos" guarda os qtd vertices ja escolhidos.
// So testa vertices a partir de "inicio" para nao repetir combinacoes.
// Chamada inicial: temClique(k, 0, 0).
int escolhidos[MAXN];

bool temClique(int k, int inicio, int qtd) {
    if (qtd == k) {
        return true;
    }

    for (int v = inicio; v < n; v++) {
        bool ligadoATodos = true;

        for (int i = 0; i < qtd; i++) {
            int w = escolhidos[i];

            if (adj[w][v] == 0 || adj[v][w] == 0) {
                ligadoATodos = false;
            }
        }

        if (ligadoATodos) {
            escolhidos[qtd] = v;

            if (temClique(k, v + 1, qtd + 1)) {
                return true;
            }
        }
    }

    return false;
}


/* ========================================================================
 *  CONSTRUCAO DE GRAFO EULERIANO (comando 7)
 *
 *  Ideia: um grafo conexo com todos os graus pares e euleriano.
 *    1. Comeca com um ciclo passando por todos os vertices (n arestas,
 *       todos com grau 2, e ja e conexo).
 *    2. As arestas que faltam sao adicionadas em forma de ciclos menores.
 *       Cada ciclo soma 2 no grau de cada vertice dele, entao os graus
 *       continuam pares.
 *
 *  Faltando 1 ou 2 arestas nao da para formar um ciclo, entao nesses casos
 *  o ciclo inicial passa so por n-2 vertices e os 2 vertices que sobraram
 *  entram em um triangulo (sobra 1) ou em um quadrado (sobra 2).
 * ======================================================================== */

// Liga ciclo[0] -> ciclo[1] -> ... -> ciclo[L-1] -> ciclo[0].
void adicionarCiclo(int L, int ciclo[]) {
    for (int i = 0; i < L; i++) {
        adicionarAresta(ciclo[i], ciclo[(i + 1) % L]);
    }
}

// Procura um ciclo de L vertices que use apenas arestas que AINDA NAO existem.
// Ja temos qtd vertices no caminho; tenta estender com mais um (backtracking).
bool acharCicloNovo(int L, int ciclo[], int qtd, bool usado[]) {
    int ultimo = ciclo[qtd - 1];

    if (qtd == L) {
        // caminho completo: so falta poder fechar de volta no primeiro
        return adj[ultimo][ciclo[0]] == 0;
    }

    for (int v = 0; v < n; v++) {
        if (usado[v] || adj[ultimo][v] > 0) {
            continue;
        }

        usado[v] = true;
        ciclo[qtd] = v;

        if (acharCicloNovo(L, ciclo, qtd + 1, usado)) {
            return true;
        }

        usado[v] = false;               // nao deu certo: desfaz e tenta outro
    }

    return false;
}

void construirEuleriano() {
    int vertices = lerInteiro();
    int m = lerInteiro();

    novoGrafo(vertices, false);

    int faltam = m - n;     // arestas alem do ciclo inicial

    // Ciclo inicial 0 -> 1 -> ... -> tamanho-1 -> 0.
    int tamanho = n;
    if (faltam == 1 || faltam == 2) {
        tamanho = n - 2;
    }

    int ciclo[MAXN];
    for (int i = 0; i < tamanho; i++) {
        ciclo[i] = i;
    }
    adicionarCiclo(tamanho, ciclo);

    if (faltam == 1) {
        int triangulo[3] = {0, n - 2, n - 1};
        adicionarCiclo(3, triangulo);
        return;
    }

    if (faltam == 2) {
        int quadrado[4] = {0, n - 2, 1, n - 1};
        adicionarCiclo(4, quadrado);
        return;
    }

    // Vai adicionando triangulos. Se faltarem exatamente 4 ou 5 arestas,
    // usa um ciclo desse tamanho (com triangulos sobraria 1 ou 2).
    while (faltam > 0) {
        int L = 3;
        if (faltam == 4 || faltam == 5) {
            L = faltam;
        }

        bool usado[MAXN] = {false};
        bool achou = false;

        for (int inicio = 0; inicio < n && !achou; inicio++) {
            usado[inicio] = true;
            ciclo[0] = inicio;
            achou = acharCicloNovo(L, ciclo, 1, usado);
            usado[inicio] = false;
        }

        if (!achou) {
            printf("Nao consegui construir um grafo euleriano com %d vertices e %d arestas\n", n, m);
            exit(1);
        }

        adicionarCiclo(L, ciclo);
        faltam -= L;
    }
}


/* ========================================================================
 *  SAIDA
 * ======================================================================== */

void mostrarGrafo() {
    if (dirigido) {
        printf("Dirigido, %d vertices\n", n);
    } else {
        printf("Nao dirigido, %d vertices\n", n);
    }

    printf("\nLista de adjacencia:\n");

    for (int i = 0; i < n; i++) {
        printf("%d:", i + 1);

        for (int j = 0; j < n; j++) {
            for (int k = 0; k < adj[i][j]; k++) {
                printf(" %d", j + 1);   // repete o vizinho se houver arestas multiplas
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

const char *simNao(bool valor) {
    if (valor) {
        return "sim";
    }
    return "nao";
}

void classificarGrafo() {
    printf("Simples: %s\n", simNao(ehSimples()));
    // Ciclo: simples, conexo, pelo menos 3 vertices e todos com grau 2.
    printf("Ciclo: %s\n", simNao(n >= 3 && ehSimples() && todosComGrau(2, NENHUM) && ehConexo(NENHUM)));
    // Completo: simples e cada vertice ligado a todos os outros (grau n-1).
    printf("Completo: %s\n", simNao(ehSimples() && todosComGrau(n - 1, NENHUM)));
    printf("Roda: %s\n", simNao(ehRoda()));
    printf("Euleriano: %s\n", verificarEuleriano());
    printf("Bipartido: %s\n", simNao(ehBipartido()));
    printf("Conexo: %s\n", simNao(ehConexo(NENHUM)));
    printf("Clique de tamanho 3: %s\n", simNao(temClique(3, 0, 0)));
    printf("Clique de tamanho 5: %s\n", simNao(temClique(5, 0, 0)));
    printf("________________________________\n");
}


/* ========================================================================
 *  PROGRAMA PRINCIPAL
 * ======================================================================== */

int main() {
    entrada = fopen("entrada.txt", "r");

    if (entrada == NULL) {
        printf("entrada.txt nao encontrado, lendo do teclado\n");
        entrada = stdin;
    }

    int comando = lerInteiro();

    while (comando != 0) {
        if (comando == 1) {
            lerMatrizAdjacencia();
            mostrarGrafo();
        } else if (comando == 2) {
            lerMatrizIncidencia(false);
            mostrarGrafo();
        } else if (comando == 3) {
            lerMatrizIncidencia(true);
            mostrarGrafo();
        } else if (comando == 4) {
            lerListaAdjacencia();
            mostrarGrafo();
        } else if (comando == 5) {
            mostrarGrafo();
        } else if (comando == 6) {
            classificarGrafo();
        } else if (comando == 7) {
            construirEuleriano();
            mostrarGrafo();
            classificarGrafo();
        }

        comando = lerInteiro();
    }

    return 0;
}
