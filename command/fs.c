#include "stdio.h"
#include "type.h"
#include "string.h"
int main(int argc, char * argv[])
{
	if(argc <= 2){
		printf("WRONG PARAMETAR NUM\n");
		return -1;
	}
	else if(strcmp(argv[1],"ls")==0){
		printf("command ls\n");
		ls("/");
	}
	else if(strcmp(argv[1],"mkdir")==0){

		printf("command mkdir\n");

		if(argc <= 3){
                	printf("WRONG PARAMETAR NUM\n");
                	return -1;
        	}

		int fd = open(argv[2], O_RDWR);
    		if (fd != -1){
        		printf("Failed to create a new directory with name %s\n", argv[2]);
			close(fd);
        		return -1;
    		}
		mkdir(argv[2]);
	}
	else if(strcmp(argv[1],"touch")==0){
		
		printf("command touch\n");

		if(argc <= 3){
			printf("WRONG PARAMETAR NUM\n");
                        return -1;
		}
		
		int fd = open(argv[2], O_RDWR);

   		if (fd != -1){
        		printf("Failed to create a new file with name %s\n", argv[2]);
			close(fd);
        		return -1;
    		}
	
		fd = open(argv[2], O_CREAT);
	
		if(fd == -1){
			printf("Failed to create a new file with name %s\n", argv[2]);
        	    	return -1;	
		}

		printf("File created: %s \n", argv[2]);
		
        }
	else if(strcmp(argv[1],"rm")==0){
		printf("command rm\n");

                if(argc <= 3){
                        printf("WRONG PARAMETAR NUM\n");
                        return -1;
                }

		int fd = open(argv[2], O_RDWR);

        	if (fd == -1){
                	printf("Failed to delete a file with name %s\n", argv[2]);
              		return -1;
        	}

		close(fd);

		unlink(argv[2]);

        }
	else {
		printf("WRONG COMMAND\n");
	}
	return 0;
}
