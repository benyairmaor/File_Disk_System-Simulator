//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~INITALIZATION~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//    
#ifndef SIM_DISK
#define SIM_DISK

#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256

void  decToBinary(int n , char &c); // array to store binary number 

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~fsInode~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
class fsInode { //fsInode Class
    int fileSize; //The size of the file
    int block_in_use; //How many block in used by the file
    int *directBlocks; //Array of all the direct Blocks
    int singleInDirect; //Index of the inDirect
    int num_of_direct_blocks; //How many direct are
    int block_size;
    
    public:
    fsInode(int _block_size, int _num_of_direct_blocks); //fsInode_Constructor
    int getFileSize(){return fileSize;} //Get Method for File size
    void setFileSize(int x){fileSize += x;} //Set Method for File size += x
    int getBlockInUsed(){return block_in_use;} //Get Method for block in used
    void addBlockInUsed(){block_in_use ++;} //Add to the block in used 1
    int getSingleInDirect(){return singleInDirect;} //Get Method for singleInDirects
    void setSingleInDirect(int x){singleInDirect = x;} //Set Method for singleInDirect = x
    int getNumOfDirectBlocks(){return num_of_direct_blocks;} //Get Method for num_of_direct_blocks
    int getBlockSize(){return block_size;} //Get Method for File size
    int* getDirectBlocks(){return directBlocks;} //Get Method for directBlocks
    ~fsInode() {delete directBlocks;} //Destructor
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FileDescriptor~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse; //TRUE if the file is open | FALSE if the file is close
    bool isAlive; //TRUE if the file is not deleted | FALSE if the file is deleted 

    public:
    FileDescriptor(string FileName, fsInode* fsi); //FileDescriptor_Constructor
    string getFileName() {return file.first;} //Get Method for file name
    void setFileName(string File) {file.first = File;} //Get Method for file name
    fsInode* getInode() {return file.second;} //Get Method for dsInode
    void setInode(fsInode* fsi) {file.second = fsi;} //Set Method for fsInode = fsi
    void deleteFileDescriptor() {file.first.erase();file.second = NULL;} //Set Method for fsInode = NULL
    bool isInUse() {return (inUse);} //Get Method for directBlocks
    void setInUse(bool _inUse) {inUse = _inUse ;} //Set Method for inUse = _inUse
    void setIsAlive(bool _isAlive) {isAlive = _isAlive;} //Set Method for isAlive = _isAlive
    bool getIsAlive() {return isAlive;} //Get Method for directBlocks

};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~fsDisk~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
class fsDisk {
    FILE *sim_disk_fd; //The File
    bool is_formated; //TRUE if the disk formated | FALSE if the disk is not formated
    int BitVectorSize; //BitVector - "bit" (int) vector, indicate which block in the disk is free or not.  (i.e. if BitVector[0] == 1 , means that the first block is occupied.
    int *BitVector;
    map<string, fsInode*>  MainDir ; 
    vector< FileDescriptor > OpenFileDescriptors;
    int direct_enteris;
    int block_size;
    // Unix directories are lists of association structures, 
    // each of which contains one filename and one inode number.
    // OpenFileDescriptors --  when you open a file, 
	// the operating system creates an entry to represent that file
    // This entry number is the file descriptor. 
    
    public:
    fsDisk(); //fsDisk_Constructor
    void listAll(); //listAll method print all theOpenFileDescriptors and all the disk
    void fsFormat( int blockSize, int direct_Enteris_); //fsFormat method Format the disk
    int CreateFile(string fileName); //CreateFile method create a file if it isn't exist yet
    int OpenFile(string fileName); //OpenFile method open a file if it is not open
    string CloseFile(int fd); //CloseFile method close a file if it is not close
    int WriteToFile(int fd, char *buf, int len ); //WriteToFile method writes data to the Disk if possible. "Save our files on the disk" 
    int DelFile( string FileName ); //DelFile method delete all the data in the Disk that relevent to this file
    int ReadFromFile(int fd, char *buf, int len ); //ReadFromFile method read all the data in the Disk that relevent to this file and <= len
    int getBlockAvilable (); //getBlockAvilable method checks and sets a "free" block to write
    bool checkBlocksAvilable(int num_of_blocks); //checkBlocksAvilable method checks if there is enough free blocks in the disk (num_of_blocks)
    int getIndex(string fileName); //getIndex method gets string fileName return his place in OpenFileDescriptors vector  
    int getIndex(FileDescriptor file); //(overriding) getIndex method gets FileDescriptor fileName return his place in OpenFileDescriptors vector 
    ~fsDisk(); //Destructor
};

#endif