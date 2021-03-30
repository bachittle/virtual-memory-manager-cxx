#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cstdint>
#include <fstream>
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

// fetches the required page from BACKING_STORE.
// page number is inputted to find the corresponding BACKING_STORE index.
// returns the frame number in physical memory which stores the page loaded from BACKING_STORE.
// frames are 256 bytes. 
FrameNum fetch_from_store(PageNum page_num) {
    // get the frame number from BACKING_STORE
    backing_store.seekg(page_num * FRAME_SIZE);
    uint8_t buf[FRAME_SIZE]{};
    backing_store.read((char*)&buf, sizeof(buf));
    if (!backing_store) {
        cerr << "error! could only read " << backing_store.gcount() << "elements. \n";
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

// takes a logical address, and translates it into its corresponding physical address
// steps are outlined in textbook. page_num is indexed via the page_table to get frame_num
// then indexing physical memory with offset gets the number stored in BACKING_STORE.bin
// each piece of backing_store is loaded when necessary. 
void translate_address(uint16_t logical_addr) {
    // offset is last 8 bits
    uint8_t offset = logical_addr & 255;
    // page_num is first 8 bits
    PageNum page_num = (logical_addr >> 8) & 255;
    FrameNum frame_num;

    // either get the frame number from the page table, or if it doesn't exist, 
    // from the BACKING_STORE.
    {
        auto frame_iter = page_table.find(page_num);
        if (frame_iter != page_table.end()) {
            frame_num = frame_iter->second;
        } else {
            frame_num = fetch_from_store(page_num);
        }
    }

    uint16_t phys_addr = (frame_num << 8) | offset;
    auto value = memory[phys_addr];
    result_os << "Virtual address: " << logical_addr 
        << " Physical address: " << phys_addr
        << " Value: " << static_cast<int>(value) << '\n'; 
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        cout << "usage: ./a.out addresses.txt\n";
        return 0;
    }
    ifstream addresses;
    addresses.open(argv[1], ifstream::in);

    backing_store.open("BACKING_STORE.bin", ifstream::binary | ifstream::in);
    result_os.open("result.txt", ifstream::out);

    uint32_t a; // read in 32-bit integer numbers
    int num_translated_addrs{};
    while(addresses >> a) {
        // mask this value into a 16-bit number, ignoring upper 16 bits
        uint16_t mask = a & 65535;
        translate_address(mask);
        num_translated_addrs++;
    }
    result_os << "Number of Translated Addresses = " << num_translated_addrs << '\n'
        << "Page Faults = " << pageFaults << '\n'
        << "Page Fault Rate = " << (pageFaults / static_cast<float>(num_translated_addrs)) 
        << "\n\n";
}