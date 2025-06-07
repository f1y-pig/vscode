#include <iostream>
#include <vector>
#include <tuple>

using namespace std;

class DiskBitmapManager {
private:
    const int heads = 4;
    const int cylinders = 40;
    const int sectors = 8;
    const int total_blocks = heads * cylinders * sectors;

    vector<int> bitmap;  // 位示图，1表示占用，0表示空闲

public:
    DiskBitmapManager() {
        bitmap.resize(total_blocks, 0); // 初始化为全空闲
    }

    // 打印位示图状态，每32个块一行
    void printBitmap() {
        cout << "\n当前位示图状态 (1=占用, 0=空闲):" << endl;
        for (int i = 0; i < total_blocks; ++i) {
            if (i % 32 == 0) cout << "块[" << i / 32 << "]: ";
            cout << bitmap[i] << " ";
            if ((i + 1) % 32 == 0) cout << endl;
        }
        cout << endl;
    }

    // 将位示图的索引转换为物理地址（柱面，磁头，扇区）
    tuple<int, int, int> indexToAddress(int index) {
        int cylinder = index / (heads * sectors);
        int remainder = index % (heads * sectors);
        int head = remainder / sectors;
        int sector = remainder % sectors;
        return make_tuple(cylinder, head, sector);
    }

    // 将物理地址转换为位示图索引
    int addressToIndex(int cylinder, int head, int sector) {
        return cylinder * heads * sectors + head * sectors + sector;
    }

    // 分配num_blocks个磁盘块，返回分配成功的物理地址列表
    vector<tuple<int, int, int>> allocate(int num_blocks) {
        vector<tuple<int, int, int>> allocated;
        for (int i = 0; i < total_blocks && (int)allocated.size() < num_blocks; ++i) {
            if (bitmap[i] == 0) {
                bitmap[i] = 1;
                allocated.push_back(indexToAddress(i));
            }
        }

        if ((int)allocated.size() < num_blocks) {
            cout << "空间不足，回滚已分配的块。" << endl;
            for (const auto& addr : allocated) {
                int idx = addressToIndex(get<0>(addr), get<1>(addr), get<2>(addr));
                bitmap[idx] = 0;
            }
            allocated.clear();
        } else {
            cout << "成功分配 " << num_blocks << " 个磁盘块:" << endl;
            for (const auto& addr : allocated) {
                cout << "  -> 柱面=" << get<0>(addr)
                     << ", 磁头=" << get<1>(addr)
                     << ", 扇区=" << get<2>(addr) << endl;
            }
            printBitmap();
        }

        return allocated;
    }

    // 释放指定的磁盘块，输入为物理地址列表
    void release(const vector<tuple<int, int, int>>& blocks) {
        bool any_released = false;
        for (const auto& addr : blocks) {
            int cyl = get<0>(addr);
            int head = get<1>(addr);
            int sec = get<2>(addr);

            // 检查地址合法性
            if (cyl < 0 || cyl >= cylinders || head < 0 || head >= heads || sec < 0 || sec >= sectors) {
                cout << "无效的地址: (" << cyl << ", " << head << ", " << sec << ")，跳过。" << endl;
                continue;
            }

            int idx = addressToIndex(cyl, head, sec);
            if (bitmap[idx] == 1) {
                bitmap[idx] = 0;
                cout << "已释放: 柱面=" << cyl
                     << ", 磁头=" << head
                     << ", 扇区=" << sec << endl;
                any_released = true;
            } else {
                cout << "块未被占用或无效释放: (" << cyl << ", " << head << ", " << sec << ")" << endl;
            }
        }
        if (any_released) {
            printBitmap();
        } else {
            cout << "没有有效释放的块。" << endl;
        }
    }
};

int main() {
    DiskBitmapManager disk;
    int choice;

    while (true) {
        cout << "\n=== 磁盘位示图管理器 ===" << endl;
        cout << "1. 分配磁盘块" << endl;
        cout << "2. 释放磁盘块" << endl;
        cout << "3. 显示位示图" << endl;
        cout << "4. 退出" << endl;
        cout << "请输入你的选择: ";
        cin >> choice;

        if (choice == 1) {
            int count;
            cout << "请输入需要分配的块数量: ";
            cin >> count;
            if (count <= 0) {
                cout << "分配数量必须大于0。" << endl;
                continue;
            }
            disk.allocate(count);
        } else if (choice == 2) {
            int count;
            cout << "请输入要释放的块数量: ";
            cin >> count;
            if (count <= 0) {
                cout << "释放数量必须大于0。" << endl;
                continue;
            }

            vector<tuple<int, int, int>> to_release;
            for (int i = 0; i < count; ++i) {
                int cyl, head, sec;
                cout << "输入块 " << i + 1 << " 的柱面、磁头、扇区 (空格分隔): ";
                cin >> cyl >> head >> sec;
                to_release.emplace_back(cyl, head, sec);
            }

            disk.release(to_release);

        } else if (choice == 3) {
            disk.printBitmap();
        } else if (choice == 4) {
            cout << "退出程序。" << endl;
            break;
        } else {
            cout << "无效选项，请重新输入！" << endl;
        }
    }

    return 0;
}
