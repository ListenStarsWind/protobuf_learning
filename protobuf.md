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

我这里采用本地编译安装的方式, 他的优点是与本地系统所使用的工具链可以更加兼容, 缺点是本地编译往往有很多坑, 而且耗时可能很长, 尽管如此, 我本人还是更喜欢这种方式, 或许我有点"强迫症"? 我因为喜欢 `clangd` 的语法分析效果, 所以入坑了 LLVM这套工具链, 所以系统上的默认C/C++编译器都是clang, clang++, 标准库用的也不是 GUN 的 `stdlibc++`, 而是 LLVM 的`libc++`, 但绝大多数的库或者工具都是用 GUN 编译生成的, GUN 和 LLVM 混用的话容易产生兼容性问题, 所以我喜欢整条链全用 LLVM,  再加上`protobuf`也不是多大的库, 我之前可本地编译过 BOOST, 所以对我来说, 是可以接受的

下面的这些脚本, 首先是安装必要的工具链(如果你直接用我提供的已经编译好的二进制文件, 只需要执行这个脚本即可) 接着是本地编译并安装 `protobuf` 的依赖库 `abseil`, 最后是本地编译并安装 `protobuf` , 另外, 为了不污染 `/usr/local`, 这个目录默认放的是 GUN 编译出来的库头文件可执行工具什么的, 而我们用的是 LLVM, 所以我会把它安装到 `/opt/libcxx-pkgs`

```shell
#!/usr/bin/env bash
# 用途：一次性安装 Clang 21 + 完整现代 C++ 开发全家桶（不科学上网）
set -e

echo "=================================================="
echo " 一键安装 Clang 21 + 完整现代 C++ 开发环境（国内优化版）"
echo " 包含：clang/clang++/clangd/lld/cmake/ninja/libc++/git 等"
echo "=================================================="

# 1. 添加清华大学 LLVM 21 国内高速镜像
sudo rm -f /etc/apt/sources.list.d/*llvm*        # 先清理可能残留的旧源
sudo tee /etc/apt/sources.list.d/llvm21.list <<EOF
deb https://mirrors.tuna.tsinghua.edu.cn/llvm-apt/noble/ llvm-toolchain-noble-21 main
EOF

# 2. 添加官方签名密钥（只需一次）
wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc >/dev/null

# 3. 安装 
sudo apt update
sudo DEBIAN_FRONTEND=noninteractive apt install -y \
    clang-21 lld-21 clangd-21 clang-tools-21 \
    libc++-21-dev libc++abi-21-dev libunwind-21-dev \
    cmake ninja-build git curl wget unzip build-essential pkg-config

# 4. 设置系统默认编译器为 Clang 21
sudo update-alternatives --install /usr/bin/clang   clang   /usr/bin/clang-21   210
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-21 210
sudo update-alternatives --install /usr/bin/clangd  clangd  /usr/bin/clangd-21  210
sudo update-alternatives --install /usr/bin/ld      ld      /usr/bin/ld.lld-21  210

# 5. 让动态链接器永久认 LLVM-21 的 libc++
echo "/usr/lib/llvm-21/lib/x86_64-linux-gnu" | sudo tee /etc/ld.so.conf.d/llvm-21.conf >/dev/null
sudo ldconfig

# 6. 创建安装目录
sudo mkdir -p /opt/libcxx-pkgs
sudo chown $USER:$USER /opt/libcxx-pkgs

# 7. 完成提示
echo ""
echo "国内镜像源安装完成！当前版本信息："
clang++-21 --version | head -n1
cmake --version | head -n1
ninja --version | head -n1
echo "默认标准库：libc++（已通过 -stdlib=libc++ 自动启用）"
```

