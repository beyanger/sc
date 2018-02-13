
/*
 
	space complexity: O(1)
	time complexity: O(NlogN)
	not stable
 */
#include <iostream>
#include <vector>
using namespace std;

void HeapSort(vector<int>& arr, int size) {
	if (size <= 1) return;

	for (int i = size/2; i >= 0; i--) {
		int l = i * 2;
		int r = i * 2 + 1;

		if (r < size && arr[i] < arr[r]) {
			swap(arr[i], arr[r]);	
		}

		if (l < size && arr[i] < arr[l]) {
			swap(arr[i], arr[l]);
		}
	}
	swap(arr[0], arr[size-1]);
	HeapSort(arr, size-1);
}

int main(int argc, char *argv[])
{
	vector<int> a = {0,16,20,3,11,17,8};
	HeapSort(a, a.size());
	cout << endl;
	for (int i = 0;i < a.size(); i++)
		cout << a[i] << " ";
	cout << endl;
	return 0;
}

