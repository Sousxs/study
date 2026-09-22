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
    void preOrderrec(TreeNode* root, vector<int> &x){
        if(root){
            x.push_back(root->val);
            preOrderrec(root->left, x);
            preOrderrec(root->right, x);
        }
    }

    vector<int> preorderTraversal(TreeNode* root) {
        vector<int> x;
        preOrderrec(root, x);
        return x;
    }
};