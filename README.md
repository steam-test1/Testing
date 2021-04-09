# SuperBLT
An open source Lua hook for Payday 2, designed and created for ease of use for both players and modders.

This is a unofficial continuation of the BLT modloader, with additional features aimed at allowing things
not possible in standard Lua, such as patching XML files that are loaded directly by the engine, or playing
3D sounds.

This is the developer repository, and should only be used if you know what you're doing (or running GNU+Linux - explained below). If you don't, visit the website at [SuperBLT.znix.xyz](https://superblt.znix.xyz/) to get an up-to-date drag-drop install.
The Lua component of the BLT which controls mod loading can be found in it's own repository, [payday2-superblt-lua](https://gitlab.com/znixian/payday2-superblt-lua).

## Download
Visit [The SuperBLT Site](https://superblt.znix.xyz/) to get the latest stable download for Windows. 
Dribbleondo maintains a Linux download that is primarily for Debian-based Distro's, 
which [can be accessed here](https://drive.google.com/open?id=1qcZ3-FFTbmI075pzNyY2h_XtRdnnTTDl). See below for building
GNU+Linux binaries for yourself if you have no success with any other option.

## Documentation
Documentation for the BLT can be found on the [GitHub Wiki](https://github.com/JamesWilko/Payday-2-BLT/wiki) for the project.

Documentation for SuperBLT can be found on the [SuperBLT Website](https://superblt.znix.xyz).

## Contributors
- SuperBLT Team
	* [Campbell Suter](https://znix.xyz)

- Payday 2 BLT Team
	* [James Wilkinson](http://jameswilko.com/) ([Twitter](http://twitter.com/_JamesWilko))
	* [SirWaddlesworth](http://genj.io/)
	* [Will Donohoe](https://will.io/)

- BLT4L Team
	* [Roman Hargrave](https://github.com/RomanHargrave)
	* [Campbell Suter](https://znix.xyz)
	* [Leonard Koenig](https://github.com/LeonardKoenig)

- Contributors, Translators, Testers and more
	* saltisgood
	* Kail
	* Dougley
	* awcjack
	* BangL
	* chromKa
	* xDarkWolf
	* Luffyyy
	* NHellFire
	* TdlQ
	* Mrucux7
	* Simon
	* goontest
	* aayanl
	* cjur3
	* Kilandor
	* Joel Juvél
	* Brskt
	* [Dribbleondo](http://twitter.com/dribbleondo)
	* zekesonxx
	* TRSGuy
	* luck3y
	* Ozymandias
	* BreakinBenny
	* and others who haven't been added yet

## Development

How to contribute to SuperBLT:

SuperBLT uses the CMake build system. This can generate a variety of build files,
including GNU Makefiles and MSVC Project/Solution files, from which the program can
then be built.

SuperBLT depends on two sets of libraries: those commonly available (such as OpenSSL
and ZLib) which you are expected to install via your package manager (GNU+Linux) or
by downloading and compiling (Windows).

It also has libraries that are very small projects, rarely available in package managers,
that come included as git submodules. These are automatically added as targets by CMake,
so you don't need to make any special efforts to use them.

### GNU+Linux

If you are using SELinux, you may require running either `sudo setsebool -P selinuxuser_execheap 1` or `sudo setenforce 0` before
doing anything. This allows the loader to adjust the memory of the PAYDAY 2 executable.

First, clone this repository, and pull all required projects and repositories into one folder (Note: You **NEED** to do this, otherwise you'll get runtime and compile errors):

```
git clone --recursive https://gitlab.com/znixian/payday2-superblt.git
```

Second, create and `cd` into a build directory:

```
mkdir build
cd build
```

Then run CMake:

```
cmake ..
```

Alternatively, if you want debug symbols for GDB, add it as an argument:

```
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

If the previous step told you that you're missing some libraries (which is quite likely), install them, and be
sure to install their development headers too. This can be done via your package manager.

These packages are normally as follows: `libcurl4-openssl-dev`, `libpng-dev`, `libssl-dev`, 
`zlib-dev` (often called `zlib1g-dev` in debian distro's), and `libopenal-dev`. This is distribution-dependant, naturally, and may also require installing the non-dev-header packages too (`libpng-dev` and `libpng` for example).

Next, compile the loader. You can speed up the compilation process by replacing the number
"4" with the amount of threads your CPU has (generally it's twice the core count, so
for example: 2 cores will have 4 threads, 4 cores will have 8 threads, and so on):

```
make -j 4
```

Go into the build folder and copy the resulting `libsuperblt_loader.so` into the root directory
of PAYDAY 2's install folder (the same folder as the binary executable; `payday2_release`).

And finally, add the `LD_PRELOAD` enviornment variable to the Launch Options

```
env LD_PRELOAD="$LD_PRELOAD ./libsuperblt_loader.so" %command%
```

This environment variable will tell the game to look for the SuperBLT loader when you run PAYDAY 2.

Be sure to install the basemod from [GitLab:znixian/payday2-superblt-lua](https://gitlab.com/znixian/payday2-superblt-lua),
as the automatic installer isn't currently implemented on GNU+Linux.

### Windows

Payday2 BLT requires the following dependencies, which are all statically linked.
* zlib
* cURL
* OpenSSL
* OpenAL

#### zLib
zLib should be compiled as static.

If you don't want to stuff with NASM for the assembly module, there is a parameter you
can set to only use C. The performance difference isn't important for something used
on occasion (unpacking mod updates).

#### cURL
cURL should be compiled as static, with the WITH_SSL parameter set to 'static'.

#### OpenSSL
OpenSSL should be compiled as a static library.

#### OpenALSoft
Again, compile OpenALSoft as a static library.

#### Configuration

Set some environment variables, telling CMake where to find the required external libraries.

I'd suggest you put them into a batch file with the CMake build command, to make things
easier when you next have to run CMake.

They are as follows (search for them in `CMakeLists.txt` if you want more information):

```
ZLIB_ROOT
ZLIB_LIBRARY
CURL_INCLUDE_DIR
CURL_LIBRARY
OPENSSL_ROOT_DIR
OPENAL_LIBRARY
OPENAL_INCLUDE_DIR
```

(Anything ending in _LIBRARY should be the filename of a .lib static-linked library)

TODO: Finish writing instructions

### Code Conventions
- Avoid `std::shared_ptr` and the likes unless you have a decent reason to use it. If you
need the reference counting go ahead, but please don't use it when a regular pointer would
work fine.
- Don't **ever** use CRLF.
- Please ensure there is a linefeed (`\n`) as the last byte of any files you create.
- Please use `git patch`, don't commit multiple unrelated or loosely related things in a
single commit. Likewise, please don't commit whitespace-only changes. `git blame` is a valuable
tool.
- Please run the source code though `clang-format` to ensure stuff like brace positions and whitespace
are consistent. Later, this will be put into a Continuous Integration task to mark offending
commits, along with testing stuff like compiling in GCC.
- Please ensure your code doesn't cause any compiler warnings (not counting libraries). This is
enforced for GCC, please watch your output if you're using Visual Studio.