```shell
#!/usr/bin/env bash
# filename: 02_build_abseil_static.sh
set -e

INSTALL_PREFIX="/opt/libcxx-pkgs"
sudo mkdir -p $INSTALL_PREFIX
sudo chown $USER:$USER $INSTALL_PREFIX

echo "=== 编译 Abseil 20250814.0 LTS（静态 + fPIC）==="
cd /tmp
rm -rf abseil-cpp
git clone --depth 1 --branch 20250814.0 https://github.com/abseil/abseil-cpp.git
cd abseil-cpp

cmake -B build -G Ninja \
    -DCMAKE_C_COMPILER=clang-21 \
    -DCMAKE_CXX_COMPILER=clang++-21 \
    -DCMAKE_CXX_FLAGS="-stdlib=libc++ -O3 -DNDEBUG" \
    -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -lc++abi" \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DABSL_PROPAGATE_CXX_STD=ON

cmake --build build -j$(nproc)
sudo cmake --install build

echo "Abseil 静态版已安装到 $INSTALL_PREFIX"
ls $INSTALL_PREFIX/lib/libabsl_*.a | wc -l
```

```shell
#!/usr/bin/env bash
# filename: 03_build_protobuf.sh
set -e

INSTALL_PREFIX="/opt/libcxx-pkgs"

echo "=== 编译 Protobuf 34.0.0（带静态 Abseil）==="
cd /tmp
rm -rf protobuf
git clone --depth 1 https://github.com/protocolbuffers/protobuf.git
cd protobuf

cmake -B build -G Ninja \
    -DCMAKE_C_COMPILER=clang-21 \
    -DCMAKE_CXX_COMPILER=clang++-21 \
    -DCMAKE_CXX_FLAGS="-stdlib=libc++ -O3 -DNDEBUG" \
    -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -lc++abi" \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
    -DCMAKE_PREFIX_PATH=$INSTALL_PREFIX \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DBUILD_SHARED_LIBS=ON \
    -Dprotobuf_BUILD_TESTS=OFF \
    -Dprotobuf_ABSL_PROVIDER=package

cmake --build build -j$(nproc)
sudo cmake --install build

echo "Protobuf 34.0.0 安装完成"
$INSTALL_PREFIX/bin/protoc --version
```

请特别不要改动 `cmake` 构建参数, 这是我试了好几次才得出的.

当然, 为了节省各位的时间, 我在我的库根目录下也准备了一个压缩包, 你可以直接解压, 并运行里面的一键安装脚本, 这样, 你就不需要去执行后两个脚本了.

```shell
[whisper@starry-sky protobuf_learning]$ ls
contacts1.0  protobuf.md  wind-libcxx-pkg-2025.tar.gz
[whisper@starry-sky protobuf_learning]$ tar -xzf wind-libcxx-pkg-2025.tar.gz
[whisper@starry-sky protobuf_learning]$ cd wind-libcxx-pkg-2025/
[whisper@starry-sky wind-libcxx-pkg-2025]$ sudo ./install.sh
```

另外, 我们再装一个 `proto` 的语法分析器, 首先来到 `bufbuild`这个 `github` 项目, 在其中复制`-x86_64`后缀的文件夹地址, 然后`wget`直接安装

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

接着咱们就简单使用一下其中的序列化和反序列化方法,  把联系人在不同状态下彼此转换一下.

![image-20251121141349740](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121141349740.png)

```shell
[wind@Ubuntu build]$ cmake .. -G "Ninja"
-- The CXX compiler identification is Clang 21.1.5
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/lib/llvm-21/bin/clang++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- 已强制启用 Clang 21 + libc++
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE  
-- Found ZLIB: /usr/lib/x86_64-linux-gnu/libz.so (found version "1.3")  
-- 
-- ==================================================
--  项目      : ContactsDemo
--  编译器    : /usr/lib/llvm-21/bin/clang++ (Clang)
--  标准库    : libc++
--  Proto 文件: /home/wind/protobuf_learning/contacts1.0/proto/contacts.proto
--  生成目录  : /home/wind/protobuf_learning/contacts1.0/build/gen/proto
--  可执行文件: demo
-- ==================================================
-- 
-- Configuring done (1.2s)
-- Generating done (0.0s)
-- Build files have been written to: /home/wind/protobuf_learning/contacts1.0/build
[wind@Ubuntu build]$ cmake --build .
[4/4] Linking CXX executable demo
[wind@Ubuntu build]$ ./demo 

芙宁娜·德·枫丹 
姓名:芙宁娜·德·枫丹, 年龄500多岁
[wind@Ubuntu build]$ 

```

