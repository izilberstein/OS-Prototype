README.TXT
Itai Zilberstein


- To compile the program use command 'make'. I have used a makefile to compile all the source and header files into the executable mykernel
- Comments about functionality are in the source files
- I have provided testfile.txt, which uses script.txt, script1.txt, script2.txt and script3.txt to demonstrate functionality
- Some notes about backing store, PCB, and exec: the programs are stored as files in the backing store labelled PID_x where x is an integer corresponding to when the program is called. This x s the PID and stored in the PCB of the process.
example:
exec s1 s2 s3
exec s4 s5 s1

will store s1 as PID_0, s2 as PID_1, s3 as PID_2, s4 as PID_3, s5 as PID_4 and the second s1 as PID_5

- My exec command is able to handle programs with more than 10 pages of code. It handles this by storing the frame of page n at index n mod 10 in the page table.

- NEW FOR DISKDRIVER;
- My disk uses non-contiguous index allocation:
  - it allocates the next free block on disk to a file only when it writes -- all new created files are intially empty
  - the next free block is found when the partition is mounted - it is always the first free block from the start of the data region as there is no 'delete'. We then increment the next free block unti lthe end of the partition.
  - the block pointers are usde for index allocation, i.e. blockPtr[i] contains the block location on disk of block i of the file
  - the meta data region is the first 'data_start' bytes of the file (set to 200, but can easily be enlarged by changing the variable)
  - the format of the meta data is: '#blocks,sizeblocks:[fat entry][fat entry]....[fat entry]-----:DATA REGION'
  - the writeBlock and readBlock are wrapped by a file system read and write that deals with the overhead
  - partitionFS() works so that it will not overwrite/recreate an existing partition -- if a partition exists it is mounted so that its previous data is persistent, not destroyed
    example:
    mount partition1 5 5
    write file [this]
    mount partition1 1 1
    read file i
    print i
    --> outputs: this
