# CMakeList.txt : CMake project for OpenBus.Vulkan, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

find_package (Vulkan REQUIRED FATAL_ERROR)
find_package (Qt5Core REQUIRED)
find_package (Qt5Gui REQUIRED)
find_package (Qt5Widgets REQUIRED)

add_executable (OpenBus "Main.cpp" "UI/MainWindow.h" "UI/MainWindow.cpp" "Game/Game.h" "Game/Game.cpp" "Engine/Screen.h" "Engine/Screen.cpp")

target_compile_definitions(OpenBus PUBLIC "APP_VERSION=\"v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${GIT_SHA1}\"")

target_include_directories (OpenBus
    PUBLIC "${LIBRARY_DIR}/glfw/include"
    PUBLIC "${LIBRARY_DIR}/glm"
)

target_link_libraries (OpenBus Vulkan::Vulkan)
target_link_libraries (OpenBus glfw)

# Qt configurations
qt5_use_modules (OpenBus Widgets)

add_custom_command (TARGET OpenBus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:OpenBus>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin> $<TARGET_FILE_DIR:OpenBus>/plugins/platforms/qwindowsd.dll
)
