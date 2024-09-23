
# TMX Ubuntu Build image Dockerfile
#
ARG distro=jammy

FROM ubuntu:$distro

LABEL author="baumgardner@battelle.org"
LABEL description="Rail-Crossing Violation Warning (RCVW)"
LABEL version="$version"

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=America/New_York

# Install base dependencies
RUN apt-get update
RUN apt-get install -y libboost-program-options1.74.0 librdkafka++1 libusb-1.0-0 libxerces-c3.2 kmod gpsd python3-gps gettext jq

# Add the plugin user and script
RUN useradd -M -d /var/lib/plugin -s /bin/false -r plugin
ADD --chmod=755 Docker/deploy/rcvw-rbs/runtime-rbs/configure-ubx.sh /usr/local/bin/configure-ubx.sh
ADD --chmod=755 Docker/deploy/rcvw-rbs/runtime-rbs/rcvw-rbs.sh /usr/local/bin/rcvw-rbs.sh

ARG TMX_ARCH
ARG TMX_CONFIG
ARG TMX_VERSION

ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-firmware.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-Unspecified.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-tmx-message.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-tmx-plugin.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-tmx-broker.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-tmxctl.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-mapplugin.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-hristatusplugin.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-rsuimmediateforwardplugin.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-rtcmplugin.tar.gz /

RUN rm -f /var/lib/plugin/*/manifest.json
ADD Docker/deploy/rcvw-rbs/runtime-rbs/HRIStatus/manifest.json /var/lib/plugin/HRIStatus/manifest.json.disable
ADD Docker/deploy/rcvw-rbs/runtime-rbs/MAP/manifest.json /var/lib/plugin/MAP/manifest.json.disable
ADD Docker/deploy/rcvw-rbs/runtime-rbs/RSUImmediateForward/manifest.json /var/lib/plugin/RSUImmediateForward/manifest.json.disable
ADD Docker/deploy/rcvw-rbs/runtime-rbs/RTCM/manifest.json /var/lib/plugin/RTCM/manifest.json.disable

# Set permissions
RUN chown -R root:root /usr/local
RUN chown -R plugin:plugin /var/lib/plugin
RUN ldconfig

ENV RCVW_COMPONENT=RBS
ENV RCVW_VERSION=${TMX_VERSION}

ENTRYPOINT [ "/bin/bash", "/usr/local/bin/rcvw-rbs.sh" ]