#include<pthread.h>
#include<iostream>
#include<semaphore.h>
using namespace std;
#define N 25
//////////////////////
sem_t semaphore;
//////////////////////
int a[N];
// функция потока
void* ThreadFunc(void*){
    int pause;
    sem_wait(&semaphore);
    for(int i=0;i<N;i++){
        cout << " " << a[i] << " ";
        // замедляем вывод для наглядности
        for(long j=0;j<1000000;j++)
            pause++;
     }
    cout << endl;
    sem_post(&semaphore);
    return 0;
}

int main(void){
    ///////////////////////
    sem_init(&semaphore, 0, 1);
    ///////////////////////
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
    sem_wait(&semaphore);
    for(int i=0;i<N;i++){
        cout << " " << a[i] << " ";
        for(long j=0;j<1000000;j++)//10000000
            pause++;
    }
    cout <<endl;
    sem_post(&semaphore);
    // дожидаемся завершения работы потока
    pthread_join(Thread, NULL);
    sem_destroy(&semaphore);
    return 0;
}




