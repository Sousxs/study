#include <iostream>
#include <queue>
#include <stack>
using namespace std;

class Aluno {
public:
    int mat;
    float nota;
};

class Node {
public:
    int key;
    Aluno A;
    Node* left;
    Node* right;
};

class BBTree {
private:
    Node* root;
    int n;

    void inOrder(Node* R);
    void preOrder(Node* R);
    void posOrder(Node* R);
    bool insertRec(Node** R, Node* P, int k);
    bool remove(Node** R, int k);
    void destroy(Node* R);
    int  height(Node* R);
    int  countNodes(Node* R);
    int  countLeaves(Node* R);
    void invert(Node* R);
    static Node* minimo(Node* R);
    static Node* maximo(Node* R);

public:
    BBTree();
    ~BBTree();

    void inOrder();
    void preOrder();
    void posOrder();
    void levelOrder();
    void inOrderIter();

    bool insert(int k, Aluno a);
    bool insertRec(int k, Aluno a);
    bool remove(int k);
    Aluno* search(int k);

    int  height();
    int  countNodes();
    int  countLeaves();
    void invert();
    int  size();
};

// ---------- construtor / destrutor ----------

BBTree::BBTree() : root(nullptr), n(0) {}

BBTree::~BBTree(){ destroy(root); }

void BBTree::destroy(Node* R){
    if(R){
        destroy(R->left);
        destroy(R->right);
        delete R;
    }
}

// ---------- percursos ----------

void BBTree::inOrder(Node* R){
    if(R){
        inOrder(R->left);
        cout << R->key << " ";
        inOrder(R->right);
    }
}

void BBTree::preOrder(Node* R){
    if(R){
        cout << R->key << " ";
        preOrder(R->left);
        preOrder(R->right);
    }
}

void BBTree::posOrder(Node* R){
    if(R){
        posOrder(R->left);
        posOrder(R->right);
        cout << R->key << " ";
    }
}

void BBTree::inOrder()  { inOrder(root);  cout << endl; }
void BBTree::preOrder() { preOrder(root); cout << endl; }
void BBTree::posOrder() { posOrder(root); cout << endl; }



// in-order sem recursao (pilha explicita)
void BBTree::inOrderIter(){
    stack<Node*> s;
    Node* R = root;
    while(R || !s.empty()){
        while(R){ s.push(R); R = R->left; }   // desce tudo a esquerda
        R = s.top(); s.pop();
        cout << R->key << " ";                // visita
        R = R->right;                         // vai p/ direita
    }
    cout << endl;
}

// ---------- insercao ----------

bool BBTree::insert(int k, Aluno a){
    Node* P = new Node;
    P->key = k;
    P->A = a;
    P->left = P->right = nullptr;

    if(!root){ root = P; n++; return true; }

    Node* R = root;
    while(true){
        if(k == R->key){ delete P; return false; }
        if(k < R->key){
            if(!R->left){ R->left = P; n++; return true; }
            R = R->left;
        } else {
            if(!R->right){ R->right = P; n++; return true; }
            R = R->right;
        }
    }
}

bool BBTree::insertRec(Node** R, Node* P, int k){
    if(!(*R)){ *R = P; n++; return true; }
    if(k == (*R)->key){ delete P; return false; }
    if(k < (*R)->key) return insertRec(&((*R)->left), P, k);
    return insertRec(&((*R)->right), P, k);
}

bool BBTree::insertRec(int k, Aluno a){
    Node* P = new Node;
    P->key = k;
    P->A = a;
    P->left = P->right = nullptr;
    return insertRec(&root, P, k);
}

// ---------- busca ----------

Aluno* BBTree::search(int k){
    Node* R = root;
    while(R && R->key != k)
        R = (k < R->key) ? R->left : R->right;
    return R ? &R->A : nullptr;
}

Node* BBTree::minimo(Node* R){
    while(R && R->left) R = R->left;
    return R;
}

Node* BBTree::maximo(Node* R){
    while(R && R->right) R = R->right;
    return R;
}

// ---------- remocao (versao do quadro) ----------

bool BBTree::remove(Node** R, int k){
    if(!(*R)) return false;                    // nao tem

    if((*R)->key == k){                        // ACHOU
        if(!(*R)->left){                       // sem filho a esq (cobre folha)
            Node* P = *R;
            *R = P->right;
            delete P; n--;
            return true;
        }
        if(!(*R)->right){                      // sem filho a dir
            Node* P = *R;
            *R = P->left;
            delete P; n--;
            return true;
        }
        // 2 filhos: troca com o sucessor (minimo da subarvore direita)
        Node* M = minimo((*R)->right);
        swap((*R)->key, M->key);
        swap((*R)->A,   M->A);
        return remove(&((*R)->right), k);     // k agora esta num no sem filho a esq
    }

    if(k < (*R)->key) 
        return remove(&((*R)->left), k);
    return remove(&((*R)->right), k);
}

bool BBTree::remove(int k){ return remove(&root, k); }

// ---------- operacoes de prova ----------

// altura em ARESTAS: vazia = -1, so raiz = 0
// (se o professor contar em nos: vazia = 0, troque -1 por 0)
int BBTree::height(Node* R){
    if(!R) return -1;
    int hl = height(R->left);
    int hr = height(R->right);
    return 1 + (hl > hr ? hl : hr);
}

int BBTree::countNodes(Node* R){
    if(!R) return 0;
    return 1 + countNodes(R->left) + countNodes(R->right);
}

int BBTree::countLeaves(Node* R){
    if(!R) return 0;
    if(!R->left && !R->right) return 1;
    return countLeaves(R->left) + countLeaves(R->right);
}

// espelha a arvore (LeetCode 226). Depois disso NAO e mais BST crescente!
void BBTree::invert(Node* R){
    if(R){
        swap(R->left, R->right);
        invert(R->left);
        invert(R->right);
    }
}

int  BBTree::height()      { return height(root); }
int  BBTree::countNodes()  { return countNodes(root); }
int  BBTree::countLeaves() { return countLeaves(root); }
void BBTree::invert()      { invert(root); }
int  BBTree::size()        { return n; }

// ---------- main ----------

int main(){
    BBTree T;
    int keys[] = {50, 30, 70, 20, 40, 60, 80};
    for(int k : keys) T.insert(k, Aluno{k * 10, k / 10.0f});

    cout << "Pre-order:   "; T.preOrder();
    cout << "In-order:    "; T.inOrder();
    cout << "In-order it: "; T.inOrderIter();
    cout << "Pos-order:   "; T.posOrder();
    cout << "Level-order: "; T.levelOrder();
    cout << "n = " << T.size() << " | nos = " << T.countNodes()
         << " | folhas = " << T.countLeaves()
         << " | altura = " << T.height() << endl;

    cout << "insert 50 repetido: " << T.insert(50, Aluno{0, 0}) << endl;

    Aluno* a = T.search(40);
    cout << "search 40: " << (a ? a->mat : -1) << endl;
    cout << "search 99: " << (T.search(99) ? "achou" : "nao achou") << endl;

    T.remove(20);  cout << "remove 20 (folha):    "; T.inOrder();
    T.remove(30);  cout << "remove 30 (1 filho):  "; T.inOrder();
    T.remove(50);  cout << "remove 50 (2 filhos): "; T.inOrder();
    cout << "Pre-order apos remocoes: "; T.preOrder();

    T.invert();
    cout << "Invertida in-order:  "; T.inOrder();
    cout << "Invertida pre-order: "; T.preOrder();
    return 0;
}