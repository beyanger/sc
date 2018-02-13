
/*
	space complexity: O(1)
	time complexity: 
		avg:   O(NlogN)

	not stable
 */
static int partition(int *arr, int low, int high)
{
	int pivot = arr[low];

	while (low < high) {
		while (low < high && arr[high] >= pivot) 
			--high;
		arr[low] = arr[high];
		while (low < high && arr[low] <= pivot)
			++low;
		arr[high] = arr[low];
	}
	arr[low] = pivot;
	return low;
}



void qsort(int *arr, int low, int high)
{
	if (low < high) {
		int q = partition(arr, low, high);
		qsort(arr, low, q-1);
		qsort(arr, q+1, high);
	}
}


int main() {
	int arr[] = {0, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	qsort(arr, 0, sizeof(arr)/sizeof(arr[0])-1);
	return 0;

}

