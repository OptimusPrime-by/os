#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
using namespace std;

int savages, pieces, maxPieces;
sem_t sem;
pthread_t cook;

void* cookFunc(void* arg) {
    cout << "cookin' dinner...\n";
    sleep(2);
    pieces += maxPieces;
    cout << "cook's back to sleeping'\n";

    return 0;
}

void* ThreadFunc(void* arg) {
    int savageNum = *(int*)arg;
    sem_wait(&sem);

    if (pieces == 0) {
        cout << "there's no meat. waiting for cook to finish his job...\n";
        if (pthread_create(&cook, NULL, cookFunc, NULL) != 0) {
            cerr << "thread creation err" << endl;
            cin.get();
            return 0;
        };
        pthread_join(cook, NULL);
        cout << "the fresh meat is ready to be eatten\n";
    }

    pieces--;
    cout << "savage " << savageNum + 1 << " is eatting...\n\n";
    sleep(1);
    
    sem_post(&sem);

    return 0;
}

int main() {
    cout << "savages: ";
    cin >> savages;
    cout << "max pieces: ";
    cin >> maxPieces;
    pieces = maxPieces;
    cout << endl;
    pthread_t tid[savages];

    sem_init(&sem, 0, 1);

    for (int i = 0; i < savages; i++) {
        int* a = new int;
        *a = i;

        if (pthread_create(&tid[i], NULL, ThreadFunc, a) != 0) {
            cerr << "thread creation err" << endl;
            cin.get();
            return 1;
        }
    }

    for (int i = 0; i < savages; i++) {
        if (pthread_join(tid[i], NULL) != 0) {
            cerr << "thread joining err" << endl;
            cin.get();
            return 1;
        }
    }

    

    cout << "savages are done eating the human flesh" << endl;
    sem_destroy(&sem);

    return 0;
}