# Minecraft-MCPE

This repo includes the leak files for MCPE 0.6.1 version. Im looking into adding more features to make it better and interesting to play. (ports, adjustments, fixes) 

# Todos
- Build the project for all supported platforms and confirm they all work fine.
  - [x] Android
  - [x] IOS
  - [ ] Raspberry PI
  - [x] Server
  - [x] Win32 (GL project shouldve used OpenGL (not GLES) but its buggy with OpenGL and GLES is almost the same so its deprecated and will be replaced as DirectX version before UI changes are done)
- [ ] Make the Win32 Release more friendly to end-user. (hiding console on release, adding icon and proper name for the app, add mouse support and fix up the keyboard binds properly.)

- Add Ports to other Platforms
  - [ ] Linux
  - [ ] WinRT
  - [ ] UWP (Windows Phone, Desktop, Xbox)


- [ ] Port to Open GL ES 2.0
- [ ] Design Proper Settings Menu or Redesign the whole UI to look and functions more like Bedrock
- [ ] Backport Legacy Console, Bedrock and Java Features or adjustmnets so it feels more like Regular Minecraft.

## Current bugs
- Mouse Movement in Win32 can be buggy.
- Critical Issue: When joining to an server or an friend, you get spawned to default cordinates and more. singleplayer works without issues.

# Building

## Win32
1. Obtain Visual Studio with vs_xxx C++ toolchain (works from VS 2015 to VS 2022 with latest previews)
2. Open the sln project file in VS
3. Choose build config you prefer
4. Build

## Android

1. Download NDK r10e from here for your platform:
https://github.com/android/ndk/wiki/unsupported-downloads
2. Add the r10e to PATH so ndk-build is detected but dont add to ANDROID_HOME at any costs, it will break everything.
3. Add NDK_MODULE_PATH to Environment Paths and set it to the lib_projects folder.
4. Download Android Studio and for SDK, choose an folder that isnt the NDK, then choose Android-30 platform (Red Velvet Cake) and for sdk, look for 20.5xxxxxx version.
5. Make sure you have Java 9 JDK (must be JDK!!) installed and the jdk folder in Program Files/ Java (Windows Directory) folder points to JAVA_HOME environment variable. gradlew will tell if you did it correctly when you will build.
6. once SDK, NDK and Java part is done, go to the android folder in the project folder and open the terminal or command prompt.
7. then run gradlew to initialize everything and to make sure everything will work fine.
8. once gradlew command returns "BUILD SUCCESSFUL", then run ndk-build and confirm it builds the so library for minecraft.
9. Run gradlew build to compile the apk and to package everything (if you need specific ABI (Architecture like Arm64, Arm, x86)), check the buildcommands.txt.
10. Sideload to whatewer device you want and run it.

Notes for modern android users:
  - The UI on big screens is going to be small (change DPI if you really need), this is going to be fixed when UI overhaul is going to happen.
  - Issues with saving can happen, (Android 11+) make sure you granted the permissions for files.

## IOS
 - Prerequisities:
   - Mac os 10.7 or 10.8 install (Real Hardware or VM doesnt matter).
   - Xcode 4.5

- Keep in mind these specs are for how it was intended to be built back in that time. You can use newer Xcode however you want.

1. Open the IOS project file when you have Xcode installed and working
2. Choose for what Target you want to build (Demo or Regular)
3. Press Control and B Buttons on the keyboard to build the target (⌘ + B) (Win + B key for VM users on Windows)
4. Choose the device you will be running simulator for in top left (Ipad or Iphone)
5. Press Run to test (Keep in mind performance isnt going to be great).

## Running on an real device (IPhone)
1. Navigate to XCode Package Contents with Right Clicking XCode app in finder and selecting Show Package Contents
2. Navigate to Contents/Developer/Platforms/IphoneOS.platform/Developer/SDKs/(For what sdk your building for)/SDKSettings.plist
3. Open with XCode, Expand Default Properties and select AD_HOC_CODE_xxx to YES and CODE_SIGNING_REQUIRED to NO (must be uppercase)
4. Save the file, close XCode completely and reopen with the Project
5. Navigate to Project Settings and change the Basic Listing to All
6. Change under Code Signing  and Where Release and Debug are from Iphone Developer to Adhoc Signing (if not already changed)
7. Under Platform in the top toolbar choose Archive and once its done under the Archived project, right click and select Find in Finder
8. Where the archive is, right click and select show Package Contents and Navigate to Products / Application
9. In desktop or where you want, make an folder called Payload and copy the minecraftpe to it.
10. Compress the payload folder with right click and then rename zip to ipa.
11. Sideload the ipa how you want and you can play the game.

## Dedicated Server

## Windows

1. Open the Project File in dedicated_server folder, choose Release or Debug and build. same just like building for Win32.

## Linux
- Prerequisities:
  - Make sure these packages are installed and (Cmake, Make are on PATH):
    TODO  this section

1. Open dedicated_server folder in terminal
2. run "cmake"
3. if everything went fine with cmake, run "make" command

Now it should output mcpe_server which you can run by ./mcpe_server

- Keep in mind its super barebones and it even has some issues when joined to so its best to stay joined directly to an friend from an client.



        
  
