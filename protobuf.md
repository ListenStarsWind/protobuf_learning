# protobuf

## 背景

关于 `protobuf` 的学习实际上是无意的, 因为学到了 `Qt` 的网络操作, 所以我先开始温习之前的 Linux 网络编程, 而为了实现其中的自定义应用层协议, 鉴于之前使用的是 `json`, 所以这次我打算换一种方案, 那就是 `protobuf`,  在自己胡写了几段代码后, 我发现, 还是应该先系统性学习一下, 不然一走一个坑, 再考虑到它的学习内容不多, 所以我打算顺手学一下.

## 初识 protobuf

计算机, 从某种角度来说, 就是"信息处理机", 而在信息处理的过程中, 免不了对信息的存储和传送, 此时就需要所谓的序列化和反序列化, 人体和机器是完全不一样的结构, 所以尽管对于我们人来说, 在特定范围内的光信号, 声信号什么的, 是比较适用的, 但对于机器来说, 这种模拟信号就不太友好, 所以我们要想点办法, 把我们的信号转化成对机器更友好的数字信号. 举个例子, 打电话时, 我们人类交流使用的是声音信号, 但真正传送的是, 按照一定规则, 基于我们的声信号进行再编码的光电信号, 把我们的声音, 转化成机器的光电信号, 那就是一种广义上的序列化, 把光电信号转化成人声, 那就是一个反序列化的过程. 现在, 我们把场景换成网络传输, 此时我们的基本交流单位, 就是被称为"对象"的结构化数据, 但网络里直接传的可不是这些对象, 而是基于这些对象按照一定规则生成的二进制序列, 对象到二进制序列的过程, 就是狭义上的序列化, 方向反过来, 那就是反序列化.

序列化和反序列化的另一个常见场景就是对象的存储, 我们把数据往磁盘上放, 不能直接放, 也是要先转成二进制序列再存的.

对于序列化和反序列化的实现手段, 常用的是 `json` `xml` `protobuf`. 最后一个就是我们的学习对象.

我对`protobuf`的初影响是: 感觉有点像 `Qt` 的`qrc`机制, 它们都是代码生成代码, 我们只需要先写一个特定的文件, 然后相应的编译器会以我们的文件为基础, 生成一份对应的C++ 代码, 然后我们就可以用了, 也就是说, 它的工作模式是代码生成代码.

在以往, 我们写一个有关序列化和反序列化的类, 需要在这个`class`里面写类的属性字段, 读写字段的方法, 即各种 get set, 最关键的事它们的序列化和反序列化接口.而在`protobuf`这里, 我们就只需要在`.proto`文件里面定义一个消息即可, 而这个 message 中, 我们只需要写好其中的属性字段即可, 其它的, `proto`的编译器`protoc`就可以自己生成, 所以就大大减轻了我们的负担, 我们从此之后就不需要太关心序列化和反序列化细节了.

下面, 是 `protobuff`的使用说明示意图

![image-20251118213324818](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251118213324818.png)

## 环境搭建

windows 环境:

没什么好讲的, 直接去 `github` 上搜一下就行, 当前最新版本是 33.1

![image-20251118214801671](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251118214801671.png)

把压缩包`win64.zip`下载下来后, 把它解压到合适的路径下, 然后再把其中的`protoc.exe`, 也就是`.porto`的编译器所在目录添加到环境变量中即可.

![image-20251118215404355](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251118215404355.png)

![image-20251118215525550](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251118215525550.png)

此时, 我们打开 `PowerShell`, 输入指令`protoc --version`, 就能看到系统找到了它

![image-20251118215737747](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251118215737747.png)

Linux(Ubuntu)

我们还是去 `github` 上下载已经预编译好的二进制文件, 直接解压, 安装

我们还是来到发布界面, 去复制后缀是`-x86_64.zip`的下载链接, 右键点击, 复制链接地址

![image-20251118234539274](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251118234539274.png)

然后用 `wget` 工具把它下载下来, 解压, 安装到系统目录

```shell
mkdir protobuf && cd protobuf
wget 你的链接
sudo unzip *-linux-x86_64.zip -d /usr/local
sudo chmod +x /usr/local/bin/protoc
```

