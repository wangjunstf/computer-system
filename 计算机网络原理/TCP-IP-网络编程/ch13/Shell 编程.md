# Shell 编程

# Shell编程——变量，输入及输出

Shell是Linux的一个外壳，是用户与操作内核交互的一个接口。Shell由c语言写出，作为一个命令行解释器，将用户输入的命令解释执行，支持条件判断，循环操作等语法，因此可以作为一种脚步语言。利用Shell语言，可以方便地写出一些小工具。

Shell编程就是Linux命令+控制语句

本教程参考网道项目。

## 1、初识Shell编程：Hello world

echo 用于在终端中输出字符串

```shell
$ echo Hello World
# 在终端中原样输出字符串
```

用引号输出多行语句

```shell
$ echo "<html>
> <body> </body>
> </html>"
<html>
<body> </body>
</html>
```

分号连接多个Shell命令

```shell
$ echo abc; echo def
abc
def
```

`-n`参数：消除末尾的换行

```shell
$ echo -n abc; echo def
abcdef
```

`-e`参数：解释引号里的特殊字符，例如`\n`

```shell
$ echo -e  "Hello\nWorld"
Hello
World
```



## 2、Shell脚本结构

可以把多个Shell命令放到一个文件里，作为脚本，由Shell解释器依次解释执行。

一个典型的Shell脚本：hello.sh

第一行指定命令解释器程序bash，即用哪个Shell解释器执行脚本

第二行为注释：以#开头的为注释语句

```shell
#!/bin/bash
# 输入Hello World
echo Hello World
```

Linux系统默认创建的脚本文件没有执行权限，需要手动赋予其可执行权限

```shell
$ chmod u+x hello.sh
$ ./hello.sh
Hello World
```



Shell脚本可以包含：顺序结构，分支结构和循环结构



## 3、执行方式

除了2中介绍的方式，还有两种方式不需要修改权限即可执行

类似于执行python程序，使用bash shell脚本的方式执行

```shell
$ bash hello.sh 
Hello World
```

将shell脚本重定向给bash

```shell
$ bash < hello.sh Hello World
```

source shell脚本

```shell
$ source hello.sh Hello World
```



## 4、变量

### 4.1 环境变量

Bash 变量名区分大小写，`HOME`和`home`是两个不同的变量。

查看单个环境变量的值，可以使用`printenv`命令或`echo`命令。

shell预定义变量，用于设置系统运行环境(HOME)

用户不能更改，用大些字符串表示

`env`或`printenv`显示所有环境变量

* BASHPID :bash 进程的进程ID
* BASHOPTS:当前Shell的参数，可以用`shopt`命令修改
* DISPLAY: 图形环境的显示器名字，通常是`:0`，表示X Serve的第一个显示器
* EDITOR: 默认的文本编辑器
* HOME: 用户的主目录
* HOST：当前主机的名称
* IFS：词与词之间的分隔符，默认为空格
* LANG：字符集以及语言编码，比如zh_CN.UTF-8
* PATH：由冒号分开的目录列表，当输入可执行程序名后，会搜索这个目录列表
* PS1：Shell提示符
* PS2：输入多行命令时，次要的Shell提示符
* PWD：当前工作目录
* RANDOM：返回一个0到32767之间的随机数
* SHELL：Shell的名字
* SHELLOPTS：启动当前Shell的`set`命令的参数
* TERM：终端类型名，即终端仿真器所使用的协议
* UID：当前用户的ID编号
* USER：当前用户的用户名

### 4.2 位置变量

`命令名 参数1 参数2 参数3`

命令名对应 $0

参数1对应 $1

参数2对应 $2

参数3对应 $3

依次类推



### 4.3 自定义变量

#### 特点

* 无类型

* 直接赋值

`set`命令可以显示所有变量（包括环境变量和自定义变量），以及所有的 Bash 函数。

```shell
$ set
```

#### 创建变量

变量规则：

* 字母、数字和下划线组成
* 第1个字符必须是1个字母或1个下划线，不能是数字
* 不允许出现空格和标点符号



变量声明：等号左边是变量名，等号右侧是变量的值，等号两侧不能包含空格

```
variable=value
```



Bash没有数据类型的概念，所有的变量值都是字符串

下面是一些自定义变量的例子

```shell
$ a=hello     									# a赋值为字符串 "hello"$ b="Hello World"								# 变量值包含空格，就必须放在引号里面$ c="a string: $b"    					# 变量值可以引用其他变量的值$ d="onw\ttwo\tthree\n1\t2\t3" 	# 变量可以使用转移字符，输出时加参数 -e$ echo -e $donw	two	three1		2		3$ e=$(ls -l)										# 变量值可以是命令的执行结果$ f=$((8*8))										# 变量值可以是数学运算结果
```



