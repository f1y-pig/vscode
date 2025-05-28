#include <iostream>
#include <vector>
#include <tuple>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

class DiskBitmapManager {
private:
    const int heads = 4;             // Number of heads
    const int cylinders = 40;        // Number of cylinders
    const int sectors = 8;           // Sectors per head per cylinder
    const int total_blocks = heads * cylinders * sectors;  // Total blocks

    vector<int> bitmap;              // Bitmap (0 = free, 1 = allocated)

public:
    DiskBitmapManager() {
        bitmap.resize(total_blocks, 0); // Initialize all free
    }

    void printBitmap() {
        cout << "\nCurrent bitmap status (1 = allocated, 0 = free):" << endl;
        for (int i = 0; i < total_blocks; ++i) {
            if (i % 32 == 0)
                cout << "Bit[" << i / 32 << "]: ";
            cout << bitmap[i] << " ";
            if ((i + 1) % 32 == 0)
                cout << endl;
        }
        cout << endl;
    }

    tuple<int, int, int> indexToAddress(int index) {
        int cylinder = index / (heads * sectors);
        int remainder = index % (heads * sectors);
        int head = remainder / sectors;
        int sector = remainder % sectors;
        return make_tuple(cylinder, head, sector);
    }

    int addressToIndex(int cylinder, int head, int sector) {
        return cylinder * heads * sectors + head * sectors + sector;
    }

    vector<tuple<int, int, int>> allocate(int num_blocks) {
        vector<tuple<int, int, int>> allocated;
        for (int i = 0; i < total_blocks && (int)allocated.size() < num_blocks; ++i) {
            if (bitmap[i] == 0) {
                bitmap[i] = 1;
                allocated.push_back(indexToAddress(i));
            }
        }

        if ((int)allocated.size() < num_blocks) {
            cout << "Not enough space, rolling back allocated blocks." << endl;
            for (auto& addr : allocated) {
                int idx = addressToIndex(get<0>(addr), get<1>(addr), get<2>(addr));
                bitmap[idx] = 0;
            }
            allocated.clear();
        }
        else {
            cout << "Successfully allocated " << num_blocks << " blocks:" << endl;
            for (auto& addr : allocated) {
                cout << "  -> Cylinder: " << get<0>(addr)
                     << ", Head: " << get<1>(addr)
                     << ", Sector: " << get<2>(addr) << endl;
            }
        }

        return allocated;
    }

    void release(const vector<tuple<int, int, int>>& blocks) {
        cout << "Releasing disk blocks..." << endl;
        for (const auto& addr : blocks) {
            int idx = addressToIndex(get<0>(addr), get<1>(addr), get<2>(addr));
            if (idx >= 0 && idx < total_blocks && bitmap[idx] == 1) {
                bitmap[idx] = 0;
                cout << "  -> Released: Cylinder=" << get<0>(addr)
                     << ", Head=" << get<1>(addr)
                     << ", Sector=" << get<2>(addr) << endl;
            }
            else {
                cout << "  Invalid or unallocated block: "
                     << "(" << get<0>(addr) << ", " << get<1>(addr) << ", " << get<2>(addr) << ")" << endl;
            }
        }
    }
};

int main() {
    DiskBitmapManager disk;
    int choice;

    while (true) {
        cout << "\n=== Disk Bitmap Manager ===" << endl;
        cout << "1. Allocate disk blocks" << endl;
        cout << "2. Release disk blocks" << endl;
        cout << "3. Show bitmap" << endl;
        cout << "4. Exit" << endl;
        cout << "Please enter your choice: ";
        cin >> choice;

        if (choice == 1) {
            int n;
            cout << "Enter the number of blocks to allocate: ";
            cin >> n;
            auto allocated = disk.allocate(n);
            disk.printBitmap();

        } else if (choice == 2) {
            int count;
            cout << "Enter the number of blocks to release: ";
            cin >> count;
            vector<tuple<int, int, int>> blocks;
            for (int i = 0; i < count; ++i) {
                int c, h, s;
                cout << "  Enter Cylinder, Head, Sector for block " << i + 1 << ": ";
                cin >> c >> h >> s;
                blocks.emplace_back(c, h, s);
            }
            disk.release(blocks);
            disk.printBitmap();

        } else if (choice == 3) {
            disk.printBitmap();

        } else if (choice == 4) {
            cout << "Exiting program." << endl;
            break;

        } else {
            cout << "Invalid choice, please try again." << endl;
        }
    }

    return 0;
}
