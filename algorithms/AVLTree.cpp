#include <bits/stdc++.h>

using namespace std;

class Trem {
    int valor;
};

class Node {
public:
    int key;
    Trem T;
    Node* left;
    Node* right;
    int fb; // fator de balanceamento
};

class AVLTree {
    private:
        Node* root;
        int n;

        //percursos recursivos
        void inOrder(Node* R);
        void preOrder(Node* R);
        void posOrder(Node* R);

        //rotacoes
        void RSD(Node** R);
        void RSE(Node** R);
        void RDD(Node** R);
        void RDE(Node** R);

        //funcoes auxiliares
        static bool insertRec(Node** R, Node* P, int k);
        static bool remove(Node** R, int k);

        int size(Node* R);

    public:
        AVLTree();
        ~AVLTree();
};

void AVLTree::inOrder(Node* R) {
    if (R) {
        inOrder(R->left);
        cout << R->key << " ";
        inOrder(R->right);
    }
}

void AVLTree::preOrder(Node* R) {
    if (R) {
        cout << R->key << " ";
        preOrder(R->left);
        preOrder(R->right);
    }
}

void AVLTree::posOrder(Node* R) {
    if (R) {
        posOrder(R->left);
        posOrder(R->right);
        cout << R->key << " ";
    }
}

void AVLTree::RSD(Node** R) {
    Node* a = *R;
    Node* b = a->left;
    a->left = b->right;
    b->right = a;
    *R = b;

    if(b->fb == -1){
        a->fb = b->fb = 0;
    } else {
        a->fb = -1;
        b->fb = 1;
    }
}

void AVLTree::RSE(Node** R) {
    Node* a = *R;
    Node* b = a->right;
    a->right = b->left;
    b->left = a;
    *R = b;

    if(b->fb == 1){
        a->fb = b->fb = 0;
    } else {
        a->fb = 1;
        b->fb = -1;
    }
}

void AVLTree::RDD(Node** R) {
    Node* a = *R;
    Node* b = a->left;
    Node* c = b->right;

    a->left = c->right;
    b->right = c->left;
    c->left = b;
    c->right = a;
    *R = c;

    if(c->fb == -1){
        a->fb = 1;
        b->fb = 0;
    } else if(c->fb == 1){
        a->fb = 0;
        b->fb = -1;
    } else {
        a->fb = b->fb = 0;
    }
}

void AVLTree::RDE(Node** R) {
    Node* a = *R;
    Node* b = a->right;
    Node* c = b->left;

    a->right = c->left;
    b->left = c->right;
    c->right = b;
    c->left = a;
    *R = c;

    if(c->fb == 1){
        a->fb = -1;
        b->fb = 0;
    } else if(c->fb == -1){
        a->fb = 0;
        b->fb = 1;
    } else {
        a->fb = b->fb = 0;
    }
}

static bool AVLTree::insertRec(Node** R, Node* P, int k) {
    if(!(*R)){
        *R = P;
        return true;
    }
    if(k < (*R)->key){
        if(insertRec(&((*R)->left), P, k)){
            return false;
        }
        if((*R)->fb == 1){
            (*R)->fb = 0;
            return false;
        }
        if((*R)->fb == 0){
            (*R)->fb = -1;
            return true;
        }
        if(((*R)->left)->fb == -1){
            AVLTree::RSD(R);
            return false;
        }
        AVLTree::RDD(R);
        return false;
    }
    if(k > (*R)->key){
        if(insertRec(&((*R)->right), P, k)){
            return false;
        }
        if((*R)->fb == -1){
            (*R)->fb = 0;
            return false;
        }
        if((*R)->fb == 0){
            (*R)->fb = 1;
            return true;
        }
        if(((*R)->right)->fb == 1){
            AVLTree::RSE(R);
            return false;
        }
        AVLTree::RDE(R);
        return false;
}