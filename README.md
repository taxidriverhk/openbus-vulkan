# Open Bus Driving Simulator #

This is a project for building a bus driving game from scratch. This game is written in `C++17` and uses `Vulkan` (and potentially other libraries such as `Direct3D 12`) for graphics and `Bullet Physics` for physics. This game is intended to allow content creators to develop maps and vehicles that are highly configurable for game play.

Although a good progress has been made to have the most critical graphics and game components working, there is still a long way to go and the game still lacks of validation and has many bugs. Many features and functions such as advanced vehicle physics, editor and reflections are yet to implement.

Please stay tuned for any updates on this project.

## Requirements

While the game performance highly depends on the map, vehicle and graphics settings, here are the minimum requirements that should meet in order to launch the game successfully.
* **Operating System**: Windows 10

* **Memory**: 8 GB

* **Disk Space**: 1 GB (without any addons)

## Installation

This game is highly portable, to launch the game, simply extract the binaries into any directory of your choice. Put in whatever addons you have in appropriate sub-directories, then you can enjoy the game.

This is [a sample map and vehicle package](https://1drv.ms/u/s!AjxD9FDQGclqlrNF5E0lRrs11Yu9tQ?e=gb2Ege) that you could use to try out how the game works.

## Development

This project is configured with `CMake`, although the build only generates Windows 64-bit-compatible binaries. To start the development, you need to first install the following SDKs.
* Vulkan SDK - https://vulkan.lunarg.com - choose `SDK Installer`

* Qt Open Source - https://www.qt.io/download-open-source - remember to select `MinGW 4.7` under `Qt 5.15.2` during installation

* Python 3.x - https://www.python.org - any binary will work, as long as it is `3.x` and it is not deprecated

* CMake - https://cmake.org - choose `Installer` under `Binary distributions`

In addition to that, clone the following Git repositories and then put them under `<source-code-directory>/Library/shaderc/third_party`
* https://github.com/KhronosGroup/glslang

* https://github.com/KhronosGroup/SPIRV-Headers

* https://github.com/KhronosGroup/SPIRV-Tools

Open `Advanced System Settings` from Windows Control Panel, verify that the following system variables are set.
* `CMAKE_PREFIX_PATH`: `C:\Qt\5.15.2\msvc2019_64`

* `CMAKE_SYSTE_NAME`: `Windows`

* `PATH`: contains `C:\VulkanSDK\1.2.170.0\Bin`

* `SFML_DIR`: `<source-code-directory>/Library/sfml/lib/cmake/SFML`

* `VK_SDK_PATH`: `C:\VulkanSDK\1.2.170.0`

* `VULKAN_INCLUDE_DIRS`: `C:\VulkanSDK\1.2.170.0\Include`

* `VULKAN_LIB_LIST`: `C:\VulkanSDK\1.2.170.0\Lib`

* `VULKAN_SDK`: `C:\VulkanSDK\1.2.170.0`

Once the steps above are taken, then open `cmake-gui`, and select this source directory to generate the files for your preferred IDE (ex. `.sln` file will be generated for `Visual Studio`). You should then be able to build and run/debug the game from your IDE.

## Download

To download the game, please click the link below, and then choose `Artifacts`, and an archive in `.zip` format should show up for you to download the binary.

| Platform | Status |
| -------- | ------ |
| **Windows x64** | [![AppVeyor build status](https://ci.appveyor.com/api/projects/status/bitbucket/taxidriverhk/open-bus-driving-simulator-vulkan?branch=master&svg=true)](https://ci.appveyor.com/project/taxidriverhk/open-bus-driving-simulator-vulkan) |