变量可以重复赋值，后面的赋值会覆盖前面的赋值。

```shell
$ a=12$ a=13
```



### 4.4 读取变量

读取变量的时候，直接在变量名前加上`$`就可以了。

```shell
$ v=12$ echo $v12
```



如果变量不存在，Bash 不会报错，而会输出空字符。



由于`$`在 Bash 中有特殊含义，把它当作美元符号使用时，一定要非常小心，

```shell
echo The total is $100.00The total is 00.00
```

上述代码，Bash把$1当成变量，该变量为空，就输出00.00



如果要使用$符号，则必须进行转义

```shell
echo The total is \$100.00The total is $100.00
```



变量名也可以用{}括起来，例如`$a`可与写成`${a}`，用于与其他字符连用

```shell
$ a=foomygit@ubuntu:~$ echo $a_file         # 将a_file当作一个整体，其变量为空mygit@ubuntu:~$ echo ${a}_filefoo_file
```



如果变量的值本身也是变量，可以使用${!varname}语法，读取最终的值

```shell
$ name=larymygit@ubuntu:~$ var=namemygit@ubuntu:~$ echo ${!var}lary
```



如果变量值包含连续空格（或制表符和换行符），最好放在双引号里面读取。

```shell
$ var="one    two    three"mygit@ubuntu:~$ echo $varone two threemygit@ubuntu:~$ echo "$var"one    two    three
```



### 4.5 删除变量

`unset`命令用来删除一个变量

```shell
$ unset var
```



删除一个变量，等价于将变量置为空

```
$ var=''$ var=
```



### 4.6 输出变量，export命令

用户创建的变量仅可用于当前Shell，子Shell默认读取不到父 Shell 定义的变量。为了把变量传递给子Shell，需要使用`export`命令，这样输出的变量，对于子Shell来说就是环境变量。

```shell
$ var=foo$ export var
```



上面命令输出了变量`NAME`。变量的赋值和输出也可以在一个步骤中完成。

```shell
$ export var=foo
```



子Shell如果修改继承的变量，不会影响父Shell

```shell
# 输出变量$export v=1024# 新建子 Shell$ bash# 读取 $v$ echo $v1024# 修改继承的变量$ v=1000# 退出子 Shell$ exitexit# 读取父Shell中的变量$ echo $v1024
```



### 4.7 特殊变量

Bash提高一些特殊变量，这些变量由Shell提供，用户不能进行赋值

(1) $？

$？为上一个命令的退出码，用来判断上一个命令是否执行成功。返回0，表示上一个命令执行成功；如果非零，上一个命令执行失败。

```shell
$ rm tttrm: cannot remove 'ttt': No such file or directory$ echo $?1$ echo "Hello World"Hello World$ echo $?0
```



(2) $$

$$ 为当前Shell的进程ID

```shell
$ echo $$14001
```

这个特殊变量名可以用来命名临时文件

```shell
$ file=/tmp/output.$$
```



（3）`$_`

$_为上一个命令的最后一个参数

```shell
$ ls -ltotal 8-rw-rw-r-- 1 mygit mygit 265 May 12 11:38 grade2.sh-rw-rw-r-- 1 mygit mygit 445 May 12 11:26 grade.sh$ echo $_-l
```



（4）`$!`

$!为最近一个后台执行的异步命令的进程ID



（5）`$0`

`$0`为当前Shell的名称或者脚本名

```shell
$ echo $0bash
```



（6）`$-`

`$-`为当前Shell的启动参数

```shell
$ echo $-himBHs
```



（7）`$@` 和`$#`

`$@` 和`$#`表示脚本的参数数量



4.8 变量的默认值

Bash提供四个特殊语法，跟变量的默认值有关，目的是为空变量提供默认值

```shell
${varname:-word}
```

varname存在且不为空，则返回它的值，否则返回word



```shell
${varname:=word}
```

varname存在且不为空，则返回它的值，否则将其初始化为word，并返回word



```shell
${varname:+word}
```

varname存在且不为空，则返回word，否则返回空值。它的目的是测试变量是否存在。



```shell
${varname:?message}
```

varname存在且不为空，则返回它的值，否则打印出varname：message，并中断脚本的执行。如果省略了message，则输出默认消息："parameter null or not set."，它的目的是防止变量未定义



上面四种语法如果用在脚本中，变量名的部分可以用数字`1`到`9`，表示脚本的参数。

