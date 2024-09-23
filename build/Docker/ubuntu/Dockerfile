#
# TMX Ubuntu Build image Dockerfile
#
ARG distro=latest
FROM ubuntu:$distro

LABEL author="baumgardner@battelle.org"
LABEL description="Transportation Message eXchange build image"
LABEL version="1.0"

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=America/New_York

# Install base development tools
RUN apt-get update
RUN apt-get install -y git gcc g++ gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf libboost-all-dev icu-devtools openssl libxerces-c-dev libcurl4-openssl-dev libsnmp-dev libmysqlclient-dev libmysqlcppconn-dev uuid-dev libz3-dev libgps-dev libev-dev libuv1-dev libusb-1.0-0-dev liboctave-dev libbluetooth-dev openjdk-8-jre-headless lsb-release aptly

# Need to install latest version of cmake
RUN apt-get install -y wget gpg
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg
RUN . /etc/os-release && echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $UBUNTU_CODENAME main" | tee /etc/apt/sources.list.d/kitware.list
RUN apt-get update
RUN apt-get install -y cmake

# Compile and install the latest ASN1C compiler
RUN git clone https://github.com/vlm/asn1c.git
RUN cd asn1c && (test -f configure || autoreconf -iv)
RUN cd asn1c && ./configure
RUN cd asn1c && make install

RUN git clone https://github.com/terminatorul/asn1c-cmake-module.git
RUN cp asn1c-cmake-module/*.cmake /usr/share/cmake-*/Modules

RUN . /etc/os-release && \
        echo "${NAME} ${VERSION} build image created."
