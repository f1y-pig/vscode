#include <iostream>
#include <thread>
#include <vector>
#include <deque>
#include <algorithm> 
using namespace std;

struct process {
    int* PCB; // PCB
    double request_time; // 要求运行时间
    double arrive_time; // 到达时间
    process* ptr; // 指向下一个进程
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "参数输入有误" << endl;
        return 1;
    }
    int algorithm = atoi(argv[1]);
    if (algorithm != 1 && algorithm != 2) {
        cerr << "算法选择有误" << endl;
        return 2;
    }

    vector<process> Process(4);
    double arrivetime[] = {0, 2, 4, 5};
    double requesttime[] = {5, 4, 1, 6};

    for (int i = 0; i < Process.size(); i++) {
        Process[i].PCB = new int(i);
        Process[i].arrive_time = arrivetime[i];
        Process[i].request_time = requesttime[i];
    }

    for (int i = 0; i < 4; i++) {
        Process[i].ptr = &Process[(i + 1) % 4];
    }

    if (algorithm == 1) { 
        // --- 时间片轮转（Round Robin）---
        cout << "采用RR算法(时间片轮转):" << endl;
        int time_piece = 2; // 时间片长度
        deque<process> process_queue;
        for (auto& p : Process) {
            process_queue.push_back(p); // 全部进程入队
        }

        while (!process_queue.empty()) {
            process work_process = process_queue.front();
            process_queue.pop_front();

            if (work_process.request_time <= time_piece) {
                cout << "进程" << *(work_process.PCB) << "执行完毕" << endl;
            } else {
                cout << "进程" << *(work_process.PCB) << "运行一个时间片" << endl;
                work_process.request_time -= time_piece;
                process_queue.push_back(work_process);
            }
        }
    } 
    else if (algorithm == 2) {
        cout << "采用SJF算法(短作业优先）：" << endl;
        vector<process> ready_queue;
        for (auto& p : Process) {
            ready_queue.push_back(p);
        }

        while (!ready_queue.empty()) {
            // 找最短作业
            sort(ready_queue.begin(), ready_queue.end(), [](const process& a, const process& b) {
                return a.request_time < b.request_time;
            });

            process work_process = ready_queue.front();
            ready_queue.erase(ready_queue.begin()); // 移除执行的进程

            cout << "执行进程" << *(work_process.PCB) 
                 << "，运行时间：" << work_process.request_time << endl;
        }
    }

    // 释放内存
    for (auto& p : Process) {
        delete p.PCB;
    }

    return 0;
}