```shell
filename=${1:?"filename missing."}
```

上面代码出现在脚本中，`1`表示脚本的第一个参数。如果该参数不存在，就退出脚本并报错。



## 5、declare 命令

`declare`命令可以声明一些特殊类型的变量，为变量设置一些限制，比如声明只读类型的变量和整数类型的变量。



它的语法形式如下。

```shell
declare OPTION VARIABLE=value
```

`declare`命令的主要参数（OPTION）如下。

- `-a`：声明数组变量。
- `-f`：输出所有函数定义。
- `-F`：输出所有函数名。
- `-i`：声明整数变量。
- `-l`：声明变量为小写字母。
- `-p`：查看变量信息。
- `-r`：声明只读变量。
- `-u`：声明变量为大写字母。
- `-x`：该变量输出为环境变量。

`declare`命令如果用在函数中，声明的变量只在函数内部有效，等同于`local`命令。

不带任何参数时，`declare`命令输出当前环境的所有变量，包括函数在内，等同于不带有任何参数的`set`命令。

```shell
$ declare
```



**（1）`-i`参数**

`-i`参数声明整数变量以后，可以直接进行数学运算。

```shell
$ declare -i val1=12 val2=5$ declare -i result$ result=val1*val2$ echo $result60
```

上面例子中，如果变量`result`不声明为整数，`val1*val2`会被当作字面量，不会进行整数运算。另外，`val1`和`val2`其实不需要声明为整数，因为只要`result`声明为整数，它的赋值就会自动解释为整数运算。

注意，一个变量声明为整数以后，依然可以被改写为字符串。

```shell
$ declare -i var=12$ var=foo$ echo $var0
```

上面例子中，变量`var`声明为整数，覆盖以后，Bash 不会报错，但会赋以不确定的值，上面的例子中可能输出0，也可能输出的是3。



**（2）`-x`参数**

`-x`参数等同于`export`命令，可以输出一个变量为子 Shell 的环境变量。

```
$ declare -x foo# 等同于$ export foo
```

**（3）`-r`参数**

`-r`参数可以声明只读变量，无法改变变量值，也不能`unset`变量。

```shell
$ declare -r bar=1$ bar=2bash: bar：只读变量$ echo $?1$ unset barbash: bar：只读变量$ echo $?1
```

上面例子中，后两个赋值语句都会报错，命令执行失败。



**（4）`-u`参数**

`-u`参数声明变量为大写字母，可以自动把变量值转成大写字母。

```shell
$ declare -u foo$ foo=upper$ echo $fooUPPER
```



**（5）`-l`参数**

`-l`参数声明变量为小写字母，可以自动把变量值转成小写字母。

```shell
$ declare -l bar$ bar=LOWER$ echo $barlower
```



**（6）`-p`参数**

`-p`参数输出变量信息。

```shell
$ foo=hello$ declare -p foodeclare -- foo="hello"$ declare -p barbar：未找到
```

上面例子中，`declare -p`可以输出已定义变量的值，对于未定义的变量，会提示找不到。

如果不提供变量名，`declare -p`输出所有变量的信息。

```shell
$ declare -p
```



**（7）`-f`参数**

`-f`参数输出当前环境的所有函数，包括它的定义。

```shell
$ declare -f
```



**（8）`-F`参数**

`-F`参数输出当前环境的所有函数名，不包含函数定义。

```shell
$ declare -F
```



## 6、readonly 命令

`readonly`命令等同于`declare -r`，用来声明只读变量，不能改变变量值，也不能`unset`变量。

```shell
$ readonly foo=1$ foo=2bash: foo：只读变量$ echo $?1
```

上面例子中，更改只读变量`foo`会报错，命令执行失败。

`readonly`命令有三个参数。

- `-f`：声明的变量为函数名。
- `-p`：打印出所有的只读变量。
- `-a`：声明的变量为数组。



## 10、let命令

`let`命令声明变量时，可以直接执行算术表达式。

```shell
$ let foo=1+2$ echo $foo3
```

上面例子中，`let`命令可以直接计算`1 + 2`。

`let`命令的参数表达式如果包含空格，就需要使用引号。

```shell
$ let "foo = 1 + 2"
```

`let`可以同时对多个变量赋值，赋值表达式之间使用空格分隔。

```shell
$ let "v1 = 1" "v2 = v1++"$ echo $v1,$v22,1
```

上面例子中，`let`声明了两个变量`v1`和`v2`，其中`v2`等于`v1++`，表示先返回`v1`的值，然后`v1`自增。



