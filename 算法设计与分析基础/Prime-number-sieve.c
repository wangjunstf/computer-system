#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define SIZE 100

bool num[SIZE];
int main()
{
    num[1] = true;
    printf("%d",num[0]);
    printf("%d", num[1]);
    printf("%d", num[3]);
    // num[0] = true, num[1] = true; 
    // for(int i=2; i*i<100; ++i){
        
    //     if(num[i]==0){
    //         for(int j=i*i; j*j<100; j+=i){
    //             num[j]=1;
    //         }
    //     }
    // }

    // for(int i=0; i<=100; i++){
    //     if(!num[i]){
    //         printf("%d ",i);
    //     }
    // }
    return 0;
}
