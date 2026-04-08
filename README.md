# SKSE plugin

C++ SKSE plugin to tweak Skyrim

# What does it do?

- first goal 2026-04 is reminding a user to finish the setup guide in the starting realm.

# CommonLibSSE NG

Because this uses [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG), it supports Skyrim SE, AE, GOG, and VR.

[CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) is a fork of the popular [powerof3 fork](https://github.com/powerof3/CommonLibSSE) of the _original_ `CommonLibSSE` library created by [Ryan McKenzie](https://github.com/Ryan-rsm-McKenzie) in [2018](https://github.com/Ryan-rsm-McKenzie/CommonLibSSE/commit/224773c424bdb8e36c761810cdff0fcfefda5f4a).

# Requirements

- [Visual Studio 2022](https://visualstudio.microsoft.com/) (_the free Community edition_)
- [`vcpkg`](https://github.com/microsoft/vcpkg)
  - 1. Clone the repository using git OR [download it as a .zip](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)
  - 2. Go into the `vcpkg` folder and double-click on `bootstrap-vcpkg.bat`
  - 3. Edit your system or user Environment Variables and add a new one:
    - Name: `VCPKG_ROOT`  
      Value: `C:\path\to\wherever\your\vcpkg\folder\is`

## Opening the project

Once you have Visual Studio 2022 installed, you can open this folder in basically any C++ editor, e.g. [VS Code](https://code.visualstudio.com/) 
- > _for VS Code, if you are not automatically prompted to install the [C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) and [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extensions, please install those and then close VS Code and then open this project as a folder in VS Code_

You may need to click `OK` on a few windows, but the project should automatically run CMake!

It will _automatically_ download [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) and everything you need to get started making your new plugin!

# Project setup

By default, when this project compiles it will output a `.dll` for your SKSE plugin into the `build/` folder.

If you want to configure this project to output your plugin files
into your "`mods`" folder:  
(_for Mod Organizer 2_)

- Set the `SKYRIM_MODS_FOLDER` environment variable to the path of your mods folder:  
  e.g. `C:\game\SkyrimModpacks\YourModList\mods`  

Reboot your PC after changing environment variables.

# commonlibsse-ng update

vcpkg-configuration.json
Update the baseline to the latest commit from the above repo. 6309841a.. = 2023-05-13 = latest on colored glass as of 2024-10
see https://github.com/CharmedBaryon/CommonLibSSE-NG?tab=readme-ov-file#use

# tweaks

- to enable searching commonlib HEADERS (not source) but exclude most other build/ stuff :
- copy .vscode/settings.json.dist to .vscode/settings.json

# misc

- project setup derived from mrowrpurr's "Hello World" template : https://github.com/SkyrimScripting/SKSE_Template_HelloWorld