我们看到, 年龄部分就是不可见的, 另外, 尽管直接用含不可见字符的数组初始化 string 是不好的, 因为 C 语言的终止符是 `\0`, 但是, string 是用内部的 size 字段获知内部二进制流大小的, 所以 protobuf 会用其他方法, 比如, 可能是迭代器为 string 赋值, 所以你用 string 做 流容器也是完全可以的, 实际上, 也主要用 string 做容器, 因为是个高级语言就有 string , 泛用性很广.

`protobuf` 的二进制流也是一个优点, 相比 `json xml`来说, 二进制不可读, 所以传输内容在天然上就有一定的破解成本.

这里我就不贴命令行生成指令, 太长了,  还是让 `cmake` 根据 config 文件自己判断吧, 下面是 `CMakeLists.txt`的内容

```cmake
# 最低 cmake 版本
cmake_minimum_required(VERSION 3.22)

# 使用 C++ 这门语言
project(ContactsDemo LANGUAGES CXX)

# ==============================================================================
# 1. 强制使用 Clang-21 + libc++ 以配合我们之前本地编译的库文件
# ==============================================================================
if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    set(CMAKE_CXX_COMPILER clang++-21)
endif()

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "本项目必须用 Clang 21 + libc++ 编译！请执行：export CXX=clang++-21")
endif()

# 创建一些变量, 控制编译行为
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)                 # 关闭 GNU 扩展，更纯粹
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # 生成 clangd 引导文件
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 强制使用 libc++
add_compile_options("-stdlib=libc++")
add_link_options("-stdlib=libc++" "-lc++abi")

message(STATUS "已强制启用 Clang 21 + libc++")

# ==============================================================================
# 2. 查找我们自己编译的纯净 Protobuf + Abseil
# ==============================================================================
# 为环境变量中的库搜索路径增加新的前缀 /opt/libcxx-pkgs
list(PREPEND CMAKE_PREFIX_PATH
    "$ENV{LIBCXX_PKGS}"
    "/opt/libcxx-pkgs"
)

# 让 cmake 以 config 模式寻找它们
find_package(absl CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# ==============================================================================
# 3. 自动查找并生成所有 .proto 文件（支持 proto/ 任意层级子目录）
# ==============================================================================
set(PROTO_DIR "${CMAKE_SOURCE_DIR}/proto")
file(GLOB_RECURSE PROTO_FILES "${PROTO_DIR}/*.proto")

if (NOT PROTO_FILES)
    message(FATAL_ERROR "在 ${PROTO_DIR} 目录下没有找到任何 .proto 文件！")
endif()

set(PROTO_GEN_DIR "${CMAKE_BINARY_DIR}/gen/proto")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

set(PROTO_SRCS "")
set(PROTO_HDRS "")

# 以循环的方式, 外部调用 protoc 编译生成对应代码
foreach(proto ${PROTO_FILES})
    file(RELATIVE_PATH proto_rel "${PROTO_DIR}" "${proto}")
    get_filename_component(proto_dir "${proto_rel}" DIRECTORY)
    get_filename_component(proto_name "${proto}" NAME_WE)

    set(out_dir "${PROTO_GEN_DIR}/${proto_dir}")
    file(MAKE_DIRECTORY "${out_dir}")

    set(out_cc "${out_dir}/${proto_name}.pb.cc")
    set(out_h  "${out_dir}/${proto_name}.pb.h")

    add_custom_command(
        OUTPUT "${out_cc}" "${out_h}"
        COMMAND protobuf::protoc
            --cpp_out=${PROTO_GEN_DIR}
            --proto_path=${PROTO_DIR}
            "${proto}"
        DEPENDS "${proto}" protobuf::protoc
        COMMENT "Generating ${proto_name}.pb.cc/h"
        VERBATIM
    )
    list(APPEND PROTO_SRCS "${out_cc}")
    list(APPEND PROTO_HDRS "${out_h}")
endforeach()

# ==============================================================================
# 4. 主程序（只有一个 demo）
# ==============================================================================
# 明确依赖关系, 生成可执行程序时为 PROTO_* 初始化, 即调用前面的外部命令
add_executable(demo
    src/main.cc
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

# 指定项目级的头文件搜索路径
target_include_directories(demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PROTO_GEN_DIR}
)

# 进行必要链接
target_link_libraries(demo PRIVATE
    protobuf::libprotobuf
    absl::strings
    absl::log
)

# ==============================================================================
# 5. 编译信息
# ==============================================================================
message(STATUS "")
message(STATUS "==================================================")
message(STATUS " 项目      : ${PROJECT_NAME}")
message(STATUS " 编译器    : ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})")
message(STATUS " 标准库    : libc++")
message(STATUS " Proto 文件: ${PROTO_FILES}")
message(STATUS " 生成目录  : ${PROTO_GEN_DIR}")
message(STATUS " 可执行文件: demo")
message(STATUS "==================================================")
message(STATUS "")
```

