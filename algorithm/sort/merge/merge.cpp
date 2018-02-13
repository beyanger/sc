

/*
	space complexity: O(N)
	time complexity: O(NlogN)
	not stable
 
 */

#include <vector>
using namespace std;

void merge(vector<int>& arr, int left, int mid, int right, vector<int>& t) {
	int i = left, j = mid+1, k = 0;

	while (i <= mid && j <= right) {
		if (arr[i] <= arr[j])
			t[k++] = arr[i++];
		else 
			t[k++] = arr[j++];
	}

	while (i <= mid)
		t[k++] = arr[i++];
	while (j <= right)
		t[k++] = arr[j++];
	k = 0;
	while (left <= right)
		arr[left++] = t[k++];
}

void sort(vector<int>& arr, int left, int right, vector<int>& t) {
	if (left < right) {
		int mid = (left + right) / 2;
		sort(arr, left, mid, t);
		sort(arr, mid+1, right, t);
		merge(arr, left, mid, right, t);
	}
}


int main() {

	return 0;
}
