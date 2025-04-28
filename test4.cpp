#include <iostream>
#include <thread>
#include <vector>
#include <deque>
#include <algorithm> 
using namespace std;

struct process {
    int* PCB; // PCB号
    double request_time; // 要求运行时间
    double arrive_time;  // 到达时间
    process* ptr;        // 指向下一个进程
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

    double current_time = 0; // 当前总时间

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

            if (current_time < work_process.arrive_time) {
                // 等待到达
                current_time = work_process.arrive_time;
            }

            if (work_process.request_time <= time_piece) {
                current_time += work_process.request_time;
                cout << "进程" << *(work_process.PCB) 
                     << " 执行完成，用时：" << work_process.request_time
                     << "，当前总时间：" << current_time << endl;
            } else {
                cout << "进程" << *(work_process.PCB)
                     << " 运行时间片：" << time_piece
                     << "，剩余时间：" << work_process.request_time - time_piece
                     << "，当前总时间：" << current_time + time_piece << endl;
                work_process.request_time -= time_piece;
                current_time += time_piece;
                process_queue.push_back(work_process); // 剩余进程回队尾
            }
        }
    } 
    else if (algorithm == 2) { 
        // --- 短作业优先（SJF）---
        cout << "采用SJF算法(短作业优先）：" << endl;
        vector<process> ready_queue;
        vector<process> waiting_queue = Process; // 初始化所有进程都在等待队列

        while (!waiting_queue.empty() || !ready_queue.empty()) {
            // 把到达的进程放到ready_queue
            for (auto it = waiting_queue.begin(); it != waiting_queue.end();) {
                if (it->arrive_time <= current_time) {
                    ready_queue.push_back(*it);
                    it = waiting_queue.erase(it);
                } else {
                    ++it;
                }
            }

            if (ready_queue.empty()) {
                // 当前没有可运行的进程，时间推进到下一个到达的进程
                double next_arrive = waiting_queue.front().arrive_time;
                for (auto& p : waiting_queue) {
                    if (p.arrive_time < next_arrive) {
                        next_arrive = p.arrive_time;
                    }
                }
                current_time = next_arrive;
                continue;
            }

            // 选择请求时间最短的
            auto shortest = min_element(ready_queue.begin(), ready_queue.end(), [](const process& a, const process& b) {
                return a.request_time < b.request_time;
            });

            process work_process = *shortest;
            ready_queue.erase(shortest);

            cout << "执行进程" << *(work_process.PCB)
                 << "，运行时间：" << work_process.request_time
                 << "，开始时间：" << current_time
                 << "，完成时间：" << current_time + work_process.request_time
                 << endl;

            current_time += work_process.request_time;
        }
    }

    // 释放内存
    for (auto& p : Process) {
        delete p.PCB;
    }

    return 0;
}
