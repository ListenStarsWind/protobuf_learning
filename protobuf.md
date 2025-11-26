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
    
cmake -B build -G Ninja 
    -DCMAKE_C_COMPILER=clang-21 
    -DCMAKE_CXX_COMPILER=clang++-21 
    -DCMAKE_CXX_FLAGS="-stdlib=libc++ -O3 -DNDEBUG" 
    -DCMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -lc++abi" 
    -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX 
    -DCMAKE_PREFIX_PATH=$INSTALL_PREFIX 
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON 
    -DBUILD_SHARED_LIBS=ON 
    -Dprotobuf_BUILD_TESTS=OFF 
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
[whisper@starry-sky wind-libcxx-pkg-2025]$ ./install.sh
```

另外, 在这里, 我们再安装一个 `protobuf` 的语法分析器, 叫做`Tooltitude for Protobuf`, 直接在 VS Code 插件市场搜一下:

![image-20251123135245859](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123135245859.png)

装完它会弹出一个协议界面，因为这是个闭源的商业扩展（不是完全开源的那种），直接点右下角的 Accept 就行，不用纠结。尽管如此, 它的基本功能, 比如语法高亮、补全、跳转、悬停, 什么的都是免费的, 而且也比较新.

需要注意的是, 和其它的`protobuf`语法分析服务, 比如更加传统的`vscode-protoc`一样, 它们的语法分析没有那么严格: 即使有些错误的语法, 它也不一定能分析出来.

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

这里我就不贴命令行生成指令, 太长了,  还是让 `cmake` 根据 config 文件自己判断吧, 下面是配套的项目路径和 `CMakeLists.txt, fix-proto-dep.cmake`的内容

```shell
[wind@Ubuntu contacts2.4]$ tree .
.
├── build
├── CMakeLists.txt
├── fix-proto-dep.cmake
├── include
├── proto
│   ├── contacts.proto
└── src
    └── main.cc

5 directories, 5 files
[wind@Ubuntu contacts2.4]$ 
```

```cmake
# 最低 cmake 版本要求
cmake_minimum_required(VERSION 3.22)
project(ContactsDemo LANGUAGES CXX)

# ==============================================================================
# 1. 强制用 Clang-21 + libc++
# ==============================================================================
if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    set(CMAKE_CXX_COMPILER clang++-21)
endif()

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "本项目必须用 Clang 21 + libc++ 编译，先执行：export CXX=clang++-21")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options("-stdlib=libc++")
add_link_options("-stdlib=libc++" "-lc++abi")
message(STATUS "Clang 21 + libc++ 已启用")

