## What is it?

- a SKSE plugin for Skyrim modding with c++ code

## What does it do?

- first goal is reminding the player to finish the setup guide in the starting area.

## developer web quickstart

- if you have a github account you can use the github website to edit code and compile the DLL
- if you are part of fg staff, just ask for access to the main org/repo
- otherwise create your own fork of the repo on the github website
- edit c++ code files like src/*.cpp directly on the github website
- a github workflow should automatically compile the DLL when you push to the main branch (about 2 minutes)
- you can download a zip with the DLL on the project/fork page under "actions" 
- you can configure workflow compile trigger conditions etc in .github/workflows/build.yml

## developer local quickstart

- install visual studio 2022 (free community edition), only needed for commandline compiler tools, see Requirements
- install cmake, see Requirements
- install vcpkg, needed for requirements like "CommonLibSSE NG", see Requirements
- open vscode (visual studio code), our main development IDE
- in the bar on the left under extensions install "CMake Tools" (by microsoft) and close+reopen vscode
- on the vscode welcome page, click "Clone Git Repository" and enter https://github.com/GobTacles/FGTweak.git
- if it asks for debug or release in the top center box, choose release for now
- it should start downloading and compiling "CommonLibSSE NG", can take a few minutes
- at the bottom of the window is a gear icon -> that builds the project and creates a DLL file
- you can manuall copy the dll from build/release/ to mods\FGTweak\SKSE\Plugins  (mo2 .. menu : new empty mod FGTweak)
- or set a windows environment var SKYRIM_MODS_FOLDER e.g. `C:\game\SkyrimModpacks\YourModList\mods`
- with that it would be automatically copied to the mod folder
- note that if the game is running, the copy will fail

### Requirements

- [Visual Studio 2022](https://visualstudio.microsoft.com/) (_the free Community edition_)
- [`cmake`](https://cmake.org/download/)
- [VS Code](https://code.visualstudio.com/)
- [`vcpkg`](https://github.com/microsoft/vcpkg)
  - 1. Clone the repository using git OR [download it as a .zip](https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip)
  - 2. Go into the `vcpkg` folder and double-click on `bootstrap-vcpkg.bat`
  - 3. Edit your system or user Environment Variables and add a new one:
    - Name: `VCPKG_ROOT`  
      Value: `C:\path\to\wherever\your\vcpkg\folder\is`

### Opening the project in VS Code

Once you have Visual Studio 2022 installed, you can open this folder in basically any C++ editor, e.g. [VS Code](https://code.visualstudio.com/) 
- _for VS Code, if you are not automatically prompted to install the 
- [C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) 
- [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) 
- extensions, please install those and then close VS Code and then open this project as a folder in VS Code_
- You may need to click `OK` on a few windows, but the project should automatically run CMake!

### SKYRIM_MODS_FOLDER

By default, when this project compiles it will output a `.dll` for your SKSE plugin into the `build/release/` folder.

If you want to configure this project to copy your plugin dll file
to `.../YourModList/mods/PROJECTNAME/SKSE/Plugins/PROJECTNAME.dll`
(_for Mod Organizer 2_)

- Set the `SKYRIM_MODS_FOLDER` environment variable to the path of your mods folder:  
  e.g. `C:\game\SkyrimModpacks\YourModList\mods`  

Reboot your PC after changing environment variables.

### code search / browsing

- ctrl/alt click or rclick jump to definition should work for commonlib symbols
- to include commonlib in search in all files :
- copy .vscode/settings.json.dist to .vscode/settings.json

## background infos

### github workflow

- to debug workflow timing add the following to CMakePresets.json under ci-release..cacheVariables 
- `"VCPKG_INSTALL_OPTIONS": "--debug"`

### CommonLibSSE NG info

- CommonLibSSE NG will be automatically downloaded and built by vcpkg when you open the project in vscode.
- Because this uses [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG), it supports Skyrim SE, AE, GOG, and VR.

[CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) is a fork of the popular [powerof3 fork](https://github.com/powerof3/CommonLibSSE) of the _original_ `CommonLibSSE` library created by [Ryan McKenzie](https://github.com/Ryan-rsm-McKenzie) in [2018](https://github.com/Ryan-rsm-McKenzie/CommonLibSSE/commit/224773c424bdb8e36c761810cdff0fcfefda5f4a).

- to update "CommonLibSSE NG" to a new version:

- vcpkg-configuration.json
- Update the baseline to the latest commit from the above repo. 6309841a.. = 2023-05-13 = latest on colored glass as of 2026-04
- see https://github.com/CharmedBaryon/CommonLibSSE-NG?tab=readme-ov-file#use

### misc

- SimpleIni.h/CSimpleIniA: simpleini will be automatically installed by vcpkg, https://github.com/brofield/simpleini
- project setup derived from mrowrpurr's "Hello World" template : https://github.com/SkyrimScripting/SKSE_Template_HelloWorld
