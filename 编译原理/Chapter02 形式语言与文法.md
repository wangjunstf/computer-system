

# Chapter02 形式语言与文法



## 一、文法的直观概念和语言概述

当我们表述一种语言时，无非是说明这种语言的句子，如果语言只含有有穷多个句子，则只需列出句子的有穷集就行了，但对于含有无穷句子的语言来讲，存在着如何给出它的有穷表示的问题。

以自然语言为例，人们无法列出全部句子，但是人们可以给出一些规则，用这些规则来说明(或者定义)句子的组成结构，比如汉语句子可以是由主语后随谓语而成，构成谓语的是动词和直接宾语，我们采用第2章所介绍的EBNF来表示这种句子的构成规则：

**“我是大学生”。是汉语的一个句子**

我们可以定义一些规则来生成类似上述句子的语言，定义规则如下：

〈句子〉∷=〈主语〉〈谓语〉
〈主语〉∷=〈代词〉｜〈名词〉
〈代词〉∷= 我｜你｜他
〈名词〉∷= 王明｜大学生｜工人｜英语
〈谓语〉∷=〈动词〉〈直接宾语〉
〈动词〉∷= 是｜学习
〈直接宾语〉∷=〈代词〉｜〈名词〉 

有了上述规则，按照如下方式用它们导出句子：找到::=左端带有<句子>的规则，并把它由::=右端的符号串代替，这个动作可以表示为：<句子>  => <主语> <谓语>

 然后再得到的串<主语> <谓语>，选取〈主语〉或〈谓语〉，再用相应规则的::=右端代替，比如选取了〈主语〉，并采用规则〈主语〉::=〈代词〉，那么将得到：<主语>〈谓语〉=> 〈代词〉〈谓语〉

重复上述过程：句子“我是大学生”的全部动作是：

<句子> => <主语>〈谓语〉=>  <代词> 〈谓词〉=>  <代词>〈动词〉〈直接宾语〉=> 〈代词〉〈动词〉〈名词〉=>我 〈动词〉〈名词〉=> 我是〈名词〉=> 我是大学生

**上述定义的规则就称为文法，能有该文法生成的句子，称为该文法的语言**



## 二、字母表和符号串

### 2.1 字母表

- 符号的非空有限集合
- 典型的符号是字符，数字，各种标点符号和运算符等。

### 2.2 符号串

* **定义在某一字母表上**
* 由该字母表中的符号组成的有限符号序列
* 同义词：句子，字

### 2.3 符号串的几个概念

* 长度

  - 符号串α的长度是指a中出现的符号个数，记作|α|。
  - 空串的长度为0，常用ε表示。

* 前缀

  符号串α的前缀是指从符号串α的末尾删除0个或多个符号后得到的符号串。如：stu是student的前缀

* 后缀

  符号串α的后缀是指从符号串α的开头删除0个或多个符号后得到的符号串。如dent是student的后缀

* 子串

  符号串α的子串是指删除了α的前缀或后缀后得到的符号串。如tud是student的子串

* 真前缀，真后缀，真子串

  如果非空符号串β是α的前缀，后缀或子串，并且β≠α，则称β是α的真前缀，真后缀或真子串

* 子序列

  符号串α的子序列是指从α中删除0个或多个符号（这些符号可以是不连续的）后得到的符号串。例如：sdt是student的子序列



## 三、符号串运算

### 3.1 连接

符号串α和符号串β的连接αβ是把符号串β加在符号串α之后得到的符号串

若α=ab，β=cd，则αβ=abcd，βα=cdba

对任何符号串α来说，都有εα=αε=α

### 3.2 幂

若α是符号串，α的n次幂α<sup>n</sup>定义为：n个α连接

当n==0时，α<sup>0</sup>=ε。

假如α=ab，则有：α<sup>0</sup>=ε，α<sup>1</sup>=ab，α<sup>2</sup>=abab



## 四、语言

### 4.1 语言

在某一确定字母表上的符号串的集合。

空集φ，集合{ε}也是符合次定义的语言。

这个定义并没有把任何意义赋予语言中的符号串。

### 4.2 语言的运算

假设L和M表示两个语言

L和M的并记作L⋃M：L⋃M={s|s⋳L 或s⋳M}

