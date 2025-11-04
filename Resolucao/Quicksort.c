int particao(int arr[], int low, int high) {

    // Initializar o valor escolhido como o primeiro elemento
    int p = arr[low];
    int i = low;
    int j = high;
    int temp;

    while (i < j) {

        // Encontrar o primeiro elemento maior que o escolhido
        while (arr[i] <= p && i <= high - 1) {
            i++;
        }

        // Encontrar o primeiro elemento menor que o escolhido
        while (arr[j] > p && j >= low + 1) {
            j--;
        }
        if (i < j) {
            temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    temp = arr[low];
    arr[low] = arr[j];
    arr[j] = temp;
    return j;
}

void quickSort(int arr[], int low, int high) {
    if (low < high) {

        // Chamada da função de partição para encontrar o índice de partição
        int pi = particao(arr, low, high);

        // Chamada Recursiva do quickSort() para esquerda e direita
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int main() {
  
    int arr[] = { 4, 2, 5, 3, 1 };
    int n = sizeof(arr) / sizeof(arr[0]);

    // Chamada do quickSort() para ordenar a array
    quickSort(arr, 0, n - 1);

    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);

    return 0;
}