#include<pthread.h>
#include<iostream>
using namespace std;
#define N 25
//////////////////////
pthread_mutex_t mutex;
//////////////////////
int a[N];
// функция потока
void* ThreadFunc(void*){
int pause;
pthread_mutex_lock(&mutex);
for(int i=0;i<N;i++){
    cout << " " << a[i] << " ";
// замедляем вывод для наглядности
for(long j=0;j<1000000;j++)
    pause++;
 }
cout << endl;
pthread_mutex_unlock(&mutex);
return 0;
}

int main(void){
    pthread_t Thread;
    int pause;
    int res;
    // заполняем глобальный массив
    for(int i=0;i<N;i++)
        a[i]=i;
    res = pthread_create(&Thread, NULL, ThreadFunc, NULL);
    if(res!= 0){
        cerr << "Ошибка при создании потока" << endl;
        cin.get();
        return 0;
    }
    pthread_mutex_lock(&mutex);
    for(int i=0;i<N;i++){
        cout << " " << a[i] << " ";
        for(long j=0;j<1000000;j++)//10000000
            pause++;
    }
    cout <<endl;
    pthread_mutex_unlock(&mutex);
    // дожидаемся завершения работы потока
    pthread_join(Thread, NULL);
    return 0;
}
