FROM jcernuda/symbios:1.0.2

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
ENTRYPOINT service ssh restart && bash