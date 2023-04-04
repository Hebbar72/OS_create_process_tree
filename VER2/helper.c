#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/signal.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>

int* ptr;
int parent;
int shmid;
int parent1;

int createProcessTree(int n) {
    if(n < 3)
        return -1;
    if(n > 6)
        return -2;

    int arr[n]; // contains the number of children associated with each process on the second level
    int pid_arr[n]; // array of pids of the processes in the second level
    parent = getpid();

    //generating arr
    for(int i = 0;i < n; i += 2) {
        arr[i] = n - i/2;
    }
    for(int i = 1;i < n;i += 2) {
        arr[i] = i/2 + 1;
    }

    int i;
    int pid;
    
    for(i = 0;i < n; i++) {
        pid = fork();
        if(!pid) {
            parent1 = getpid();
            kill(getpid(), SIGSTOP);
            printf("value returned by fork : child pid = %d : %d\n", pid, getpid());
            printf("pid, _, no of children: %d %d %d\n", getpid(), i, arr[i]);
            int value = arr[i];
            // creating processes in the third level
            for(int j = 0;j < value;j++) {
                pid = fork();
                if(!pid) {
                    kill(getpid(), SIGSTOP);
                    printf("parent pid:%d\n", getppid());
                    shmid = shmget(getppid(), sizeof(arr), 0777);
                    ptr = (int*)shmat(shmid, NULL, 0); // attaching shared memory to child process
                    break;
                }
                else {
                    pid_arr[j] = pid;
                    waitpid(pid, NULL, WUNTRACED);
                }
            }
            if(parent1 == getpid()) {
                for(int j = 0;j < value - 1;j++) {
                    for(int k = 0;k < value - 1;k++) {
                        if(pid_arr[k] > pid_arr[k + 1]) {
                            int temp = pid_arr[k];
                            pid_arr[k] = pid_arr[k + 1]; 
                            pid_arr[k + 1] = temp;
                        }
                    }
                }
                // creating shared memory
                shmid = shmget(getpid(), sizeof(arr), 0777|IPC_CREAT);
                ptr = (int*)shmat(shmid, NULL, 0);

                for(int j = 0;j < value;j++) {
                    *(ptr + j) = pid_arr[j];
                }
            }
            int flag = 0;
            for(int j = 0;j < value;j++) {
                int temp = *(ptr + j);
                if(temp != -1 && temp%2 == 1) {
                    *(ptr + j) = -1;
                    kill(temp, SIGCONT);
                    while(wait(NULL) != -1);
                    shmdt(ptr);
                    if(parent1 == getpid()) {
                        shmctl(shmid, IPC_RMID, NULL); // destroying shared memory
                        flag = 1;
                        break;
                    }
                    exit(0);
                }
            }
            if(flag) {
                while(wait(NULL) != -1);
                shmid = shmget(getppid(), sizeof(arr), 0777);
                ptr = (int*)shmat(shmid, NULL, 0); // attaching shared memory to child process
                break;
            }
            for(int j = 0;j < value;j++) {
                int temp = *(ptr + j);
                if(temp != -1 && temp%2 == 0) {
                    *(ptr + j) = -1;
                    kill(temp, SIGCONT);
                    while(wait(NULL) != -1);
                    shmdt(ptr);
                    if(parent1 == getpid()) {
                        shmctl(shmid, IPC_RMID, NULL); // destroying shared memory
                        break;
                    }
                    exit(0);
                }
            }
            if(getpid() == parent1) { 
                while(wait(NULL) != -1);
                shmid = shmget(getppid(), sizeof(arr), 0777);
                ptr = (int*)shmat(shmid, NULL, 0); // attaching shared memory to child process
                break;
            }
            
            exit(0);
        }
        else if(pid == -1) {
            perror("Child not created");
            exit(-4);
        }
        else {
            pid_arr[i] = pid;
            printf("childlist : %d\n", pid);
            waitpid(pid, NULL, WUNTRACED);
        }
    }
    if(parent == getpid()) {
        for(int i = 0;i < n - 1;i++) {
            for(int j = 0;j < n - 1;j++) {
                if(pid_arr[j] > pid_arr[j + 1]) {
                    int temp = pid_arr[j];
                    pid_arr[j] = pid_arr[j + 1]; 
                    pid_arr[j + 1] = temp;
                }
            }
        }
        // creating shared memory
        shmid = shmget(getpid(), sizeof(arr), 0777|IPC_CREAT);
        ptr = (int*)shmat(shmid, NULL, 0);
        for(int i = 0;i < n;i++) {
            *(ptr + i) = pid_arr[i];
        }
    }
    for(int i = 0;i < n;i++) {
        
        int temp = *(ptr + i);
        if(temp != -1 && temp%2 == 1) {
            *(ptr + i) = -1;
            kill(temp, SIGCONT);
            while(wait(NULL) != -1);
            shmdt(ptr);
            if(parent == getpid())
                shmctl(shmid, IPC_RMID, NULL); // destroying shared memory
            exit(0);
        }
    }
    for(int j = 0;j < n;j++) {
        int temp = *(ptr + j);
        if(temp != -1 && temp%2 == 0) {
            *(ptr + j) = -1;
            kill(temp, SIGCONT);
            while(wait(NULL) != -1);
            shmdt(ptr);
            if(parent == getpid())
                shmctl(shmid, IPC_RMID, NULL); // destroying shared memory
            exit(0);
        }
    }
    return 0;
}
