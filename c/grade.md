```c++
#include <iostream>

using namespace std;

int main()
{
    int grade;
    int n;
    printf("学生个数: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        cout << "分数: ";
        cin>>grade;
    
        if(grade<80){
            if(grade>=70){
                cout << "中" <<endl;
            }else if(grade>=60){
                cout<< "及格" <<endl;
            }else{
                cout<< "不及格" <<endl;
            }
        }else{
            if(grade<90){
                cout<< "良" <<endl;
            }else{
                cout<< "优" <<endl;
            }
        }
    }
    return 0;
}
```



```shell
$ g++ -g grade.cpp -o grade
$ ./grade
学生个数: 5
分数: 90
优
分数: 80
良
分数: 70
中
分数: 60
及格
分数: 50
不及格
gdb grade
(gdb) l
1       #include <iostream>
2
3       using namespace std;
4
5       int main()
6       {
7           int grade;
8           int n;
9           printf("学生个数: ");
10          scanf("%d", &n);
(gdb) b 7
Breakpoint 1 at 0xa3c: file grade.cpp, line 7.
(gdb) r
Starting program: /home/mygit/computer-system/c/grade 

Breakpoint 1, main () at grade.cpp:9
9           printf("学生个数: ");
(gdb) n
10          scanf("%d", &n);
(gdb) 

```

