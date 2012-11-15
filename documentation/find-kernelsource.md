How to find the correct kernel sources for your device
======================================================

Get the kernel version
-----------------------
Go to `Settings --> About Phone --> Kernel version`. The output has a format like this:

	2.6.35.14-al-g9b2f4dd user@host
	
	or
	
	2.6.35.15 user@host
	
If no version suffix like `-g9b2f4dd` is given, this is a hint the version is an official release.
Otherwise you have a concreate git commit for which you can look for.

Obtain the kernelsources
-------------------------
	1. If you use the manufacturers image, have a look at its homepage for the kernelsources. They have to publish them.
	
	2. For Google device [1] provides a good explanation on howto get the right kernelsoruces.
	
	3. For CyanogenMod look at [2] and browse cms repositories. Don't search for a concrete device. Rather look for the platform your devices use, e.g. HTC Desire Z is based on msm7230.
	
	
Sources
=======
[1] http://source.android.com/source/building-kernels.html
[2] https://github.com/CyanogenMod
	
