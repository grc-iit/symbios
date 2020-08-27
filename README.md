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
make install```