啊, 对了, 如果像用`clangd`语法分析器的话, 除了前面环境搭建的第一个脚本安装 `clangd`本体之外, 还要在code中安装同名的扩展哟, 并且在库的根目录下, 指明一下 `clangd`引导文件, `compile_commands.json`的位置在 build 下.

```shell
[wind@Ubuntu build]$ ls -a ~/protobuf_learning/
.  ..  contacts1.0  .git  protobuf.md  .vscode  wind-libcxx-pkg-2025.tar.gz
[wind@Ubuntu build]$ ls -a ~/protobuf_learning/.vscode/
.  ..  settings.json
[wind@Ubuntu build]$ cat ~/protobuf_learning/.vscode/settings.json 
{
    "clangd.arguments": [
        "--compile-commands-dir=${workspaceFolder}/build",
        "--background-index",
        "--clang-tidy",
        "--cross-file-rename",
        "--header-insertion=never"
    ]
}[wind@Ubuntu build]$ 
```

code 打开的文件夹就是`${workspaceFolder}`, 所以不要直接用 code 打开整个库, 而是具体的工作区文件夹, 有 build 这个子文件夹的那种.

## 通讯录 2.0

在上面的通讯录1.0 中, 我们简单地使用了一下 `protobuf` 生成代码的一些接口, 下面, 我们就要来写通讯录2.0 了, 在2.0中, 我们将会引入新的功能要求, 并且正式开始学习 `protobuf` 的各个语法.

我们新引入的要求如下:

- 在上面, 我们只写了一个联系人, 并没有写通讯录, 这次, 我们就把通讯录真正地写出来
- 我们将会把通讯录写入到文件里, 以进行持久化保存
- 为联系人新增一些其他属性, 比如, 电话号码

在其中, 由于通讯录中有多个联系人, 为此我们需要在通讯录中定义一个数组字段, 而这就涉及到了 `protobuf`的字段属性语法, 我们也会在其中说说消息定义的其它事项.

我们先把联系人加个新的属性, 电话号码, 电话号码可能有多个, 为此, 我们将把它们定义为数组. 在 `protobuf`中, 字段可以通过 `singular` 和 `repeated` 这两个关键字修饰, 如果你不写, 将默认使用 `singular` 这个关键字, 表示这个字段要么是空的, 要么就一个, `repeated` 则表示该字段是个数组, `protobuf` 就会生成针对性的接口, 为该数组的操作提供方法. 

