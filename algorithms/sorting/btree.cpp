#include <iostream>
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

public:
    BBTree();
    ~BBTree();

    void inOrder();
    void preOrder();
    void posOrder();
    bool insert(int k, Aluno a);
    bool insertRec(int k, Aluno a);
    Aluno* search(int k);
    int size();
};

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

// ---------- inserção ----------

bool BBTree::insert(int k, Aluno a){
    Node* P = new Node;
    P->key = k;
    P->A = a;
    P->left = P->right = 0;

    if(!root){ 
        root = P;
        n++;
        return true;
    }

    Node* R = root;
    while(true){
        if(k == R->key){ 
            delete P; 
            return false; 
        }
        if(k < R->key){
            if(!R->left){ R->left = P; n++; return true; }
            R = R->left;
        } else {
            if(!R->right){ 
                R->right = P;
                n++; 
                return true; 
            }
            R = R->right;
        }
    }
}

bool BBTree::insertRec(Node** R, Node* P, int k){
    if(!(*R)){ 
        *R = P; n++; 
        return true; 
    }
    if(k == (*R)->key){
        delete P;
        return false; 
    }
    if(k < (*R)->key) 
        return insertRec(&((*R)->left), P, k);
    return insertRec(&((*R)->right), P, k);
}

bool BBTree::insertRec(int k, Aluno a){
    Node* P = new Node;
    P->key = k;
    P->A = a;
    P->left = P->right = 0;
    return insertRec(&root, P, k);
}

// ---------- busca ----------

Aluno* BBTree::search(int k){
    Node* R = root;
    while(R && R->key != k)
        R = (k < R->key) ? R->left : R->right;
    return R ? &R->A : nullptr;
}

int BBTree::size(){ 
    return n; 
}

// ---------- main ----------

int main(){
    BBTree T;
    Aluno a1 = {123, 8.5f}, a2 = {456, 7.0f}, a3 = {789, 9.0f};

    T.insert(2, a2);
    T.insert(1, a1);
    T.insertRec(3, a3);

    cout << "In-order: ";   T.inOrder();  cout << endl;
    cout << "Pre-order: ";  T.preOrder(); cout << endl;
    cout << "Post-order: "; T.posOrder(); cout << endl;
    cout << "n = " << T.size() << endl;

    Aluno* found = T.search(2);
    if(found) cout << "Found mat " << found->mat << " nota " << found->nota << endl;
    else      cout << "Not found" << endl;

    return 0;
}