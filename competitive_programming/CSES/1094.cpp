// increasing array with bubble sort count number of moves
// input: 
// 10
// 1000000000 1 1 1 1 1 1 1 1 1
// output: 
// 8999999991


#include <bits/stdc++.h>
using namespace std;
void solve(vector<int> x){
    long long n = 0;
    for(int i = 0; i < x.size(); i++)
        for(int j = 0; j < x.size()-1; j++)
            if(x[j] > x[j+1]){
                swap(x[j], x[j+1]);
                n++;
            }
    cout << n << endl;

}

int main(){
    cin.tie(nullptr)->sync_with_stdio(false);
    int n; cin >> n;
    vector<int> a(n);
    for(int i = 0; i < n; i++)
        cin >> a[i];
    solve(a);
    return 0;    
}