L和M的连接记作LM：LM={st|s⋳L并且t⋳M}

L的闭包记作L<sup>*</sup>：即L的0次或若干次连接。

L的正闭包记作L<sup>+</sup>：即L的1次或若干次连接。



把幂运算推广到语言

L<sup>0</sup>={ε}，L<sup>n</sup>=L<sup>n-1</sup>L，于是L<sup>n</sup>是语言L与其自身的n-1次幂连接。

L={A,B,...,Z,a,b,...,z}，D={0,1,...,9}

* 可以把L和D看作是字母表
* 可以把L和D看作是语言

**语言运算举例：**

| 语言               | 描述                                             |
| ------------------ | ------------------------------------------------ |
| L⋃D                | 全部字母和数字的集合                             |
| LD                 | 由一个字母后跟一个数字组成的所有符号串的集合     |
| L<sup>4</sup>      | 由4个字母组成的所有符号串的集合                  |
| L<sup>*</sup>      | 由字母组成的所有符号串（包括ε）的集合            |
| L(L⋃D)<sup>*</sup> | 由字母开头，后跟字母，数字组成的所有符号串的集合 |
| D<sup>+</sup>      | 由一个或若干个数字组成的所有符号串的集合         |

## 五、文法及其形式定义

**文法**：所谓文法就是描述语言的语法结构的形式规则。

任何一个文法都可以表示为一个四元组（V<sub>T</sub>，V<sub>N</sub>，S，φ）

> V<sub>T</sub>为非空的有限集合，它的每个元素称为终结符号。
>
> V<sub>N</sub>是一个非空的有限集合，它的每个元素称为非终结符号
>
> S是一个特殊的非终结符，称为文法的开始符号。
>
> φ是一个非空的有限集合，它的每个元素称为产生式。



**产生式的形式为：α -> β**

"->" 表示 “定义为” 或 “由...组成”

α，β⋳(V<sub>T</sub>UV<sub>N</sub>)<sup>*</sup> α≠ε

左部相同的产生式α -> β<sub>1</sub>，α -> β<sub>2</sub>，α -> β<sub>3</sub>，α -> β<sub>n</sub>可以缩写

α -> β<sub>1</sub>|β<sub>2</sub>|...|β<sub>n</sub>

"|"表示“或”，每个β<sub>i</sub>(i=1,2,....,n)称为α的一个候选式



元符号：$\rightarrow$ ， ::=，  |，  <>

**习惯表示**

大写字母：终结符

小写字母：非终结符

A $\rightarrow$ AB

A $\rightarrow$ Ax｜ y

B $\rightarrow$ z



## 六、文法的分类

根据对产生式施加的限制不同，定义了四类文法和相应的四种形式语言类。

| 文法类型                        | 产生式形式的限制                                             | 文法产生的语言类          |
| ------------------------------- | ------------------------------------------------------------ | ------------------------- |
| 0型文法                         | α$\rightarrow$β，其中α,β∊(V<sub>T</sub>UV<sub>N</sub>)<sup>*</sup>，\|a\|≠0 | 0型语言                   |
| 1型文法，即上下文有关文法       | α$\rightarrow$β，其中α,β∊(V<sub>T</sub>UV<sub>N</sub>)<sup>*</sup>，\|α\|≤\|β\| | 1型语言，即上下文有关语言 |
| 2型文法，即上下文无关文法       | A$\rightarrow$β，其中A∈V<sub>N</sub>，β∈(V<sub>T</sub>UV<sub>N</sub>)<sup>*</sup> | 2型语言，即上下文无关语言 |
| 3型文法，即正规文法（线性文法） | A$\rightarrow$α或A$\rightarrow$αB（右线性），或A$\rightarrow$α或A$\rightarrow$Bα，其中A，B∈V<sub>N</sub>，α∈V<sub>T</sub>U{ε} | 3型语言，即正规语言       |

> 什么是上下文无关文法？
>
> 所有产生式的左边只有一个非终结符，例如S -> aSb，S -> ab，这就是上下文无关文法，因为你只要找到符合产生式右边的串，就可以把它归约为对应的非终结符。
>
> 什么是上下文有关文法？
>
> 所有产生式的左边有不只一个符号，例如aSb -> aaSbb，S -> ab，这就是上下文有关文法，当你替换S的时候，如果想用第一条规则，就得确保其有正确的上下文，即左右两边分别为a,b。



