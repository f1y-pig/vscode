//在Linux平台上用多线程方法实现浮点向量的点积计算
#include <iostream>  
#include <vector>    
#include <thread>   
#include <chrono> 
using namespace std;
struct ThreadData {
    int start, end;//每一个线程负责的部分
    const vector<float> *a;//向量a的指针
    const vector<float> *b;//向量b的指针
    double partial_sum = 0.0;//每一个线程的结果
};//线程数据的结构
void compute_thread(ThreadData &data){
    double sum = 0.0;
    for (int i = data.start; i < data.end; ++i) {
        sum += (*data.a)[i] * (*data.b)[i];
    }
    data.partial_sum = sum;
}
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " thread_num N" << endl;
        return -1;
    }

    int thread_num = stoi(argv[1]);
    int N = stoi(argv[2]);

    if (thread_num < 1 || thread_num > 16 || N < 100000) {
        cout << "Invalid arguments: thread_num must be 1~16, N must be >=100000" << endl;
        return -1;
    }

    vector<float> a(N), b(N);
    // 初始化向量
    for (int i = 0; i < N; ++i) {
        if(i%3==0){
            a[i] = 1.0f;
            b[i] = 1.0f;

        }
        else if (i%3==1)
        {
            a[i] = -1.0f;
            b[i] = -1.0f;
        }
        else if (i%3==2)
        {
            a[i] = 0.0f;
            b[i] = 0.0f;
        }
    }

    vector<ThreadData> thread_data(thread_num);
    vector<thread> threads;

    // 计时开始
    auto start_time = chrono::high_resolution_clock::now();

    // 创建线程
    int chunk_size = N / thread_num;
    for (int i = 0; i < thread_num; ++i) {
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == thread_num - 1) ? N : (i + 1) * chunk_size;
        thread_data[i].a = &a;
        thread_data[i].b = &b;
        threads.emplace_back(compute_thread, ref(thread_data[i]));
    }

    // 等待线程结束
    for (auto &t : threads) {
        t.join();
    }

    // 计时结束
    auto end_time = chrono::high_resolution_clock::now();
    long long elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    // 汇总所有线程的结果
    double result = 0.0;
    for (const auto& data : thread_data) {
        result += data.partial_sum;
    }

    printf("s=%.2f t=%lld(ms)\n", result, elapsed_ms);

    return 0;
}