又或者, 你可以采用本地编译安装的方式, 他默认使用你所在时间的当前最新版本. 我本人更喜欢这种方式, 下面主要使用的也是这个本地编译的`protobuf`, 因为是本地工具链编译出来的, 所以对机器本地环境更加兼容, `protobuf`也不是多大的库, 我有这种喜好的原因是因为, 有时候我会用 Boost 库, 但直接安装的应该是 `g++`, `stdlibc++`编译出来的, 但我一般用`clang++`, `linc++`, 所以`clangd`经常会报一些奇奇怪怪的警告, 这让我很不高兴,  为此, 我把 Boost 本地编译安装了, 那可是一个大库, 花了半天时间编译.

下面这些脚本, 第一个是安装所需要的开发工具链, 第二个是克隆`protobuf`库, 并本地编译安装, 你可以直接复制到 `shell`上, 直接运行

```shell
#!/bin/bash
echo "🔧 安装 Clang + libc++ + Ninja 开发环境..."
sudo apt-get update
sudo apt-get install -y \
  clang \
  lld \
  cmake \
  ninja-build \
  libc++-dev \
  libc++abi-dev \
  curl \
  wget \
  unzip \
  git

echo "✅ 工具链安装完成！"
echo "Clang: $(clang --version | head -1)"
echo "CMake: $(cmake --version | head -1)"
echo "Ninja: $(ninja --version)"
echo "libc++: $(find /usr -name "libc++.so.1" 2>/dev/null | head -1)"
```

```shell
#!/bin/bash
set -e

echo "📦 克隆并编译 Protobuf v33.1..."

git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
git checkout v33.1  # 明确指定与预编译版相同的版本
git submodule update --init --recursive

mkdir build && cd build
cmake \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_FLAGS="-stdlib=libc++" \
  -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -lc++abi" \
  -DCMAKE_BUILD_TYPE=Release \
  -Dprotobuf_BUILD_TESTS=OFF \
  -G Ninja \
  ..

ninja
sudo ninja install
sudo ldconfig

echo "✅ Protobuf v33.1 编译安装完成: $(protoc --version)"
```

这脚本也不是我写的, 你可以很明显看出它是 AI 写的, 我大概看了一下, 应该没有问题, 特别是编译选项, 与我是相同的. 

另外, 我们再装一个 `proto` 的语法分析器, 首先来到 `bufbuild`这个 `github` 项目, 在其中复制`-x86_64`后缀的文件夹地址, 然后还是`wget`再直接安装

![image-20251119222250260](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251119222250260.png)

```shell
[wind@Ubuntu ~]$ wget https://github.com/bufbuild/buf/releases/download/v1.60.0/buf-Linux-x86_64
[wind@Ubuntu ~]$ # 1. 移动文件到系统目录
sudo mv buf-Linux-x86_64.1 /usr/local/bin/buf

# 2. 设置执行权限
sudo chmod +x /usr/local/bin/buf

# 3. 验证安装（不需要修改 PATH）
buf --version
1.60.0
[wind@Ubuntu ~]$
```

接着, 我们在 code 上再安装配套的插件, 名字叫`Buf`, 安装, 然后刷新一下窗口

![image-20251119222457451](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251119222457451.png)

在下面的学习中, 我们将通过 "项目推进" 的方式来进行: 由于 `protobuf` 是由一个个小知识点组成的, 所以我们将通过一系列的项目认识它的各种用法, 先从最基本的入手, 再逐步深入.

## 通讯录 1.0

在通讯录 1.0 版本中, 我们将会聚焦于序列化和反序列化的数据存储, 实现一个简单的通讯录. 以掌握基本的`protobuf`用法, 包括以下内容:

- 将一个联系人的信息使用 `protobuf` 序列化并打印出来
- 对序列化后的内容进行反序列化, 解析出联系人并打印
- 联系人包含两个字段: 姓名, 年龄

下面, 我们就正式开始 `.proto` 文件的编写, 我的项目结构如下

```shell
[wind@Ubuntu contacts1.0]$ tree .
.
├── build
├── CMakeLists.txt
├── include
├── proto
│   └── contacts.proto
└── src

5 directories, 2 files
[wind@Ubuntu contacts1.0]$ 
```

