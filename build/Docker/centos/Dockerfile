#
# TMX CentOS Build image Dockerfile
#
ARG distro=latest
FROM centos:$distro

LABEL author="baumgardner@battelle.org"
LABEL description="Transportation Message eXchange build image"
LABEL version="1.0"

RUN yum install -y git cmake gcc gcc-c++ boost-devel libicu-devel net-snmp-devel openssl xerces-c-devel libcurl-devel libsnmp-dev uuid-devel redhat-lsb java-1.8.0-openjdk-headless centos-release-scl scl-utils devtoolset-7-runtime
RUN yum install -y devtoolset-7-gcc*
RUN echo "$(lsb_release -r -i -s) build image created"