![image-20251121170006069](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121170006069.png)

又或者, 因为你需要把电话附加一些别的信息, 比如电话的种类, 国际电话区号(86)什么的, 你就可以再定义一个消息, 在其中增加相应的字段.

![image-20251121170648428](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121170648428.png)

又或者, 你可以嵌套定义

![image-20251121170936658](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121170936658.png)

如果 `PeopleInfo` 定义在另一个 `.proto`中, 我们就需要 "include", 在 `protobuf` 中, "include" 是用关键字 "import" 的

![image-20251121171307638](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121171307638.png)

![image-20251121174008894](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121174008894.png)

值得注意的是, `.proto` 中对于命名域的指定和 C++ 有些不同, 它是用 `.` 而非 `::`

接下来, 我们就写一下通讯录, 其内部其实就是一个 `PeopleInfo` 数组

![image-20251121173951414](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121173951414.png)

接下来, 我们就生成对应的 C++ 代码

```shell
[wind@Ubuntu build]$ rm -rf ./*
[wind@Ubuntu build]$ cmake .. -G "Ninja" && cmake --build .
-- The CXX compiler identification is Clang 21.1.5
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/lib/llvm-21/bin/clang++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- 已强制启用 Clang 21 + libc++
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE  
-- Found ZLIB: /usr/lib/x86_64-linux-gnu/libz.so (found version "1.3")  
-- 
-- ==================================================
--  项目      : ContactsDemo
--  编译器    : /usr/lib/llvm-21/bin/clang++ (Clang)
--  标准库    : libc++
--  Proto 文件: /home/wind/protobuf_learning/contacts2.0/proto/contacts.proto;/home/wind/protobuf_learning/contacts2.0/proto/phone.proto
--  生成目录  : /home/wind/protobuf_learning/contacts2.0/build/gen/proto
--  可执行文件: demo
-- ==================================================
-- 
-- Configuring done (1.2s)
-- Generating done (0.0s)
-- Build files have been written to: /home/wind/protobuf_learning/contacts2.0/build
[6/6] Linking CXX executable demo
[wind@Ubuntu build]$ ls
build.ninja  CMakeCache.txt  CMakeFiles  cmake_install.cmake  compile_commands.json  demo  gen
[wind@Ubuntu build]$ tree gen/
gen/
└── proto
    ├── contacts.pb.cc
    ├── contacts.pb.h
    ├── phone.pb.cc
    └── phone.pb.h

2 directories, 4 files
[wind@Ubuntu build]$ 
```

![image-20251121174845475](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121174845475.png)

这里就是 `PeopleInfo` 中两个接口的读写方法

`contacts.pb.h` 则包含了之前的 `phone.pb.h`

我们重点看这里的数组接口

![image-20251121175504747](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121175504747.png)

`phones_size`方法, 返回数组的大小, `add_phones`方法, 会在数组中插入一个默认的对象, 然后把这个对象的指针返回出来, 以供我们修改. `mutable_phones(int index)`可以返回指定下标对象的原始指针进行修改, 无参的是返回数组指针, 或者说, 是首元素指针, `phones`和 `mutable_`们类似, 只不过是返回引用.

通讯录这边也是类似

![image-20251121181128828](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121181128828.png)

下面, 我们就来改 `main.cc`了

我们的 `main` 函数将会分为三个大部分, 首先, 我们将打开一个文件, 作为通讯录的数据文件, 通讯录会在其中进行反序列以进行初始化, 接着就是添加一个新的联系人, 其中有些小点需要注意, 最后就是把通讯录重新序列化, 写入到之前的文件中.

