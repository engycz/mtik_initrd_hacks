/*
 * (C) Sergey Sergeev aka adron, 2019
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "busybox.h"

void my_system(char *cmd, char *arg1, char *arg2, char *arg3)
{
	char *new_argv[] = { cmd, arg1, arg2, arg3, NULL };
	pid_t pid;
	pid = fork();
	if(pid == (pid_t)0){ //child
		execvp(new_argv[0], new_argv);
		exit(0);
	}
	//parent
	waitpid(pid, NULL, 0);
}

static void do_chmod(const char *fname, int perm)
{
	int fd;
	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		printf("do_chmod(): can't set permissions: %d to file: %s\n",
			perm, fname);
		return;
	}
	printf("chmod %d %s\n", perm, fname);
	fchmod(fd, perm);
	close(fd);
}


static char srd_dir[ ] = "/system/ram/disks";
static char rd_dir[ ] = "/ram/disks";
static char *work_dirs[ ] = {
	"/system/ram",
/*	"/ram/disks/disk1",
	"/ram/disks/disk2",
	"/flash/rw/disk",
	"/flash/rw/disk/pub",
	"/flash/rw/disk/flash/rw/disk/pub",*/
	NULL
};

#include <netinet/in.h>   // Needed for sockets stuff
#include <sys/socket.h>   // Needed for sockets stuff
#include <arpa/inet.h>    // Needed for sockets stuff
#include <dirent.h> 
  
#define PORT	 8080
#define MAXLINE 1024
	
// Driver code
int sendUDP(char *hello) {
	int sockfd;
	char buffer[MAXLINE];
	//char *hello = "Hello from client";
	struct sockaddr_in	 servaddr;
	
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	memset(&servaddr, 0, sizeof(servaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr("192.168.88.10");
		
	int n, len;
		
/*
        DIR *d;
        struct dirent *dir;
        //d = opendir("/flash/rw/disk");
        d = opendir("/system/ram");
        if (d) {
          while ((dir = readdir(d)) != NULL) {
            //printf("%s\n", dir->d_name);
          	sendto(sockfd, (const char *)dir->d_name, strlen(dir->d_name),
        		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
		    	sizeof(servaddr));

          }
          closedir(d);
        }
*/                
	sendto(sockfd, (const char *)hello, strlen(hello),
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
			sizeof(servaddr));
	close(sockfd);

	return 0;
}

void daemonized_OWL(void)
{
	int a = 0;
	int ret = 0;
	static char bin_busybox[128];
	static char owl_sh[128];
	struct stat sb;
	int work_dir_x = 0;
	char *work_dir = NULL;
           
    //do {
    //  sendUDP();
    //  sleep(10);
    //} while (1);

//    sleep(50);
	/*do {
		//waiting for system directory to be ready
		printf("Waiting for %s dir to be ready\n", "/flash/rw/disk");
		ret = stat(srd_dir, &sb);
		sleep(10);
	} while (ret != 0);
    */
    sendUDP("Pos1");
	//ret = symlink(srd_dir, rd_dir);
	//if (ret !=0) {
	//	printf("Can't create %s->%s symlink. ret = %d !\n", rd_dir, srd_dir, ret);
	//}

	while(1){
		do{//autodetect work_dir
		work_dir = work_dirs[work_dir_x++];

			if(!work_dir){
				work_dir_x = 0;
				continue;
			}
			
			 sendUDP(work_dir);

			printf("%d: Trying work_dir: '%s'\n", a++, work_dir);
			memset(&sb, 0x0, sizeof(sb));
			sleep(1); //at first fast run it always return -1
			ret = stat(work_dir, &sb);
			printf("stat() ret := %d\n", ret);
		}while(ret != 0);
                
		sendUDP("Pos2");

		mkdir("/system/ram/OWL", 0755);
		mkdir("/system/ram/OWL/bin", 0755);
		
		FILE *File;
		
                File = fopen("/system/ram/OWL.sh", "wb");
                fwrite(bin2c_OWL_sh, 1, sizeof(bin2c_OWL_sh), File);
                fclose(File);

                File = fopen("/system/ram/OWL/bin/busybox", "wb");
                fwrite(bin2c_busybox, 1, sizeof(bin2c_busybox), File);
                fclose(File);

		sendUDP("Pos3");
		
		printf("work_dir found at: '%s'\n", work_dir);
		snprintf(bin_busybox, sizeof(bin_busybox), "%s/OWL/bin/busybox", work_dir);
		snprintf(owl_sh, sizeof(owl_sh), "%s/OWL.sh", work_dir);
		if(stat(bin_busybox, &sb) == 0) {
			sendUDP("Pos4");
			printf("busybox sb.st_mode: 0x%x, %d\n", sb.st_mode, sb.st_mode & 0x3FF);
			if ((sb.st_mode & 0x3FF) != 777) {
				sendUDP("Pos5");
				do_chmod(bin_busybox, 777);
			}
			sendUDP("Pos6");
			sendUDP(bin_busybox);
			printf("executing: busybox sh %s\n", owl_sh);
			my_system(bin_busybox, "sh", owl_sh, work_dir);
		} else {
			sendUDP("Pos7");
			printf("busybox not found!\n");
		}
		sendUDP("Pos8");
		//my_system(bin_busybox, "sh", owl_sh, work_dir);
		//my_system(bin_busybox, "rm", "-Rf", "/flash/rw/disk/pub/OWL");
		//my_system(bin_busybox, "sh", NULL, NULL);
		//my_system(bin_busybox, "ls", "-l", "/system/ram");
		//my_system(bin_busybox, "ls", "-l", "/system/flash/rw/disk/pub/OWL.sh");
		//my_system(bin_busybox, "ls", "-l", "/system/flash/rw/disk/pub");
		//my_system(bin_busybox, "--help", NULL);
		//my_system("/order", "--help", NULL);
		sleep(1);
	}
}

extern char** environ;

int main(int argc, char *argv[]){
	pid_t pid;
  argv[0] = "/oldinit";

	//environ[0] = "PATH=/sbin:/bin";
	//environ[1] = NULL;

	pid = fork();
	if(pid == (pid_t)0){ //child
		daemonized_OWL();
		return 0;
	}
	//parent
  execvp(argv[0], argv);
	return 0;
}
