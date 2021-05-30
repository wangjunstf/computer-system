#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

void *thread_main(void *args);
int main(){
    pthread_t t_id;
    if(pthread_create(&t_id, NULL, thread_main, NULL) !=0 ){
        puts("pthread_create error");
        return -1;
    }
    
    pthread_detach(t_id);

    sleep(10);
    puts("end of main");
    return 0;
}

void* thread_main(void *args){
    int n = 100;
    while(n>0){
        printf("子进程\n");
        n--;
        sleep(1);
    }
    return NULL;
}.eee