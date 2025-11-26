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
find_package(Threads REQUIRED)

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
        Threads::Threads
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
message(STATUS " 项目              : ${PROJECT_NAME}")
message(STATUS " 编译器            : ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})")
message(STATUS " 标准库            : libc++")
message(STATUS " Proto 输出目录     : ${PROTOBUF_GEN_ROOT}")
message(STATUS " 目标可执行文件     : server、client")
message(STATUS " 构建示例          : cmake --build build --target server")
message(STATUS "                   : cmake --build build --target client")
message(STATUS "==================================================")
message(STATUS "")
