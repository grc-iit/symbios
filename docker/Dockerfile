FROM jcernuda/symbios:1.0.3

RUN apt-get update \
  && apt-get install -y ssh \
    build-essential \
    gcc \
    g++ \
    gdb \
    clang \
    cmake \
    rsync \
    tar \
    python \
  && apt-get clean

USER user
RUN export SOURCE_DIR=/software/source \
&& export INSTALL_DIR=/software/install \
&& export PATH=${INSTALL_DIR}/bin:${INSTALL_DIR}/sbin:$PATH \
&& export LD_LIBRARY_PATH=${INSTALL_DIR}/lib:${INSTALL_DIR}/lib64 \
&& export LDFLAGS="$LDFLAGS -L${INSTALL_DIR}/lib -L${INSTALL_DIR}/lib64" \
&& export CFLAGS="-I${INSTALL_DIR}/include -I${INSTALL_DIR}/include/mongocxx/v_noabi -I${INSTALL_DIR}/include/bsoncxx/v_noabi $CFLAGS" \
&& export CXXFLAGS="-I${INSTALL_DIR}/include -I${INSTALL_DIR}/include/mongocxx/v_noabi -I${INSTALL_DIR}/include/bsoncxx/v_noabi $CXXFLAGS"

USER root
ENTRYPOINT service ssh restart && bash