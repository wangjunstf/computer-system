# Web服务器配置

## 1．实验题目 

Web 服务器配置

## 2．作业内容（5分）

(1) Web服务器软件的安装
(2) 虚拟目录配置
(3) 虚拟主机配置
(4) 用户个人站点配置
(5) 基于主机的授权、基于用户的认证

## 3．作业步骤（75分）

### 3.1 Web服务器软件的安装、启动及测试

(1) 检查 apache2 是否安装

![截屏2021-06-06 上午11.09.35](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 上午11.09.35.png)

```shell
$ dpkg -s apache2
Package: apache2
Status: install ok installed
Priority: optional
Section: httpd
Installed-Size: 523
Maintainer: Ubuntu Developers <ubuntu-devel-discuss@lists.ubuntu.com>
Architecture: amd64
Version: 2.4.29-1ubuntu4.15
Replaces: apache2.2-bin, apache2.2-common
Provides: httpd, httpd-cgi
```



(2) 若未安装，则安装![截屏2021-06-06 上午11.10.56](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 上午11.10.56.png)



(3) 启动 Apache 2<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 上午11.15.01.png" alt="截屏2021-06-06 上午11.15.01" style="zoom: 67%;" />

```shell
$ sudo /etc/init.d/apache2 start
[ ok ] Starting apache2 (via systemctl): apache2.service.
```



(3) 关闭服务器![截屏2021-06-06 上午11.17.18](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 上午11.17.18.png)

```shell
$ sudo /etc/init.d/apache2 stop
[ ok ] Stopping apache2 (via systemctl): apache2.service.
```



(4) 重启服务器![截屏2021-06-06 上午11.18.21](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 上午11.18.21.png)

```shell
$ sudo /etc/init.d/apache2 restart
[ ok ] Restarting apache2 (via systemctl): apache2.service.
```



(5) 测试服务器是否打开，打开浏览器，访问地址：http://10.211.55.22/，10.211.55.22为服务器的IP地址<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 上午11.22.37.png" alt="截屏2021-06-06 上午11.22.37" style="zoom: 50%;" />

能正常打开默认网页，说明Apache 2安装成功。



### 3.2虚拟目录配置

(1) **创建子网站路径**及**创建相应子网站的index.html![截屏2021-06-06 上午11.50.03](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 上午11.50.03.png)**

```shell
$ mkdir -p /var/www/blogs
$ mkdir -p /var/www/tools
$ echo "this is blog web" > /var/www/blogs/index.html
$ echo "this is tools web" > /var/www/tools/index.html
$ echo "this is blogs web" > /var/www/blogs/index.html
```



(2) 修改配置文件，并在末尾加入以下内容：![截屏2021-06-06 上午11.46.16](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 上午11.46.16.png)

```shell
$ vim apache2.conf
Alias /blogs "/var/www/blogs"
Alias /tools "/var/www/tools"
```



(3) 访问测试

测试：http://10.211.55.22/blogs/ ![截屏2021-06-06 上午11.53.15](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 上午11.53.15.png)



测试：http://10.211.55.22/tools/

![截屏2021-06-06 上午11.53.46](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 上午11.53.46.png)

### 3.3虚拟主机配置

#### 3.3.1 基于域名的虚拟机配置

1. **创建子网站路径**及**创建相应子网站的index.html![截屏2021-06-06 下午12.17.18](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午12.17.18.png)**

   ```shell
   root@ubuntu:/var/www# mkdir -p /var/www/math
   root@ubuntu:/var/www# mkdir -p /var/www/history
   root@ubuntu:/var/www# echo "This is math web" > /var/www/math/index.html
   root@ubuntu:/var/www# echo "This is history web" > /var/www/history/index.html
   ```

2. 创建虚拟主机

   创建文件/etc/apache2/sites-available/math.conf，并写入以下内容：

   ```shell
   <VirtualHost *:80>
       ServerName math.com
       ServerAlias www.math.com
       ServerAdmin webmaster@math.com
       DocumentRoot /var/www/math
   
       <Directory /var/www/math>
           Options -Indexes +FollowSymLinks
           AllowOverride All
       </Directory>
   
       ErrorLog ${APACHE_LOG_DIR}/math.com-error.log
       CustomLog ${APACHE_LOG_DIR}/math.com-access.log combined
   </VirtualHost>
   ```

   创建文件/etc/apache2/sites-available/history.conf，并写入以下内容：

   ```shell
   <VirtualHost *:80>
       ServerName history.com
       ServerAlias www.history.com
       ServerAdmin webmaster@history.com
       DocumentRoot /var/www/history
   
       <Directory /var/www/history>
           Options -Indexes +FollowSymLinks
           AllowOverride All
       </Directory>
   
       ErrorLog ${APACHE_LOG_DIR}/history.com-error.log
       CustomLog ${APACHE_LOG_DIR}/history.com-access.log combined
   </VirtualHost>
   ```

   执行以下命令：

   ```
   $ a2ensite math
   $ a2ensite history
   ```

   打开 /etc/hosts文件，并加入如下内容

   ```
   10.211.55.22 www.math.com
   10.211.55.22 www.history.com
   ```

   重启apache 2服务器：![截屏2021-06-06 下午4.58.30](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午4.58.30.png)

   ```shell
   $ systemctl restart apache2.service
   ```

   测试：

   浏览器访问：www.math.com![截屏2021-06-06 下午4.59.50](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午4.59.50.png)

   浏览器访问：www.history.com![截屏2021-06-06 下午5.01.10](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午5.01.10.png)



#### 3.3.2 基于端口号的虚拟主机

(1) **创建子网站路径**及**创建相应子网站的index.html![截屏2021-06-06 下午5.30.17](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午5.30.17.png)**

