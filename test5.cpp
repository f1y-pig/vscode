#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <set>
#include <map>
using namespace std;

// 生成320条指令序列
vector<int> generateInstructions() {
    vector<int> instructions;
    srand(time(0)); // 初始化随机数种子

    int count = 0;
    while (count < 320) {
        int m = rand() % 320;
        instructions.push_back(m);
        count++;
        if (count >= 320) break;

        instructions.push_back((m + 1) % 320);
        count++;
        if (count >= 320) break;

        int m_prime = rand() % (m + 2); // [0, m+1]
        instructions.push_back(m_prime);
        count++;
        if (count >= 320) break;

        instructions.push_back((m_prime + 1) % 320);
        count++;
        if (count >= 320) break;

        int m_second = m_prime + 2 + rand() % (320 - (m_prime + 2)); // [m'+2, 319]
        instructions.push_back(m_second);
        count++;
    }
    return instructions;
}

// 指令地址转换成页号（每10条指令一页）
vector<int> convertToPages(const vector<int>& instructions) {
    vector<int> pages;
    for (int addr : instructions) {
        pages.push_back(addr / 10);
    }
    return pages;
}

// FIFO置换算法
double simulateFIFO(const vector<int>& pages, int frames) {
    set<int> memory;
    deque<int> pageQueue;
    int miss = 0;

    for (int page : pages) {
        if (memory.find(page) != memory.end()) {
            // 命中
            continue;
        }
        miss++;
        if (memory.size() >= frames) {
            int victim = pageQueue.front();
            pageQueue.pop_front();
            memory.erase(victim);
        }
        memory.insert(page);
        pageQueue.push_back(page);
    }
    return 1 - (double)miss / pages.size();
}

// LRU置换算法
double simulateLRU(const vector<int>& pages, int frames) {
    set<int> memory;
    map<int, int> lastUsed; // page -> last used time
    int miss = 0;
    int time = 0;

    for (int page : pages) {
        if (memory.find(page) != memory.end()) {
            // 命中
            lastUsed[page] = time;
        } else {
            miss++;
            if (memory.size() >= frames) {
                // 找到最久未使用的页面
                int lruPage = -1, lruTime = 1e9;
                for (int p : memory) {
                    if (lastUsed[p] < lruTime) {
                        lruTime = lastUsed[p];
                        lruPage = p;
                    }
                }
                memory.erase(lruPage);
                lastUsed.erase(lruPage);
            }
            memory.insert(page);
            lastUsed[page] = time;
        }
        time++;
    }
    return 1 - (double)miss / pages.size();
}

// OPT置换算法
double simulateOPT(const vector<int>& pages, int frames) {
    set<int> memory;
    int miss = 0;

    for (size_t i = 0; i < pages.size(); i++) {
        int page = pages[i];
        if (memory.find(page) != memory.end()) {
            continue;
        }
        miss++;
        if (memory.size() >= frames) {
            // 预测未来
            map<int, int> nextUse;
            for (int p : memory) {
                nextUse[p] = 1e9; // 默认未来不会再用了
                for (size_t j = i + 1; j < pages.size(); j++) {
                    if (pages[j] == p) {
                        nextUse[p] = j;
                        break;
                    }
                }
            }
            // 找最晚使用的页
            int farthestPage = -1, farthestTime = -1;
            for (auto& [p, t] : nextUse) {
                if (t > farthestTime) {
                    farthestTime = t;
                    farthestPage = p;
                }
            }
            memory.erase(farthestPage);
        }
        memory.insert(page);
    }
    return 1 - (double)miss / pages.size();
}

// NRU置换算法（简单模拟版）
double simulateNRU(const vector<int>& pages, int frames) {
    set<int> memory;
    map<int, bool> referenced; // 页是否被引用
    int miss = 0;
    int clock = 0;

    for (int page : pages) {
        if (memory.find(page) != memory.end()) {
            referenced[page] = true;
        } else {
            miss++;
            if (memory.size() >= frames) {
                // 找第一个referenced为false的页
                int victim = -1;
                for (int p : memory) {
                    if (!referenced[p]) {
                        victim = p;
                        break;
                    }
                }
                if (victim == -1) {
                    // 如果都被引用了，随便找一个
                    victim = *memory.begin();
                }
                memory.erase(victim);
                referenced.erase(victim);
            }
            memory.insert(page);
            referenced[page] = true;
        }

        // 定期清除引用位，模拟“最近未使用”
        clock++;
        if (clock % 50 == 0) {
            for (auto& [p, ref] : referenced) {
                ref = false;
            }
        }
    }
    return 1 - (double)miss / pages.size();
}

int main() {
    auto instructions = generateInstructions();
    auto pages = convertToPages(instructions);

    vector<int> frameSizes = {4, 8, 12, 16, 24, 32};

    for (int frames : frameSizes) {
        cout << "内存页数：" << frames << endl;
        cout << "FIFO命中率: " << simulateFIFO(pages, frames) << endl;
        cout << "LRU命中率:  " << simulateLRU(pages, frames) << endl;
        cout << "OPT命中率:  " << simulateOPT(pages, frames) << endl;
        cout << "NRU命中率:  " << simulateNRU(pages, frames) << endl;
        cout << "----------------------------------" << endl;
    }

    return 0;
}
