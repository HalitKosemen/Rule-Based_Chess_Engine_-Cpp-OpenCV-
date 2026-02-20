cmake_minimum_required(VERSION 3.15)
project(chessapp)  # Proje adını "obje_takip" olarak değiştirin

# C++ standartlarını ayarla
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenCV kütüphanesini bul
find_package(OpenCV REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(chess_app src/main.cpp)  # Çalıştırılabilir dosya adını "obje_takip_app" yapın
add_executable(test_app src/test.cpp)
# OpenCV kütüphanelerini bağla
target_include_directories(chess_app PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${Python3_NumPy_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
    ${SFML_INCLUDE_DIR}
)
target_include_directories(test_app PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${Python3_NumPy_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
    ${SFML_INCLUDE_DIR}
)
target_link_libraries(chess_app PRIVATE 
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    Python3::Python
    Python3::NumPy
    sfml-graphics
    sfml-window
    sfml-system
)
target_link_libraries(test_app PRIVATE 
    ${OpenCV_LIBS}
    ${Boost_LIBRARIES}
    Python3::Python
    Python3::NumPy
    sfml-graphics
    sfml-window
    sfml-system
)

set(CMAKE_VERBOSE_MAKEFILE ON)