# ==============================================================================
# 2. 查找我们自己装的 Protobuf + Abseil
# ==============================================================================
list(PREPEND CMAKE_PREFIX_PATH "/opt/libcxx-pkgs")
find_package(absl CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# ==============================================================================
# 3. 手动调用 protoc 控制生成流程 
# ==============================================================================
set(PROTO_DIR "${CMAKE_SOURCE_DIR}/proto")
set(PROTO_GEN_DIR "${CMAKE_BINARY_DIR}/gen/proto")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# 找所有 .proto 文件
file(GLOB_RECURSE PROTO_FILES RELATIVE "${PROTO_DIR}" "${PROTO_DIR}/*.proto")
if (NOT PROTO_FILES)
    message(FATAL_ERROR "在 ${PROTO_DIR} 里没找到 .proto 文件")
endif()

# 存放生成出来的 .cc/.h
set(PROTO_SRCS "")
set(PROTO_HDRS "")

# 逐个调用 protoc
foreach(proto_rel IN LISTS PROTO_FILES)
    get_filename_component(proto_dir  "${proto_rel}" DIRECTORY)      # 可能的子目录
    get_filename_component(proto_name "${proto_rel}" NAME_WE)        # 不带后缀名

    set(out_dir   "${PROTO_GEN_DIR}/${proto_dir}")
    set(out_cc    "${out_dir}/${proto_name}.pb.cc")
    set(out_h     "${out_dir}/${proto_name}.pb.h")
    set(dep_file "${out_cc}.d") # cmake protobuf 源文件依赖描述文件

    # 确保输出目录
    file(MAKE_DIRECTORY "${out_dir}")

    add_custom_command(
        OUTPUT  "${out_cc}" "${out_h}"
        COMMAND protobuf::protoc
                --cpp_out=${PROTO_GEN_DIR}
                --proto_path=${PROTO_DIR}
                --dependency_out=${dep_file}
                "${PROTO_DIR}/${proto_rel}"
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --blue --bold "Fixing depfile paths: ${dep_file}"
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/fix-proto-dep.cmake"
                "${dep_file}" "${CMAKE_SOURCE_DIR}"
        DEPENDS "${PROTO_DIR}/${proto_rel}" protobuf::protoc
        DEPFILE "${dep_file}"                          # 让 CMake 自动读取这个 .d 文件
        COMMENT "protoc → ${proto_rel}"
        VERBATIM
        USES_TERMINAL
    )

    list(APPEND PROTO_SRCS "${out_cc}")
    list(APPEND PROTO_HDRS "${out_h}") 
endforeach()

# ==============================================================================
# 4. 主程序入口
# ==============================================================================
add_executable(demo
    src/main.cc
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_include_directories(demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PROTO_GEN_DIR}          # 生成的 .pb.h 都放这里
)

target_link_libraries(demo PRIVATE
    protobuf::libprotobuf
    absl::strings
    absl::log
)

# ==============================================================================
# 5. 打印一下关键信息
# ==============================================================================
message(STATUS "")
message(STATUS "==================================================")
message(STATUS " 项目          : ${PROJECT_NAME}")
message(STATUS " 编译器        : ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})")
message(STATUS " 标准库        : libc++")
message(STATUS " Proto 文件    : ${PROTO_FILES}")
message(STATUS " 生成目录      : ${PROTO_GEN_DIR}")
message(STATUS " 可执行文件    : demo")
message(STATUS "==================================================")
message(STATUS "")

```

```shell
# fix-proto-dep.cmake
# 用法: cmake -P fix-proto-dep.cmake <depfile> <source_dir>

set(depfile "${ARGV0}")
set(src_dir "${ARGV1}")

if(NOT EXISTS "${depfile}")
    return()
endif()

file(READ "${depfile}" content)

# 把所有 ${src_dir}/ 开头的绝对路径替换成相对路径
string(REPLACE "${src_dir}/" "" relative_content "${content}")

# 可选：把 Windows 反斜杠也统一成正斜杠（更干净）
string(REPLACE "\\" "/" relative_content "${relative_content}")

file(WRITE "${depfile}" "${relative_content}")

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

在上面, 我们主要做的是将通讯录新增一个联系人, 接下来, 我们将会再写一份代码, 主要是将通讯录重新反序列化出来, 并打印显示. 为了以示区分, 之前的 `main` 函数内容会被放在 `func1`里面.

`func2` 的具体内容没什么可说的

![image-20251122195409143](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251122195409143.png)

```shell
[wind@Ubuntu build]$ cmake --build .
[2/2] Linking CXX executable demo
[wind@Ubuntu build]$ ./demo 
联系人姓名: 赤城
联系人年龄: 100
第1份电话号码: 83425

[wind@Ubuntu build]$ 
```

在上面, 我们用代码以自然语言的方式查看了二进制文件的内容, 但是呢? 写代码感觉有些重了, 可能我只是想稍微查看一下, 看看符不符合自己的预期, 此时我们就可以用 `protoc`的 `decode` 选项进行快速查看. 这个选项将会以标准输入的方式读取二进制流, 我们只需要为他提供涉及到的 `.proto` 源文件和直接涉及到的具体的消息类型即可.

```shell
[wind@Ubuntu build]$ /opt/libcxx-pkgs/bin/protoc --proto_path=../proto/ --decode=contacts.Contacts contacts.proto < contacts.bin 
contacts {
  name: "\350\265\244\345\237\216"
  age: 100
  phones {
    country: "86"
    number: "83425"
  }
}
[wind@Ubuntu build]$ 
```

因为 `protoc` 默认使用 ASCII 码, 所以中文没有正常显示. 

## 通讯录 2.1

下面, 我们将会给通讯录引入一个小改动: 我们将会在 `PhoneInfo` 消息中引入一个新的字段, 该字段是一个枚举类型, 用于描述号码的类型, 比如, 固定电话, 又或者, 移动电话.

![image-20251122213134609](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251122213134609.png)

关于枚举类型，其中字段后的数字，与其理解成字段的编号，不如理解成枚举常量的值，更加合适。枚举常量的值必须从 0 开始，并且不能为负数。

另外，无论是在 `protobuf` 还是 `C++` 中，枚举类型里的枚举常量和类中的字段是不同的：枚举常量是一个已经实例化的常量，而类的字段只是一个声明，除非它是静态的，否则在 `main` 函数之前不会被实例化。我之所以强调这一点，是因为它关联到枚举的另一个特点：枚举会把自身的枚举常量公开到父命名域。

这两个特点结合起来，就导致在同一个命名域下的两个枚举类型中，不能出现同名的枚举常量：因为枚举常量是公开的，可以被视为存在于父命名域中；同时它已经是实例化的对象，这意味着在同一个命名域下出现同名对象, 而这是不被允许的——实际上，如果出现同名，编译器也会报错。

我们看到插件没警告, 我们之前也说过, 它的语法分析比较"宽容", 但这也没什么关系, 我们直接用 `protoc` 编译一下就行了

![image-20251122224908255](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251122224908255.png)

```shell
[wind@Ubuntu build]$ cmake --build .
[1/6] Generating contacts.pb.cc/h
FAILED: gen/proto/contacts.pb.cc gen/proto/contacts.pb.h /home/wind/protobuf_learning/contacts2.1/build/gen/proto/contacts.pb.cc /home/wind/protobuf_learning/contacts2.1/build/gen/proto/contacts.pb.h 
cd /home/wind/protobuf_learning/contacts2.1/build && /opt/libcxx-pkgs/bin/protoc-34.0.0 --cpp_out=/home/wind/protobuf_learning/contacts2.1/build/gen/proto --proto_path=/home/wind/protobuf_learning/contacts2.1/proto /home/wind/protobuf_learning/contacts2.1/proto/contacts.proto
/home/wind/protobuf_learning/contacts2.1/proto/phone.proto:18:9: "MP" is already defined in "phone.PhoneInfo".
/home/wind/protobuf_learning/contacts2.1/proto/phone.proto:18:9: Note that enum values use C++ scoping rules, meaning that enum values are siblings of their type, not children of it.  Therefore, "MP" must be unique within "phone.PhoneInfo", not just within "A".
/home/wind/protobuf_learning/contacts2.1/proto/contacts.proto:7:1: Import "phone.proto" was not found or had errors.
/home/wind/protobuf_learning/contacts2.1/proto/contacts.proto:12:14: "phone.PhoneInfo" is not defined.
[2/6] Generating phone.pb.cc/h
FAILED: gen/proto/phone.pb.cc gen/proto/phone.pb.h /home/wind/protobuf_learning/contacts2.1/build/gen/proto/phone.pb.cc /home/wind/protobuf_learning/contacts2.1/build/gen/proto/phone.pb.h 
cd /home/wind/protobuf_learning/contacts2.1/build && /opt/libcxx-pkgs/bin/protoc-34.0.0 --cpp_out=/home/wind/protobuf_learning/contacts2.1/build/gen/proto --proto_path=/home/wind/protobuf_learning/contacts2.1/proto /home/wind/protobuf_learning/contacts2.1/proto/phone.proto
/home/wind/protobuf_learning/contacts2.1/proto/phone.proto:18:9: "MP" is already defined in "phone.PhoneInfo".
/home/wind/protobuf_learning/contacts2.1/proto/phone.proto:18:9: Note that enum values use C++ scoping rules, meaning that enum values are siblings of their type, not children of it.  Therefore, "MP" must be unique within "phone.PhoneInfo", not just within "A".
ninja: build stopped: subcommand failed.
[wind@Ubuntu build]$ 
```

关键是这行, `/home/wind/protobuf_learning/contacts2.1/proto/phone.proto:18:9: "MP" is already defined in "phone.PhoneInfo".`, 就是 `protoc` 报错使用了已经定义的枚举常量, 只不过这里没停止, `clang++`接着用错的代码继续编译, 然后报了后面那些错. 

如果你把 `A`移到其它的文件, 并在这里引入它, 那也会报错, 这是同样的道理, 我就不说原因了.

好的, 现在, 我们把`phone.proto`再改回正确的, 并重新编译

![image-20251123143101917](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123143101917.png)

我们看到, 对于嵌套定义的类型来说, 其名字是有特点的, 它的父类型会以前缀的方式放在`proto`文件的类型前, 这在枚举常量的名字上也有体现, 总的来说, 这些操作是为了让`protoc`生成的代码有更高的质量, 看起来一清二楚.  另外, 这里还有两个枚举值, 我们暂且不谈--时候未到.

`PhoneInfo_PhoneType_IsValid`用于判断一个整型是否是有效的枚举常量值, `PhoneInfo_PhoneType_Name`是把枚举常量的名字以字面量形式返回.

与之对应的, `PeopleInfo`也增加了对应的字段操作接口

![image-20251123143444989](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123143444989.png)

接下来, 是把`main.cc`中的两个`func`也做对应的适配.

![image-20251123144739867](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123144739867.png)

注意直接取是枚举值

![image-20251123145131066](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123145131066.png)

为了方便起见, 我们再加个命令行参数吧

![image-20251123150222117](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123150222117.png)

```shell
[wind@Ubuntu build]$ cmake --build .
ninja: no work to do.
[wind@Ubuntu build]$ ls
build.ninja  CMakeCache.txt  CMakeFiles  cmake_install.cmake  compile_commands.json  demo  gen
[wind@Ubuntu build]$ cp ~/protobuf_learning/contacts2.0/build/contacts.bin .
[wind@Ubuntu build]$ ./demo 1
正在添加一个新的联系人: 
请输入联系人的姓名: 企业    
请输入年龄: 89
请输入联系人电话号码, 直接回车结束记录, 第1份: 731820514
您输入的电话号码类型是: 1.移动电话, 2.固定电话1
请输入联系人电话号码, 直接回车结束记录, 第2份: 
一个新的联系人已经添加
[wind@Ubuntu build]$ ./demo 2
联系人姓名: 赤城
联系人年龄: 100
第1份电话号码: 83425, 类型: MP

联系人姓名: 企业
联系人年龄: 89
第1份电话号码: 731820514, 类型: MP

[wind@Ubuntu build]$ 
```

我们看到一个特别的现象是, 原先的"2.0"版本根本没有类型, 但在这里, 我们却读出了类型, 这就和枚举类型的默认值有关, 新版本的`protobuf`会对之前老版本二进制流中不存在的字段取为默认值, 对于枚举类型来说, 就是枚举值为0的那个, 所以在这里, "赤城" 也能看到电话号码类型

如果不存在的字段是更加复杂的自定义类型, 则主要取决于语言所对应该类型的具体实现, 比如对于容器来说, 一般就是一个空的容器..

## 通讯录 2.2

在通讯录 2.2 中, 我们将会为通讯录添加一个地址字段, 但重要的不是这个字段, 而是我们将借此使用 `protobuf` 中的一种泛用类型, 名字叫做`Any` 类型. 顾名思义, 这个类型可以接收其它任何的普通`Message`类型. 我们在安装目录下, 也能找到他的源码

![image-20251123193826257](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123193826257.png)

往下看, 我们发现, `Any` 本质上也是一个 Message, 第一个字段是原来类型的名字, 第二个, 则是这个类型经过序列化形成的二进制流.

![image-20251123194454022](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123194454022.png)

由于所有的 `Message` 类型都能进行序列化, 这使得, `Any` 真正变成了一个万能类型, 不管什么类型, 都可以往里面装.

为什么有 `Any` 这种东西呢? 从设计哲学的角度来说, 这使得使用`Any` 的`Message` 有很强的兼容性和可扩展性, 可以适应各种各样的对象, 这就好比我们之前学习的一些协议, 可能某些字段现在并不使用, 但以后可能使用, 所以它仍旧在协议, 从更为现实的生态角度来看, 相信我们也能看出来, `protobuf` 是谷歌生态下的, 为了让`protobuf`融合进 Google 的生态圈, 它也必须要有这种类型.

接下来我们看看与之对应的C++代码

![image-20251123205654675](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123205654675.png)

其中`PackFrom`就是把别的  Message 装进 Any 中, `InternalPackFrom`这个函数中其实就调用了 Message 自己的序列化方法, 把自己写到`value`字段了. `UnpackTo`就是直接转换为原来的类型, 

除此之外, 下面还有一个 `Is` 接口, 用来判断原来的类型与 T 是否是同一类型

![image-20251123210548025](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123210548025.png)

补充完这些前置知识后, 我们就再修改一下`proto` 源文件

![image-20251123205439032](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123205439032.png)

重新编译后, 我们也能看到相应的编译方法

![image-20251123205518864](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123205518864.png)

不过重点不在这里, 在我们之前看到`Any`自己的接口

接下来, 我们还是老样子, 去改那两个 `func`

![image-20251123225034438](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123225034438.png)

![image-20251123225055515](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251123225055515.png)

```shell
[wind@Ubuntu build]$ cmake --build .
ninja: no work to do.
[wind@Ubuntu build]$ ls
build.ninja  CMakeCache.txt  CMakeFiles  cmake_install.cmake  compile_commands.json  demo  gen
[wind@Ubuntu build]$ cp ../../contacts2.1/build/contacts.bin .
[wind@Ubuntu build]$ ./demo 1
正在添加一个新的联系人: 
请输入联系人的姓名: 欧根亲王
请输入年龄: 87
请输入联系人电话号码, 直接回车结束记录, 第1份: 830194671
您输入的电话号码类型是: 1.移动电话, 2.固定电话1
请输入联系人电话号码, 直接回车结束记录, 第2份: 
请输入联系人地址: 铁血
一个新的联系人已经添加
[wind@Ubuntu build]$ ./demo 2
联系人姓名: 赤城
联系人年龄: 100
第1份电话号码: 83425, 类型: MP

联系人姓名: 企业
联系人年龄: 89
第1份电话号码: 731820514, 类型: MP

联系人姓名: 欧根亲王
联系人年龄: 87
第1份电话号码: 830194671, 类型: MP
联系人地址: 铁血

[wind@Ubuntu build]$ 
```

## 通讯录 2.3

在这个版本中, 我们将会使用 `oneof` 类型, 该类型可以从多个字段中最多选择一个字段使用.

在此处, 我们增加了一个新的字段--其它的联系方式, 可以从腾讯家的这两个软件中任选一个

![image-20251124145315601](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251124145315601.png)

注意, 不要自找麻烦, `othof`类型子选项不能是数组.

![image-20251124152214631](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251124152214631.png)

除了`qq, wechat`的常规接口外, 还有`other_contact_case()`接口, 该接口可以以枚举类型的形式返回当前使用的字段是谁, 如果重复设置, 以最后一次设置为准.

![image-20251124152602645](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251124152602645.png)

`OTHER_CONTACT_NOT_SET`就是谁也没设置

![image-20251124154355392](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251124154355392.png)

![image-20251124154414825](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251124154414825.png)

```shell
[wind@Ubuntu build]$ cmake --build .
ninja: no work to do.
[wind@Ubuntu build]$ cp ../../contacts2.2/build/contacts.bin .
[wind@Ubuntu build]$ ./demo 1
正在添加一个新的联系人: 
请输入联系人的姓名: 俾斯麦
请输入年龄: 85
请输入联系人电话号码, 直接回车结束记录, 第1份: 88327115
您输入的电话号码类型是: 1.移动电话, 2.固定电话2
请输入联系人电话号码, 直接回车结束记录, 第2份: 
请输入联系人地址: 铁血
请选择其它的备用联系方式: 1. qq  2. 微信 2
请输入微信号: 19415271040
一个新的联系人已经添加
[wind@Ubuntu build]$ ./demo 2
联系人姓名: 赤城
联系人年龄: 100
第1份电话号码: 83425, 类型: MP

联系人姓名: 企业
联系人年龄: 89
第1份电话号码: 731820514, 类型: MP

联系人姓名: 欧根亲王
联系人年龄: 87
第1份电话号码: 830194671, 类型: MP
联系人地址: 铁血

联系人姓名: 俾斯麦
联系人年龄: 85
第1份电话号码: 88327115, 类型: TEL
联系人地址: 铁血
备用微信联系方式: 19415271040
[wind@Ubuntu build]$ 
```

## 通讯录 2.4

接下来, 我们为联系人增加一个 `map` 类型字段, 用来作为备注信息.

![image-20251125140323737](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251125140323737.png)

关于这里的 `map` 有一些要点需要注意.

首先是 `key` 类型的范围, 它被要求是一个数值精确, 稳定, 可以被哈希比较的类型, 由于正例比较多, 这里我们举反例, 浮点类型由于不够精确所以不可以作为 `key` 类型, 字节数组 `bytes` 和 ` message` 则不可以哈希比较, 所以也不支持. 枚举类型理论上确实可以用值进行比较, 但它们的大小比较没有实际意义, 也是不行的.  这样的话, 支持的类型就大概是通讯录 1.0 类型表上去掉浮点类型后剩下的.

`map` 类型不能被  `repeated` 修饰, 并且它是无序 `map`, 没有顺序保证.

这新增的接口并没有什么要说的点

![image-20251125142254641](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251125142254641.png)

![image-20251125150503880](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251125150503880.png)

![image-20251125150522972](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251125150522972.png)

```shell
[wind@Ubuntu build]$ cmake --build .
ninja: no work to do.
[wind@Ubuntu build]$ cp ../../contacts2.3/build/contacts.bin .
[wind@Ubuntu build]$ ./demo 1
正在添加一个新的联系人: 
请输入联系人的姓名: 加贺 
请输入年龄: 104
请输入联系人电话号码, 直接回车结束记录, 第1份: 913424855
您输入的电话号码类型是: 1.移动电话, 2.固定电话2
请输入联系人电话号码, 直接回车结束记录, 第2份: 
请输入联系人地址: 重樱
请选择其它的备用联系方式: 1. qq  2. 微信 2
请输入微信号: 194265
正在为联系人添加备注信息, 备注信息由两部分组成, 如果你想完成信息添加, 请在第一部分中直接回车
请输入备注的标题信息(第一部分): 1932
请输入备注的内容信息(第二部分): 她参与过一·二八事变
请输入备注的标题信息(第一部分): 
一个新的联系人已经添加
[wind@Ubuntu build]$ ./demo 2
联系人姓名: 赤城
联系人年龄: 100
第1份电话号码: 83425, 类型: MP

联系人姓名: 企业
联系人年龄: 89
第1份电话号码: 731820514, 类型: MP

联系人姓名: 欧根亲王
联系人年龄: 87
第1份电话号码: 830194671, 类型: MP
联系人地址: 铁血

联系人姓名: 俾斯麦
联系人年龄: 85
第1份电话号码: 88327115, 类型: TEL
联系人地址: 铁血
备用微信联系方式: 19415271040
联系人姓名: 加贺
联系人年龄: 104
第1份电话号码: 913424855, 类型: TEL
联系人地址: 重樱
备用微信联系方式: 194265备注信息标题: 1932, 备注信息内容: 她参与过一·二八事变

[wind@Ubuntu build]$ 
```

看来应该在备用联系方式后面换一个行

## 通讯录 3.0

在通讯录 3.0 中, 我们将把视角聚焦于 `.proto` 源文件的更新策略. 看看其中有哪些需要注意的要点. 

对于文件的更新, 总的来说, 可以分类为新增和删减这两种. 

其实在上面的一系列过程中, 我们就一直在更新之前的源文件. 所以在这个方面, 我们就不做具体代码示例了, 对于原先旧的二进制流中, 不存在的字段, 在一开始我们已经说过, 它们会直接使用对象实例化出来的那个默认值, 对于标量来说, 就是编译器本就支持的, 那张类型表上的类型来说, 它们就会使用各自的默认值.

而对于更为复杂的类型来说, 比如 message 来说, 则要看语言使用的具体底层类的行为, 数组读不出来就是默认的空数组, 枚举读不到就是默认的开头值为0的那个常量.

另外, 对于字段编号的选择, 应该避免和已经使用或者曾经使用的编号引发冲突, 在一开始, 我就说过, 序列化后的二进制流对于每个字段采用的是编号和字段值这种 pair 形式, 因此, 如果你使用了曾经用过的编号, 另一边可能因为某些原因没有及时更新, 从而把新的值映射到原先旧的那个字段上, 之后我们会在删减上用代码实际演示现象.

除了简单的直接新增之外, 也许你还会对原先的字段类型进行修改, 此时就需要注意不同类型之间序列化和反序列化方式是否是兼容的.

其中, `int32, uint32, int64, uint64, bool` 在序列化和反序列化方面是完全兼容的, 但是需要注意一下语言层, 比如, `int, uint`之前有明显的正负之分, 如果原先`int`的二进制流中使用的是负数值, 现在换成`uint`了, 那就可能会变成一个很大的值, 当然还有`32, 64`之分, `64`位往`32`里面放, 自然会发生截断现象, 反过来倒没什么.

`sint`们做另一桌, 和其他整型不兼容; `fixed`和`sfixed`在相同位数的情况下是相互兼容的.

 对于`oneof`来说, 你可以在原来字段也在可选项中的前提下用`oneof`平替它;  若确定在代码的具体使用过程中, 对于一个消息, 明明有多个字段, 可你总是只使用其中一个字段的前提下, 你也可以把这批字段统一放进一个`oneof`中; 不能把原先就存在的字段移到也原先存在的`oneof`中.

下面就是有关删减的项目代码了, 在这里, 我采用的全新的项目层次布局, 下面是具体内容

```shell
[wind@Ubuntu contacts3.0]$ tree .
.
├── build
├── CMakeLists.txt
├── fix-proto-dep.cmake
├── include
│   ├── client
│   └── server
├── proto
│   └── contacts.proto
└── src
    ├── client
    │   └── main.cc
    └── server
        └── main.cc

9 directories, 5 files
[wind@Ubuntu contacts3.0]$
```

```shell
[wind@Ubuntu contacts3.0]$ cat CMakeLists.txt 
cmake_minimum_required(VERSION 3.22)
project(ContactsDemo LANGUAGES CXX)

# ==============================================================================
# 1. 使用 Clang-21 + libc++（保持我们的默认工具链）
# ==============================================================================
if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    set(CMAKE_CXX_COMPILER clang++-21)
endif()
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "需要用 Clang 21 + libc++ 编译：export CXX=clang++-21")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options("-stdlib=libc++")
add_link_options("-stdlib=libc++" "-lc++abi")
message(STATUS "Clang 21 + libc++ 已启用")

# ==============================================================================
# 2. 查找依赖（选择我们本地编译的依赖）
# ==============================================================================
list(PREPEND CMAKE_PREFIX_PATH "/opt/libcxx-pkgs")
find_package(absl CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# ==============================================================================
# 3. 生成 protobuf 代码（支持自定义生成目录，也兼容相对路径）
# ==============================================================================
function(add_protobuf_generation)
    cmake_parse_arguments(ARG "" "GEN_ROOT;PROTO_DIR" "" ${ARGN})

    if(NOT ARG_GEN_ROOT OR NOT ARG_PROTO_DIR)
        message(FATAL_ERROR "add_protobuf_generation() 需要提供 GEN_ROOT 和 PROTO_DIR")
    endif()

    set(proto_gen_dir "${ARG_GEN_ROOT}/proto")
    file(MAKE_DIRECTORY "${proto_gen_dir}")

    file(GLOB_RECURSE proto_files RELATIVE "${ARG_PROTO_DIR}" "${ARG_PROTO_DIR}/*.proto")
    if(NOT proto_files)
        message(FATAL_ERROR "在 ${ARG_PROTO_DIR} 里没有找到 .proto 文件")
    endif()

    set(generated_srcs "")
    set(generated_hdrs "")

    foreach(proto_rel IN LISTS proto_files)
        get_filename_component(proto_dir  "${proto_rel}" DIRECTORY)
        get_filename_component(proto_name "${proto_rel}" NAME_WE)

        set(out_dir "${proto_gen_dir}/${proto_dir}")
        set(out_cc  "${out_dir}/${proto_name}.pb.cc")
        set(out_h   "${out_dir}/${proto_name}.pb.h")
        set(dep_file "${out_cc}.d")

        file(MAKE_DIRECTORY "${out_dir}")

        add_custom_command(
            OUTPUT  "${out_cc}" "${out_h}"
            COMMAND protobuf::protoc
                    --cpp_out=${proto_gen_dir}
                    --proto_path=${ARG_PROTO_DIR}
                    --dependency_out=${dep_file}
                    "${ARG_PROTO_DIR}/${proto_rel}"
            # 将 protoc 生成的依赖引导文件解析为相对路径
            COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/fix-proto-dep.cmake"
                    "${dep_file}" "${CMAKE_SOURCE_DIR}"
            DEPENDS "${ARG_PROTO_DIR}/${proto_rel}" protobuf::protoc
            DEPFILE "${dep_file}"
            COMMENT "protoc → ${proto_rel}"
            VERBATIM
            USES_TERMINAL
        )

        list(APPEND generated_srcs "${out_cc}")
        list(APPEND generated_hdrs "${out_h}")
    endforeach()

    # 让外层也能拿到生成结果
    set(PROTOBUF_GENERATED_SRCS ${generated_srcs} PARENT_SCOPE)
    set(PROTOBUF_GENERATED_HDRS ${generated_hdrs} PARENT_SCOPE)
    set(PROTOBUF_GEN_ROOT       ${ARG_GEN_ROOT}   PARENT_SCOPE)
endfunction()

# ==============================================================================
# 4. 添加可执行文件（server / client 共用相同结构）
# ==============================================================================
function(add_contacts_executable target_name src_main)
    add_executable(${target_name}
        "${src_main}"
        ${PROTOBUF_GENERATED_SRCS}
        ${PROTOBUF_GENERATED_HDRS}
    )

    target_include_directories(${target_name} PRIVATE
        ${PROTOBUF_GEN_ROOT}                    # 生成的头文件根目录
        "${CMAKE_SOURCE_DIR}/include"           # 公共头文件
        "${CMAKE_SOURCE_DIR}/include/${target_name}"  # 每个模块自己的头文件
    )

    target_link_libraries(${target_name} PRIVATE
        protobuf::libprotobuf
        absl::strings
        absl::log
    )

    message(STATUS "已添加可执行目标 → ${target_name} (入口: ${src_main})")
endfunction()

# ==============================================================================
# 5. 主流程：先生成 proto，再创建 server / client
# ==============================================================================
add_protobuf_generation(
    GEN_ROOT   "${CMAKE_BINARY_DIR}/gen"
    PROTO_DIR  "${CMAKE_SOURCE_DIR}/proto"
)

add_contacts_executable(server "src/server/main.cc")
add_contacts_executable(client "src/client/main.cc")

# ==============================================================================
# 6. 输出一些编译信息，便于查看
# ==============================================================================
message(STATUS "")
message(STATUS "==================================================")
message(STATUS " 项目           : ${PROJECT_NAME}")
message(STATUS " 编译器         : ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})")
message(STATUS " 标准库         : libc++")
message(STATUS " Proto 输出目录 : ${PROTOBUF_GEN_ROOT}")
message(STATUS " 目标可执行文件 : server、client")
message(STATUS " 构建示例       : cmake --build build --target server")
message(STATUS "                 : cmake --build build --target client")
message(STATUS "==================================================")
message(STATUS "")
[wind@Ubuntu contacts3.0]$ 
```

```shell
[wind@Ubuntu contacts3.0]$ cat fix-proto-dep.cmake 
# fix-proto-dep.cmake
# 用法: cmake -P fix-proto-dep.cmake <depfile> <source_dir>

set(depfile "${ARGV0}")
set(src_dir "${ARGV1}")

if(NOT EXISTS "${depfile}")
    return()
endif()

file(READ "${depfile}" content)

# 把所有 ${src_dir}/ 开头的绝对路径替换成相对路径
string(REPLACE "${src_dir}/" "" relative_content "${content}")

# 可选：把 Windows 反斜杠也统一成正斜杠（更干净）
string(REPLACE "\\" "/" relative_content "${relative_content}")

file(WRITE "${depfile}" "${relative_content}")
[wind@Ubuntu contacts3.0]$ 
```

关于新的项目, 我有几点需要强调, 首先我将 `proto` 源文件夹移到了根目录下, 这更符合 C++ 社区的一般习惯, 另外, 和原先一样, `pb` 文件会被放在`build/gen/proto`目录下, 但是, 在之前, 我们直接把`gen/proto`作为头文件搜索目录添加进去, 但这次, 我把头文件目录改为了`gen`, 我认为这样是更加恰当的, 不过这意味着我们包含`protoc`生成的头文件需要带上`proto/`的前缀. 另外, 尽管我们写了 `server, client`, 但为了节省时间, 也是为了降低门槛, 实际代码中, 我们就不手写套接字了, 也就是不真正地网络通信, 而还是像以前那样, 用硬盘作为中间媒介.

在`contacts.proto`中, 我们进行了简化, 因为在这里各种类型的使用不是重点

![image-20251125200158537](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251125200158537.png)

```shell
[wind@Ubuntu build]$ cat ../src/server/main.cc 
#include <unistd.h>

#include <format>
#include <fstream>
#include <limits>
#include <string>

#include "proto/contacts.pb.h"

int main() {
    using namespace std;
    using namespace contacts;

    string fName = "contacts.bin";
    string fTName = fName + ".temp";

    fstream input(fName.c_str(), ios::in | ios::binary);

    Contacts contacts;
    if (!input) {
        cout << "配置文件不存在, 将自动创建\n";
    } else {
        if (!contacts.ParseFromIstream(&input)) {
            cout << "通讯录初始化失败\n";
            // 生命周期结束, 自动析构并关闭文件
            return 1;
        }
    }

    auto people = contacts.add_contacts();

    cout << "开始添加新联系人: \n";
    cout << "请依据提示, 依次输入新联系人的姓名, 年龄, 联系电话 \n";

    uint32_t age;
    string name, phone;

    cout << "请输入新联系人的姓名: ";
    getline(cin, name);

    cout << "请输入新联系人的年龄: ";
    cin >> age;

    // 一直读取整型与换行之前的杂项内容, 包括换行本身
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "请输入新联系人的电话号码, 支持多个号码保存, 如果确认, 请直接回车\n";
    for (int i = 0;; ++i) {
        cout << format("第{}个: ", i + 1);
        getline(cin, phone);
        if (phone.empty()) break;
        people->add_phones(phone);
    }

    people->set_name(name);
    people->set_age(age);

    cout << "一个新的联系人已经添加成功\n";

    fstream output(fTName.c_str(), ios::out | ios::binary | ios::trunc);

    if (!contacts.SerializePartialToOstream(&output)) {
        cout << "数据持久化失败";
        return 2;
    }

    ::rename(fTName.c_str(), fName.c_str());
    return 0;
}[wind@Ubuntu build]$ cat ../src/client/main.cc 
#include <format>
#include <fstream>
#include <string>

#include "proto/contacts.pb.h"

int main() {
    using namespace std;
    using namespace contacts;

    string fName = "contacts.bin";

    fstream input(fName.c_str(), ios::in | ios::binary);

    if (!input) {
        cout << "中间媒介不存在, 反序列化无法进行\n";
        return 1;
    }

    Contacts contacts;
    if (!contacts.ParseFromIstream(&input)) {
        cout << "反序列化有误, 将停止程序\n";
        return 2;
    }

    input.close();

    auto size = contacts.contacts_size();
    for (int i = 0; i < size; ++i) {
        cout << format("----------联系人{}----------\n", i + 1);
        auto people = contacts.contacts(i);
        cout << format("姓名: {}\n", people.name());
        cout << format("年龄: {}\n", people.age());
        int phones_size = people.phones_size();
        for (int j = 0; j < phones_size; ++j) {
            cout << format("电话{}: {} \n", j + 1, people.phones(j));
        }
        cout << endl;
    }

    return 0;
}[wind@Ubuntu build]$ 
```

我们的实验策略是, 首先更改 `proto` 源文件, 然后, 仅重新编译`server`, `client`则保持旧版本, 以此模拟因为客户端不更新沿用旧的代码时所产生的现象.

```shell
[wind@Ubuntu build]$ cmake --build . # 两个程序一并编译
ninja: no work to do.
[wind@Ubuntu build]$ ls
build.ninja  client  CMakeCache.txt  CMakeFiles  cmake_install.cmake  compile_commands.json  gen  server
[wind@Ubuntu build]$ 
```

现在, 我们在 `proto` 源文件中将原先的年龄字段在语义层面重新修改(就是换个面向人的字段名, 所以语义改变了, 但面向机器的字段编号不变)

![image-20251125211259398](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251125211259398.png)

![image-20251125211247955](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251125211247955.png)

![image-20251125211317269](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251125211317269.png)

```shell
[wind@Ubuntu build]$ cmake --build . --target server # 只重新编译服务端
[0/4] protoc → contacts.proto
[4/4] Linking CXX executable server
[wind@Ubuntu build]$ ./server 
配置文件不存在, 将自动创建
开始添加新联系人: 
请依据提示, 依次输入新联系人的姓名, 生日, 联系电话 
请输入新联系人的姓名: 张三
请输入新联系人的生日: 229
请输入新联系人的电话号码, 支持多个号码保存, 如果确认, 请直接回车
第1个: 12345
第2个: 
一个新的联系人已经添加成功
[wind@Ubuntu build]$ ./client 
----------联系人1----------
姓名: 张三
年龄: 229
电话1: 12345 

[wind@Ubuntu build]$ 
```

现在, 我们就能明显发现, 新的 `server` 和旧的 `client` 就 "2" 这个字段有了分歧, 我们输入端认为是生日, 但另一个读端则认为是年龄.

在前面我也说过, 添加新字段的时候, 不应该使用正在使用或者曾经使用的字段编号. 但如果光靠人肉眼检查比较麻烦, 能不能把检查这个任务交给机器呢? 当然是可以的, `protobuf` 中有关键字 `reserved`, 可以将某个编号设置为"保留编号", 也可以将某个名字设置为"保留名字", 这样, 当重新使用保留编号或者保留名字的时候, 在 `protoc` 这里就会编译不通过.

![image-20251125212727584](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251125212727584.png)

```shell
[wind@Ubuntu build]$ cmake --build . --target server # 只重新编译服务端
[0/4] protoc → contacts.proto
/home/wind/protobuf_learning/contacts3.0/proto/contacts.proto:6:14: Field "birthday" uses reserved number 2.
/home/wind/protobuf_learning/contacts3.0/proto/contacts.proto:6:14: Suggested field numbers for contacts.People: 4
FAILED: gen/proto/contacts.pb.cc gen/proto/contacts.pb.h /home/wind/protobuf_learning/contacts3.0/build/gen/proto/contacts.pb.cc /home/wind/protobuf_learning/contacts3.0/build/gen/proto/contacts.pb.h 
cd /home/wind/protobuf_learning/contacts3.0/build && /opt/libcxx-pkgs/bin/protoc-34.0.0 --cpp_out=/home/wind/protobuf_learning/contacts3.0/build/gen/proto --proto_path=/home/wind/protobuf_learning/contacts3.0/proto --dependency_out=/home/wind/protobuf_learning/contacts3.0/build/gen/proto//contacts.pb.cc.d /home/wind/protobuf_learning/contacts3.0/proto/contacts.proto && /usr/bin/cmake -P /home/wind/protobuf_learning/contacts3.0/fix-proto-dep.cmake /home/wind/protobuf_learning/contacts3.0/build/gen/proto//contacts.pb.cc.d /home/wind/protobuf_learning/contacts3.0 && /usr/bin/cmake -E cmake_transform_depfile Ninja gccdepfile /home/wind/protobuf_learning/contacts3.0 /home/wind/protobuf_learning/contacts3.0 /home/wind/protobuf_learning/contacts3.0/build /home/wind/protobuf_learning/contacts3.0/build /home/wind/protobuf_learning/contacts3.0/build/gen/proto/contacts.pb.cc.d /home/wind/protobuf_learning/contacts3.0/build/CMakeFiles/d/bd8e4d5aa94056246f7e30c2d30f737712ed7bcb1d07db378986d0a9a3418063.d
ninja: build stopped: subcommand failed.
[wind@Ubuntu build]$ 
```

第一行的报错就提示, `birthday` 使用了被保留的数字

此时, 我们再改成未曾使用的 "4" 作为编号

注: `proto3`语法, `age` 是要带双引号的.

![image-20251125213842927](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251125213842927.png)

```shell
[wind@Ubuntu build]$ cmake --build . --target server # 只重新编译服务端
[0/4] protoc → contacts.proto
[4/4] Linking CXX executable server
[wind@Ubuntu build]$ ./server 
开始添加新联系人: 
请依据提示, 依次输入新联系人的姓名, 生日, 联系电话 
请输入新联系人的姓名: 李四
请输入新联系人的生日: 15
请输入新联系人的电话号码, 支持多个号码保存, 如果确认, 请直接回车
第1个: 14680
第2个: 
一个新的联系人已经添加成功
[wind@Ubuntu build]$ ./client 
----------联系人1----------
姓名: 张三
年龄: 229
电话1: 12345 

----------联系人2----------
姓名: 李四
年龄: 0
电话1: 14680 

[wind@Ubuntu build]$ 
```

此处, 旧的 `client` 由于没有接收到 `2` 字段, 所以使用了默认值. (之前那个用的是以前的二进制流, 所以还是这样)

另外, `reserved` 也可以批量保留编号 `reserved 100 to 200;`

----

在上面的通讯录 3.0 中, 可以确定的是最新的服务端确实保存了新联系人的用户信息, 并将其序列化到了硬盘上, 但是老版的客户端有没有保存这些对他来说, 可以算得上是"无效未知"的数据呢? 

这要视 `protobuf` 的具体版本而定, 在 `3.5` 前, 它不会保存未知字段, 而在之后, 就会保存到特定位置并提供特定接口进行访问, 对于我们使用的版本来说, 已经远远高于 `3.5` 了, 所以, 会保存无效字段.

对于未知字段的访问方法, 则需要结合 `protobuf` 的复用结构来看

![image-20251126144810579](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251126144810579.png)

我们看到, `Message` 一方面从`MessageLite`那里继承到了最为基本的序列化和反序列化能力, 另一方面, `Descriptor`的使用, 为其添加了对自定义消息进行语义描述的能力(比如, 获取自定义消息的名字, 所有字段的描述), `Reflection` 负责提供字段读写的动态方法(就是同一个接口, 通过参数的不同, 能够实现对所有字段的访问, 我们之前用的是静态读写方法), 其中也包括对于未知字段的读写.

`Reflection` 中的`GetUnknownFields`, 则可以获取`UnknownFieldSet`, 其中保存了一批未知字段, 我们可以将其中的单个未知字段用`field`获取, 而`UnknownField` 内部又会依据二进制流的特点, 将字段的基本类型进行大类上的细分

![image-20251126151906079](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251126151906079.png)

 `number` 可以把字段编号拿出来, `Type` 返回字段类型, `VARINT`表示可变长度整数, 包括 `int, uint, bool, enum`, `FIXED32` 表示定长32类, 包括 `fixed32, sfixed32, float`, `FIXED64` 定长64大类, 包括 `fixed64, sfixed64, double`, `LENGTH_DELIMITED`由长度, 内容组合成的通用类型, 包括`string, bytes, message, packed repeated`, `packed repeated`是被压缩的数组, 他把数组中的元素按顺序展开, 但单个元素不会再注明下标, 而只会注明总元素大小, 从而避免下标引发的空间资源浪费. `GROUP` 更老版本的 `protobuf` 类型, 可以认为已经不用了. 我们需要使用对应类型的接口, 才能把他们的值进行读写.

下面我们将用代码实际演示一下. 我们的思路是, 先将 `proto` 源文件回退到最早的版本, 然后对 `client` 进行修改, 增加对未知字段的访问, 然后只编译客户端, 读取之前的二进制文件.

![image-20251126154844404](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251126154844404.png)

![image-20251126154900091](https://wind-note-image.oss-cn-shenzhen.aliyuncs.com/image-20251126154900091.png)

```shell
[wind@Ubuntu build]$ cmake --build . --target client # 只重新编译客户端
[0/4] protoc → contacts.proto
[4/4] Linking CXX executable client
[wind@Ubuntu build]$ ./client 
----------联系人1----------
姓名: 张三
年龄: 229
电话1: 12345 

----------联系人2----------
姓名: 李四
年龄: 0
电话1: 14680 
字段编号: 4, 内容: 15

----------联系人3----------
姓名: 王五
年龄: 0
电话1: 12678 
字段编号: 4, 内容: 1020

[wind@Ubuntu build]$ 
```

----

在计算机界, 有前后兼容的概念, 结合`protobuf` 这个具体例子来说, 向前兼容指的是老模块能够正确识别新模块生成或者发出的协议, 新增加的字段会以未知字段的形式继续保存下来, 向后兼容指的是新模块能够正确识别老模块生成或者发出的协议. 前后兼容可以确保一个不能同时更新的分布式系统不会因为更新不同步导致灾难性问题.

----

在 `protobuf`源文件中, 我们可以使用 `option` 选项控制 `protoc` 的编译细节, 按照控制层次, 分为文件级, 字段级, 消息级等

比如这里, 在默认情况下, 它继承的是 `Message`, 但我们设置功能开关后, 就变成了`MessageLite`.

```shell
[wind@Ubuntu option]$ vim option.proto 
[wind@Ubuntu option]$ cat option.proto 
edition = "2024";

//option optimize_for = LITE_RUNTIME;

message People{
  string name  = 1;
}
[wind@Ubuntu option]$ /opt/libcxx-pkgs/bin/protoc --cpp_out=. option.proto
[wind@Ubuntu option]$ grep -n "class People" option.pb.h
55:class People;
70:class People final : public ::google::protobuf::Message
[wind@Ubuntu option]$ # 把注释去掉
[wind@Ubuntu option]$ vim option.proto 
[wind@Ubuntu option]$ /opt/libcxx-pkgs/bin/protoc --cpp_out=. option.proto
[wind@Ubuntu option]$ grep -n "class People" option.pb.h
49:class People;
64:class People final : public ::google::protobuf::MessageLite
[wind@Ubuntu option]$ 
```

`optimize_for` 是一种常用选项, 用于控制生成文件细节, 主要是在优化级别方面:

- `SPEED`, 这是`protoc`的默认选择, 此时, 生成的代码是⾼度优化的, 代码运行效率高, 但是空间占用较大
- `LITE_RUNTIME`, 不需要完整的`proto`功能, 只需要最核心的序列化, 反序列化, 字段静态读写方法, 以达到服务的轻量化, 效率高的同时空间也较小
- `CODE_SIZE`, 用时间换空间, 效率低, 但空间最小, 适用于资源及其紧张的设备.

`allow_alias` 也是另一种常用的选项, 有`true, false`两种选择, 默认`false`, 表示不许同一个字段编号有多个字段名, 开启后, 同一个编号的字段可以有多个名字, 实现别名的效果.
```shell
[wind@Ubuntu option]$ vim option.proto 
[wind@Ubuntu option]$ cat option.proto 
edition = "2024";

option optimize_for = LITE_RUNTIME;

enum Phone{
  option allow_alias = true;
  MP = 0;
  TEL = 1;
  LANDLINE = 1;
}

message People{
  string name  = 1;
}
[wind@Ubuntu option]$ /opt/libcxx-pkgs/bin/protoc --cpp_out=. option.proto
[wind@Ubuntu option]$ # 没有报字段编号重复的错误
```

`ProtoBuf` 也允许⾃定义选项并使⽤, 但对于我们来说, 用不上, 不说了.

## 网络通讯录

在通讯录 4.0 中, 我们将真正使用网络进行进程间的信息交互.

客户端可以选择对通讯录进行以下操作:

- 新增一个联系人
- 删除一个联系人
- 查询通讯列表
- 查询一个联系人的详细信息

服务端将提供上述服务, 并对联系人信息进行持久化操作.

为了降低成本和门槛, 我们在这里就不纯手写套接字了, 而是使用一个头文件级别的开源库, 叫做`Httplib`, 只需要包含`httplib.h`, 并连接上原生线程库`pthread`即可直接使用.

你可以直接获取它

```shell
wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
```

针对客户端的每个特定功能, 服务端都有对应的后台接口可以完整实际操作, 而它们, 正是依靠对应的 `message` 来进行通信的. 当用户选择了一个特定的功能时, 它会收集用户输入的信息, 将其序列化为请求负载, 再通过`Httplib`实际发送到对端, 对端对负载反序列化后, 调用对应的逻辑接口, 并把结果再一次序列化, 形成应答负载, 通过`Httplib`传送回去.

四个功能, 每个请求, 应答各一个 `message`, 这样差不多要写八个`message`, 不过我们实际上只把第一个功能实现一下, 其它的都差不多, 所以就不写了.  

```shell
[wind@Ubuntu contacts4.0]$ tree .
.
├── build
├── CMakeLists.txt
├── fix-proto-dep.cmake
├── include
│   ├── client
│   ├── httplib.h
│   └── server
├── proto
│   └── contacts.proto
└── src
    ├── client
    │   └── main.cc
    └── server
        └── main.cc

9 directories, 6 files
[wind@Ubuntu contacts4.0]$ # 和之前没有多大区别, CMakeLists.txt 只需要稍微改一改:
[wind@Ubuntu contacts4.0]$ sed -n "31p" CMakeLists.txt 
find_package(Threads REQUIRED)
[wind@Ubuntu contacts4.0]$ sed -n "108,113p" CMakeLists.txt 
    target_link_libraries(${target_name} PRIVATE
        protobuf::libprotobuf
        absl::strings
        absl::log
        Threads::Threads
    )
[wind@Ubuntu contacts4.0]$ # 就是把 pthread 链接上而已
```

```shell
[wind@Ubuntu build]$ cmake .. -G Ninja && cmake --build .
-- The CXX compiler identification is Clang 21.1.5
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/clang++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Clang 21 + libc++ 已启用
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE  
-- Found ZLIB: /usr/lib/x86_64-linux-gnu/libz.so (found version "1.3")  
-- 已添加可执行目标 → server (入口: src/server/main.cc)
-- 已添加可执行目标 → client (入口: src/client/main.cc)
-- 
-- ==================================================
--  项目              : ContactsDemo
--  编译器            : /usr/bin/clang++ (Clang)
--  标准库            : libc++
--  Proto 输出目录     : /home/wind/protobuf_learning/contacts4.0/build/gen
--  目标可执行文件     : server、client
--  构建示例          : cmake --build build --target server
--                    : cmake --build build --target client
-- ==================================================
-- 
-- Configuring done (0.8s)
-- Generating done (0.0s)
-- Build files have been written to: /home/wind/protobuf_learning/contacts4.0/build
[0/7] protoc → contacts.proto
[7/7] Linking CXX executable client
[wind@Ubuntu build]$ ls
build.ninja  client  CMakeCache.txt  CMakeFiles  cmake_install.cmake  compile_commands.json  gen  server
[wind@Ubuntu build]$ 
```



# 完