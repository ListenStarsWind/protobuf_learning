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
