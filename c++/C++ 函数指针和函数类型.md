# C++ 函数指针和函数类型

## 函数指针和函数类型

- **函数指针**指向的是函数而非对象。和其他指针类型一样，函数指针指向某种特定类型。
- **函数类型**由它的返回值和参数类型决定，与函数名无关。



```cpp
bool length_compare(const string &, const string &);
```

上述函数类型是：`bool (const string &, const string &)`;
 上述函数指针pf：`bool (*pf)(const string &, const string &)`;

### 使用函数指针

- 当把函数名作为一个值使用时，该函数自动的转换成指针，如：



```dart
pf = length_compare <=>等价于pf = &length_compare
```

### 函数指针形参

- 函数类型不能定义为形参，但是形参可以是指向函数的指针；

- 函数作为实参使用时，会自动的转换成函数指针；

  

  ```cpp
  typedef bool Func(const string &, const string &) // Func是函数类型；
  typedef bool (*FuncP)(const string &, const string &) // FuncP是函数指针类型；
  ```

  

  ```cpp
  typedef decltype(length_compare) Func2  // Func2是函数类型；
  typedef decltype(length_compare) *Func2P // Func2是函数指针类型；
  ```

  > 注意decltype(length_compare)返回的是函数类型，而不是函数指针类型；



```cpp
using FTtype = int(int,int); //函数类型
typedef int (*pf)(int, int); //函数指针

int func(int a, int b){return a+b;}
void print(int a, int b, FTtype fn){
    // 编译器将其隐式转化成函数指针
    cout << fn(a,b) << endl;
}

int main()
{
    print(1,2,func);
    cout << typeid(FTtype).name() << endl;  // FiiiE
    cout << typeid(func).name() << endl;    // FiiiE
    cout << typeid(decltype(func)).name() << endl;  // FiiiE
    cout << typeid(pf).name() << endl;  // PFiiiE
    return 0;
}
```

- 下面两个声明语句是同一个函数，因为编译器会自动的将FTtype 转换成函数指针类型。

  

  ```cpp
  void print(int a, int b, FTtype  fn);
  void print(int a, int b, pf fn);
  ```

### 返回指向函数的指针

虽然不能返回一个函数，但是能返回执行函数类型的指针。和函数参数不同，编译器不会自动地将函数返回类型当作指针类型处理，必须显示的将返回类型指定为指针。如：



```cpp
using F = int(int*, int);
using PF = int(*)(int*,int);
F  f1(int);    //错误： F是函数类型
PF  f1(int);   //正确： PF是函数指针类型
```

f1也可以写出下面两种形式：



```csharp
int (*f1(int))(int*, int);
auto f1(int)->int(*)(int*, int);
```



作者：georgeguo
链接：https://www.jianshu.com/p/6ecfd541ec04