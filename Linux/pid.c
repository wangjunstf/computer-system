#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main(){
    pid_t pid;

    pid = fork();
    if(pid==0){
        pid = fork();
        if (pid == 0)
        {
            while (1)
            {
                printf("子进程的子进程\n");
                sleep(1);
            }
        }
        
        while(1){
            printf("子进程\n");
            sleep(1);
        }
    }else{
        while (1)
        {
            printf("父进程\n");
            sleep(1);
        }
    }

    return 0;
}