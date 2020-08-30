FROM ubuntu:bionic

RUN apt-get update \
&& apt-get install -y build-essential git cmake autoconf libtool pkg-config wget

ENV SOURCE_DIR=/root/software/source
ENV INSTALL_DIR=/root/software/install

ENV PATH=${INSTALL_DIR}/bin:${INSTALL_DIR}/sbin:$PATH
ENV LD_LIBRARY_PATH=${INSTALL_DIR}/lib;${INSTALL_DIR}/lib64 

#RUN mkdir -p ${SOURCE_DIR}
#RUN mkdir -p ${INSTALL_DIR}

WORKDIR ${SOURCE_DIR}
RUN echo "Downloading Modules" \ 
&& wget http://www.mpich.org/static/downloads/3.3.2/mpich-3.3.2.tar.gz \
&& wget https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz \
&& tar zxf mpich-3.3.2.tar.gz \
&& tar zxf boost_1_74_0.tar.gz

WORKDIR  ${SOURCE_DIR}/mpich-3.3.2/
RUN echo "Installing MPI" \
&& ./configure -prefix=${INSTALL_DIR} --disable-fortran \
&& make -j 4 \
&& make install

WORKDIR  ${SOURCE_DIR}/boost_1_74_0/
RUN echo "Installing Boost" \
&& ./bootstrap.sh --prefix=${INSTALL_DIR} \
&& ./b2 install

WORKDIR /root/git/