首先, 对于`.proto`文件来说, 第一行正文需要指定使用的 `protobuf`语法版本, `protobuf`当前有三个语法版本, 首先就是可以认为已经被淘汰的`proto2`版本, 接着是目前主流的`proto3`版本, 除此之外, 在 `1.22.0`中, `protobuf`又引入了名为`editions`版本, 我们用的主要是`editions`这个语法版本, 并夹带对`proto3`的对比, 其实, 在环境搭建中对于`protobuf`有更简单的现成二进制文件安装指令, 但那个指令默认使用的是`1.22.1`版本, 由于版本比较早, 所以对于`editions`的支持可能并不完善, 因此, 我选择了让你直接去 `github` 发布页面复制资源链接这个方法, 这样, 你就可以获取对`editions`支持更为完善的版本.

如果使用`proto3`版本, 第一行正文你应该是写为`syntax = "proto3";`, 如果是`editions`, 应该写作`edition = "2024";` 相信你从写法上也能看出, `editions`对于语法的把控更为精确, 它是用年份来描述的.

接下来我们指定一下命名空间, 在 `proto`这里, 它使用`package`作为关键字, 所以如果你想让等会生成的C++代码对应类处于`contacts`这个命名空间的话, 就应该写作`package contacts;`

然后就是写一个`message`了, 我们只需要写一下姓名, 年龄这两个字段就行了.

![image-20251119223038636](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251119223038636.png)

至此, 工程已毕.

其中, 对于`message`中的每个字段, 我们可以看到由三部分构成, 首先是`proto`自己的数据类型, 接着是面向我们人类的字段名, 最后一个是一个编号, 这个编号是面向机器的字段名, 这个编号是一定要有的, 因为 `proto`底层实际上是用这些编号来区分字段的, 所以编号一定是不能重复的;

然后我们简要说一下最前面的类型, 见下表

| Protobuf 类型 | C++ 类型      | 取值范围               | 使用建议                   | 注意事项                   |
| :------------ | :------------ | :--------------------- | :------------------------- | :------------------------- |
| **标量类型**  |               |                        |                            |                            |
| `int32`       | `int32_t`     | -2³¹ to 2³¹-1          | 通用整数，节省空间时使用   | 负值编码效率较低           |
| `int64`       | `int64_t`     | -2⁶³ to 2⁶³-1          | 大整数、时间戳、ID         | 在32位系统上效率较低       |
| `uint32`      | `uint32_t`    | 0 to 2³²-1             | 非负整数，如数量、年龄     | 比 `int32` 编码效率高      |
| `uint64`      | `uint64_t`    | 0 to 2⁶⁴-1             | 大非负整数                 | 注意32位系统兼容性         |
| `sint32`      | `int32_t`     | -2³¹ to 2³¹-1          | **有符号且可能为负**的整数 | **ZigZag编码，负值效率高** |
| `sint64`      | `int64_t`     | -2⁶³ to 2⁶³-1          | **大负整数**               | **负值编码最优选择**       |
| `fixed32`     | `uint32_t`    | 0 to 2³²-1             | 大数值(>2²⁸)或哈希值       | 固定4字节，大数值效率高    |
| `fixed64`     | `uint64_t`    | 0 to 2⁶⁴-1             | 极大数值                   | 固定8字节                  |
| `sfixed32`    | `int32_t`     | -2³¹ to 2³¹-1          | 需要精确32位表示           | 固定4字节，无编码开销      |
| `sfixed64`    | `int64_t`     | -2⁶³ to 2⁶³-1          | 需要精确64位表示           | 固定8字节                  |
| `float`       | `float`       | ±3.4e±38 (~7 digits)   | 科学计算，节省空间         | 精度有限                   |
| `double`      | `double`      | ±1.7e±308 (~15 digits) | **默认浮点类型**           | 推荐使用                   |
| `bool`        | `bool`        | true/false             | 布尔标志                   | 编码为1字节                |
| `string`      | `std::string` | UTF-8字符串            | 文本数据                   | **必须为UTF-8**            |
| `bytes`       | `std::string` | 任意字节序列           | 二进制数据、加密数据       | 不验证编码                 |

