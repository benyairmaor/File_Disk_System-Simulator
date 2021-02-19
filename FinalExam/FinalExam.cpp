//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~INITALIZATION~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//  
#include "FinalExam.hh"
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
#define OFFSET_FOR_CHAR 33

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~decToBinary~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~// 
void  decToBinary(int n , char &c) {  // array to store binary number 
    int binaryNum[8]; 
    int i = 0; // counter for binary array 
    while (n > 0) { // storing remainder in binary array 
        binaryNum[i] = n % 2; 
        n = n / 2; 
        i++; 
    } 
    for (int j = i - 1; j >= 0; j--) {  // printing binary array in reverse order 
        if (binaryNum[j]==1)
            c = c | 1u << j;
    }
 } 

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~fsInode_Constructor~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~// 
fsInode::fsInode(int _block_size, int _num_of_direct_blocks) { 
    fileSize = 0; 
    block_in_use = 0; 
    block_size = _block_size;
    num_of_direct_blocks = _num_of_direct_blocks;
    directBlocks = new int[num_of_direct_blocks];
	assert(directBlocks);
    for (int i = 0 ; i < num_of_direct_blocks; i++) {   
        directBlocks[i] = -1;
    }
    singleInDirect = -1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FileDescriptor_Constructor~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~// 
FileDescriptor::FileDescriptor(string FileName, fsInode* fsi){
    file.first = FileName;
    file.second = fsi;
    inUse = true;
    isAlive = true;
}

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~fsDisk_Constructor~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
fsDisk::fsDisk() {
    sim_disk_fd = fopen( DISK_SIM_FILE , "r+" );
    assert(sim_disk_fd);
    for (int i=0; i < DISK_SIZE ; i++) {
        int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
        ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
        assert(ret_val == 1);
    }
    fflush(sim_disk_fd);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~listAll~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void fsDisk::listAll() {
    int i = 0;    
    for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
        cout << "index: " << i << ": FileName: " << it->getFileName() <<  " , isInUse: " << it->isInUse() << endl; 
        i++;
    }
    char bufy;
    cout << "Disk content: '" ;
    for (i=0; i < DISK_SIZE ; i++) {
        int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
        ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
        cout << bufy;              
    }
    cout << "'" << endl;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~fsFormat~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void fsDisk::fsFormat(int blockSize = 4, int direct_Enteris_ = 3) { //fsFormat method Format the disk
    if(is_formated == true){
        cout << "FORMAT DISK: DISK already formated" << endl;
        return;
    }
    block_size = blockSize; //Initalize block_size to be blockSize
    direct_enteris = direct_Enteris_; //Initalize direct_enteris to be direct_Enteris_
    BitVectorSize = DISK_SIZE/block_size; //Initalize BitVectorSize to be DISK_SIZE/block_size
    BitVector = (int*)malloc(sizeof(int)*BitVectorSize); //Initalize an array of BitVector size BitVectorSize (remember all the data block)
    is_formated = true; //Initalize is_formated to be true
    cout << "FORMAT DISK: number of blocks: " << DISK_SIZE/block_size << endl;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~CreateFile~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
int fsDisk::CreateFile(string fileName) { //CreateFile method create a file if it isn't exist yet
    int idx = -1;
    int idxToInsert = -1;
    if(is_formated == false){ //Check if the disk is formated 
        cout << "CreateFile: ERROR: The Disk is not format" << endl;
        return -1; 
    }
    if(checkBlocksAvilable(1) == false){ //If there is no enough disk to allocate
        cout << "CreateFile: ERROR: There Is Not Enough Space On The Disk" << endl;
        return -1;
    }
    else if(MainDir.find(fileName).operator!=(MainDir.end())){ //Check in the mainDir (our "living" files) if there is a file with the same name 
        cout << "CreateFile: ERROR: Already Exist" << endl;
        return -1;
    }
    string toComper = "";
    for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) { //Check in the OpenFileDescriptors (all our files) if there is a file with the same name if there is so this name has been deleted
        idx++;
        if(it->getFileName().compare(toComper) == 0){
            idxToInsert = idx;
            break;
        }
    }
    if(idxToInsert > -1){ //If there is a file that has been deleted with the same name set this fsInode to the place of the one that has been deleted
        fsInode *fsi = new fsInode(block_size, direct_enteris); // New fsInode
        OpenFileDescriptors[idxToInsert].setInode(fsi); //Set "deleted file" pointer to the new fsInode
        OpenFileDescriptors[idxToInsert].setFileName(fileName);
        MainDir.insert({fileName, fsi}); //Push to MainDir this "alive file" pair
        OpenFileDescriptors[idxToInsert].setInUse(true); //Set InUse to true
        OpenFileDescriptors[idxToInsert].setIsAlive(true); //Set InAlive to true
        return idxToInsert;
    }
    else if(is_formated == true){ //If there is not any file that has been deleted with the same name 
        fsInode *fsi = new fsInode(block_size, direct_enteris); // New fsInode
        FileDescriptor newFD = FileDescriptor(fileName, fsi); // New FileDescriptor
        MainDir.insert({fileName, fsi}); //Push to MainDir this "alive file" pair
        OpenFileDescriptors.push_back(newFD); //Push to OpenFileDescriptors FileDescriptor
        return getIndex(newFD);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OpenFile~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
int fsDisk::OpenFile(string fileName) { //OpenFile method open a file if it is not open
    if(is_formated == false){ //If the disk hasn't been formated
        cout << "OpenFile: ERROR: The disk did not format" << endl;
        return -1;
    }
    int fd = getIndex(fileName); //fd is the location of fileName in OpenFileDescriptors
    if(fd == -1){ //If there is no such a file
        cout << "OpenFile: ERROR: There Is No Such A File" << endl;
        return -1;
    }
    else if(OpenFileDescriptors.at(fd).getIsAlive() == false){ //If the file has been deleted
        cout << "OpenFile: ERROR: File Deleted" << endl;
        return -1;
    }
    else if(OpenFileDescriptors.at(fd).isInUse() == true){ //If the file already open
        cout << "OpenFile: ERROR: Already Open" << endl;
        return -1;
    }  
    OpenFileDescriptors.at(fd).setInUse(true); //Open file
    return fd;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~CloseFile~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
string fsDisk::CloseFile(int fd) { //CloseFile method close a file if it is not close
    if(is_formated == false){ //If the disk hasn't been formated
        cout << "CloseFile: ERROR: The disk did not format" << endl;
        return "-1";
    }
    if(OpenFileDescriptors.at(fd).getIsAlive() == false){ //If the file has been deleted
        cout << "CloseFile: ERROR: File Deleted" << endl;
        return "-1";
    }
    else if(fd >= OpenFileDescriptors.size()){ //If there is no such a file
        cout << "CloseFile: ERROR: File Not Exist" << endl;
        return "-1";
    }
    else if(OpenFileDescriptors.at(fd).isInUse() == false){ //If the file already close
        cout << "CloseFile: ERROR: File Already Closed" << endl;
        return "-1";
    }
    OpenFileDescriptors.at(fd).setInUse(false); //Close file
    return OpenFileDescriptors.at(fd).getFileName();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~WriteToFile~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
int fsDisk::WriteToFile(int fd, char *buf, int len ) { //WriteToFile method writes data to the Disk if possible. "Save our files on the disk" 
    if(is_formated == false){ //If the disk hasn't been formated
        cout << "WriteToFile: ERROR: The disk did not format" << endl;
        return -1;
    }
    if(OpenFileDescriptors.at(fd).getIsAlive() == false){ //If the file has been deleted
        cout << "WriteToFile: ERROR: File Deleted" << endl;
        return -1;
    }
    if(fd >= OpenFileDescriptors.size()){ //If there is no such a file
        cout << "WriteToFile: ERROR: There is no File Descriptor such as" << fd << endl;
        return -1;
    }
    if(OpenFileDescriptors.at(fd).isInUse() == false){ //If the file is close
        cout << "WriteToFile: ERROR: This File Isn't Open" << endl;
        return -1;
    }
    int maxSize = (direct_enteris + block_size) * block_size;
    int sizeAfterWrite = OpenFileDescriptors.at(fd).getInode()->getFileSize() + len;
    if(sizeAfterWrite > maxSize){ //If the size of the file after the write will be bigger than max size
        cout << "WriteToFile: ERROR: The File Size Is Exceed From Max Size File" << endl;
        return -1;
    } 
    int blocksNeeded = 0; //Counter blocksNeeded for how many block for data we will need
    bool isInDirect = false; //Boolean isInDirect if we need the in direct for the first time now
    char toWrite; //toWrite is the char that need to be writen
    int toWriteIdx = 0; //toWriteIdx is the index of the char that need to be writen
    int toCheck; //The number of free block that we need to ensure blocksNeeded + (1 if there is isInDirect)
    if(sizeAfterWrite <= direct_enteris*block_size || OpenFileDescriptors.at(fd).getInode()->getFileSize() > direct_enteris*block_size){ //If there is no need to allocate the inDirect (block size < direct*block size or the file already has inDirect)
        if(OpenFileDescriptors.at(fd).getInode()->getFileSize()%block_size != 0){ //If there is an "offset" for write in the last block that has been writen (the last block isn't full)
            if(len <= block_size-(OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size)){ //If the buf size < the remain place in the last block don't neet to alocate blocks
                toCheck = ((len - (block_size-(OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size))) / block_size);
            }
            else if((len - (block_size-(OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size))) % block_size == 0){ //if the ((len - the remain place in the last block) % block_size = 0 need to alocate (len - the remain place in the last block) % block_size) / block_size 
                toCheck = ((len - (block_size-(OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size))) / block_size); 
            }
            else{ //if the (len - the remain place in the last block) % block_size != 0 need to alocate (((len - the remain place in the last block) % block_size) / block_size ) + (1 because it's int, for example 4.5=4 and we need 5 blocks)
                toCheck = ((len - (block_size-(OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size))) / block_size) + 1;
            }
        }
        else{ //If there is no an "offset" for write in the last block that has been writen (the last block is full) 
            if(len % block_size == 0){ //if the len % block_size = 0 need to alocate len / block_size
                toCheck = (len / block_size);
            }
            else{ //if the len % block_size != 0 need to alocate len / block_size + (1 because it's int, for example 4.5=4 and we need 5 blocks)
                toCheck = (len / block_size) + 1;
            }
        }
        blocksNeeded = toCheck; //Num of blocksNeeded = toCheck there is no use for inDirect for now
    }
    else{ //If there is need to allocate the inDirect
        isInDirect = true;
        if(OpenFileDescriptors.at(fd).getInode()->getFileSize()%block_size != 0){ //If there is an "offset" for write in the last block that has been writen (the last block isn't full)
            if((len - (block_size-(OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size))) % block_size == 0){ //if the ((len - the remain place in the last block) % block_size = 0 need to alocate (len - the remain place in the last block) % block_size) / block_size + (1 for the inDirect)
                toCheck = ((len - (block_size-(OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size))) / block_size) + 1; 
            }
            else{ //if the ((len - the remain place in the last block) % block_size != 0 need to alocate (len - the remain place in the last block) % block_size) / block_size + (2, 1 for the inDirect and 1 for example in ints 4.5=4 and we need 5 blocks)
                toCheck = ((len - (block_size-(OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size))) / block_size) + 2;
            }
        }
        else{ //If there is no an "offset" for write in the last block that has been writen (the last block is full) 
            if(len % block_size == 0){ //if the len % block_size = 0 need to alocate len / block_size + (1 for the inDirect)
                toCheck = (len / block_size) + 1;
            }
            else{ //if the len % block_size != 0 need to alocate len / block_size + (2, 1 for the inDirect and 1 for example in ints 4.5=4 and we need 5 blocks)
                toCheck = (len / block_size) + 2;
            }
        } 
        blocksNeeded = toCheck -1; //Num of blocksNeeded = toCheck + 1 there is use for inDirect for now
    }
    if(checkBlocksAvilable(toCheck) == false){ //If there is no enough disk to allocate
        cout << "WriteToFile: ERROR: There Is Not Enough Space On The Disk" << endl;
        return -1;
    }
    if(isInDirect == true){ //If there is inDirect allocate for it a block
        OpenFileDescriptors.at(fd).getInode()->setSingleInDirect(getBlockAvilable());
    }
    int ptr;
    int directIdx = 0;
    int inDirectIdx = 0;
    if(OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size != 0 && OpenFileDescriptors.at(fd).getInode()->getFileSize() < direct_enteris*block_size){ //If there is need to fill the gap but in any of directs
        int toComplite = OpenFileDescriptors.at(fd).getInode()->getDirectBlocks()[OpenFileDescriptors.at(fd).getInode()->getBlockInUsed() - 1] * block_size + (OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size); //toComplite = the last block that has been writen
        ptr = fseek (sim_disk_fd , toComplite , SEEK_SET); //Move the cursor to the right place
        int insertIndex = OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size; //insertIndex the index insert to
        while (toWriteIdx < len && insertIndex < block_size){ //While the last block is not full
            toWrite = buf[toWriteIdx]; //toWrite = the current char we need to write
            toWriteIdx++;
            fputc(toWrite, sim_disk_fd);//Write toWrite to the file
            insertIndex++;
        }  
    }
    else if(OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size != 0 && OpenFileDescriptors.at(fd).getInode()->getFileSize() > direct_enteris*block_size){ //If there is need to fill the gap but in any directs from the inDirect
        int readFrom = OpenFileDescriptors.at(fd).getInode()->getBlockInUsed() - direct_enteris - 1; //readFrom = num of used blocks - direct_enteris - 1 (the index of the direct in the inDirect)
        readFrom += OpenFileDescriptors.at(fd).getInode()->getSingleInDirect() * block_size; //readFrom += inDirect block * block_size (the index we need to read to know the index of the block)
        ptr = fseek (sim_disk_fd , readFrom , SEEK_SET); //Move the cursor to the right place
        char location = fgetc(sim_disk_fd); //location = the readen char form the disk (the index of the block we need to read from)
        int toComplite = ((int)(location)-OFFSET_FOR_CHAR)*block_size + (OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size); //toComplite = (cast to int to location - OFFSET_FOR_CHAR (for the char in the disk look good :)) * block size + how many blocks already writen there
        ptr = fseek (sim_disk_fd , toComplite , SEEK_SET); //Move the cursor to the right place
        int insertIndex = OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size; //insertIndex the index insert to
        while (toWriteIdx < len && insertIndex < block_size){ //While the last block is not full
            toWrite = buf[toWriteIdx]; //toWrite = the current char we need to write
            toWriteIdx++;
            fputc(toWrite, sim_disk_fd);//Write toWrite to the file
            insertIndex++;
        }  
    }
    while (blocksNeeded != 0){ //If there are more blocks to insert
        if(OpenFileDescriptors.at(fd).getInode()->getBlockInUsed() < direct_enteris){ //If there is free space in the directs
            if(OpenFileDescriptors.at(fd).getInode()->getDirectBlocks()[directIdx] == -1){ //If this block is unused
                OpenFileDescriptors.at(fd).getInode()->getDirectBlocks()[directIdx] = getBlockAvilable(); //get a free block to current direct  
                ptr = fseek (sim_disk_fd , OpenFileDescriptors.at(fd).getInode()->getDirectBlocks()[directIdx]*block_size , SEEK_SET); //Move the cursor to the right place
                int insertIndex = 0; //insertIndex the index insert to
                while (toWriteIdx < len && insertIndex < block_size){ //While the last block is not full
                    toWrite = buf[toWriteIdx]; //toWrite = the current char we need to write
                    toWriteIdx++;
                    fputc(toWrite, sim_disk_fd); //Write toWrite to the file
                    insertIndex++;
                }
                OpenFileDescriptors.at(fd).getInode()->addBlockInUsed(); //Add 1 to block in used
                blocksNeeded--; 
            }
            directIdx++;
        }
        else{ //If there is not free space in the directs
            toWrite = buf[toWriteIdx];  //toWrite = the current char we need to write
            int inDirectInUsed = OpenFileDescriptors.at(fd).getInode()->getBlockInUsed() - direct_enteris; //Set inDirectInUsed to be the num of block in used in the in direct
            char blockPtr = '\0';
            int blockIdx = getBlockAvilable(); //blockIdx = free available block
            decToBinary(blockIdx + OFFSET_FOR_CHAR, blockPtr); //cast blockIdx from int to Binary (for right it in the block that contains only chars)  
            int inDirectLoc = OpenFileDescriptors.at(fd).getInode()->getSingleInDirect()*block_size; //inDirectLoc = the position of the first element in the indirect
            ptr = fseek (sim_disk_fd , inDirectLoc + inDirectInUsed , SEEK_SET); //Move the cursor to the right place
            fputc(blockPtr, sim_disk_fd);
            int insertIndex = 0; //insertIndex the index insert to
            ptr = fseek (sim_disk_fd , blockIdx*block_size , SEEK_SET); //Move the cursor to the right place
            while (toWriteIdx < len && insertIndex < block_size){ //While the last block is not full
                toWrite = buf[toWriteIdx]; //toWrite = the current char we need to write
                toWriteIdx++;
                fputc(toWrite, sim_disk_fd); //Write toWrite to the file
                insertIndex++;
            }
            OpenFileDescriptors.at(fd).getInode()->addBlockInUsed(); //Add 1 to block in used
            blocksNeeded--;
            inDirectIdx++;    
        }
    }
    OpenFileDescriptors.at(fd).getInode()->setFileSize(len); //Set File Size to be += len
    fflush(sim_disk_fd);
    return fd;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~DelFile~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
int fsDisk::DelFile( string FileName ) { //DelFile method delete all the data in the Disk that relevent to this file
    if(is_formated == false){ //If the disk hasn't been formated
        cout << "DelFile: ERROR: The disk did not format" << endl;
        return -1;
    }
    if(MainDir.find(FileName).operator==(MainDir.end())){ //Check in the mainDir if this file exist
        cout << "DelFile: ERROR: There is no File Name such as " << FileName << endl;
        return -1;
    }
    int toReturn = getIndex(FileName); //toReturn = the index of FileName in OpenFileDescriptor
    fsInode *fsi = MainDir.find(FileName)->second; //*fsi is a pointer to fsInode of FileName 
    int idxBlockToDel = 0;
    int ptr = 0;
    int deleteIdx = 0;
    char zero = '\0';
    int BlocksToDelete = fsi->getBlockInUsed(); //BlocksToDelete num of full blocks to delete 
    for(int i = 0; i < BlocksToDelete; i++){
        if(i < direct_enteris){ //If the block to delete is direct
            idxBlockToDel = fsi->getDirectBlocks()[i] * block_size; //Set index to delete
            ptr = fseek (sim_disk_fd , idxBlockToDel , SEEK_SET); //Move the cursor to the right place
            for(int j = 0; j < block_size; j++){ //Zero the block
                fputc(zero, sim_disk_fd);    
            }
            BitVector[fsi->getDirectBlocks()[i]] = 0; //Zero BitVector[blockToDelete]
        }
        else if(i >= direct_enteris){ //If the block to delete is in the inDirect
            int blockIdx = i - direct_enteris; //blockIdx = the current iteration - direct_enteris
            blockIdx += fsi->getSingleInDirect()*block_size;  //Set index to delete
            ptr = fseek (sim_disk_fd , blockIdx , SEEK_SET);  //Move the cursor to the right place
            char location = fgetc(sim_disk_fd); //location = the readen char form the disk (the index of the block we need to read from)
            idxBlockToDel = ((int)(location)-OFFSET_FOR_CHAR)*block_size;  //idxBlockToDel = (cast to int to location - OFFSET_FOR_CHAR (for the char in the disk look good :)) * block size + how many blocks already writen there 
            ptr = fseek (sim_disk_fd , idxBlockToDel , SEEK_SET);  //Move the cursor to the right place
            for(int j = 0; j < block_size; j++){  //Zero the block
            fputc(zero, sim_disk_fd);    
            }
            BitVector[(int)(location)-OFFSET_FOR_CHAR] = 0; //Zero BitVector[blockToDelete] - OFFSET_FOR_CHAR (for the char in the disk look good :))
        }
    }
    if(fsi->getFileSize() > direct_enteris*block_size){ //If there is inDirect need to delete
        ptr = fseek (sim_disk_fd , fsi->getSingleInDirect()*block_size , SEEK_SET);  //Move the cursor to the right place
        for(int j = 0; j < block_size; j++){  //Zero the block
            fputc(zero, sim_disk_fd);    
        }
        BitVector[fsi->getSingleInDirect()] = 0; //Zero BitVector[blockToDelete]
    }
    int i = getIndex(FileName);
    OpenFileDescriptors.at(i).deleteFileDescriptor();
    delete(fsi); //Delete the fsInode
    MainDir.erase(FileName); //Erase the file from the map
    OpenFileDescriptors.at(toReturn).setInUse(NULL); //Set inUsed to NULL
    OpenFileDescriptors.at(toReturn).setIsAlive(false); //Set in to NULL
    fflush(sim_disk_fd);
    return toReturn;    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ReadFromFile~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
int fsDisk::ReadFromFile(int fd, char *buf, int len ) { //ReadFromFile method read all the data in the Disk that relevent to this file and <= len
    for(int i = 0; i< DISK_SIZE; i++){ //Reset buf 
        buf[i] = '\0';
    } 
    if(is_formated == false){ //If the disk hasn't been formated
        cout << "ReadFromFile: ERROR: The disk did not format" << endl;
        return -1;
    }
    if(OpenFileDescriptors.at(fd).getIsAlive() == false){ //If the file has been deleted
        cout << "ReadFromFile: ERROR: File Deleted" << endl;
        return -1;
    }
    if(fd >= OpenFileDescriptors.size()){ //If there is no such a file
        cout << "ReadFromFile: ERROR: There is no File Descriptor such as " << fd << endl;
        return -1;
    }
    if(OpenFileDescriptors.at(fd).isInUse() == false){ //If the file is close
        cout << "ReadFromFile: ERROR: This File Isn't Open" << endl;
        return -1;
    }
    if(len < 0){ //If the len is invalid
        cout << "ReadFromFile: ERROR: len Is Invalid" << endl;
        return -1;
    }
    int blockToRead = -1;
    int ptr = 0;
    int readIdx = 0;
    int fullBlocksToRead;
    int offsetBlockToRead;
    if(len <= OpenFileDescriptors.at(fd).getInode()->getFileSize()){ //If len < fileSize set fullBlocksToRead and offsetBlockToRead based of len
        fullBlocksToRead = len / block_size;
        offsetBlockToRead = len % block_size;
    }
    else{  //If len > fileSize set fullBlocksToRead and offsetBlockToRead based of file_size
        fullBlocksToRead = OpenFileDescriptors.at(fd).getInode()->getFileSize() / block_size;
        offsetBlockToRead = OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size;
    }
    for(int i = 0; i < fullBlocksToRead; i++){ //For all the blocks -1 to delete 
        if(i < direct_enteris){ //If the block is direct
            blockToRead = OpenFileDescriptors.at(fd).getInode()->getDirectBlocks()[i] * block_size; //Index of the first char of the block we wont to read
            ptr = fseek (sim_disk_fd , blockToRead , SEEK_SET); //Move the cursor to the right place
            for(int j = 0; j < block_size; j++){ //Reset all the block
                buf[readIdx] = fgetc(sim_disk_fd);
                readIdx++;
            }
        }
        else if(i >= direct_enteris){ //If the block is not direct
            int blockIdx = i - direct_enteris; //blockIdx = the curent index in the inDirect
            blockIdx += OpenFileDescriptors.at(fd).getInode()->getSingleInDirect()*block_size; //Index of the first char of the block we wont to read
            ptr = fseek (sim_disk_fd , blockIdx , SEEK_SET); //Move the cursor to the right place
            char location = fgetc(sim_disk_fd); //location = the readen char form the disk (the index of the block we need to read from)
            blockToRead = ((int)(location)-OFFSET_FOR_CHAR)*block_size + (OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size); //blockToRead = (cast to int to location - OFFSET_FOR_CHAR (for the char in the disk look good :)) * block size + how many blocks already writen there
            ptr = fseek (sim_disk_fd , blockToRead , SEEK_SET); //Move the cursor to the right place
            for(int j = 0; j < block_size; j++){ //Reset all the block
                buf[readIdx] = fgetc(sim_disk_fd);
                readIdx++;
            }
        }
    }
    blockToRead = OpenFileDescriptors.at(fd).getInode()->getDirectBlocks()[fullBlocksToRead] * block_size; //Index of the first char of the block we wont to read
    ptr = fseek (sim_disk_fd , blockToRead , SEEK_SET); //Move the cursor to the right place
    if(fullBlocksToRead < direct_enteris){ //If the last block is direct
        for(int j = 0; j < offsetBlockToRead; j++){ //Reset all the left chars
            buf[readIdx] = fgetc(sim_disk_fd);
            readIdx++;
        }
    }
    else if(fullBlocksToRead >= direct_enteris){ //If the last block is in the inDirect
        int blockIdx = fullBlocksToRead - direct_enteris; //blockIdx = used block in the indirect (index in the inDirect)
        blockIdx += OpenFileDescriptors.at(fd).getInode()->getSingleInDirect()*block_size; //Index of the first char of the block we wont to read
        ptr = fseek (sim_disk_fd , blockIdx , SEEK_SET); //Move the cursor to the right place
        char location = fgetc(sim_disk_fd); //location = the readen char form the disk (the index of the block we need to read from)
        blockToRead = ((int)location-OFFSET_FOR_CHAR)*block_size + (OpenFileDescriptors.at(fd).getInode()->getFileSize() % block_size); //blockToRead = (cast to int to location - OFFSET_FOR_CHAR (for the char in the disk look good :)) * block size + how many blocks already writen there
        ptr = fseek (sim_disk_fd , blockToRead , SEEK_SET); //Move the cursor to the right place
        for(int j = 0; j < offsetBlockToRead; j++){//Reset all the left chars
            buf[readIdx] = fgetc(sim_disk_fd);
            readIdx++;
        }
    }
    return fd;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~getBlockAvilable~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
int fsDisk::getBlockAvilable (){ //getBlockAvilable method checks and sets a "free" block to write
    int toReturn = -1;
    for(int i = 0; i < BitVectorSize; i++){
        if(BitVector[i] == 0){ //If the block is "free" block return the number of the block
            BitVector[i] = 1;
            toReturn = i;
            break;
        }
    }
    return toReturn;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~checkBlocksAvilable~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
bool fsDisk::checkBlocksAvilable (int num_of_blocks){ //checkBlocksAvilable method checks if there is enough free blocks in the disk (num_of_blocks)
    int counter = num_of_blocks;
    bool toReturn = false;
    for(int i = 0; i < BitVectorSize; i++){
        if(BitVector[i] == 0){ //If there is a "free" block counter--
            counter--;
        }
        if(counter == 0){ //If there is enough "free" block break
            toReturn = true;
            break;
        }
    }
    return toReturn;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~getIndex~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
int fsDisk::getIndex(string fileName){ //getIndex method gets string fileName return his place in OpenFileDescriptors vector  
    for(int i = 0; i < OpenFileDescriptors.size() ; i++){
        if(fileName.compare(OpenFileDescriptors[i].getFileName()) == 0){
            return i;
        }
    }
    return -1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~getIndex~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
int fsDisk::getIndex(FileDescriptor file){ //(overriding) getIndex method gets FileDescriptor fileName return his place in OpenFileDescriptors vector 
    for(int i = 0; i < OpenFileDescriptors.size(); i++){
        if(file.getFileName().compare(OpenFileDescriptors[i].getFileName()) == 0 && file.getInode() == OpenFileDescriptors[i].getInode()){
            return i;
        }
    }
    return -1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Distructor~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
fsDisk::~fsDisk() { //Distructor
    delete(sim_disk_fd); //Delete sim_disk_fd
    delete(BitVector); //Delete BitVector
    for (int i = 0; i < OpenFileDescriptors.size(); i++) { //Delete all OpenFileDescriptors[i]
        if(OpenFileDescriptors[i].getIsAlive() == true){ //If isnot already deleted
            DelFile(OpenFileDescriptors[i].getFileName());
        }
    }
    OpenFileDescriptors.clear(); //Clear the vector
}
