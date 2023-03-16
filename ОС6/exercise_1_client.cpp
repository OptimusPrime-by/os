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

        if (((message*)shm_ptr)->status == MUST_READ && ((message*)shm_ptr)->recipient == CLIENT)
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
    cout << "КЛИЕНТ" << endl << endl;

    // отркытие именованного семафора
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

    shm_fd = shm_open(SHM_NAME, O_RDWR /* <-- с O_RDONLY почему-то не работает */, PERM);
    if (shm_fd == -1)
    {
        cerr << " ! ошибка shm_open: " << strerror(errno) << endl;
        return -1;
    }
    else cout << " - область памяти открыта" << endl;

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
                ((message*)shm_ptr)->recipient = SERVER;

                left_to_send = (left_to_send <= MESSAGE_CONTENT_LENGTH) ? 0 : left_to_send - MESSAGE_CONTENT_LENGTH;
            }

            if (sem_post(sem) != 0)
            {
                cerr << " ! ошибка sem_post: " << strerror(errno) << endl;
            }
            // -----------------------------
        }
    }

    pthread_join(reading_thread, NULL);

    if (munmap(shm_ptr, SHM_SIZE) == -1)
    {
        cerr << " ! ошибка munmap: " << strerror(errno) << endl;
        return -1;
    }
    close(shm_fd);

    return 0;
}