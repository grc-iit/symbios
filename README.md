# symbios
# Dependencies

## Redis

### Hireds

A C library for interacting with Redis. Redis-plus-plus depends on it.

```bash
wget https://github.com/redis/hiredis/archive/v1.0.0.tar.gz  
tar -xvzf v1.0.0.tar.gz  
cd hiredis*  
make  PREFIX=/path/to/hiredis/install  
sudo make PREFIX=/path/to/hiredis/install install   
```

### Redis-Plus-Plus

A library for interacting with a Redis database in C++, located [here](https://github.com/sewenew/redis-plus-plus)

```bash
wget https://github.com/sewenew/redis-plus-plus/archive/1.1.2.tar.gz      
tar -xvzf 1.1.2.tar.gz      
cd redis-plus-plus*    
mkdir build  
cd build  
cmake -DCMAKE_PREFIX_PATH=/path/to/hiredis -DCMAKE_INSTALL_PREFIX=/path/to/install/redis-plus-plus -DCMAKE_BUILD_TYPE=Release ../  
make  
sudo make install  
```
## MongoDB

### Mongodb C Driver

```bash
wget https://github.com/mongodb/mongo-c-driver/releases/download/1.17.0/mongo-c-driver-1.17.0.tar.gz  
tar -xvzf mongo-c-driver-1.17.0.tar.gz    
cd mongo-c-driver-1.17.0  
mkdir build  
cd build  
cmake -DCMAKE_INSTALL_PREFIX="/path/to/install/mongo-C-driver" -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ../  
make -j8  
sudo cmake --build . --target install  
```

### Mongodb C++ Driver

```bash
curl -OL https://github.com/mongodb/mongo-cxx-driver/archive/r3.5.1.tar.gz  
tar -xzf r3.5.1.tar.gz    
cd mongo-cxx-driver-r3.5.1/build  
cmake .. -DCMAKE_PREFIX_PATH="/path/to/mongo-C-driver" -DBUILD_VERSION=3.5.1 -DCMAKE_INSTALL_PREFIX="/path/to/install/mongo-C++-driver" -DCMAKE_CXX_STANDARD=17 -DBSONCXX_POLY_USE_BOOST=1 -DCMAKE_BUILD_TYPE=Release     
make -j8  
sudo cmake --build . --target install  
```

## Dependencies
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