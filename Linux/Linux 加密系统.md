### Linux 加密系统

Linux中的密码会加密后存储在**/etc/shadow**中

密码存放位置：

* 用户密码的存放文件：/etc/shadow

* 组密码的存放文件/etc/gshadow

单向加密算法：md5、sha1、sha224、sha256、sha384、sha512

密码的使用策略：

1. 使用随机密码
2. 最短长度不要低于8位；
3. 应该使用大写字母、小写字母、数字和特殊字符四类字符中至少三类；
4. 定期更换，不要使用重复的密码。



**MD5加密**

```shell
$ echo "Hello world!" | md5sum
59ca0efa9f5633cb0371bbc0355478d8  -
```

雪崩效应

```shell
$ echo "Hello world." | md5sum
fa093de5fc603823f08524f9801f0546  -
```

> 雪崩效应是指当输入发生最微小的改变（例如，反转一个二进制位）时，也会导致输出的不可区分性改变



**SHA加密**

