#include <iostream>
#include <vector>
#include <tuple>

using namespace std;

class DiskBitmapManager {
private:
    const int heads = 4;             // 磁头数量
    const int cylinders = 40;        // 柱面数量
    const int sectors = 8;           // 每个柱面每个磁头有8个扇区
    const int total_blocks = heads * cylinders * sectors;  // 总磁盘块数

    vector<int> bitmap;              // 位示图，0=空闲，1=占用

public:
    DiskBitmapManager() {
        // 初始化位示图，全部空闲
        bitmap.resize(total_blocks, 0);
    }

    // 打印当前位示图状态，每32位一行
    void printBitmap() {
        cout << "\nCurrent bitmap status (1=used, 0=free):" << endl;
        for (int i = 0; i < total_blocks; ++i) {
            if (i % 32 == 0)
                cout << "Bit[" << i / 32 << "]: ";
            cout << bitmap[i] << " ";
            if ((i + 1) % 32 == 0)
                cout << endl;
        }
        cout << endl;
    }

    // 将块索引转换成物理地址 (柱面号, 磁头号, 扇区号)
    tuple<int, int, int> indexToAddress(int index) {
        int cylinder = index / (heads * sectors);
        int remainder = index % (heads * sectors);
        int head = remainder / sectors;
        int sector = remainder % sectors;
        return make_tuple(cylinder, head, sector);
    }

    // 将物理地址转换成位示图中的块索引
    int addressToIndex(int cylinder, int head, int sector) {
        return cylinder * heads * sectors + head * sectors + sector;
    }

    // 分配指定数量的磁盘块，返回物理地址列表
    vector<tuple<int, int, int>> allocate(int num_blocks) {
        vector<tuple<int, int, int>> allocated;
        for (int i = 0; i < total_blocks && (int)allocated.size() < num_blocks; ++i) {
            if (bitmap[i] == 0) {   // 找到空闲块
                bitmap[i] = 1;       // 标记为占用
                allocated.push_back(indexToAddress(i));
            }
        }

        if ((int)allocated.size() < num_blocks) {
            // 空间不足，回滚之前分配的块
            cout << "Not enough space, rolling back allocated blocks." << endl;
            for (auto& addr : allocated) {
                int idx = addressToIndex(get<0>(addr), get<1>(addr), get<2>(addr));
                bitmap[idx] = 0;
            }
            allocated.clear();
        } else {
            cout << "Successfully allocated " << num_blocks << " disk blocks:" << endl;
            for (auto& addr : allocated) {
                cout << "  -> Cylinder: " << get<0>(addr)
                     << ", Head: " << get<1>(addr)
                     << ", Sector: " << get<2>(addr) << endl;
            }
        }

        return allocated;
    }

    // 释放指定的磁盘块列表
    void release(const vector<tuple<int, int, int>>& blocks) {
        cout << "Releasing disk blocks..." << endl;
        for (const auto& addr : blocks) {
            int idx = addressToIndex(get<0>(addr), get<1>(addr), get<2>(addr));
            // 校验索引合法且当前块被占用才清除
            if (idx >= 0 && idx < total_blocks && bitmap[idx] == 1) {
                bitmap[idx] = 0;
                cout << "  -> Released: Cylinder=" << get<0>(addr)
                     << ", Head=" << get<1>(addr)
                     << ", Sector=" << get<2>(addr) << endl;
            } else {
                cout << "  Invalid or already free block: ("
                     << get<0>(addr) << ", " << get<1>(addr) << ", " << get<2>(addr) << ")" << endl;
            }
        }
    }
};

int main() {
    DiskBitmapManager disk;

    // === 示例流程 ===
    cout << "=== Disk Block Allocation Example ===" << endl;
    // 申请5个磁盘块
    auto allocated_blocks = disk.allocate(5);
    disk.printBitmap();

    // 假设释放前面申请的第2、4个磁盘块
    if (allocated_blocks.size() >= 4) {
        vector<tuple<int, int, int>> to_release = {allocated_blocks[1], allocated_blocks[3]};
        disk.release(to_release);
        disk.printBitmap();
    }

    // 再申请3个磁盘块
    disk.allocate(3);
    disk.printBitmap();

    // 结束示例
    cout << "=== Example Finished ===" << endl;

    return 0;
}