### 6.1 文法举例：

**1型文法**

G[S]:

S $\rightarrow$CD

C $\rightarrow$ aCA

AD $\rightarrow$aD

Aa $\rightarrow$bD



**2型文法**

G[S]:

S$\rightarrow$CD

C$\rightarrow$aCA

C$\rightarrow$bCB



**3型文法**

G[S]:右线性文法

S$\rightarrow$0A|1B|0

A$\rightarrow$0A|1B|0S

B$\rightarrow$1B|1|0



G[l]:左线性文法

l$\rightarrow$T1

l$\rightarrow$1

T$\rightarrow$T1

T$\rightarrow$1



### 6.2 文法的包含关系

四种文法之间的逐级“包含”



![文法包含关系](https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8A%E5%8D%8810.24.08r32Wp8.png)

### 6.3 文法与语言

0型文法产生的语言称为0型语言
1型文法或上下文有关文法（ CSG ）产生的语言称为1型语言或上下文有关语言（CSL）
2型文法或上下文无关文法（ CFG ）产生的语言称为2型语言或上下文无关语言（ CF L ） 
3型文法或正则（正规）文法（ RG ）产生的语言称为3型语言正则（正规）语言（ RL ） 



## 七、上下文无关文法及相应的语言

所定义的语法单位(或称语法实体)完全独立于这种语法单位可能出现的上下文环境

现有程序设计语言中，许多语法单位的结构可以用上下文无关文法来描述。

例：描述算术表达式的文法G：
 G=({ i, +, -, *, /, (, ) }, {<表达式>, <项>, <因子>}, <表达式>， φ)

其中φ：<表达式>$\rightarrow$<表达式>+<项> | <表达式>-<项> | <项>
			  <项>$\rightarrow$<项>*<因子> | <项>/<因子> | <因子>
              <因子>$\rightarrow$(<表达式>) | i

L(G)是所有包括加，减，乘，除四则运算的算术表达式的集合。



### 7.1 BNF（Backus-Normal Form）表示法

**元语言**

::=        表示  “定义为” 或 “由……组成”
<……>     表示非终结符号
|          表示“或”



**算术表达式文法的BNF表示：**

<表达式> ::= <表达式>+<项> | <表达式>-<项> | <项>
<项> ::= <项>*<因子> | <项>/<因子> | <因子>
<因子> ::= (<表达式>) | i



### 7.2 文法书写约定

**终结符**

* 次序靠前的小写字母，如：a、b、c

* 运算符号，如：+、-、*、/

* 各种标点符号，如：括号、逗号、冒号、等于号

* 数字1、2、…、9

* 数字1、2、…、9

  

**非终结符**

* 次序靠前的大写字母，如：A、B、C
* 大写字母S常用作文法的开始符号
* 小写的斜体符号串，如：expr、term、factor、stmt



**文法符号**

次序靠后的大写字母，如：X、Y、Z



**终结符号串**

次序靠后的小写字母，如：u、v、…、z



**文法符号串**

小写的希腊字母，如：α、β、γ、δ



可以直接用产生式的集合代替四元组来描述文法，

第一个产生式的左部符号是文法的开始符号。



### 7.3 推导和短语

例：考虑简单算术表达式的文法G：

G=({+, *, (, ), i}, {E, T, F}, E ,φ)

φ: E$\rightarrow$E+T | T

​     T$\rightarrow$T*F | F

​	  F$\rightarrow$(E) | i



**文法所产生的语言**

从文法的开始符号出发，反复连续使用产生式对非终结符号进行替换和展开，就可以得到该文法定义的语言。



**推导**

假定A$\rightarrow$γ是一个产生式，α和β是任意的文法符号串，则有：

αAβ$\Rightarrow$αγβ

”$\Rightarrow$“表示 “一步推导”

即利用产生式对左边符号串中的一个非终结符号进行替换，得到右边的符号串。



称αAβ**直接推导出**αγβ

也可以说αγβ是αAβ的**直接推导**

或者说αAβ直接规约到αγβ



如果有直接推导序列：α<sub>1</sub>$\Rightarrow$α<sub>2</sub>$\Rightarrow$α<sub>3</sub>$\Rightarrow$...$\Rightarrow$α<sub>n</sub>

