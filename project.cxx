#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cstdint>
#include <fstream>
#include <bitset>
#include <string>
#include <map>

using namespace std;
using PageNum = uint8_t;
using FrameNum = uint8_t;

constexpr auto FRAME_SIZE = 256; // size of each individual frame, in bytes
constexpr auto FRAMES_AMOUNT = 256; // amount of total frames in page table

// page table maps a page number to a frame 
map<PageNum, FrameNum> page_table;
int frameIndex{};                   // the current frame index, increments as frames are added
int pageFaults{};                   // page fault occurs when one must fetch from backing store. 
char memory[FRAME_SIZE * FRAMES_AMOUNT]; // memory in RAM, used when fetching data from BACKING_STORE.bin

ifstream backing_store;     // backing store is open throughout the program run, closed on program exit. 
ofstream result_os;         // result output stream which will be stored in result.txt

FrameNum fetch_from_store(PageNum page_num) {
    // get the frame number from BACKING_STORE
    backing_store.seekg(page_num * FRAME_SIZE);
    signed char buf[FRAME_SIZE]{};
    backing_store.read((char*)&buf, sizeof(buf));
    if (!backing_store) {
        cerr << "error! could only output " << backing_store.gcount() << "elements. \n";
        throw backing_store.gcount();
    }
    FrameNum f = frameIndex++;
    if (frameIndex > FRAMES_AMOUNT) {
        throw length_error("length exceeds " + to_string(FRAMES_AMOUNT));
    }
    page_table[page_num] = f;
    copy(begin(buf), end(buf), begin(memory) + (f << 8));
    pageFaults++;
    return f;
}

// takes a logical address, and translates it into its corresponding 
// physical address
//
// first, we must get the page and offset number. Then, we follow the 
// steps in the textbook regarding how to find a physical address.
void translate_address(uint16_t logical_addr) {
    // offset is last 8 bits
    uint8_t offset = logical_addr & 255;
    // page_num is first 8 bits
    PageNum page_num = (logical_addr >> 8) & 255;
    #if DEBUG
    bitset<8> of(offset);
    bitset<8> pn(page_num);
    bitset<16> la(logical_addr);
    cout << la << ", " << logical_addr << '\n';
    cout << pn << ", " << page_num << '\n';
    cout << of << ", " << offset << '\n';
    #endif
    FrameNum f;
    try {
        f = page_table.at(page_num);
    } catch(const out_of_range& oor) {
        f = fetch_from_store(page_num);
    }
    uint16_t phys_addr = (f << 8) | offset;
    auto value = memory[phys_addr];
    result_os << "Virtual address: " << logical_addr 
        << " Physical address: " << phys_addr
        << " Value: " << static_cast<int>(value) << '\n'; 
    // read from physical address the value
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        cout << "usage: ./a.out addresses.txt\n";
        return 0;
    }

    ifstream addresses;
    addresses.open(argv[1], ifstream::in);

    // backing store is open here
    backing_store.open("BACKING_STORE.bin", ifstream::binary | ifstream::in);
    result_os.open("result.txt", ifstream::out);

    uint32_t a; // read in 32-bit integer numbers
    int num_translated_addrs{};
    while(addresses >> a) {
        // mask this value into a 16-bit number, ignoring upper 16 bits
        uint16_t mask = a & 65535;
        translate_address(a);
        num_translated_addrs++;
    }
    result_os << "Number of Translated Addresses = " << num_translated_addrs << '\n';
    result_os << "Page Faults = " << pageFaults << '\n';
    result_os << "Page Fault Rate = " << (pageFaults / (float)num_translated_addrs) << '\n';
    return 0;
}