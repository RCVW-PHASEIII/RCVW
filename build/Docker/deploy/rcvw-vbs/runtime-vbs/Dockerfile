
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
RUN apt-get install -y --fix-missing libboost-program-options1.74.0 librdkafka++1 gpsd python3-gps gettext jq openssl apache2

# Add the plugin user and script
RUN useradd -M -d /var/lib/plugin -s /bin/false -r plugin
ADD --chmod=755 Docker/deploy/rcvw-vbs/runtime-vbs/configure-ubx.sh /usr/local/bin/configure-ubx.sh
ADD --chmod=755 Docker/deploy/rcvw-vbs/runtime-vbs/rcvw-vbs.sh /usr/local/bin/rcvw-vbs.sh

ARG TMX_ARCH
ARG TMX_CONFIG
ARG TMX_VERSION

ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-firmware.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-Unspecified.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-tmx-message.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-tmx-plugin.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-tmx-broker.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-tmxctl.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-differentialgpsplugin.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-messagereceiverplugin.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-rcvwplugin.tar.gz /
ADD ${TMX_ARCH}-${TMX_CONFIG}/rcvw-${TMX_VERSION}-Linux-x86_64-rcvwhmi.tar.gz /

# Disable the plugins
RUN rm -f /var/lib/plugin/*/manifest.json
ADD Docker/deploy/rcvw-vbs/runtime-vbs/DifferentialGPS/manifest.json /var/lib/plugin/DifferentialGPS/manifest.json.disable
ADD Docker/deploy/rcvw-vbs/runtime-vbs/MessageReceiver/manifest.json /var/lib/plugin/MessageReceiver/manifest.json.disable
ADD Docker/deploy/rcvw-vbs/runtime-vbs/RCVW/manifest.json /var/lib/plugin/RCVW/manifest.json.disable

# Set permissions
RUN chown -R root:root /usr/local
RUN chown -R plugin:plugin /var/lib/plugin
RUN ldconfig

ENV RCVW_COMPONENT=VBS
ENV RCVW_VERSION=${TMX_VERSION}

ENTRYPOINT [ "/bin/bash", "/usr/local/bin/rcvw-vbs.sh" ]