# File_Disk_System-Simulator

This program is iterative program that simulat File Disk System. 
** General Information of the program: There is a DEFINE named as OFFSET_FOR_CHAR. When we write to the inDirect the block num
cast to char it look not good beacuse of the ASCII code (4 in ASCII code is N/A). So I set OFFSET_FOR_CHAR = 33 to make the char look nicer :) 
For example: !@#$%^ and not some char unrecognized or displayable. OFFSET_FOR_CHAR can be change to 0 to set the program to the regular with no offset in the dec to binary.
In this program you simulat File Disk System, you can change the block size and num of directs and the DISK SIZE is 256. each file can be in maximum size of
(num_of_direct_blocks + block_size) * block_size. 
This program contains following methods:
- LIST ALL - print all the Open File Descriptor and there index (fd number), and if there are in used. (If you delete file, you will not see it in Open File Descriptor). 
- FS FORMAT - Format the disk, set how many directs will be for this simulation, the block size and how many block will be = DISK SIZE / BLOCK SIZE.
	Possible Errors from this method:
		"FORMAT DISK: DISK already formated" - If the disk formated already.
- CREATE FILE - Create a file, assigne new fsInode to this file and set it a name. push them together as a pair to OpenFileDescriptor and to the mainDir.
	This is not possible to have 2 files with the same name, but if you delete file the next file you will create will be with the deleted file fd.
	Possible Errors from this method:
		"CreateFile: ERROR: There Is Not Enough Space On The Disk" - There is not enough space on the disk. delete some file if you want to write something.
		"CreateFile: : ERROR: The Disk is not format" - You should format the Disk first.
		"CreateFile: : ERROR: Already Exist" - There is a file with the same name, you need to create a file with the name you want.
- OPEN FILE - In case the file is close, open the file. open mean give you the abilty to write to the file or to read from the file.
	Possible Errors from this method:
		"OpenFile: ERROR: The disk did not format" - You should format the Disk first.
		"OpenFile: ERROR: There Is No Such as File" - The file you want to close does not exist.
		"OpenFile: ERROR: Already Open" - Means that the file you want to open already open.
		"OpenFile: ERROR: File Deleted" - This file deleted.
- CLOSE FILE - In case the file is open, close the file. close mean block your abilty to write to the file or to read from the file.
	Possible Errors from this method:
		"CloseFile: ERROR: The disk did not format" - You should format the Disk first.
		"CloseFile: ERROR: There Is No Such A File" - The file you want to close does not exist.
		"CloseFile: ERROR: Already close" - Means that the file you want to close already close.
		"CloseFile: ERROR: File Deleted" - This file deleted.
- WRITE TO FILE - Write to file if open. Assigne blocks for the data of the file as much as you need to wirte all the data you want to write. 
	Each file can contain direct_entity directs and single inDirect contains block size directs. means total (num_of_direct_blocks + block_size) * block_size chars.
	The write method use the blocks by FIFO method use the first block that free, and as on. The data on the disk DOESN'T need to be by order of the files!
	Possible Errors from this method:
		"WriteToFile: ERROR: The disk did not format" - You should format the Disk first.
		"WriteToFile: ERROR: There is no File Descriptor such as File" - There is a file with the same FileDescriptor.
		"WriteToFile: ERROR: This File Isn't Open" - This file is close you can't edit nothing. Open the file and than you will be able to write.
		"WriteToFile: ERROR: The File Size Is Exceed From Max Size File" - The file is Exceed of the bound that format to each file to use. Separate the file to two files or use less chars.
		"WriteToFile: ERROR: There Is Not Enough Space On The Disk" - There is not enough space on the disk. delete some file if you want to write something.
		"WriteToFile: ERROR: File Deleted" - This file deleted.
- READ FROM FILE - Read from the file if open. Bring as len chars for the file. First print all the directs that in use one by one. If there is inDirect too, 
	printing the blocks that store in the inDirect one by one. DOESN'T read the blocks in the inDirect.
	Possible Errors from this method:	
		"ReadFromFile: ERROR: The disk did not format" - You should format the Disk first.
		"ReadFromFile: ERROR: There is no File Name such as File" - There is a file with the same name, you need to create a file with the name you want.
		"ReadFromFile: ERROR: File Deleted" - This file deleted.
		"ReadFromFile: ERROR: There is no File Descriptor such as File" - There is a file with the same FileDescriptor.
		"ReadFromFile: ERROR: This File Isn't Open" - This file is close you can't edit nothing. Open the file and than you will be able to write.
		"ReadFromFile: ERROR: len Is Invalid" - The len have to be Possitive number.
- DELETE FILE - Delete file from the disk. Delete all the data from the disk that related to the file you want to delete. Delete its fsInode, set 0 at the blocks
	that deleted in the BitVector (represnt the blocks in used), remove from OpenFileDescriptor and if inDirect was in use delete its block to and set the 
	bitVector to 0 too.
	Possible Errors from this method:
		"DelFile: ERROR: The disk did not format" - You should format the Disk first.
		"DelFile: ERROR: There is no File Name such as File" - There is a file with the same name, you need to create a file with the name you want. 

For compilation:
press ctrl+shift+B
or in Terminal use "g++ FinalExam.cpp main.cpp -o FianlExam".

For run:
press ctrl+f5
or in Terminal use "./FianlExam".

About the files:
FinalExam.hh - Header File include all the methods in of that program.
FinalExam.cpp - Implementaion for all the methods in the Header File.
main.cpp - The main of the program.
makefile - For compiling the program. 
inside .vscode:
	launch.JSON - For debug the program.
	tasks.JSON - For compiling the program. 

About the input:
This program is iterative program:
Each step you should write a command as a number. for the beggining you should format the disk. To do that 
press 2 (format the disk) than set the block size you would like to simulat with, and for the last how many directs you would like to simulat with.
by pressing 0: You will Delete all the files and exit the program.
by pressing 1: You will see all opened File Descriptor and all the data in the disk.
by pressing 2: Yow will format the disk, requires block size and number of directs.
by pressing 3: Yow will create a new file, requires name for the file.
by pressing 4: Yow will open a file, requires the name of the file you would like to open.
by pressing 5: Yow will close a file, requires the file descriptor of the file you would like to close.
by pressing 6: Yow will write to file, requires the file descriptor of the file you would like to write to and the text you wolud like to write.
by pressing 7: Yow will read from file, requires the file descriptor of the file you would like to read from and the lenth of the text you would like to read <=0.
by pressing 8: Yow will delete the file, requires the name of the file you would like to delete.

About the output:
by pressing 0: No output. Exit the program.
by pressing 1: The output: All opened File Descriptor and all the data in the disk.
by pressing 2: The output: How many block you will have in the simulation by the data you entered.
by pressing 3: The output: The name of the file you created and its file descriptor.
by pressing 4: The output: The name of the file you opened and its file descriptor.
by pressing 5: The output: The name of the file you closed and its file descriptor.
by pressing 6: No output. Write into the file.
by pressing 7: The output: Substring that readen from the file. The lenth as you asked.
by pressing 8: The output: The name of the file you deleted and its file descriptor.