则说α<sub>1</sub>推导出α<sub>n</sub>，记作α<sub>1</sub>$\stackrel{*}{\Rightarrow}$α<sub>n</sub>

称这个序列是从α<sub>1</sub>到α<sub>n</sub>的长度为n的推导

"$\stackrel{*}{\Rightarrow}$"表示0步或多步推导



### 7.4 最左推导，最右推导

**最左推导**

如果有α$\stackrel{*}{\Rightarrow}$β，并且在每“一步推导中”，都替换α**最左边**的非终结符，则称这样的推导为最左推导，记作α$\mathop{\stackrel{*}{\Rightarrow}}\limits_{lm}$β

E$\Rightarrow$E+T $\Rightarrow$T+T$\Rightarrow$F+T$\Rightarrow$i+T$\Rightarrow$i+F$\Rightarrow$i+i

**最右推导**

α$\stackrel{*}{\Rightarrow}$β，并且在每“一步推导”中，都替换α中**最右边**的非终结符，则称这样的推导为最右推导。记作α$\mathop{\stackrel{*}{\Rightarrow}}\limits_{rm}$β，最右推导也称为==规范推导==

E$\Rightarrow$E+T$\Rightarrow$E+F$\Rightarrow$E+i$\Rightarrow$T+i$\Rightarrow$F+i$\Rightarrow$i+i



例如：

G：S$\rightarrow$0S1，S$\rightarrow$01

S$\Rightarrow$0S1

0S1$\Rightarrow$00S11

00S11$\Rightarrow$000S111

000S111$\Rightarrow$00001111

S$\Rightarrow$0S1$\Rightarrow$00S11$\Rightarrow$000S111$\Rightarrow$00001111



### 7.5 句型，句子和语言

**句型**

对于文法G=（V<sub>T</sub>，V<sub>N</sub>，S，φ）

如果有S$\stackrel{*}{\Rightarrow}$α，则称α是当前文法的一个句型

若S$\mathop{\stackrel{*}{\Rightarrow}}\limits_{lm}$α，则α是当前文法的一个**左句型**

若S$\mathop{\stackrel{*}{\Rightarrow}}\limits_{rm}$α，则α是当前文法的一个**右句型**



**句子**

仅含有终结符的句型是文法的一个句子



**语言**

文法G产生的所有句子组成的集合是文法G所定义的语言，记作L(G)

L(G) = {α | S $\stackrel{*}{\Rightarrow}$ α，S为文法开始符号，并且α∈V<sub>T</sub><sup>*</sup>}



例：G： S→0S1， S→01
S $\Rightarrow$0S1 $\Rightarrow$00S11 $\Rightarrow$000S111 $\Rightarrow$00001111
G的句型S,0S1 ,00S11 ,000S111,00001111
G的句子00001111, 01



**文法，语言的定义**

由文法G生成的语言记为L(G),它是文法G的一切句子的集合:                                       

L(G) = {α | S $\stackrel{*}{\Rightarrow}$ α，S为文法开始符号，并且α∈V<sub>T</sub><sup>*</sup>}



例：G： S→0S1， S→01
L(G)={0<sup>n</sup>1<sup>n</sup>|n≥1}



### 7.6 文法的等价

若L(G<sub>1</sub>)=L(G<sub>2</sub>)，则称文法G<sub>1</sub>和G<sub>2</sub>是等价的。



如文法G<sub>1</sub>[A]：A→0R  A→01 A→A1 与文法G<sub>2</sub>[S]：S→0S1 S→01 等价 



### 7.7 短语，直接短语和句柄

对于文法G=（V<sub>T</sub>，V<sub>N</sub>，S，φ），假定αβδ是文法G的一个句型，如果存在：

​		S $\stackrel{*}{\Rightarrow}$ αAδ，并且A $\stackrel{+}{\Rightarrow}$ β，则称β是句型αβδ关于非终结符A的==短语==。

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/IMG_91CD2FE503AE-1v5k3P0.jpeg" alt="IMG_91CD2FE503AE-1" style="zoom:25%;" />

如果存在：

​        S $\stackrel{*}{\Rightarrow}$ αAδ，并且A ${\Rightarrow}$ β，则称β是句型αβδ关于非终结符A的==直接短语==。

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%885.32.44A6Wl2u.png" alt="截屏2021-04-14 下午5.32.44" style="zoom: 25%;" />



