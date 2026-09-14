// maior quantidade de caracteres iguais consecutivos
#include <bits/stdc++.h>
using namespace std;

void solve(string s)
{
    int n = 0;
    for(int i = 0; i < s.size(); i++)
    {
        int j = i;
        while(j < s.size() && s[j] == s[i])
            j++;
        n = max(n, j - i);
        i = j - 1;
    }    
    cout << n << endl;
}

int main(){
    cin.tie(nullptr)->sync_with_stdio(false);
    string s;
    cin >> s;

    solve(s);
    return 0;

}