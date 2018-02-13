
#include <iostream>

using namespace std;


int Sunday(const string& T, const string& P) {
	int m = T.length(), n = P.length();
	int shift[256];


	for (int i = 0; i < 256; i++)
		shift[i] = m+1;

	for (int i = 0; i < m; i++) 
		shift[(unsigned char)P[i]] = m - i;

	for (int s = 0; s <= m-n; s += shift[(unsigned char)T[s+m]]) {
		for (int j = 0; T[s+j] == P[j]; ) {
			if (++j >= n) {
				return s;
			}
		}

	}
	return -1;
}

int main() {
	string T, P;
	while (true) {
		getline(cin, T);
		getline(cin, P);
		int res = Sunday(T, P);
		cout << res << endl;
	}
	return 0;
}