一个句型的最左直接短语称为该句型的==句柄==。



## 八、分析树及二义性

### 7.1 分析树

推导的图形表示，又称推导树

具有树的性质

分析树的特点：每一个节点都有标记。

* 根节点由文法开始符号标记；
* 每个内部节点由非终结符标记，它的子节点由这个非终结符的这次推导所用产生式的右部各符号从左到右依次标记；
* 叶节点由非终结符或终结符号标记，它们从左到右排列起来，构成句型。

例如：假如现有一文法G[E]

G[E]=({+, *, (, ), i}, {E, T, F}, E ,φ)

φ: E$\rightarrow$E+T | T

​     T$\rightarrow$T*F | F

​	  F$\rightarrow$(E) | i

i\*(i+T)的推导过程如下：

E  ${\Rightarrow}$ T  ${\Rightarrow}$ T\*F  ${\Rightarrow}$ T\*(E)  ${\Rightarrow}$ i\*(E) ${\Rightarrow}$i\*(E+T)  ${\Rightarrow}$ i\*(T+T) ${\Rightarrow}$ i\*(F+T) ${\Rightarrow}$i\*(i+T)

上述推导过程可用分析树表示如下：

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%885.53.39MBqKNU.png" alt="截屏2021-04-14 下午5.53.39" style="zoom: 50%;" />



### 7.2 子树

分析树中一个特有==节点==，连同它的==全部后裔节点==，连结这些节点的==边==，以及这些节点的==标记==。

子树的根节点可能不是文法的开始符号。

如果子树的根节点标记为非终结符A，则称该子树为A-子树



<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%886.05.47Odq6y1.png" alt="截屏2021-04-14 下午6.05.47" style="zoom: 50%;" />

### 7.3 子树与短语的关系

一棵==子树的所有叶节点==自左至右排列起来，形式次句型相对于该子树根的短语；

分析树中。

分析树中==只有父子两代==的子树的所有叶节点自左至右排列起来，形成此句型相对于子树根的直接短语

分析树中==最左边==的那棵只有父子两代的子树的所有叶节点自左至右排列起来，就是该句型的==句柄==

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%886.16.45e86k69.png" alt="截屏2021-04-14 下午6.16.45" style="zoom:50%;" />



G[E]=({+, *, (, ), i}, {E, T, F}, E ,φ)

φ: E$\rightarrow$E+T | T

​     T$\rightarrow$T*F | F

​	  F$\rightarrow$(E) | i



E  ${\Rightarrow}$ E+T ${\Rightarrow}$ T+T${\Rightarrow}$ F+T ${\Rightarrow}$ a+T ${\Rightarrow}$ a+T\*F ${\Rightarrow}$ a+F\*F ${\Rightarrow}$ a+a\*F ${\Rightarrow}$ a+a\*a

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%886.22.287lGB4P.png" alt="截屏2021-04-14 下午6.22.28" style="zoom:50%;" />



==分析树看不出句型中符号被替代的顺序==



## 九、上下文无关文法的语法树的用处

例：G[S]:

​	S $\rightarrow$ aAS

​	A $\rightarrow$ SbA

​	A $\rightarrow$ SS

​	S $\rightarrow$ a

​	A $\rightarrow$ ba

句型aabbaa的语法树（推导树）

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%886.43.5712U0Qw.png" alt="截屏2021-04-14 下午6.43.57" style="zoom:50%;" />



## 十、二义性

如果一个文法的某个句子有不止一棵分析树，则这个句子是==二义性的==。



例如：考虑文法

G = ({+,*,(,),i}, {E} , E, φ) 

φ : E $\rightarrow$ E+E | E\*E | (E) | id

句子 id+id\*id 存在两个不同的最左推导：

有两棵不同的分析树

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%886.56.09uVXZRW.png" alt="截屏2021-04-14 下午6.56.09" style="zoom:50%;" />



### 9.1 文法二义性的消除

映射程序设计语言中IF语句的文法：

stmt $\rightarrow$ if expr then stmt
     | if expr then stmt else stmt
     | other

句子：if E1 then if E2 then S1 else S2有两棵不同的分析树：

