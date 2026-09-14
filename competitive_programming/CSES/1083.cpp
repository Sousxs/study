//missing number
#include <bits/stdc++.h>

using namespace std;

void solve(vector<int> x){
    int n = x.size();
    vector<int> v(n+1, 0);
    for(int i = 0; i < n-1; i++)
        v[x[i]]++;
    for(int i = 1; i <= n; i++)
        if(v[i] == 0)
            cout << i << endl;
}

int main(){
    cin.tie(nullptr)->sync_with_stdio(false);
    int n; cin >> n;
    vector<int> a(n);
    for(int i = 0; i < n-1; i++)
        cin >> a[i];

    solve(a);
    return 0;
}