第一部分, 没什么好讲的, 值得注意的是, 文件要以二进制方式打开, 因为 `protobuf` 是二进制流, 在第一次运行程序, 因为还没有数据文件, 所以我们实际上只需要创建, 不用初始化, 之后, 若数据文件没有被删除, 那么, 它就会读取其中内容进行反序列化

第二部分, 我们将在标准输入中依次输入 姓名, 年龄, 及若干电话号码, 用回车这个方式进行分隔, 特别要注意的是, 在输出年龄时, 由于使用的是流提取符号, 所以我们只会把数字给读上来, 但不会吧换行读出, 而之后的号码输入, 是否输入空行又是结束循环的重要依据, 所以我们需要 `ignore` 接口, 把换行手动读一下, 否则之后的 `getline` 读到的就是空行, 进而结束循环, 导致号码无法记录

在第三部分, 为了确保文件的安全存储, 我们的实际思路是, 先创建一个写的临时文件, 往其中序列化, 之后再调用系统接口把它改成数据文件的名字从而完成对旧数据文件的覆写. 这样更加安全: 如果直接写, 首先要清空旧文件, 这会导致旧数据丢失, 此时若恰好写入异常, 一方面, 旧数据没了, 另一方面, 新数据没写完, 那就非常不好了.

![image-20251121205737232](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121205737232.png)

![image-20251121205753505](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121205753505.png)

![image-20251121205808233](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121205808233.png)

注: 运行时, 如果想要删除, 应该按 ctrl + 删除键, 而不是单按删除键

```shell
[wind@Ubuntu build]$ cmake --build .
ninja: no work to do.
[wind@Ubuntu build]$ ls
build.ninja  CMakeCache.txt  CMakeFiles  cmake_install.cmake  compile_commands.json  demo  gen
[wind@Ubuntu build]$ ./demo 
配置文件不存在, 已经自动创建
正在添加一个新的联系人: 
请输入联系人的姓名: 赤城  
请输入年龄: 100
请输入联系人电话号码, 直接回车结束记录, 第1份: 83425
请输入联系人电话号码, 直接回车结束记录, 第2份: 
一个新的联系人已经添加
[wind@Ubuntu build]$ ls
build.ninja  CMakeCache.txt  CMakeFiles  cmake_install.cmake  compile_commands.json  contacts.bin  demo  gen
[wind@Ubuntu build]$ 
```

 由于是二进制的, 有看不到的字符也很正常

![image-20251121210044278](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251121210044278.png)

如果你想命令行看一看的话, 或许你可以使用  Linux 自带的 `hexdump`, 这是一个二进制文件查看工具, 用于将二进制文件转换成十六进制和可读字符串形式(如果可读的话)

```shell
[wind@Ubuntu build]$ hexdump -C contacts.bin
00000000  0a 17 0a 06 e8 b5 a4 e5  9f 8e 10 64 1a 0b 0a 02  |...........d....|
00000010  38 36 12 05 38 33 34 32  35                       |86..83425|
00000019
[wind@Ubuntu build]$ 
```

题外话, 对于更老的 `protobuf`版本, `main`函数开头一般执行一个宏, `GOOGLE_PROTOBUF_VERIFY_VERSION;`, 该宏的作用是, 确保在机器安装了多个版本的`protobuf`时, 不会出现版本混乱, 比如, 头文件用的是那个版本的, 库文件用的是另一个版本; 但对于我们所使用的版本来说, 它已经足够新: 会自动检查这种清空是否存在, 所以新版本写上也没什么意义.

另外还有静态成员函数 `google::protobuf::ShutdownProtobufLibrary();  `用于析构 `protobuf` 创建的全局属性对象: 可能你正在做很底层的东西, 底层到它可能没有完善的自动析构体系, 此时如果你希望, `protobuf`全局属性对象刷新或者析构, 你就可以调用它, 以确保有明确的全局属性析构或者刷新, 它如果要写的话, 一般写在执行流的末尾, 比如 `main` 函数末尾.



# 完