```shell
$ systemctl restart apache2.service
$ mkdir -p /var/www/game
$ mkdir -p /var/www/science
$ echo "This is game web" > /var/www/game/index.html
$ echo "This is science web" > /var/www/science/index.html
```



(2) 创建虚拟主机

创建文件/etc/apache2/sites-available/game.conf，并写入以下内容：

```shell
<VirtualHost *:2020>
    DocumentRoot /var/www/game
</VirtualHost>
```



创建/etc/apache2/sites-available/science.conf，并写入以下内容

```shell
<VirtualHost *:2021>
  DocumentRoot "/var/www/science"
</VirtualHost>
```



执行以下命令

```shell
$ a2ensite game
$ a2ensite science
```



重启apache 2服务器![截屏2021-06-06 下午5.34.42](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午5.34.42.png)

```shell
systemctl restart apache2.service
```



测试访问：

浏览器访问：10.211.55.22:2020![截屏2021-06-06 下午5.37.35](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午5.37.35.png)



浏览器访问：10.211.55.22:2021![截屏2021-06-06 下午5.36.35](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午5.36.35.png)

#### 3.3.3 基于IP的虚拟主机

(1) 为一块网卡绑定多个IP

编辑文件/etc/netplan/01-network-manager-all.yaml，修改为以下内容：<img src="/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午8.36.23.png" alt="截屏2021-06-06 下午8.36.23" style="zoom:50%;" />

```shell
network:
  version: 2
  renderer: networkd
  ethernets:
    enp0s5:
      dhcp4: no
      addresses:
        - 10.211.55.21/24
        - 10.211.55.22/24
        - 10.211.55.23/24
      gateway4: 10.211.55.1
      nameservers:
          addresses: [8.8.8.8, 1.1.1.1]
```



(2) 创建网站目录![截屏2021-06-06 下午8.37.31](/Users/wangjun/Desktop/截图/截屏2021-06-06 下午8.37.31.png)



(3) 创建虚拟主机

创建文件/etc/apache2/sites-available/wangjun.conf，并写入以下内容：

```shell
<VirtualHost 10.211.55.22>
    DocumentRoot /var/www/wangjun
</VirtualHost>
```



创建/etc/apache2/sites-available/science，并写入以下内容

```shell
<VirtualHost >
  DocumentRoot "/var/www/jack"
</VirtualHost>
```



执行以下命令

```shell
$ a2ensite wangjun
$ a2ensite jack
```



重启apache2 服务器![截屏2021-06-06 下午9.01.35](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午9.01.35.png)

```shell
$ systemctl restart apache2.service
```



访问测试：

浏览器访问：http://10.211.55.22/![截屏2021-06-06 下午9.03.22](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午9.03.22.png)



浏览器访问：http://10.211.55.23/![截屏2021-06-06 下午9.03.56](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午9.03.56.png)



### 3.4用户个人站点配置

(1) 禁止root用户登录,修改文件/etc/apache2/mods-available/userdir.conf为以下内容

```shell
<IfModule mod_userdir.c>
	UserDir public_html
	UserDir disabled root

	<Directory /home/*/public_html>
		AllowOverride FileInfo AuthConfig Limit Indexes
		Options MultiViews Indexes SymLinksIfOwnerMatch IncludesNoExec
		Require method GET POST OPTIONS
	</Directory>
</IfModule>
```



(2) 创建个人网站目录![截屏2021-06-06 下午9.49.54](/Users/wangjun/Library/Application Support/typora-user-images/截屏2021-06-06 下午9.49.54.png)

```shell
root@ubuntu:/home# mkdir /home/www/public_html
root@ubuntu:/home# rmdir /home/mygit/public_html/
root@ubuntu:/home# rmdir /home/www/public_html/
root@ubuntu:/home# mkdir /home/mygit/public_html
root@ubuntu:/home# mkdir /home/www/public_html
root@ubuntu:/home# echo "This is mygit user" > /home/mygit/index.html
root@ubuntu:/home# echo "This is www user" > /home/www/index.html
```





### 3.5基于主机的授权、基于用户的认证

## 4.实验中遇到的问题及解决方法（10分）

### 4.1 关于创建虚拟主机的方法

由于我用的是Ubuntu 18.04，和centos 7配置方法不一样。

通过查阅资料，它们创建虚拟主机的主要区别是：

ubuntu 18.04下，apache的配置文件目录为：/etc/apache2

它的主配置文件为：apache2.conf

虚拟主机配置目录为：/etc/apache2/sites-available

* 需要为每一个虚拟主机各创建一个配置文件
* 执行命令：a2ensite 上一步新建的文件名



### 4.2 配置静态IP

我使用的的系统版本是:Ubuntu 18.04版本，默认为DHCP获取IP，通过查阅资料：

发现可以使用Netplan来配置静态IP。



### 4.3 给一块网卡配置多个IP

查阅了很多资料，大部分教程是 修改/etc/network/interface，我尝试了之后没有用，之后我又看了其他方法，找到了一种更简单有效的方法，和配置静态IP一样，使用Netplan来配置。具体就是修改/etc/netplan/01-network-manager-all.yaml

修改为以下内容

```shell
network:
  version: 2
  renderer: networkd
  ethernets:
    enp0s5:
      dhcp4: no
      addresses:
        - 10.211.55.21/24
        - 10.211.55.22/24
        - 10.211.55.23/24
      gateway4: 10.211.55.1
      nameservers:
          addresses: [8.8.8.8, 1.1.1.1]
```



### 4.4 基于域名的虚拟机配置

修改/etc/hosts文件

只能在本机访问，因为修改只能本机有效。

## 5.实训总结（10分)



（1）查询telnet-server是否被安装，若没有安装，则安装。
具体操作截图
（2）....