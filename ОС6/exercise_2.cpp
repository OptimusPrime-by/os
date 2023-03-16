#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <mutex>

using namespace std;

#define MESSAGE_LENGTH 256
#define PERM 0777
#define QUIT_COMMAND ":q"

class fifotest
{
private:
    char* name1 = "fifo1";
    char* name2 = "fifo2";
    int ths_fd;
    int othr_fd;
    pthread_mutex_t mymutex;

    bool finished = false;

    void server()
    {
        this->ths_fd = open(name1, O_RDONLY | O_NONBLOCK);
        if (this->ths_fd == -1)
        {
            cerr << " ! ошибка open: " << strerror(errno) << endl;
            return;
        }
        do 
        {
            this->othr_fd = open(name2, O_RDONLY | O_NONBLOCK);
            usleep(1);
        } while (this->othr_fd == -1);
        this->othr_fd = open(name2, O_RDWR | O_NONBLOCK);
        if (this->othr_fd == -1)
        {
            cerr << " ! ошибка open для записи клиенту: " << strerror(errno) << endl;
            return;
        }

        pthread_t listening;
        if (pthread_create(&listening, NULL, thread_listen, (void*)this) != 0)
        {
            cerr << " ! ошибка pthread_create: " << strerror(errno) << endl;
            return;
        }

        string message;
        while (!this->finished)
        {
            cout << ">>> ";
            getline(cin, message, '\n');
            if (message == QUIT_COMMAND)
            {
                finished = true;
                break;
            }

            pthread_mutex_lock(&(this->mymutex));
            if (write(this->othr_fd, message.c_str(), MESSAGE_LENGTH) == -1)
            {
                cerr << " ! ошибка write: " << strerror(errno) << endl;
                pthread_mutex_unlock(&(this->mymutex));
                return;
            }
            pthread_mutex_unlock(&(this->mymutex));
        }

        pthread_join(listening, NULL);
    }

    void client()
    {
        this->ths_fd = open(name2, O_RDONLY | O_NONBLOCK);
        this->othr_fd = open(name1, O_RDONLY | O_NONBLOCK);
        if (this->ths_fd == -1 || this->othr_fd == -1)
        {
            cerr << " ! ошибка open: " << strerror(errno) << endl;
            return;
        }
        this->othr_fd = open(name1, O_RDWR | O_NONBLOCK);
        if (this->othr_fd == -1)
        {
            cerr << " ! ошибка open для записи серверу: " << strerror(errno) << endl;
            return;
        }

        pthread_t listening;
        if (pthread_create(&listening, NULL, thread_listen, (void*)this) != 0)
        {
            cerr << " ! ошибка pthread_create: " << strerror(errno) << endl;
            return;
        }

        string message;
        while (!this->finished)
        {
            cout << ">>> ";
            getline(cin, message, '\n');
            if (message == QUIT_COMMAND)
            {
                finished = true;
                break;
            }

            pthread_mutex_lock(&(this->mymutex));
            if (write(this->othr_fd, message.c_str(), MESSAGE_LENGTH) == -1)
            {
                cerr << " ! ошибка write: " << strerror(errno) << endl;
                pthread_mutex_unlock(&(this->mymutex));
                return;
            }
            pthread_mutex_unlock(&(this->mymutex));
        }

        pthread_join(listening, NULL);
    }

    static void* thread_listen(void* fftst)
    {
        fifotest* entity = (fifotest*)fftst;

        char buf[MESSAGE_LENGTH];
        while (!entity->finished)
        {
            pthread_mutex_lock(&(entity->mymutex));
            int rd = read(entity->ths_fd, buf, MESSAGE_LENGTH);
            if (rd > 0)
            {
                cout << " - " << buf << endl;
            }
            pthread_mutex_unlock(&(entity->mymutex));
            usleep(1);
        }
    }

public:
    fifotest()
    {
        pthread_mutex_init(&(this->mymutex), NULL);
        if (mkfifo(this->name1, PERM) != -1)
        { // сервер
            server();
        }
        else
        { // клиент
            if (errno != EEXIST)
            {
                cerr << " ! ошибка mkfifo: " << strerror(errno) << endl;
                return;                
            } 
            else if (mkfifo(this->name2, PERM) == -1)
            {
                cerr << " ! ошибка mkfifo у клиента: " << strerror(errno) << endl;
                return;  
            }

            client();
        }
    }

    ~fifotest()
    {
        close(this->ths_fd);
        close(this->othr_fd);
        pthread_mutex_destroy(&(this->mymutex));
        remove(name1);
        remove(name2);
    }
};

int main()
{
    cout << "запуск . . ." << endl;

    fifotest();

    return 0;
}
