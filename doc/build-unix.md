UNIX BUILD NOTES
====================
Some notes on how to build BTCU in Unix.

To Build
---------------------

Before you start building, please make sure that your compiler supports C++11.

It is recommended to create a build directory to build out-of-tree.

```bash
    mkdir build
    cd build
    cmake ..
    make
```

This will build btcu-qt as well.

Dependencies
---------------------

These dependencies are required:

 Library     | Purpose          | Description
 ------------|------------------|----------------------
 libssl      | Crypto           | Random Number Generation, Elliptic Curve Cryptography
 libboost    | Utility          | Library for threading, data structures, etc
 libevent    | Networking       | OS independent asynchronous networking
  libgmp      | Bignum Arithmetic  | Precision arithmetic

Optional dependencies:

 Library     | Purpose          | Description
 ------------|------------------|----------------------
 miniupnpc   | UPnP Support     | Firewall-jumping support
 libdb18.1       | Berkeley DB      | Wallet storage (only needed when wallet enabled)
 qt          | GUI              | GUI toolkit (only needed when GUI enabled)
 protobuf    | Payments in GUI  | Data interchange format used for payment protocol (only needed when BIP70 enabled)
 libqrencode | QR codes in GUI  | Optional for generating QR codes (only needed when GUI enabled)
 univalue    | Utility          | JSON parsing and encoding (bundled version will be used unless --with-system-univalue passed to configure)
 libzmq3     | ZMQ notification | Optional, allows generating ZMQ notifications (requires ZMQ version >= 4.0.0)

For the versions used, see [dependencies.md](dependencies.md)

Memory Requirements
--------------------

C++ compilers are memory-hungry. It is recommended to have at least 1.5 GB of
memory available when compiling BTCU. On systems with less, gcc can be
tuned to conserve memory with additional CXXFLAGS:

```bash
    cmake .. -DCXXFLAGS="--param ggc-min-expand=1 --param ggc-min-heapsize=32768"
```

Dependency Build Instructions: Ubuntu & Debian
----------------------------------------------
Build requirements:

```bash
    sudo apt-get install git build-essential libtool bsdmainutils autotools-dev autoconf pkg-config automake python3-dev libzmq3-dev libevent-dev libjsonrpccpp-dev libsnappy-dev libbenchmark-dev libnorm-dev libpgm-dev
```

GCC version 9.0:
```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test &&
sudo apt update &&
sudo apt install gcc-9 g++-9 &&
sudo ln -f -s /usr/bin/gcc-9 /usr/bin/gcc &&
sudo ln -f -s /usr/bin/g++-9 /usr/bin/g++
```

**Note:** In order to build it on Ubuntu 20.04 you may need to instal additionally:
```bash
    sudo apt-get install python-is-python3
```

Additionally if you need to perform a static build (-DSTATIC_BUILD=ON) you will have to build a static version of libnorm from the sources by the followed commands:
```bash
    git clone --recurse-submodules https://github.com/USNavalResearchLaboratory/norm.git

    cd norm
    ./waf configure --prefix=/usr --enable-static-library && ./waf install
    cd -
```

It is required if you intend to use ZeroMQ.

Installing GTest:
```bash
    sudo apt-get install libgtest-dev
    cd /usr/src/googletest
    sudo cmake . && sudo cmake --build . --target install && cd -
```

## Linux Distribution Specific Instructions

### Ubuntu  & Others

#### Dependency Build Instructions (script)

You can build the BTCU project from scratch by using a special bash script:

```bash
    mkdir btcu
    cd btcu
    wget https://raw.githubusercontent.com/bitcoin-ultimatum/btcu/master/install_ubuntu.sh
    sudo bash install_ubuntu.sh
```

**Note:** The script install_ubuntu.sh has a two possible options to pass.
You can get the description by adding the flag `-h`. Thus you'll get:
```bash
    Usage: ./install_ubuntu.sh [-u|-s]
        -u Update the solution instead of cloning the BTCU repository
        -s Build a static type build (additionally will build from the sources: libnorm, libzmq3 and qt5)
```

#### Dependency Build Instructions (manual)

1. Build requirements:

```bash
    sudo apt-get install git build-essential libtool bsdmainutils autotools-dev autoconf pkg-config automake python3 libzmq3-dev libevent-dev libjsonrpccpp-dev libsnappy-dev libbenchmark-dev libgtest-dev 
```

