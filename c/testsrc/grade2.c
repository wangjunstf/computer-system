#include <stdio.h>

int main()
{
    int grade;
    int n;
    printf("学生个数: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        printf("分数: ");
        scanf("%d", &grade);

        if (grade >= 70 && grade < 79)
        {
            printf("中等\n");
            continue;
        }

        if (grade >= 80 && grade<89)
        {
            printf("良好\n");
            continue;
        }

        if (grade >= 60 && grade <70)
        {
            printf("及格\n");
            continue;
        }

        if (grade >= 90)
        {
            printf("优秀\n");
            continue;
        }

        if (grade < 60)
        {
            printf("不及格\n");
        }
    }
    return 0;
}