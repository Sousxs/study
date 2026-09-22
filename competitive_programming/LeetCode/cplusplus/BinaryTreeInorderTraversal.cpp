/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode() : val(0), left(nullptr), right(nullptr) {}
 *     TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
 *     TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
 * };
 */
class Solution {
public:

    void inOrderRec(TreeNode* root, vector<int> &x){
        if(root){
            inOrderRec(root->left, x);
            x.push_back(root->val);
            inOrderRec(root->right, x);
        }
    }
    vector<int> inorderTraversal(TreeNode* root) {
        vector<int> x;
        inOrderRec(root, x);
        return x;
    }
};