(Optional) If you need to build in a static mode (-DSTATIC_BUILD=ON as a cmake parameter) you will have to install:
```bash
    sudo apt-get install liblz4-dev libzstd-dev libbz2-dev libsodium-dev
```

2. Cmake installing:

On Debian Buster (10), `cmake` should be installed from the backports repository:

```bash
    echo "deb http://deb.debian.org/debian buster-backports main" | sudo tee -a /etc/apt/sources.list
    sudo apt-get update
    sudo apt-get -t buster-backports install cmake
```

On Ubuntu 20.04 and later:

```bash
    sudo apt-get install cmake
```

On previous Ubuntu versions, the `cmake` package is too old and needs to be installed from the Kitware APT repository:

```bash
    sudo apt-get install apt-transport-https ca-certificates gnupg software-properties-common wget
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
```

Add the repository corresponding to your version (see [instructions from Kitware](https://apt.kitware.com)). For Ubuntu Bionic (18.04):

```bash

    sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
```

Then update the package list and install `cmake`:

```bash
    sudo apt update
    sudo apt install cmake
```

Also you can build cmake by yourself:
```bash
    version=3.14
    build=1
    wget https://cmake.org/files/v$version/cmake-$version.$build.tar.gz
    tar -xzvf cmake-$version.$build.tar.gz
    cd cmake-$version.$build
    ./bootstrap --prefix=/usr && make -j$(nproc) && sudo make install
    cd -
```

Options when installing required Boost library files:

On at least Ubuntu 16.04+ and Debian 9+ there are generic names for the
individual boost development packages, so the following can be used to only
install necessary parts of boost:

```bash
        sudo apt-get install libboost-system-dev libboost-filesystem-dev libboost-test-dev libboost-thread-dev
```

If that doesn't work, you can install all boost development packages with:

```bash
        sudo apt-get install libboost-all-dev
```

Please make sure you have installed Boost 1.71.0.

Alternativetely you can build Boost from a source code:
```bash
        wget https://boostorg.jfrog.io/artifactory/main/release/1.71.0/source/boost_1_71_0.tar.gz
        tar -xf boost_1_71_0.tar.gz

        cd boost_1_71_0
        ./bootstrap.sh --prefix=/usr --with-python=python3 &&
        sudo ./b2 stage -j$(nproc) threading=multi link=static,shared --with-regex --with-test --with-filesystem --with-date_time --with-random --with-system --with-thread --with-program_options --with-chrono --with-fiber --with-log --with-context --with-math && sudo ./b2 install --prefix=/usr
        cd -
```

3. For Ethereum VM smart contracts:

```bash
    sudo apt-get install librocksdb-dev
```

4. Now, you can either build from self-compiled [depends](/depends/README.md) or install the required dependencies:

```bash

    sudo apt-get install libssl-dev libgmp-dev libevent-dev libboost-all-dev
```

5. BerkeleyDB 18.1.32 is required for the wallet. 
If exists previous version like 4.8 (Bitcoin default) then remove:

```bash
    sudo apt-get purge libdb4.8-dev libdb4.8++-dev
```

Since as of 5th March 2020 the Oracle moved Barkeley DB to login-protected tarball for 18.1.32 version we added the dependency as a static file included in the repository.

Install:

```bash
        tar zxvf depends/packages/static/berkeley-db-18.1.32/berkeley-db-18.1.32.tar.gz -C ./
        cd  db-18.1.32/build_unix
        ../dist/configure --enable-cxx --disable-shared --disable-replication --with-pic --prefix=/opt
        make
        sudo make install
        cd -
```

See the section "Disable-wallet mode" to build BTCU without wallet.

6. Minipupnc dependencies (can be disabled by passing `-DWITH_MINIUNPC=OFF` on the cmake command line):

```bash
    sudo apt-get install libminiupnpc-dev miniupnpd
```

7. ZMQ dependencies (provides ZMQ API, can be disabled by passing `-DENABLE_ZMQ=OFF` on the cmake command line):

```bash
    sudo apt-get install libzmq3-dev
```

Dependencies for the GUI: Ubuntu & Debian
-----------------------------------------

If you want to build btcu-qt, make sure that the required packages for Qt development
are installed. Qt 5 is necessary to build the GUI.
To build without GUI pass `-DENABLE_GUI=OFF` on the cmake command line.

(Optional) If you need to perform a static build you will have to build a static version of the libgraphite2 as well:
```bash
    git clone https://github.com/silnrsi/graphite
    cd graphite
    cmake . -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX:PATH=/usr && sudo cmake --build . --target install --config Release
    cd -
```

To build with Qt 5 you need the following:

```bash
    sudo apt-get install libprotobuf-dev protobuf-compiler libpng-dev
```

Modern Unix .deb packages doesn't include static libraries. If you wont build with -DBUILD_STATIC=OFF you can just run:
```bash
    sudo apt-get install qt5-default libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libqt5svg5-dev libqt5charts5-dev
```

Alternatively in a case if you want to build a static build you will have to build the QT5 from the sources:
```bash
    sudo apt-get install libglu1-mesa-dev libx11-dev libxcb-glx0-dev libdrm-dev libx11-xcb-dev libxcb-icccm4-dev libxcb-image0-dev libxcb-shm0-dev libxcb-util-dev libxcb-keysyms1-dev libxcb-randr0-dev libxcb-render-util0-dev libxcb-render0-dev libxcb-shape0-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-xinerama0-dev libxcb-xkb-dev libxcb1-dev libxkbcommon-x11-dev libegl1-mesa-dev libwayland-dev libxkbcommon-dev libxext-dev libxau-dev libxdmcp-dev libffi-dev libgraphite2-dev libpcre3-dev uuid-dev libxi-dev libxrender-dev
    sudo apt-get install libfontconfig1-dev libfreetype6-dev
    wget https://download.qt.io/archive/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz
    tar xvf qt-everywhere-src-5.15.2.tar.xz -C ./
    cd qt-everywhere-src-5.15.2

    sudo mkdir /opt/qt5
    export QT5PREFIX=/opt/qt5

    ./configure -prefix $QT5PREFIX                        \
                -sysconfdir /etc/xdg                      \
                -confirm-license                          \
                -opensource                               \
                -openssl-linked                           \
                -nomake examples                          \
                -nomake tests                             \
                -no-rpath                                 \
                -system-zlib                              \
                -static                                   \
                -xcb                                      \
                -xcb-xlib                                 \
                -bundled-xcb-xinput                       \
                -system-freetype                          \
                -fontconfig                               \
                -skip qtwebengine                         \
                -I "/usr/include/freetype2"               \
                -I "/usr/include/fontconfig"              \
                -L "/usr/lib/x86_64-linux-gnu"            &&
    make &&
    sudo make install
    sudo find $QT5PREFIX/ -name \*.prl \
    -exec sed -i -e '/^QMAKE_PRL_BUILD_DIR/d' {} \;
    cd -
```

You can change the build parameters according to your system configuration. Please read more about the parameters [here](https://doc.qt.io/qt-5/configure-options.html).

If you need just a shared libraries build you can install libqrencode with just an apt-get command:
```bash
    sudo apt-get install libqrencode-dev
```

But if you need to build a static build (with a flag -DSTATIC_BUILD=ON) you will have to compile it from the sources:
```bash
    git clone https://github.com/fukuchi/libqrencode.git
    cd libqrencode
    ./autogen.sh && ./configure --prefix=/usr --enable-static --enable-shared && make && sudo make install
    cd -
```


**Note:** Ubuntu versions prior to Bionic (18.04), and Debian version prior to Buster, do not have the `libqt5charts5-dev` package. If you are compiling on one of these older versions, you will need to omit `libqt5charts5-dev` from the above command.

Once these are installed, they will be found by configure and a btcu-qt executable will be
built by default.



Dependency Build Instructions: Fedora
-------------------------------------
Build requirements:

```bash
    sudo dnf install which gcc-c++ libtool make autoconf automake compat-openssl10-devel libevent-devel boost-devel cmake openssl-devel gmp-devel python3
```

Minipupnc dependencies (can be disabled by passing `-DENABLE_ZMQ=OFF` on the cmake command line):

```bash
    sudo dnf install miniupnpc-devel
```

ZMQ dependencies (can be disabled by passing `-DENABLE_ZMQ=OFF` on the cmake command line):

```bash
    sudo dnf install zeromq-devel
```

To build with Qt 5 you need the following:

```bash
    sudo dnf install qt5-qttools-devel qt5-qtbase-devel qt5-qtsvg-devel qt5-qtcharts-devel protobuf-devel qrencode-devel libpng-dev
```

libqrencode dependencies:

```bash
    sudo dnf install qrencode-devel
```

Notes
-----
The release is built with GCC and then "strip btcud" to strip the debug
symbols, which reduces the executable size by about 90%.


miniupnpc
---------

[miniupnpc](https://miniupnp.tuxfamily.org) may be used for UPnP port mapping.  It can be downloaded from [here](
https://miniupnp.tuxfamily.org/files/).  UPnP support is compiled in and
turned off by default.  See the cmake options for upnp behavior desired:

    ENABLE_UPNP            Enable UPnP support (miniupnp required, default ON)
    START_WITH_UPNP        UPnP support turned on by default at runtime (default OFF)

Berkeley DB
-----------
It is recommended to use Berkeley DB 18.1.32. If you have to build it yourself,
use commands:

```bash
        tar zxvf depends/packages/static/berkeley-db-18.1.32/berkeley-db-18.1.32.tar.gz -C ./
        cd  db-18.1.32/build_unix
        ../dist/configure --enable-cxx --disable-shared --disable-replication --with-pic --prefix=/opt
        make
        sudo make install
        cd -
```

from the root of the repository.

**Note**: You only need Berkeley DB if the wallet is enabled (see [*Disable-wallet mode*](/doc/build-unix.md#disable-wallet-mode)).
Boost
-----
For documentation on building Boost look at their official documentation: http://www.boost.org/build/doc/html/bbv2/installation.html

Security
--------
To help make your BTCU installation more secure by making certain attacks impossible to
exploit even if a vulnerability is found, binaries are hardened by default.
This can be disabled by passing `-DDISABLE_HARDENING=ON`.

Hardening enables the following features:
* _Position Independent Executable_: Build position independent code to take advantage of Address Space Layout Randomization
    offered by some kernels. Attackers who can cause execution of code at an arbitrary memory
    location are thwarted if they don't know where anything useful is located.
    The stack and heap are randomly located by default, but this allows the code section to be
    randomly located as well.

    On an AMD64 processor where a library was not compiled with -fPIC, this will cause an error
    such as: "relocation R_X86_64_32 against `......' can not be used when making a shared object;"

    To test that you have built PIE executable, install scanelf, part of paxutils, and use:

```bash
      scanelf -e ./btcud
```
    The output should contain:

      TYPE
      ET_DYN

* _Non-executable Stack_: If the stack is executable then trivial stack-based buffer overflow exploits are possible if
    vulnerable buffers are found. By default, BTCU should be built with a non-executable stack,
    but if one of the libraries it uses asks for an executable stack or someone makes a mistake
    and uses a compiler extension which requires an executable stack, it will silently build an
    executable without the non-executable stack protection.

    To verify that the stack is non-executable after compiling use:

```bash
      scanelf -e ./btcud
```
    The output should contain:

      STK/REL/PTL
      RW- R-- RW-

    The `STK RW-` means that the stack is readable and writeable but not executable.

Disable-wallet mode
--------------------
When the intention is to run only a P2P node without a wallet, BTCU may be compiled in
disable-wallet mode by passing `-DENABLE_WALLET=OFF` on the cmake command line.
In this case there is no dependency on Berkeley DB 18.1.

Mining is also possible in disable-wallet mode using the `getblocktemplate` RPC call.

**Note:** This functionality is not yet completely implemented, and compilation using the below option will currently fail.


Additional cmake options
--------------------------
A list of the cmake options and their current value can be displayed.
From the build subdirectory (see above), run `cmake -LH ..`.

Setup and Build Example: Arch Linux
-----------------------------------
This example lists the steps necessary to setup and build a command line only,
non-wallet distribution of the latest changes on Arch Linux:

```bash
    pacman -S base-devel boost cmake git libevent ninja python
    git clone https://github.com/btcu-ultimatum/btcu --recursive
    cd btcu/
    mkdir build
    cd build
    cmake -DENABLE_WALLET=OFF -DENABLE_GUI=OFF -DENABLE_UPNP=OFF -DENABLE_ZMQ=OFF
    make
```

ARM Cross-compilation
-------------------
These steps can be performed on, for example, a Debian VM. The depends system
will also work on other Linux distributions, however the commands for
installing the toolchain will be different.

Make sure you install all the build requirements mentioned above.
Then, install the toolchain and some additional dependencies:

```bash
    sudo apt-get install autoconf automake curl g++-arm-linux-gnueabihf gcc-arm-linux-gnueabihf gperf pkg-config
```

To build executables for ARM:

```bash
    cd depends
    make build-linux-arm
    cd ..
    mkdir build
    cd build
    cmake .. -DENABLE_GLIBC_BACK_COMPAT=ON
    make
```


For further documentation on the depends system see [README.md](../depends/README.md) in the depends directory.
