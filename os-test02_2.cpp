#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
//
const int BUFFER_SIZE = 8;

struct DataItem {
    bool is_int;
    pthread_t tid;
    union {
        int number;
        char letter;
    } data;
};

struct SharedMemory {
    sem_t mutex;
    sem_t full;
    sem_t empty;
    sem_t io_lock;
    int in;
    int out;
    DataItem buffer[BUFFER_SIZE];
};

SharedMemory shm;

void* producerA(void*) {
    for (int i = 0; i < 10000; ++i) {
        sem_wait(&shm.empty);
        sem_wait(&shm.mutex);

        shm.buffer[shm.in].is_int = true;
        shm.buffer[shm.in].tid = pthread_self();
        shm.buffer[shm.in].data.number = rand() % 100000;
        shm.in = (shm.in + 1) % BUFFER_SIZE;

        sem_post(&shm.mutex);
        sem_post(&shm.full);
    }
    return nullptr;
}

void* producerB(void*) {
    bool used[26] = {false};
    int count = 0;
    while (count < 26) {
        char c = 'A' + rand() % 26;
        if (!used[c - 'A']) {
            used[c - 'A'] = true;

            sem_wait(&shm.empty);
            sem_wait(&shm.mutex);

            shm.buffer[shm.in].is_int = false;
            shm.buffer[shm.in].tid = pthread_self();
            shm.buffer[shm.in].data.letter = c;
            shm.in = (shm.in + 1) % BUFFER_SIZE;

            sem_post(&shm.mutex);
            sem_post(&shm.full);

            ++count;
        }
    }
    return nullptr;
}

void* consumerA(void*) {
    std::ofstream out("a_thread.out");
    int count = 0;
    while (count < 10000) {
        sem_wait(&shm.full);
        sem_wait(&shm.mutex);

        if (shm.buffer[shm.out].is_int) {
            DataItem item = shm.buffer[shm.out];
            shm.out = (shm.out + 1) % BUFFER_SIZE;
            ++count;

            sem_post(&shm.mutex);
            sem_post(&shm.empty);

            sem_wait(&shm.io_lock);
            out << "TID: " << item.tid << ", Number: " << item.data.number << "\n";
            sem_post(&shm.io_lock);
        } else {
            sem_post(&shm.mutex);
            sem_post(&shm.full);
            usleep(100);
        }
    }
    out.close();
    return nullptr;
}

void* consumerB(void*) {
    std::ofstream out("b_thread.out");
    int count = 0;
    while (count < 26) {
        sem_wait(&shm.full);
        sem_wait(&shm.mutex);

        if (!shm.buffer[shm.out].is_int) {
            DataItem item = shm.buffer[shm.out];
            shm.out = (shm.out + 1) % BUFFER_SIZE;
            ++count;

            sem_post(&shm.mutex);
            sem_post(&shm.empty);

            sem_wait(&shm.io_lock);
            out << "TID: " << item.tid << ", Letter: " << item.data.letter << "\n";
            sem_post(&shm.io_lock);
        } else {
            sem_post(&shm.mutex);
            sem_post(&shm.full);
            usleep(100);
        }
    }
    out.close();
    return nullptr;
}

int main() {
    srand(time(nullptr));

    shm.in = 0;
    shm.out = 0;
    sem_init(&shm.mutex, 0, 1);
    sem_init(&shm.full, 0, 0);
    sem_init(&shm.empty, 0, BUFFER_SIZE);
    sem_init(&shm.io_lock, 0, 1);

    pthread_t pA, pB, cA, cB;
    pthread_create(&pA, nullptr, producerA, nullptr);
    pthread_create(&pB, nullptr, producerB, nullptr);
    pthread_create(&cA, nullptr, consumerA, nullptr);
    pthread_create(&cB, nullptr, consumerB, nullptr);

    pthread_join(pA, nullptr);
    pthread_join(pB, nullptr);
    pthread_join(cA, nullptr);
    pthread_join(cB, nullptr);

    sem_destroy(&shm.mutex);
    sem_destroy(&shm.full);
    sem_destroy(&shm.empty);
    sem_destroy(&shm.io_lock);

    return 0;
}
