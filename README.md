# symbios


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
