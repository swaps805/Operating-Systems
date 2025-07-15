#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"


int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Usage: createfile filename filesize\n");
        exit(0);
    }

    char *filename = argv[1];
    int filesize = atoi(argv[2]);
    char *rollNo = "ES22BTECH11034"; 

    int fd = open(filename, O_CREATE | O_RDWR); 
    
    // divide thethe file allocated into approprite no. o blocks
    for(int i = 0; i<(filesize + BSIZE - 1)/ BSIZE; i++){
        write(fd, rollNo, BSIZE);
    }

    struct stat st;
    fstat(fd, &st);

    printf("Inode number: %d\n", st.ino); // inode number
    
    // fetch the disk blocks
    
    getDiskBlock(fd);
    close(fd);
    printf("\n");

    exit(0);
}
