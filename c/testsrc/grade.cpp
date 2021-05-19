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
