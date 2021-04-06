# virtual-memory-manager-cxx
Virtual memory manager for an Operating Systems course I took, written in C++.

### Building the program:
note: requires g++

just run Make, and the code will be compiled, run, and tested. 
```bash
$ make
g++  project.cxx
./a.out addresses.txt
-- If no lines follow diff, this means that the files are identical, otherwise there is an anomaly. --
diff -u result.txt correct.txt
```

### Description
The program takes logical addresses from a file, and translates them into corresponding physical addresses. 
These addresses are translated in translate_address function. It also deals with virtual memory on the hard disk
(stored in BACKING_STORE.bin). First the frame is checked for existence in a page table. 
If it is not in the page table, then it fetches from the backing store. Values from this
backing store are loaded into physical memory lazily. Results are stored in a result file 
(result.txt) and this file is tested with what a correct result should look like (in addresses.txt). 
