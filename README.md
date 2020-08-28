# symbios
## Dependencies

### Environment

You can set this environment variable to copy-paste the installation instructions.
You do not have to set it to ~/install, that's just an example.

```bash
mkdir ~/install
export DEP_INSTALL=~/install
export PATH=$DEP_INSTALL/bin:$DEP_INSTALL/include:$DEP_INSTALL/lib:$PATH
export LD_LIBARY_PATH=$DEP_INSTALL/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=$DEP_INSTALL/lib:$LD_LIBRARY_PATH
export INCLUDE=$DEP_INSTALL/include:$INCLUDE
export CPATH=\$DEP_INSTALL/include:\$CPATH
```

NOTE: If PATH, LD_LIBRARY_PATH, LIBRARY_PATH, INCLUDE, or CPATH are undefined,
do not add :$VAR_NAME after the path. Since it's undefined, it will add an
unnecessary : that may break stuff... 

### Cmake

```bash
wget https://github.com/Kitware/CMake/releases/download/v3.15.2/cmake-3.15.2.tar.gz
tar -xzf cmake-3.15.2.tar.gz
cd cmake-3.15.2
./bootstrap --prefix=$DEP_INSTALL
make -j8
make install
```

### MPICH

```bash
wget http://www.mpich.org/static/downloads/3.3.2/mpich-3.3.2.tar.gz
tar -xzf mpich-3.3.2.tar.gz -C $SRC
cd mpich-3.3.2
./configure --prefix=$DEP_INSTALL --enable-fast=03 --enable-shared --enable-romio --enable-threads --disable-fortran --disable-fc
make -j8
make install
```

### RPCLIB

```bash
wget https://github.com/rpclib/rpclib/archive/v2.2.1.tar.gz
tar -xzf v2.2.1.tar.gz
cd rpclib-2.2.1
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=1 -DCMAKE_INSTALL_PREFIX=$DEP_INSTALL ../
make -j8
make install
```

### Boost

```bash
wget https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz
tar -xzf boost_1_74_0.tar.gz
cd boost_1_74_0
./bootstrap.sh --prefix=$DEP_INSTALL
./b2 install
```

### Hireds

A C library for interacting with Redis. Redis-plus-plus depends on it.

```bash
wget https://github.com/redis/hiredis/archive/v1.0.0.tar.gz  
tar -xzf v1.0.0.tar.gz  
cd hiredis*  
make  PREFIX=$DEP_INSTALL
sudo make PREFIX=$DEP_INSTALL install   
```

### Redis-Plus-Plus

A library for interacting with a Redis database in C++, located [here](https://github.com/sewenew/redis-plus-plus)

```bash
wget https://github.com/sewenew/redis-plus-plus/archive/1.1.2.tar.gz      
tar -xzf 1.1.2.tar.gz      
cd redis-plus-plus*    
mkdir build  
cd build  
cmake -DCMAKE_PREFIX_PATH=$DEP_INSTALL -DCMAKE_INSTALL_PREFIX=$DEP_INSTALL -DCMAKE_BUILD_TYPE=Release ../  
make  
sudo make install  
```

### Mongodb C Driver

```bash
wget https://github.com/mongodb/mongo-c-driver/releases/download/1.17.0/mongo-c-driver-1.17.0.tar.gz  
tar -xzf mongo-c-driver-1.17.0.tar.gz    
cd mongo-c-driver-1.17.0  
mkdir build  
cd build  
cmake -DCMAKE_INSTALL_PREFIX=$DEP_INSTALL -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ../  
make -j8  
sudo cmake --build . --target install  
```

### Mongodb C++ Driver

```bash
curl -OL https://github.com/mongodb/mongo-cxx-driver/archive/r3.5.1.tar.gz  
tar -xzf r3.5.1.tar.gz    
cd mongo-cxx-driver-r3.5.1/build  
cmake .. -DCMAKE_PREFIX_PATH=$DEP_INSTALL -DBUILD_VERSION=3.5.1 -DCMAKE_INSTALL_PREFIX=$DEP_INSTALL -DCMAKE_CXX_STANDARD=17 -DBSONCXX_POLY_USE_BOOST=1 -DCMAKE_BUILD_TYPE=Release     
make -j8  
sudo cmake --build . --target install  
```



### RapidJson
### HCL
HCL is dependendat upon:
* glibc
* Boost
* At least one RPC library, either:
    * rpclib
    * Thallium (wrapper over Mercury)

HCL was tested with mpich 3.3.1, boost 1.69.0, rpclib 2.2.1, mercury 1.0.1, and thallium 0.4.0.

All information can be seen on https://bitbucket.org/scs-io/hcl/src/master/

#### Compile and Install
Basic Method:
```bash
cd basket
mkdir build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/install_dir -DBASKET_ENABLE_RPCLIB=true ..
make
make install
```

With git submodules:

Once the repo is cloned, you have to run two commands:
```bash
git submodule init
git submodule update
```
or while clonning the main repo:
```bash
git clone --recurse-submodules repo_url
```
Them, use
```bash
git clone --recurse-submodules repo_url
```

## Compile Symbios
As a CMake project use:
```bash
mkdir build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/wherever -DCOMPILE_HCL=true/false ..
make
make install```