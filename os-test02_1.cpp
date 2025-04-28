#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>

const int BUFFER_SIZE = 8;

struct DataItem {
    bool is_int;
    pid_t pid;
    union {
        int number;
        char letter;
    } data;
};

struct SharedMemory {
    sem_t mutex;
    sem_t full;
    sem_t empty;
    sem_t io_lock; // 新增：写文件互斥锁
    int in;
    int out;
    DataItem buffer[BUFFER_SIZE];
};

#define SHM_SIZE sizeof(SharedMemory)

void producerA(SharedMemory* shm) {
    for (int i = 0; i < 10000; ++i) {
        sem_wait(&shm->empty);
        sem_wait(&shm->mutex);

        shm->buffer[shm->in].is_int = true;
        shm->buffer[shm->in].pid = getpid();
        shm->buffer[shm->in].data.number = rand() % 100000;
        shm->in = (shm->in + 1) % BUFFER_SIZE;

        sem_post(&shm->mutex);
        sem_post(&shm->full);
    }
    exit(0);
}

void producerB(SharedMemory* shm) {
    bool used[26] = {false};
    int count = 0;
    while (count < 26) {
        char c = 'A' + rand() % 26;
        if (!used[c - 'A']) {
            used[c - 'A'] = true;

            sem_wait(&shm->empty);
            sem_wait(&shm->mutex);

            shm->buffer[shm->in].is_int = false;
            shm->buffer[shm->in].pid = getpid();
            shm->buffer[shm->in].data.letter = c;
            shm->in = (shm->in + 1) % BUFFER_SIZE;

            sem_post(&shm->mutex);
            sem_post(&shm->full);

            ++count;
        }
    }
    exit(0);
}

void consumerA(SharedMemory* shm) {
    std::ofstream out("a.out");
    int count = 0;
    while (count < 10000) {
        sem_wait(&shm->full);
        sem_wait(&shm->mutex);

        if (shm->buffer[shm->out].is_int) {
            DataItem item = shm->buffer[shm->out];
            shm->out = (shm->out + 1) % BUFFER_SIZE;
            ++count;

            sem_post(&shm->mutex);
            sem_post(&shm->empty);

            // 写文件部分互斥
            sem_wait(&shm->io_lock);
            out << "PID: " << item.pid << ", Number: " << item.data.number << "\n";
            sem_post(&shm->io_lock);

        } else {
            sem_post(&shm->mutex);
            sem_post(&shm->full);
            usleep(100);
        }
    }
    out.close();
    exit(0);
}

void consumerB(SharedMemory* shm) {
    std::ofstream out("b.out");
    int count = 0;
    while (count < 26) {
        sem_wait(&shm->full);
        sem_wait(&shm->mutex);

        if (!shm->buffer[shm->out].is_int) {
            DataItem item = shm->buffer[shm->out];
            shm->out = (shm->out + 1) % BUFFER_SIZE;
            ++count;

            sem_post(&shm->mutex);
            sem_post(&shm->empty);

            // 写文件部分互斥
            sem_wait(&shm->io_lock);
            out << "PID: " << item.pid << ", Letter: " << item.data.letter << "\n";
            sem_post(&shm->io_lock);

        } else {
            sem_post(&shm->mutex);
            sem_post(&shm->full);
            usleep(100);
        }
    }
    out.close();
    exit(0);
}

int main() {
    srand(time(nullptr));

    int shmid = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    SharedMemory* shm = (SharedMemory*)shmat(shmid, nullptr, 0);

    shm->in = 0;
    shm->out = 0;
    sem_init(&shm->mutex, 1, 1);
    sem_init(&shm->full, 1, 0);
    sem_init(&shm->empty, 1, BUFFER_SIZE);
    sem_init(&shm->io_lock, 1, 1); // 初始化输出互斥锁

    if (fork() == 0) producerA(shm);
    if (fork() == 0) producerB(shm);
    if (fork() == 0) consumerA(shm);
    if (fork() == 0) consumerB(shm);

    for (int i = 0; i < 4; ++i) wait(nullptr);

    sem_destroy(&shm->mutex);
    sem_destroy(&shm->full);
    sem_destroy(&shm->empty);
    sem_destroy(&shm->io_lock);
    shmdt(shm);
    shmctl(shmid, IPC_RMID, nullptr);

    return 0;
}
