SystemTap for Android
=====================

This repository contains a collection of scripts, tools, and sourcecode to build, install and run SystemTap on Android.
For further information about SystemTap I would appreciate you to read the documentation published by the official SystemTap project [1].

Requirments
------------
* Before you can start, you have to clone the submodules. Just run two commands:

	`git submodule init`

	`git submodule update`

Building SystemTap
------------------
To be able to compile your systemtap scripts for android, just start the buildscript:

	./build.sh

It automatically configures, build and installs systemtap for you.
You can find this installation in the `installed` directory. It is independet from any other installation on your system.
The script builds the arm binaries as well. They are located at `src/android_binaries`.

Setting up a device config
--------------------------
Before you can compile your stap script into a kernel module. You have to create a configuration for the target platform.
So far a configuration consists of a path to the kernel sourcetree.

To create such a configuration write the path in a textfile named `<device>.conf` and place it under the `config` directory.
Here is an example for the `Samsung Galaxy Nexus`:

	omap.conf, contents:

	KERNEL_ARCH=arm

	KERNEL_SRC=/path/to/my/kernel/omap
	
Compile a script for android
----------------------------
Ensure that you have created a configuration for your device under `conf/` and the kernel tree is prepared for compiling modules.
Your script should be located in the `scripts` directory.
Make sure you have an convenient toolchain in your PATH, e.g. the Android prebuilt one [2] or the one from MentorGraphics [3].
To start compilation:
	./build_module.sh <devicename> <script>

Where `<devicename` specifies the configuration to be used and `<script>` the systemtap script.
Important: Omit the filename extension from both parameters.

The compiled script is located  in `modules/<devicename>/`.

Run a compiled script (aka kernel module) on android
---------------------------------------------------
First you need to install the SystemTap Android Application located `android-app` on your device. It is a Eclipse project. So import it to Eclipse and install it on your device. (Note: You have to create a library project for SherlockActionbar aswell. A tarball containing the sources is located in `android-app/`.)
Now place the compiled script on the sdcard under the `/sdcard/systemtap/modules/` directory.
Just start the android app, select the module and start it. :-)
In addition, to the manual way you can transfer (and control as well) SystemTap via wifi. So, you may have a look at the following section.


Transfer a module to a device via wifi
--------------------------------------
1. If you haven't build `stapandroid` so far, run

	make -C tools/stapandroid

2. To transfer a module run the following command

	./tools/stapandroid/stapandroid [-p &lt;port&gt;] &lt;ip address/hostname&gt; send &lt;path to module&gt;

The tools supports further commands like
- start
- stop
- delete
- list

Tools
-----
The `tools` subdirectory contains two tools. First, two programs to extract and pack boot images for android. They are located in `tools/bootimg`.
Second, a tool to remote control an android device running the systemtapp application. It is called `stapandroid`. As presented in the previous section
it can execute several commands.

Both directories contain makefiles to simplify the build process. A simple `make -C tools/<directory>/` will do. There are targets to install/uninstall the binaries on your system.
They will ask for the root password inorder to install the binaries in `/usr/local/bin`.

Sources
--------
- [1] http://sourceware.org/systemtap/documentation.html
- [2] https://android.googlesource.com/platform/prebuilt
- [3] http://www.mentor.com/embedded-software/sourcery-tools/sourcery-codebench/editions/lite-edition/
