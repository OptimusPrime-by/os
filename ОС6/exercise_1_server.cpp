#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdio.h>  
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

#include "message.h"

using namespace std;

int shm_fd = 0;
char* shm_ptr;
bool finished = false;
sem_t* sem;

void* read_message(void*)
{
    while (!finished)
    {
        // --- TODO синхронизировать ---
        if (sem_wait(sem) != 0)
        {
            cerr << " ! ошибка sem_wait: " << strerror(errno) << endl;
        }

        if (((message*)shm_ptr)->status == MUST_READ && ((message*)shm_ptr)->recipient == SERVER)
        {
            cout << " - ";
            for (int i = 0; i < MESSAGE_CONTENT_LENGTH; i++)
            {
                cout << ((message*)shm_ptr)->content[i];
            }
            cout << endl;

            ((message*)shm_ptr)->status = ALREADY_READ;
        }

        if (sem_post(sem) != 0)
        {
            cerr << " ! ошибка sem_post: " << strerror(errno) << endl;
        }
        // -----------------------------
        usleep(0);
    }
}

int main()
{
    cout << "СЕРВЕР" << endl << endl;

    // создание именованного семафора
    sem_unlink(SEM_NAME); // если вдруг такой семфор существовал
    sem = sem_open(SEM_NAME, O_CREAT, PERM, 1);
    if (sem == SEM_FAILED)
    {
        cerr << " ! ошибка sem_open: " << strerror(errno) << endl;
        return -1;
    }
    else
    {
        int value;
        sem_getvalue(sem, &value);
        printf(" * значение семафора: %d\n", value);
    }

    // создание области разделяемой памяти
    shm_fd = shm_open(SHM_NAME, O_RDWR // <-- с O_RDONLY почему-то не работает
             | O_TRUNC /* если уже сущ-ет, то обнулить размер */ | O_CREAT, PERM);
    if (shm_fd == -1)
    {
        cerr << " ! ошибка shm_open: " << strerror(errno) << endl;
        return -1;
    }
    else cout << " - область памяти создана" << endl;

    // задаем размер области
    ftruncate(shm_fd, SHM_SIZE);

    shm_ptr = (char*)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        cerr << " ! ошибка mmap: " << strerror(errno) << endl;
        return -1;
    }

    pthread_t reading_thread;
    if (pthread_create(&reading_thread, NULL, read_message, NULL) != 0)
    {
        cerr << " ! ошибка pthread_create: " << strerror(errno) << endl;
        return -1;
    }

    ((message*)shm_ptr)->status = ALREADY_READ;

    string input;
    while (!finished)
    {
        cout << ">>> ";
        getline(cin, input, '\n');
        if (input == EXIT_COMMAND)
        {
            finished = true;
            continue;
        }
        size_t left_to_send = input.size();
        
        while (left_to_send > 0)
        {
            // --- TODO синхронизировать ---
            if (sem_wait(sem) != 0)
            {
                cerr << " ! ошибка sem_wait: " << strerror(errno) << endl;
            }

            if (((message*)shm_ptr)->status == ALREADY_READ)
            {
                memset(shm_ptr, 0, SHM_SIZE);
                strncpy(shm_ptr + sizeof(message_status) + sizeof(message_recipient),
                    input.c_str() + input.size() - left_to_send, (left_to_send <= MESSAGE_CONTENT_LENGTH) ? left_to_send : MESSAGE_CONTENT_LENGTH);
                ((message*)shm_ptr)->status = MUST_READ;
                ((message*)shm_ptr)->recipient = CLIENT;

                left_to_send = (left_to_send <= MESSAGE_CONTENT_LENGTH) ? 0 : left_to_send - MESSAGE_CONTENT_LENGTH;
            }

            if (sem_post(sem) != 0)
            {
                cerr << " ! ошибка sem_post: " << strerror(errno) << endl;
            }
            // -----------------------------

            usleep(0);
        }
    }

    pthread_join(reading_thread, NULL);

    if (munmap(shm_ptr, SHM_SIZE) == -1)
    {
        cerr << " ! ошибка munmap: " << strerror(errno) << endl;
        return -1;
    }
    close(shm_fd);

    // закрыти области
    if (shm_unlink(SHM_NAME) == -1)
    {
        cerr << " ! ошибка shm_unlink: " << strerror(errno) << endl;
        return -1;
    }
    else cout << " - область памяти удалена" << endl;

    // удаление семафора
    if (sem_close(sem) != 0)
    {
        cerr << " ! ошибка sem_close: " << strerror(errno) << endl;
        return -1;
    }
    if (sem_unlink(SEM_NAME) != 0)
    {
        cerr << " ! ошибка sem_unlink: " << strerror(errno) << endl;
        return -1;
    }

    return 0;
}