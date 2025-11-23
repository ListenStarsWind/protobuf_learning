# 最低 cmake 版本
cmake_minimum_required(VERSION 3.22)
project(ContactsDemo LANGUAGES CXX)

# ==============================================================================
# 1. 强制使用 Clang-21 + libc++
# ==============================================================================
if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    set(CMAKE_CXX_COMPILER clang++-21)
endif()

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "本项目必须用 Clang 21 + libc++ 编译！请执行：export CXX=clang++-21")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options("-stdlib=libc++")
add_link_options("-stdlib=libc++" "-lc++abi")
message(STATUS "已强制启用 Clang 21 + libc++")

# ==============================================================================
# 2. 查找我们自己编译的纯净 Protobuf + Abseil
# ==============================================================================
list(PREPEND CMAKE_PREFIX_PATH "/opt/libcxx-pkgs")
find_package(absl CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# ==============================================================================
# 3. 手写最稳的 protoc 外调生成（支持多层目录、依赖精准追踪）
# ==============================================================================
set(PROTO_DIR "${CMAKE_SOURCE_DIR}/proto")
set(PROTO_GEN_DIR "${CMAKE_BINARY_DIR}/gen/proto")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# 递归查找所有 .proto 文件
file(GLOB_RECURSE PROTO_FILES RELATIVE "${PROTO_DIR}" "${PROTO_DIR}/*.proto")
if (NOT PROTO_FILES)
    message(FATAL_ERROR "在 ${PROTO_DIR} 目录下没有找到任何 .proto 文件！")
endif()

# 用来收集所有生成的 .cc 和 .h
set(PROTO_SRCS "")
set(PROTO_HDRS "")

# 循环外调 protoc 生成对应代码
foreach(proto_rel IN LISTS PROTO_FILES)
    get_filename_component(proto_dir  "${proto_rel}" DIRECTORY)      # 子目录
    get_filename_component(proto_name "${proto_rel}" NAME_WE)        # 去掉 .proto

    set(abs_proto "${PROTO_DIR}/${proto_rel}")
    set(out_dir   "${PROTO_GEN_DIR}/${proto_dir}")
    set(out_cc    "${out_dir}/${proto_name}.pb.cc")
    set(out_h     "${out_dir}/${proto_name}.pb.h")

    # 确保输出子目录存在
    file(MAKE_DIRECTORY "${out_dir}")

    add_custom_command(
        OUTPUT  "${out_cc}" "${out_h}"
        COMMAND protobuf::protoc
                --cpp_out=${PROTO_GEN_DIR}
                --proto_path=${PROTO_DIR}
                "${abs_proto}"
        DEPENDS "${abs_proto}" protobuf::protoc
        COMMENT "protoc → ${proto_rel}"
        VERBATIM
    )

    list(APPEND PROTO_SRCS "${out_cc}")
    list(APPEND PROTO_HDRS "${out_h}")
endforeach()

# ==============================================================================
# 4. 主程序
# ==============================================================================
add_executable(demo
    src/main.cc
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_include_directories(demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PROTO_GEN_DIR}          # ← 所有 .pb.h 都在这里
)

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
message(STATUS " 项目 : ${PROJECT_NAME}")
message(STATUS " 编译器 : ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})")
message(STATUS " 标准库 : libc++")
message(STATUS " Proto 文件: ${PROTO_FILES}")
message(STATUS " 生成目录 : ${PROTO_GEN_DIR}")
message(STATUS " 可执行文件: demo")
message(STATUS "==================================================")
message(STATUS "")
