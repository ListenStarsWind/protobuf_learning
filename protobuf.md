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

这脚本也不是我写的, 你可以很明显看出它是 AI 写的, 我大概看了一下, 应该没有问题, 特别是编译选项, 与我是相同的

在下面的学习中, 我们将通过 "项目推进" 的方式来进行: 由于 `protobuf` 是由一个个小知识点组成的, 所以我们将通过一系列的项目认识它的各种用法, 先从最基本的入手, 再逐步深入.

## 通讯录 1.0

在通讯录 1.0 版本中, 我们将会聚焦于序列化和反序列化的数据存储, 实现一个简单的通讯录. 以掌握基本的`protobuf`用法, 包括以下内容:

- 将一个联系人的信息使用 `protobuf` 序列化并打印出来
- 对序列化后的内容进行反序列化, 解析出联系人并打印
- 联系人包含两个字段: 姓名, 出生日期



# 完