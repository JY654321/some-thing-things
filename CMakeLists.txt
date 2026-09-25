cmake_minimum_required(VERSION 3.20)
project(CompressDialogWin32 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 静态链接 C 运行时（对应 /MT），生成的 exe 不依赖 vcruntime
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

# WIN32 关键字：自动设置 /SUBSYSTEM:WINDOWS 和 wWinMainCRTStartup 入口
add_executable(CompressDialogWin32 WIN32 CompressDialogWin32.cpp)

# Unicode 字符集（对应 /DUNICODE /D_UNICODE）
target_compile_definitions(CompressDialogWin32 PRIVATE UNICODE _UNICODE)

# 链接系统库
target_link_libraries(CompressDialogWin32 PRIVATE
    user32
    gdi32
    comctl32
    dwmapi
    uxtheme
)
