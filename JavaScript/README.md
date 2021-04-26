# JavaScript 简介

JavaScript （也称为JS）是一种高级的，解释型的编程语言。

## 1、简洁性

其语法简洁，具有很高的抽象层次，只需很少量的代码就可实现复杂的逻辑交互，支持面向对象，命令式，函数式等编程思想。在JS中，函数也是一种类型，可以作为参数传递给某个函数。

```js
<script>
  //定义一个匿名求和函数，并将其存放到f变量中
	var f = function (a, b) { return a + b };
	console.log(f(12,12));   //输出24
</script>
```



## 2、安全性

为了保证足够的安全性，语言本身不支持文件I/O，例如读取，修改本地文件等。但可以通过宿主环境实现文件I/O功能，如通过FileReader对象可以实现文件读取。



## 3、通用性

目前JS已经被世界主流浏览器所支持，比如：Chrome, Firefox, Safari, Opera，IE 等。

越来越多的网站采用了JS来实现复杂的网页交互，数据验证等。



## 4、发展前景

Google开发的V8引擎，直接将JS代码翻译为及机器代码，极大地提高了JS的运行速度，运行缓慢已不在是JS的短板。

Node.js，使JS可以用于服务器端开发。

Electron，是GitHub开发的开源框架，采用Node.js作为后端，Chromium作为前端，是开发跨平台应用的的利器。

使用Electron开发的跨平台应用由：微软的Visual Studio Codd (VScode)

Atom





















