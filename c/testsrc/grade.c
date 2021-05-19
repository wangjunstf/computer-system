#include <stdio.h>

int main(){
    int grade;
    int n;
    printf("学生个数: ");
    scanf("%d",&n);

    for(int i=0; i<n; i++){
        printf("分数: ");
        scanf("%d",&grade);

        if(grade>=90 && grade<=100){
            printf("优秀\n");
            continue;
        }

        if (grade >= 80)
        {
            printf("良好\n");
            continue;
        }

        if (grade >= 70)
        {
            printf("中等\n");
            continue;
        }

        if (grade >= 60)
        {
            printf("及格\n");
            continue;
        }

        if (grade >= 0)
        {
            printf("不及格\n");
        }
    }
    return 0;
}