FROM ubuntu:bionic

RUN apt-get update \
&& apt-get install -y ssh build-essential git cmake autoconf libtool pkg-config wget checkinstall zlib1g-dev libssl-dev

ENV SOURCE_DIR=${HOME}/software/source
ENV INSTALL_DIR=${HOME}/software/install
RUN mkdir -p ${INSTALL_DIR}
RUN useradd -m user && yes password | passwd user

ENV PATH=${INSTALL_DIR}/bin:${INSTALL_DIR}/sbin:$PATH
ENV LD_LIBRARY_PATH=${INSTALL_DIR}/lib;${INSTALL_DIR}/lib64
ENV LDFLAGS="$LDFLAGS -L${INSTALL_DIR}/lib -L${INSTALL_DIR}/lib64"
ENV CFLAGS="-I${INSTALL_DIR}/include $CFLAGS"
ENV CXXFLAGS="-I${INSTALL_DIR}/include $CXXFLAGS"
ENV GITHUB_WORKSPACE=$HOME/work/

WORKDIR ${SOURCE_DIR}
#RUN echo "Downloading Modules" \
#&& wget http://www.mpich.org/static/downloads/3.3.2/mpich-3.3.2.tar.gz \
#&& wget https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz \
#&& git clone https://github.com/Tencent/rapidjson \
#&& wget https://github.com/rpclib/rpclib/archive/v2.2.1.tar.gz \
#&& git clone https://bitbucket.org/scs-io/hcl \
#&& wget https://github.com/redis/hiredis/archive/v1.0.0.tar.gz \
#&& wget https://github.com/sewenew/redis-plus-plus/archive/1.1.2.tar.gz \
#&& wget https://github.com/mongodb/mongo-c-driver/releases/download/1.17.0/mongo-c-driver-1.17.0.tar.gz \
#&& wget https://github.com/mongodb/mongo-cxx-driver/archive/r3.5.1.tar.gz \
#&& tar zxf mpich-3.3.2.tar.gz \
#&& tar zxf boost_1_74_0.tar.gz \
#&& tar xvf v2.2.1.tar.gz \
#&& tar -xzf v1.0.0.tar.gz \
#&& tar -xzf 1.1.2.tar.gz \
#&& tar -xzf mongo-c-driver-1.17.0.tar.gz \
#&& tar -xzf r3.5.1.tar.gz \
#&& rm *.tar.gz
#
###################### MPI ######################
#WORKDIR  ${SOURCE_DIR}/mpich-3.3.2/
#RUN echo "Installing MPI" \
#&& ./configure -prefix=${INSTALL_DIR} --disable-fortran \
#&& make -j 4 \
#&& make install
#
###################### BOOST ######################
#WORKDIR  ${SOURCE_DIR}/boost_1_74_0/
#RUN echo "Installing Boost" \
#&& ./bootstrap.sh --prefix=${INSTALL_DIR} \
#&& ./b2 -j4 install
#
####################### RapidJson ######################
#WORKDIR  ${SOURCE_DIR}/rapidjson/build
#RUN echo "Installing RapidJson" \
#&& cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} ../ \
#&& make -j 4 \
#&& make install
#
####################### RpcLib ######################
#WORKDIR  ${SOURCE_DIR}/rpclib-2.2.1/build
#RUN echo "Installing RpcLib" \
#&& sed -i 's/add_library(${PROJECT_NAME}/add_library(${PROJECT_NAME} SHARED/g' ../CMakeLists.txt \
#&& cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} ../ \
#&& make -j 4 \
#&& make install
#
####################### HCL ######################
#WORKDIR  ${SOURCE_DIR}/hcl
#RUN echo "Installing HCL" \
#&& git checkout -b release/0.0.4 \
#&& mkdir -p build \
#&& cd build \
#&& cmake -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_DIR} -DBASKET_ENABLE_RPCLIB=true .. \
#&& make -j 4 \
#&& make install
#
###################### Redis Clients ######################
### Hireds ###
#WORKDIR  ${SOURCE_DIR}/hiredis-1.0.0
#RUN echo "Installing Hireds" \
#&& make \
#&& sed -i 's&INSTALL_INCLUDE_PATH= $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)&INSTALL_INCLUDE_PATH=$(PREFIX)/include/hiredis&g' ./Makefile \
#&& sed -i 's&INSTALL_LIBRARY_PATH= $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)&INSTALL_LIBRARY_PATH=$(PREFIX)/lib&g' ./Makefile \
#&& sed -i 's&INSTALL_PKGCONF_PATH= $(INSTALL_LIBRARY_PATH)/$(PKGCONF_PATH)&INSTALL_PKGCONF_PATH=$(INSTALL_LIBRARY_PATH)/pkgconfig&g' ./Makefile \
#&& make PREFIX=${INSTALL_DIR} install
#### Redis-Plus-Plus ###
#WORKDIR  ${SOURCE_DIR}/redis-plus-plus-1.1.2/build
#RUN echo "Installing Redis-Plus-Plus" \
#&& cmake -DREDIS_PLUS_PLUS_CXX_STANDARD=17 -DCMAKE_PREFIX_PATH=${INSTALL_DIR} -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_BUILD_TYPE=Release ../ \
#&& make \
#&& make install
#
###################### Mongo Clients ######################
### Mongodb C Driver ###
#WORKDIR  ${SOURCE_DIR}/mongo-c-driver-1.17.0/build
#RUN echo "Installing Mongodb C Driver" \
#&& cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ../ \
#&& make -j 4 \
#&& cmake --build . --target install
#### Mongodb C++ Driver ###
#WORKDIR  ${SOURCE_DIR}/mongo-cxx-driver-r3.5.1/build
#RUN echo "Installing Mongodb C++ Driver" \
#&& cmake .. -DCMAKE_PREFIX_PATH=${INSTALL_DIR} -DBUILD_VERSION=3.5.1 -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_CXX_STANDARD=17 -DBSONCXX_POLY_USE_BOOST=1 -DCMAKE_BUILD_TYPE=Release \
#&& make -j 4 \
#&& cmake --build . --target install