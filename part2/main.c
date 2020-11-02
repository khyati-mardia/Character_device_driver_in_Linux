/*   A test program for /dev/rbt530
		To run the program, enter "rbt530_tester show" to show the current contend of the buffer.
				enter "rbt530_tester write <input_string> to append the <input_string> into the buffer

*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <linux/ioctl.h>
#include <linux/rtc.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define ASCENDING_ORDER_CMD 0
#define DESCENDING_ORDER_CMD 1
#define READING_TREE_CMD 2
//#define READ_ORDER_CMD _IOW('k', 1, int)
#define READ_ORDER_CMD 100

#define OPS_LIMIT 100
#define WRITE_LIMIT 50
#define _dataCount 50
#define _opsCount 100

void* rbWrite_dev1(void* randomDelay){

//printf("Hi1 \n");

	int key , data;
	char array[2];
	int fd1=0;
	int write_count1=0;
	int count = 0;
	int flag = 0;

	while(count<4){

		pthread_t myid;
		myid = pthread_self();
		printf("Thread ID = %lu\n",myid); 

	fd1 = open("/dev/rbt530_dev1", O_RDWR);

	if(write_count1<2){
		flag=1;
	}
	else{
		flag = rand() % 2;
	}

	if(flag==1){

	if (fd1 < 0 ){
		printf("Can not open device 1 file %d\n",fd1);	
		close(fd1);	
		return 0;
	}else{
		printf("rbt530_dev1 \n");

		//for(write_count1=0;write_count1<50;write_count1++){
		key = rand() % 100;
		data = rand() % 100;
		write_count1++;
		array[0] = key;
		array[1] = data;
		int res1 = write(fd1, array, 2);
		
			//if(res1==1){
			printf("Writing device 1 %d", res1);
		//}
			printf("key %d data %d \n",array[0], array[1]);
			}
		}
	else {
		unsigned long cmd = rand() % 2;
		printf("IOCTL ID %d, command %lu \n", READ_ORDER_CMD, cmd);
		long cmd_new = ioctl(fd1, READ_ORDER_CMD, cmd);
		printf("Ioctl  : %ld errno %d\n",cmd_new, errno);
		int result = read(fd1,array,100);			
				if(cmd==0)
					printf("Mnimum Value : %d\n",result);
				else
					printf("Maximum value : %d\n",result);
	}
	count++;
	printf("Total count %d", count);

	close(fd1);
	usleep(*((int*)randomDelay));
	}
pthread_exit(NULL);
}

void* rbWrite_dev2(void* randomDelay){

//printf("Hi2 \n");

	int key , data;
	char array[2];
	int fd1=0;
	int write_count1=0;
	int count = 0;
	int flag = 0;

	while(count<100){

		pthread_t myid;
		myid = pthread_self();
		printf("Thread ID dev 2= %lu\n",myid);

	fd1 = open("/dev/rbt530_dev2", O_RDWR);

	if(write_count1<50){
		flag=1;
	}
	else{
		flag = rand() % 2;
	}

	if(flag==1){

	if (fd1 < 0 ){
		printf("Can not open device 2 file %d\n",fd1);	
		close(fd1);	
		return 0;
	}else{
		printf("rbt530_dev2 \n");
		//for(write_count1=0;write_count1<50;write_count1++){
		key = rand() % 100;
		data = rand() % 100;
		write_count1++;
		array[0] = key;
		array[1] = data;
		int res1 = write(fd1, array, 2);
		
			//if(res1==1){
			printf("Writing device 2 %d", res1);
		//}
			printf("key %d data %d \n",array[0], array[1]);
			}
		}
	else{
		unsigned long cmd = rand() % 2;
		printf("IOCTL ID %d, command %lu \n", READ_ORDER_CMD, cmd);
		long cmd_new = ioctl(fd1, READ_ORDER_CMD, cmd);
		printf("IOCTL new command %lu \n", cmd_new);
		int result = read(fd1,array,1);			
				if(cmd==0)
					printf("Mnimum Value : %d\n",result);
				else
					printf("Maximum value : %d\n",result);
	}
	count++;
	printf("Total count %d", count);
	close(fd1);
	sleep(*((int*)randomDelay));
	}
pthread_exit(NULL);
}

int reading_tree(){
	char dump_tree_1[100];
	memset(dump_tree_1,0,100);

	printf("\n\n ----------------------------------- \n\n");
	printf("All the data from both trees\n");

	int rbt530_1 = open("/dev/rbt530_dev1", O_RDWR);
	if(rbt530_1 < 0) {
		printf("Error opening rb tree 1 for dump %d\n", errno);
		return -1;
	}
	long returnVal = ioctl(rbt530_1, READ_ORDER_CMD, 2);
	
	if(returnVal < 0) {
		printf("Error ioctl dump tree 1\n");
		close(rbt530_1);
		return -1;
	}
	
	int ret = read(rbt530_1,dump_tree_1,100);
	printf("Read the tree of size : %d\n",ret);
	
	//Dump all the data from the tree.
	printf("RBTree 1\n");
	int i;
	for(i=0;i<ret;i++) {
		printf("%d,",dump_tree_1[i]);
	}
 	printf ("\n");
	close(rbt530_1);
	
	char dump_tree_2[100];
	memset(dump_tree_2,0,100);

	int rbt530_2 = open("/dev/rbt530_dev2", O_RDWR);
	if(rbt530_2 < 0) {
		printf("Error opening rb tree 2 for dump %d\n", errno);
	}
	returnVal = ioctl(rbt530_2, READ_ORDER_CMD, 2);
	
	if(returnVal < 0) {
		printf("Error dumping tree 2\n");
	}
	
	ret = read(rbt530_2,dump_tree_2,100);
	printf("Read the tree of size : %d\n",ret);
	
	//Dump all the data from the tree.
	printf("RBTree 2\n");

	for(i=0;i<ret;i++) {
		printf("%d,",dump_tree_2[i]);
	}
 	printf ("\n");
	close(rbt530_2);
	
	return 0;
}

void* rbprobe(void* randomDelay){

	char input[2];
	input[0]=0;
	input[1]=1;
	char output[256];
	memset(output,0,256);
 	int fd,retVal;
	//printf("open fd %d ", fd);
	/* open devices */
	fd = open("/dev/RBprobe", O_RDWR);
	printf("open fd %d ", fd);
	if (fd < 0 ){
		printf("Can not open RBprobe file %d\n",fd);		
		return 0;
	}
	else
	{
		printf("successful opening RBProbe");
	}

	close(fd);
	pthread_exit(NULL);
	
	retVal = write(fd,input,2);
	if (retVal < 0){
		printf("registration of kprobe was not successful!\n");
        }
    else
    {
    	printf("kprobe registration successful");
    }

	while(_dataCount<WRITE_LIMIT){
		int res = 0;
		
		do {
			res = read(fd,output,256);
		} while(res==0);
		//printf("Reading the KPROBE buffer values....\n");
		
		//if(res>0){
		//int i=0;	
		//printf("Characters returned = %d\n",res);
		//for(i;i<res;i++) printf("%c",output[i]);
		
	}
	if(_opsCount > OPS_LIMIT){
		//input[1] == 0;
		retVal = write(fd,input,2);
		close(fd);
		pthread_exit(NULL);
	}
}

int main(int argc, char **argv)
{

	printf("Hello \n");

	pthread_t thread1; 
	pthread_t thread5; 
	//pthread_t thread2;
	//pthread_t thread3; 
	//pthread_t thread4;
	//int t1, t2, t3, t4;

	int delay = rand() % 10+1;

	pthread_create(&thread1, NULL, rbWrite_dev1, (void*)&delay);
	//pthread_create(&thread2, NULL, rbWrite_dev1, (void*)&delay);
	//pthread_create(&thread3, NULL, rbWrite_dev2, (void*)&delay);
	//pthread_create(&thread4, NULL, rbWrite_dev2, (void*)&delay);
	pthread_create(&thread5, NULL, rbprobe, (void*)&delay);

	pthread_join(thread1, NULL);
	//pthread_join(thread2, NULL);	
	//pthread_join(thread3, NULL);
	//pthread_join(thread4, NULL);
	pthread_join(thread5, NULL);
	reading_tree();

	return 0;
}