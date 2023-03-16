#include <iostream>
#include<pthread.h>
#include <time.h>
#include<signal.h>

using namespace std;
int **arr;
int *minInRow;
int m, n;

void* findMinInRow(void* argg){
    int arg = (intptr_t)argg;
    int temp = arr[arg][0];
    for(int i = 1; i < n; i++){

        if(arr[arg][i] < temp){
        temp = arr[arg][i];
        }
        }
        minInRow[arg] = temp;
}
int main()
{
    cout << "input m, n" << endl;
    cin >> m >> n;
    // ????? m < n or m > n
    if(m < 0 || n < 0 || m > 10000 || n > 10000){
        cerr << "incorrect input" << endl;
        return -1;
    }
    arr = new int*[m];
    for(int i = 0; i<m; i++){
        arr[i] = new int[n];
    }
    minInRow = new int[m];
    for(int i = 0; i<m; i++){
        for(int j = 0; j<n; j++){
            arr[i][j] = rand() % 10;
        }
    }
    for(int i = 0; i < m; i++){
        for(int j = 0; j < n; j++){
            cout << arr[i][j] << " ";
        }
        cout << endl;
    }
    pthread_t *t = new pthread_t[m];
    void*** status = new void**[m];
    int err;
    //String m = "";
    for(int i = 0; i<m; i++){
        err=pthread_create(&t[i],NULL, findMinInRow, (void*)(intptr_t)i);
        if(err!=0){
            cout << "Ошибка №"<<err << endl;
        }
    }
    for(int i = 0; i<m; i++){
        pthread_join(t[i], status[i]);
    }
    for(int i = 0; i < m; i++){
        cout << minInRow[i] << " ";
    }
    getchar();
    return 0;
}
