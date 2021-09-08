# C++中的可调用对象学习

## 前言

- Q: 本文讨论初衷？
- 可调用对象在TR1(Technical Report 1)之后的C++中,成为了重要概念。从C时代的函数指针，到广泛使用`tr1::function`和`tr1::bind`组件的现在,理解并运用可调用对象是学习现代C++必不可少的部分。

## 可调用对象

- 可调用对象:
- 函数
- [函数指针](https://zhuanlan.zhihu.com/write#函数指针)
- [函数对象](https://zhuanlan.zhihu.com/write#函数对象)
- [lambda表达式](https://zhuanlan.zhihu.com/write#lambda表达式)
- 组件:
- [std::tr1::bind](https://zhuanlan.zhihu.com/write#stdbind)
- [std::tr1::function](https://zhuanlan.zhihu.com/write#stdfunction)

## 函数指针

- 函数指针的出现是因为想要把`函数当作变量来处理`。抽离出参数类型和返回值后便可以确认函数类型，并用指针来表示，C时代的产物，兼容性好但是扩展性差。
- 注意区分函数类型和函数指针类型:

```cpp
using Foo = void(int, int)  //Foo是函数
using pFoo = void(*)(int, int)  //pFoo是函数指针类型
```

- 函数指针的赋值，取地址符是可选的

```cpp
void Foo(int i);
//下面两种赋值方式是等价的
void (*pFoo)(int) = Foo;
void (*pFoo)(int) = &Foo;
```

- 函数指针可以做行参，声明时*符号可选

```cpp
//下面两种声明是等价的
void Foo(int i, void(*pFoo)(int));
void Foo(int i, void pFoo(int));
```

- 函数指针也可以做返回值

```cpp
typedef void (*pFoo)(int);
pFoo bar(int i);
```

- 函数指针常和typedef一同使用，让代码更简洁

```cpp
void bar(int i);
typedef void (*pFoo)(int);
pFoo foo = bar;
```

- 函数指针和decltype的使用

```cpp
void bar(int i);
void baz(int i);
//pFoo函数返回指向bar返回类型函数的指针。这里需要加上*，因decltype返回的是函数类型不是指针。这种情况适用于不知道函数表式具体返回值时。
decltype(bar) *pFoo(float);
```

## 函数对象

- 如果在类中重载了`调用运算符`，则该类的对象称作`函数对象`。

```cpp
class AddNumber
{
public:
      //重载调用运算符
      int operator() (int firstNum, int secondNum) const
      {
          return firstNum + secondNum;
      }  
      int operator() (int firstNum, int secondNum, intthirdNum)
      {
          return firstNum + secondNum + thirdNum;
      }  
};  
int main()
{
    //实例化函数对象
    AddNumber add;
    //结果是3
    add(1,2);
    //结果是6
    add(1,2,3);  
    return 0;
}
```

- 函数对象和函数指针相比，因为是类所以可以`储存和提取状态`，而且因为函数对象的实现可以在类内修改和重载，还可以做inline函数调用，所以设计灵活性上优于函数指针。在泛型算法中大量用到了函数对象作为实参。头文件functional中定义了一组算数运算符，关系运算符，逻辑运算符的模板类作为函数对象来调用。

![img](https://pic4.zhimg.com/80/v2-5e56e2bc12075dc146dc0ba14454becb_1440w.jpg)



```cpp
//可以看下greater<Type>的源码
template <class T>
struct greater
{
    bool operator()(const T& x, const T& y) const{return x > y;}
};
```

## lambda表达式

- lanbda函数可以理解为未命名的内联函数，与一般函数不同lambda可以定义在函数内部。

```cpp
[capture list](parameter list) -> return type {functionbody}
//参数列表和返回类型可以忽略
auto foo = []{return 1;}
//调用方式和正常函数相同
std::cout << foo();
```

- 捕获列表

```cpp
//若要在lambda中使用其所在区块中的变量，需要先捕获
void foo()
{
    string m_s = "hello";  
    //值捕获，m_i和m_s是变量拷贝
    auto bar = [m_s]{return m_s;};
    m_s = "hello world";
    //结果是hello,foo里面储存的是捕获时的副本
    std::cout << bar();  
    //引用捕获，m_i和m_s是变量本身
    string m_s = "hello";
    auto bar = [&m_s]{return m_s;};
    m_s = "hello world";
    //结果是hello world
    std::cout << bar();  
    //对作用域内所有变量采用值捕获
    auto baz = [=]{return m_s;};
    //对作用域内所有变量采用引用捕获
    auto baz = [&]{return m_s;};
}
```

- 对外部变量的mutable和自定返回类型

```cpp
void foo()
{
    int m_i = 1;
    //若需要改变外部变量，需要加上关键字```mutable```
    auto bar = [m_i]() mutable {return ++m_i;};
    //结果是2
    std::cout << bar();  
    //lambda表达式中若包含了除return以外其他语句，则需要显指定返回值类型，这里使用了```返回类型后置声明```
    auto baz = [m_i]() -> int{if m_i < 0 return -m_i;elsereturn m_i;};
}
```

- lambda常和算法配合使用，大大简化一些简单函数的功能的调用

```cpp
vector<string> doc;
vector<bool> results;
results.resize(doc.size());  
//调用sort算法将doc中所有语句按字符串长度降序排序
stable_sort(doc.begin(), doc.end(),
         [](const string &a, const string &b){returna.size() < b.size();});  
//用find_if算法返回doc中第一个长度大于5的字符串的literator
auto firstLargeString = find_if(doc.begin(), doc.end(),
                               (const string &a){returna.size() > 5});  
//用for_each算法打印doc中的每条字符串并换行
for_each(doc.begin(), doc.end(),
         [](const string &a){std::cout << a << std::endl;);  
//用transform算法判断doc中每个字符串是否长度是否大于5，并将果写入results中
transform(doc.begin(), doc.end(), results.begin(),
          [](const string &a){return a.size() > 5};);
```

## std::bind

- bind常和标准库函数对象进行适配调用

```cpp
int main()
{
    //可以正常声明函数对象
    int a = 5;
    std::plus<int> addWithFive;
    //5 + 5
    addWithFive(a,5);    
    //bind可以将函数对象和特定的调用参数绑定并在函数调用时传参数
    //placeholders是参数占位符
    auto addWithFive = std::bind(std::plus<int>,std::placeholders::_1, 5);
    //5 + 5 
    addWithFive(5);  
    //在STL算法中也可以用标准库函数对象
    vector<int> foo {3,1,4,6,4,8,9,6};
    //这里会将sort默认的less变为greater来排序
    sort(foo.begin(), foo.end(), greater<int>());
}
```

## std::function

- function作为模板库中用来对可调用对象包装的包装器，可以`统一`上述四种可调用对象的`外部调用方式`，并且与可调用对象自身类型`解耦`，只依赖于call signature.
- 包装普通函数和模板函数

```cpp
int add(int i, int j)
{
    return i + j;
}
//普通函数
function<int(int, int)> f_add = add;
f_add(1,1);
//函数指针
int (*p_add)(int, int) = add;
function<int(int, int)> f_add = add;
f_add(1,1);
template<typename T>
T add(T i, T j)
{
    return i + j;
}
//模板函数
function<int<int, int>> f_add = add<int>;
f_add(1,1);
```

- 包装函数对象

```cpp
struct add
{
    int operator()(int i, int j)
    {
        return i + j;
    }
};
//非模板函数对象
function<int(int, int)> f_add = add();
f_add(1,1);
template<typename T>
struct add
{
    T operator()(T i, T j)
    {
        return i + j;
    }
};
function<int(int,int)> f_add = add<int>();
f_add(1,1);
```

- 包装lambda表达式

```cpp
auto add = [](int i, int j){return i+ j;};
//lambda表达式
function<int(int,int)> f_add = add;
f_add(1,1);
```

- 包装类成员函数

```cpp
class Math
{
public:
    int add(int i, int j)
    {
        return i + j;
    }
};
//类成员函数
Math m;
function<int(int,int)> f_add = bind(&Math::add, &m, placeholders::_1, placeholders::_2);
f_add(1,2);
template<Typename T>
class Math
{
public:
    T add(T i, T j)
    {
        return i + j;
    }
};
//模板类成员函数
Math m;
function<int(int,int)> f_add = bind(&Math::add<int>, &m, placeholders::_1, placeholders::_2);
f_add(1,2);
```



原文地址：https://zhuanlan.zhihu.com/p/110591071