![截屏2021-04-14 下午7.44.47](https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%887.44.47bPRrKF.png)



利用==“最近最后匹配原则”==

出现在then和else之间的语句必须是“匹配的”。

出现在then和else之间的语句必须是“匹配的”。

改写后的文法：

stmt $\rightarrow$  matched_stmt | unmatched_stmt
matched_stmt $\rightarrow$  if expr then matched_stmt else matched_stmt 
             | other
unmatched_stmt $\rightarrow$   if expr then stmt
             | if expr then matched_stmt else unmatched_stmt



句子：if E1 then if E2 then S1 else S2的分析树

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%887.49.57SiPH98.png" alt="截屏2021-04-14 下午7.49.57" style="zoom: 50%;" />



### 9.2 文法的二义性和语言的二义性

* 如果两个文法产生的语言相同，即L(G)=L(G`)，则称这两个文法是等价的。
* 有时，一个二义性的文法可以变换为一个等价的、无二义性的文法。
* 有些语言，根本就不存在无二义性的文法，这样的语言称为二义性的语言。
* 二义性问题是不可判定的
  - 不存在一种算法，它能够在有限的步骤内确切地判定出一个文法是否是二义性的。
  - 可以找出一些充分条件（未必是必要条件），当文法满足这些条件时，就可以确信该文法是无二义性的。

## 十一、句型的分析

句型分析就是识别一个符号串是否为某文法的句型，是某个推导的构造过程

在语言的编译实现中，把完成句型分析的程序称为分析程序或识别程序。分析算法又称识别算法。

从左到右的分析算法，即总是从左到右地识别输入符号串，首先识别符号串中的最左符号，进而依次识别右边的一个符号，直到分析结束。



### 11.1 句型的分析算法分类

分析算法可分为：

1. 自上而下分析法：

   从文法的开始符号出发，反复使用文法的产生式，寻找与输入符号串匹配的推导。

2. 自下而上分析法：

3. 从输入符号串开始，逐步进行==规约==，直至规约到文法的开始符号。



**两种方法反映了两种语法树的构造过程**

**自上而下方法**是从文法符号开始，将它做为语法树的根，向下逐步建立语法树，使语法树的结果正好是输入符号串

**自下而上方法**则是从输入符号串开始，以它做为语法树的结果，自底向上地构造语法树



### 11.2 自上而下的语法分析

例：文法G：S $\rightarrow$ cAd ,  A $\rightarrow$ ab, A $\rightarrow$ a

识别输入串w=cabd是否为该文法的句子

推导过程：S  ${\Rightarrow}$ cAd         cAd  ${\Rightarrow}$  cabd

![截屏2021-04-14 下午8.02.15](https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%888.02.15pJyNeb.png)



### 11.3 自下而上分析的语法分析

例：文法G：S $\rightarrow$ cAd ,  A $\rightarrow$ ab, A $\rightarrow$ a

识别输入串w=cabd是否为该文法的句子

规约过程构造的推导： cAd${\Rightarrow}$ cabd      S ${\Rightarrow}$ cAd

![截屏2021-04-14 下午8.05.08](https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%888.05.08hLUG61.png)



### 11.4 句型分析的有关问题

1. 在自上而下的分析方法中如何选择使用哪个产生式进行推导？

   假定要被代换的最左非终结符号是B，且有n条规则：B$ \rightarrow$ A1|A2|…|An，那么如何确定用哪个右部去替代B？

2. 在自下而上的分析方法中如何识别可归约的串？

   在分析程序工作的每一步，都是从当前串中选择一个子串，将它归约到某个非终结符号，该子串称为“可归约串”



### 11.5 识别短语，直接短语和句柄

G[E]：E $\rightarrow$ E+T|T      T$ \rightarrow$ T*F|F      F$ \rightarrow$ (E)|i

给定一个句型：i\*i+i和它的分析树，识别它的短语，直接短语和句柄

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%888.16.18NqSOKG.png" alt="截屏2021-04-14 下午8.16.18" style="zoom:50%;" />

**短语**：i<sub>1</sub>，i<sub>2</sub>，i<sub>3</sub>，i<sub>1</sub>\*i<sub>2</sub>，i<sub>1</sub>\*i<sub>2</sub>+i<sub>3</sub>

**直接短语**：i<sub>1</sub>，i<sub>2</sub>，i<sub>3</sub>

**句柄**：i<sub>1</sub>

> 为什么i<sub>1</sub>\*i<sub>2</sub>不是直接短语？
>
> S $\stackrel{*}{\Rightarrow}$ T+i<sub>3</sub>，并且T ${\Rightarrow}$ T\*F ${\Rightarrow}$F\*F ${\Rightarrow}$ i<sub>1</sub>\*F ${\Rightarrow}$  i<sub>1</sub>\*i<sub>2</sub>， i<sub>1</sub>\*i<sub>2</sub>是T经过多步推导得到，因而只是短语，而不是直接短语。



## 十二、 左递归

### 12.1 左递归

假设某一文法：

G[S]：S $\rightarrow$ Sa，A $\rightarrow$ b  L(G) = {ba<sup>n</sup>|n⩾1}

w=baa

<img src="https://wangjunblogs.oss-cn-beijing.aliyuncs.com/uPic/%E6%88%AA%E5%B1%8F2021-04-14%20%E4%B8%8B%E5%8D%888.51.514ZnNre.png" alt="截屏2021-04-14 下午8.51.51" style="zoom:50%;" />





### 12.2 左递归的消除

一个文法是左递归的，如果它有非终结符号Α，对某个文法符号串α，存在推导：

Α ${\Rightarrow}$ Αα

如果存在某个α=ε，则称该文法是有**环路**的



**消除直接左递归的方法：**

简单情况：如果文法G有产生式：A $\rightarrow$ Aα| β  

A

L(G) = {βa<sup>n</sup>| n≥1}

可以把A的这两个产生式改写为：

A $\rightarrow$ βA‘

A’ $\rightarrow$ αA‘ ｜ ε

此时L(G) =  {βa<sup>n</sup>| n≥1}

**说明这两组文法是等价的。**



示例：==消除直接左递归==

假如有一文法G[E]：

E  $\rightarrow$ E+T | T

T  $\rightarrow$ T\*F | F

F  $\rightarrow$  (E) | id

上述文法存在两处左递归，分别是E  $\rightarrow$ E+T | T，T  $\rightarrow$ T\*F | F，分别用上述方法消除左递归。

E $\rightarrow$ TE'

E' $\rightarrow$ +TE' | ε

T $\rightarrow$ FT'

T' $\rightarrow$ *FT' | ε

F $\rightarrow$ (E) | id

> 该方法巧妙借助 ε实现一个环来消除左递归。
>
> 观察上述直接左递归，其生成的语句为：T+T+T+T+T+T+T，第一个T为递归结束符，后面+T+T为循环式。
>
> 解决方法：改变E的产生式
>
> * 实现一个产生式用来生成第一个T
>
>   E $\rightarrow$ TE'，此时生成的语句开头便是T。
>
> * 实现一个环来生成后续+T，就可以消除左递归
>
>   E‘ $\rightarrow$ +TE' ｜  ε，此时便能产生+T+T+T...循环式了，而且可以用ε来结束循环



### 12.3 间接左递归的消除

例如以下文法存在间接左递归：

S $\rightarrow$ Aa|b

A $\rightarrow$ Ac|Sd|ε



**消除算法：**

输入：无环路、无-产生式的文法G
输出：不带有左递归的、与G等价的文法G’ 

(1)把文法G的所有**非终结符**号按某种顺序排列成A1,A2,…,An
(2)for (i=1; i<=n; i++)
      for (j=1; j<=i-1; j++)
         if (A<sub>j</sub>$\rightarrow$ δ<sub>1</sub>|δ<sub>2</sub>|…|δ<sub>3</sub>是关于当前A<sub>j</sub>的所有产生式) {
           把每个形如A<sub>i</sub>$\rightarrow$A<sub>j</sub>的产生式改写为：A<sub>i</sub>$\rightarrow$ δ<sub>1</sub>γ|δ<sub>2</sub>γ|…|δ<sub>3</sub>γ
           消除关于A<sub>i</sub>的产生式中的直接左递归;
         }
(3)化简第(2)步得到的文法，即去除无用的非终结符号和产生式。
这种方法得到的非递归文法可能含有ε-产生式。



**示例：**

消除下面文法中的左递归

S $\rightarrow$ Aa | b

A $\rightarrow$ Ac | Sd | ε

首先，必须保证此文法中无环路、无ε-产生式。

改写为无ε-产生式的文法：
        S $\rightarrow$ Aa|a|b
        A $\rightarrow$ Ac|c|Sd 

> 因为A可能会产生ε，在所有的产生式中添加A为ε时能产生的结果
>
> 例如S $\rightarrow$ Aa|a|b ，当A为ε时，S可以产生a，合并起来就为S $\rightarrow$ Aa|a|b

消除其中的左递归：

第一步，把文法的非终结符号排列为S、A；

第二步，由于S不存在直接左递归，所以算法第2步在i=1时不做工作；

在i=2时，把产生式S $\rightarrow$Aa|a|b代入A的有关产生式中，得到：
             A $\rightarrow$Ac|c|Aad|ad|bd

消除A产生式中的直接左递归，得到文法

S $\rightarrow$Aa|a|b

A $\rightarrow$cA‘|adA’ |bdA‘

A’ $\rightarrow$cA’|adA‘|ε



## 十三、提取左公因子

如有产生式 A $\rightarrow$ αβ<sub>1</sub>|αβ<sub>2</sub>

提取公因子α，则原产生式变为：

A $\rightarrow$ αA'

A'  $\rightarrow$ β<sub>1</sub>|β<sub>2</sub>



若产生式 A $\rightarrow$ αβ<sub>1</sub>|αβ<sub>2</sub>｜...|αβ<sub>n</sub>|γ

可用如下的产生式代替：

A $\rightarrow$ αA' ｜ γ

A'  $\rightarrow$ β<sub>1</sub>|β<sub>2</sub>｜...|β<sub>n</sub>



例如：现有如下程序设计语言IF语句的文法

stmt $\rightarrow$ if expr then stmt 

​			| if expt then stmt else stmt

​			| a

expt $\rightarrow$  b

左公因子为：if expr then stmt 

提取左公因子，得到文法：

stmt $\rightarrow$ if expr then stmt S' | a

S'  $\rightarrow$ else stmt |  ε

expt  $\rightarrow$ b



## 十四、构造不含ε-产生式的文法

为含有ε-产生式的文法G=（V<sub>T</sub>，V<sub>N</sub>，S，φ）构造不含ε-产生式的文法G‘=（V<sub>T</sub><sup>'</sup>，V<sub>N</sub><sup>'</sup>，S，φ<sup>'</sup>）

若产生式A$\rightarrow$X<sub>1</sub>X<sub>2</sub>…Xn⋳φ则把产生式A$\rightarrow$α<sub>1</sub>α<sub>2</sub>…α<sub>n</sub>加入φ'
其中：X<sub>i</sub>、α<sub>i</sub>为文法符号，即X<sub>i</sub>、α<sub>i</sub>⋳(V<sub>T</sub>UV<sub>N</sub>)



若X<sub>i</sub>不能产生ε，则α<sub>i</sub>=X<sub>i</sub>

若X<sub>i</sub>能产生ε，则α<sub>i</sub>=X<sub>i</sub>或α<sub>i</sub>=ε

注意：不能所有的α<sub>i</sub>都取ε



文法G'满足：

L(G') = L(G) - {ε}  L(G)中含有ε，或L(G) ，L(G)中不含有ε



**一个文法是ε-无关的，**

如果它没有ε-产生式（即形如A$\rightarrow$ε的产生式），或者

只有一个ε-产生式，即S$\rightarrow$ε并且文法的开始符号S不出现在任何产生式的右部。



十五、化简文法方法

文法中不含有有害规则和多余规则

有害规则：形如U→U的产生式。会引起文法的二义性

多余规则：指文法中任何句子的推导都不会用到的规则

文法中不含有不可到达和不可终止的非终结符

* 文法中某些非终结符不在任何规则的右部出现，该非终结符称为不可到达。
* 文法中某些非终结符，由它不能推出终结符号串，该非终结符称为不可终止。



**化简文法**

例如 G[S] ： 

1) S→Be

2) B → Ce

3) B → Af

4) A → Ae

5) A → e

6) C → Cf

7) D → f



D 为不可到达，C为不可终止

产生式 2) , 6),  7)为多余规则