其中 `protobuf` 类型就是在`.proto`文件中使用的类型, `C++类型`则是`.proto`被转化成对应 C++ 代码所对应的类型

我们先看两个`int`, 值得注意的是, 尽管我们看到他们的名字后面有32, 64 这种位数后缀, 但是, 在被序列化成为二进制流后, 这些类型的位数可能会发生变化, 因此我们把这种编码方式称之为"变长编码", 比如, 以`int32_t`为例, 对于一个较小的正数来说, 可能被序列化为二进制流后不是四字节, 而是二字节, 而对于一个负数来说, 为了强调它是负的, 可能实际上使用了十字节, 因此, 尽管它们确实可以被赋值为负数, 但是如果这个字段经常会遇到负数, 那么, 二进制流就会变大, 所以, 出于效率方面的考量, 对于这种情况, 应该使用`sint`, 而非`int`. `sint`的序列化逻辑针对负数进行了特化, 所以效率会好一点. 那两个`uint`也是变长编码的. 从这方面考虑的话, 年龄不会为负, 所以用`uint`更好, 但既然我们都这样写了, 那就不改了.

`fixed`则是定长编码, 选用32位, 则对应的二进制流也使用32位, 它们适合大数频率高的字段.

其它不讲了.

接下来说一下第三个元素, 也就是面向机器的字段名编号, 关于这个编号, 其范围为1 ~ 536,870,911 (2^29 - 1) ，其中 19000 ~ 19999 不可⽤。  不可用的原因是, 这些编号被官方预留了, 可能是他们自己要用, 又或者以后可能会用, 反正我们不能用就是了.

另外, 关于编号还有一个重要的点, 那就是对于越频繁使用的字段, 其编号就应该越小越好, 因为在序列化过程中, 为了区分这段二进制流是哪个字段的, 除了字段本身的值之外, 字段的编号也会一并序列化, 并且编号也是变长编码的, 因此, 频繁使用的字段, 如果使用更小的编号, 编号的序列化二进制流就会更短, 这样效率就会更高.

下面, 我们就手动编译一下`.proto`文件

```shell
[wind@Ubuntu build]$ tree ..
..
├── build
├── CMakeLists.txt
├── include
├── proto
│   └── contacts.proto
└── src

5 directories, 2 files
[wind@Ubuntu build]$ protoc -I ../proto/ --cpp_out=. contacts.proto
[wind@Ubuntu build]$ ls
contacts.pb.cc  contacts.pb.h
[wind@Ubuntu build]$ 
```

`protoc -I ../proto/ --cpp_out=. contacts.proto`的意思就是, 在`../proto/`这个目录下, 寻找`contacts.proto`, 并把它用C++语言编译出源代码, 输出到`.`这个目录下,  因为`proto`也是跨语言的, 所以指定语言也是很好理解的.

现在我们瞧一瞧生成的头文件

![image-20251119234838061](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251119234838061.png)

首先, 我们就能看到, 66行即为我们之前写的那个命名空间, 之后就是`PeopleInfo`这个类, 注意它继承了`Message`这个类.

再往下翻一翻, 我们就能看到那两个字段和与之相关的方法, 比如读写, `get` `set`

![image-20251119235207776](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251119235207776.png)

`name`其实就是`getname`, 但是他把`get`省略了, 之后我们就看到了`set_name`, `age`与之同理.

那序列化和反序列化在哪? 这就要看他的父类了, 对于`Message`, 还可以看到其继承了一个`MessageLite`

![image-20251119235752376](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251119235752376.png)

在其中, 我们可以看到一系列的`parse`方法, 这就是反序列化方法

![image-20251120000129269](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251120000129269.png)

它们提供了多种多样的反序列化途径, 比如`ParseFromIstream`是从流里面读, `ParseFromString`是从`string`里面读, `ParseFromArray`从数组里面读... 它们的返回值都是布尔, 表示反序列化是否成功.

`serialize`则是序列化方法

![image-20251120000729437](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251120000729437.png)

也是有很多输出渠道的, 比如字符串数组什么的.

# 完