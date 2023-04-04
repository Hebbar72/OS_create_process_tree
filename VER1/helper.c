#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/signal.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>

#define SHM_KEY 0x1235
int* ptr;
int parent;
int shmid;

int createProcessTree(int n)
{
    if(n < 3)
        return -1;
    if(n > 6)
        return -2;

    int arr[n]; // contains the number of children associated with each process on the second level
    int pid_arr[n]; // array of pids of the processes in the second level
    parent = getpid();

    //generating arr
    for(int i = 0;i < n; i += 2)
    {
        arr[i] = n - i/2;
    }
    for(int i = 1;i < n;i += 2)
    {
        arr[i] = i/2 + 1;
    }

    int i;
    int pid;
    
    for(i = 0;i < n; i++)
    {
        pid = fork();
        if(!pid)
        {
            kill(getpid(), SIGSTOP);
            printf("value returned by fork : child pid = %d : %d\n", pid, getpid());
            shmid = shmget(SHM_KEY, sizeof(arr), 0777);
            
            ptr = (int*)shmat(shmid, NULL, 0); // attaching shared memory to child process
            printf("pid, _, no of children: %d %d %d\n", getpid(), i, arr[i]);
            int value = arr[i];
            // creating processes in the third level
            for(int j = 0;j < value;j++)
            {
                int pid = fork();
                if(!pid)
                {
                    printf("parent pid:%d\n", getppid());
                    exit(0);
                }
                else
                    waitpid(pid, NULL, WUNTRACED);
            }
            break;
        }
        else if(pid == -1)
        {
            perror("Child not created");
            exit(-4);
        }
        else
        {
            pid_arr[i] = pid;
            printf("childlist : %d\n", pid);
            waitpid(pid, NULL, WUNTRACED);
        }
    }

    if(parent == getpid())
    {
        for(int i = 0;i < n - 1;i++)
        {
            for(int j = 0;j < n - 1;j++)
            {
                if(pid_arr[j] > pid_arr[j + 1])
                {
                    int temp = pid_arr[j];
                    pid_arr[j] = pid_arr[j + 1]; 
                    pid_arr[j + 1] = temp;
                }
            }
        }
        // creating shared memory
        shmid = shmget(SHM_KEY, sizeof(arr), 0777|IPC_CREAT);
        ptr = (int*)shmat(shmid, NULL, 0);
        for(int i = 0;i < n;i++)
        {
            *(ptr + i) = pid_arr[i];
        }
         for(int i = 0;i < n;i++)
        {
            printf("%d ", *(ptr + i));
        }
        printf("\n");
    }

    for(int i = 0;i < n;i++)
    {
        int temp = *(ptr + i);
        if(temp != -1 && temp%2 == 1)
        {
            kill(*(ptr + i), SIGCONT);
            *(ptr + i) = -1;
            while(wait(NULL) != -1);
            shmdt(ptr);
            if(parent == getpid())
                shmctl(shmid, IPC_RMID, NULL); // destroying shared memory
            exit(0);
        }
    }
    for(int i = 0;i < n;i++)
    {
        int temp = *(ptr + i);
        if(temp != -1 && temp%2 == 0)
        {
            kill(*(ptr + i), SIGCONT);
            *(ptr + i) = -1;
            while(wait(NULL) != -1);
            shmdt(ptr);
            if(parent == getpid())
                shmctl(shmid, IPC_RMID, NULL); // destroying shared memory
            exit(0);
        }
    }
    return 0;
}
