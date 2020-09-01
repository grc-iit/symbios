# symbios
## Dependencies

### Environment

You can set this environment variable to copy-paste the installation instructions.
You do not have to set it to ~/install, that's just an example.

```bash
mkdir ~/install
export DEP_INSTALL=~/install
export INSTALL_DIR=$DEP_INSTALL
export PATH=${DEP_INSTALL}/bin:$DEP_INSTALL/include:$DEP_INSTALL/lib:$PATH
export LD_LIBRARY_PATH=${DEP_INSTALL}/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=${DEP_INSTALL}/lib:$LIBRARY_PATH
export INCLUDE_PATH=${DEP_INSTALL}/include:$INCLUDE
export CPATH=${DEP_INSTALL}/include:$CPATH
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
tar -xzf mpich-3.3.2.tar.gz
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

### RapidJson
```bash
git clone https://github.com/Tencent/rapidjson
cd rapidjson
git checkout v1.1.0 -b v1.1.0
mkdir build
cd build
cmake .. -DCMAKE_CXX_FLAGS:STRING="-Wno-error=class-memaccess -Wno-error=implicit-fallthrough="  -DCMAKE_INSTALL_PREFIX=$DEP_INSTALL -DINCLUDE_INSTALL_DIR=$DEP_INSTALL/include -DLIB_INSTALL_DIR=$DEP_INSTALL/lib -DCMAKE_INSTALL_DIR=$DEP_INSTALL/cmake -DDOC_INSTALL_DIR=$DEP_INSTALL/share/doc/RapidJSON
make
make -j8
make install
```

### HCL
HCL is dependendat upon:
* glibc
* Boost
* At least one RPC library, either:
    * rpclib
    * Thallium (wrapper over Mercury)

HCL was tested with mpich 3.3.1, boost 1.69.0, rpclib 2.2.1, mercury 1.0.1, and thallium 0.4.0.

All information can be seen on https://bitbucket.org/scs-io/hcl/src/master/


### Redis Server
```bash
wget https://github.com/redis/redis/archive/6.0.6.tar.gz
tar -xzf 6.0.6.tar.gz
cd redis-6.0.6
make PREFIX=$DEP_INSTALL install
```

#### Hireds

A C library for interacting with Redis. Redis-plus-plus depends on it.
```bash
wget https://github.com/redis/hiredis/archive/v1.0.0.tar.gz  
tar -xzf v1.0.0.tar.gz  
cd hiredis*  
make  
sed -i 's&INSTALL_INCLUDE_PATH= $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)&INSTALL_INCLUDE_PATH=$(PREFIX)/include/hiredis&g' ./Makefile
sed -i 's&INSTALL_LIBRARY_PATH= $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)&INSTALL_LIBRARY_PATH=$(PREFIX)/lib&g' ./Makefile
sed -i 's&INSTALL_PKGCONF_PATH= $(INSTALL_LIBRARY_PATH)/$(PKGCONF_PATH)&INSTALL_PKGCONF_PATH=$(INSTALL_LIBRARY_PATH)/pkgconfig&g' ./Makefile
make PREFIX=$DEP_INSTALL install   
```

#### Redis-Plus-Plus

A library for interacting with a Redis database in C++, located [here](https://github.com/sewenew/redis-plus-plus)

```bash
wget https://github.com/sewenew/redis-plus-plus/archive/1.1.2.tar.gz      
tar -xzf 1.1.2.tar.gz      
cd redis-plus-plus*    
mkdir build  
cd build  
cmake -DREDIS_PLUS_PLUS_CXX_STANDARD=17 -DCMAKE_PREFIX_PATH=$DEP_INSTALL -DCMAKE_INSTALL_PREFIX=$DEP_INSTALL -DCMAKE_BUILD_TYPE=Release ../  
make
make install  
```
If the hiredis and redis-plus-plus are not colocated, then set 
DCMAKE_PREFIX_PATH to be the path to where hiredis is installed.

#### Redis Cluster Local Script Usage
```bash
cd scripts/local
./run_redis_cluster.sh redis_cluster_config_path redis_server_numbers redis_cluster_install_path
```
Manage local redis using the scripts in scripts/deploymnet. 
redis_r.sh and redis_s.sh will run and stop redis respectively.


### MongoDB
MongoDB installation is based on SCons a python module and therefore python3 must be installed.
#### Python
Python 3 is a prerequirement of Mongodb, to install from source:
```bash
wget --no-check-certificate https://www.python.org/ftp/python/3.7.9/Python-3.7.9.tgz
xz -d Python-3.7.9.tgz
tar -xzvf Python-3.7.9.tgz
cd Python-3.7.9/
./configure --prefix=$DEP_INSTALL --enable-optimizations
make && make install

curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
python get-pip.py
```

To install the actual MongoDB:
```bash
wget https://github.com/mongodb/mongo/archive/r4.4.0.tar.gz
tar -xzf r4.4.0.tar.gz
cd mongo-r4.4.0/
python3 -m pip install -r etc/pip/compile-requirements.txt
python3 buildscripts/scons.py DESTDIR=$DEP_INSTALL install-mongod
```

According to the documentation, there might be a need to use:
```bash
python3 buildscripts/scons.py install-mongod --disable-warnings-as-errors
```
Manage local mongodb using the scripts in scripts/deploymnet. 
mongodb_r.sh and mongodb_s.sh will run and stop redis respectively.

#### Mongodb C Driver

```bash
wget https://github.com/mongodb/mongo-c-driver/releases/download/1.17.0/mongo-c-driver-1.17.0.tar.gz  
tar -xzf mongo-c-driver-1.17.0.tar.gz    
cd mongo-c-driver-1.17.0/build 
cmake -DCMAKE_INSTALL_PREFIX=$DEP_INSTALL -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ../  
make -j8  
cmake --build . --target install  
```

#### Mongodb C++ Driver

```bash
sudo apt-get install libssl-dev
curl -OL https://github.com/mongodb/mongo-cxx-driver/archive/r3.5.1.tar.gz  
tar -xzf r3.5.1.tar.gz    
cd mongo-cxx-driver-r3.5.1/build  
cmake .. -DCMAKE_PREFIX_PATH=$DEP_INSTALL -DBUILD_VERSION=3.5.1 -DCMAKE_INSTALL_PREFIX=$DEP_INSTALL -DCMAKE_CXX_STANDARD=17 -DBSONCXX_POLY_USE_BOOST=1 -DCMAKE_BUILD_TYPE=Release     
make -j8  
cmake --build . --target install  
```

If the C and C++ driver aren't being installed to the same place, then
change -DCMAKE_PREFIX_PATH to whatever directory the mongo C driver is....

For reasons unbeknownst to me (Jaime) you need to add the following:
```bash
export CFLAGS="-I${DEP_INSTALL}/include/mongocxx/v_noabi -I${DEP_INSTALL}/include/bsoncxx/v_noabi $CFLAGS"
export CXXFLAGS="-I/${DEP_INSTALL}/include/mongocxx/v_noabi -I${DEP_INSTALL}/include/bsoncxx/v_noabi $CXXFLAGS"
```

## Compile Symbios
As a CMake project use:
```bash
mkdir build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/wherever -DCOMPILE_HCL=true/false ..
make
make install
```

## Storage Interface Usage
A simple example to show the usage of Storage Interface:
```c++
IOFactory io_factory;
std::shared_ptr<IOClient> io_client = io_factory.GetIOClient(FILE_IO);
/*
* Write data
* 1) For FILE_IO type, the "source_data" indicates the memory info, the "destination_data" indicates the file info.
* 2) For REDIS_IO/MONGO_IO type, the "source_data" indicates the memory info, and the "destination_data" indicates
* the storage info.
*    You must set the following information in "source_data" and "destination_data". Here I didn't do that.
*    A. Setting the memory buffer, start position and data_size in the "source_data"
*    B. Setting the destination filename/key, start write position in the "detination_data"
*/
Data source_data;
Data destination_data;
io_client.Write(source_data, destination_data);

/*
* Read data
* 1) For FILE_IO type, the "source_data" indicates the file info, the "destination_data" indicates the memory info.
* 2) For REDIS_IO/MONGO_IO type, the "source_data" indicates the storage info, and the "destination_data" indicates 
* the memory info.
*    You must set the following information in "source_data" and "destination_data". Here I didn't do that.
*    A. Setting the filename/key, start read position in the "source_data"
*    B. Setting the memory buffer, start write position and data size in the "source_data"
*/
Data source_data_;
Data destination_data_;
io_client.Read(source_data_, destination_data_);
```

## Mongo Local Script Usage
```bash
cd scripts/local
./run_mongo.sh mongo_config_path mongo_install_path
```

## Local testing of Symbios

- update conf/symbios.conf to correct the path of SERVER_LISTS, CLIENT_LISTS, and SERVER_DIR

### start server in one terminal (or clion)
```bash
/tmp/tmp.BUKlhPiLxF/build/symbios_server /tmp/tmp.BUKlhPiLxF/conf/symbios.conf
```

### start client in another terminal (or clion)
```bash
/tmp/tmp.BUKlhPiLxF/build/test/unit/unit_client /home/hdevarajan/symbios.conf
```

### run symbios-server as service
NOTE: ensure paths are correct with the service file.
```bash
sudo cp scripts/service/symbios.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl start symbios.service
systemctl status symbios.service

```

NOTE:
- you might need to set the LD_LIBRARY_PATH of all dependencies.
