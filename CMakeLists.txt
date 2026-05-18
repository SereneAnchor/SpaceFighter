#设置Cmake版本最低为3.10.0
cmake_minimum_required(VERSION 3.10.0)

#设置项目名称:SpaceFighter 编程语言:C C++
project(SpaceFighter VERSION 0.1.0 LANGUAGES C CXX)

#设置C++标准(设为17且编译器必须支持C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#设置编译选项(对于代码中未使用的变量或者函数发出警告)
add_compile_options(/W4)
add_compile_options(/utf-8)

#设置编译输出目录(原先在build/Debug目录下,现在设为在项目根路径下:CMAKE_SOURCE_DIR表示顶层CMake路径)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

#设置项目名和平台名生成最终可执行文件名如Target为:SpaceFighter-Windows
set(Target ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})


#####==========1、CMake中链接第三方库的原始写法==========#####
#1、Windows平台:添加包含目录和链接目录(main.cpp中引用了SDL2,头文件有函数声明,链接库有函数实现)
#if(WIN32)
#include_directories("E:/VSCode/Project/C++/Library/SDL2/include")
#link_directories("E:/VSCode/Project/C++/Library/SDL2/lib/x64")

#2、MacOS平台:包含目录和链接目录需要在指定的路径中存在
#elseif(APPLE)
#include_directories(/usr/local/include/SDL2)
#link_directories(/usr/local/lib)

#3、Linux平台:包含目录和链接目录需要在指定的路径中存在
#elseif(UNIX)
#include_directories(/usr/include/SDL2)
#link_directories(/usr/lib/x86_64-linux-gnu)
#endif()


#####==========2、标准的第三方库写法(该库需要提供cmake文件夹)==========#####
#必须要找到SDL2库,否则无法编译执行(SDL2库是跨平台的,它本身必须要有自己的cmake文件)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

#添加可执行文件(最终的可执行文件名称为SpaceFighter)
add_executable(${Target}
                "src/main.cpp"
                "src/Game.cpp"
                "src/Scene.cpp"
                "src/MenuScene.cpp"
                "src/GameUI.cpp"
                "src/ResultScene.cpp"
                "src/MainScene.cpp"
                "src/EnemySystem.cpp"
                "src/ParticleSystem.cpp"
                "src/PowerUpSystem.cpp"
                "src/WaveSystem.cpp"
                "src/EventBus.cpp"
)

#指定链接库(链接核心库和主函数入口库:SDL2和SDL2main)
target_link_libraries(
    ${Target} ${SDL2_LIBRARIES} 
    SDL2::SDL2 
    SDL2::SDL2main
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
    SDL2_ttf::SDL2_ttf
)




#assets文件:存放项目静态资源(图片、字体、音频)
#src文件:存放项目源代码
#gcc:按照C语言编译,链接时不会自动包含C++标准库
#g++:按照C++语言编译,链接时自动包含C++标准库
