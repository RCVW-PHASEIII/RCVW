#######################################################################################################################
# RCVW CBS Azure Function App Containers
#
# These functions are written in Python and were constructed from the Azure Function Core Tools utilities
#######################################################################################################################

#######################################################################################################################
# (1) rcvw-cbs-azure-functions-python
#   The base container for writing Azure Function Apps in Python
#   This sets the environment for execution and installs dependent software, such as the ODBC client
#######################################################################################################################
FROM mcr.microsoft.com/azure-functions/python:4-python3.10 AS rcvw-cbs-azure-functions-python

ENV AzureWebJobsScriptRoot=/home/site/wwwroot \
    AzureFunctionsJobHost__Logging__Console__IsEnabled=true

# Need to install ODBC tools
RUN apt update
RUN apt install -y lsb-release
RUN curl https://packages.microsoft.com/keys/microsoft.asc | tee /etc/apt/trusted.gpg.d/microsoft.asc
RUN curl https://packages.microsoft.com/config/debian/$(lsb_release -rs)/prod.list | tee /etc/apt/sources.list.d/mssql-release.list
RUN apt update

RUN ACCEPT_EULA=Y apt install -y msodbcsql18
RUN ACCEPT_EULA=Y apt install -y mssql-tools18
RUN apt install -y unixodbc-dev

#######################################################################################################################
# (2) rcvw-cbs-ingest-azure
#   An Azure Function App for writing incoming records to the RCVW CBS Database
#   This function is triggered when a message arrives at the Azure Service Bus subscription.
#   It is dependent on pyodbc for connecting to Azure SQL.
#   Additionally, this container includes a portion of TMX code for decoding the J2735 messages
#######################################################################################################################
FROM rcvw-cbs-azure-functions-python AS rcvw-cbs-ingest-azure

RUN apt update
RUN apt install -y gcc

COPY Docker/deploy/rcvw-cbs/rcvw-cbs-ingest/requirements.txt /
RUN pip install -r /requirements.txt

COPY Docker/deploy/rcvw-cbs/rcvw-cbs-ingest/. /home/site/wwwroot

ARG TMX_ARCH
ARG TMX_CONFIG
ARG TMX_VERSION

COPY ${TMX_ARCH}-${TMX_CONFIG}/tmx/tmx-message/j2735-202007/src/tmx/message/j2735/202007/* /tmp/build/src/

RUN cd /tmp/build && gcc -c -Isrc src/*.c
RUN cd /tmp/build && ar rcs libtmxasn_j2735_202007.a *.o

COPY Docker/deploy/rcvw-cbs/rcvw-cbs-ingest/src/read_j2735.c /tmp/build/src/
RUN cd /tmp/build && gcc -o /usr/local/bin/read_j2735 -Isrc src/read_j2735.c libtmxasn_j2735_202007.a
RUN rm -rf /tmp/requirements.txt /tmp/build

COPY Docker/deploy/rcvw-cbs/rcvw-cbs-ingest/*.* /home/site/wwwroot/

ENV RCVW_COMPONENT=CBS
ENV RCVW_VERSION=${TMX_VERSION}

#######################################################################################################################
# (3) rcvw-cbs-event-mgr-azure
#   An Azure Function App for monitoring the current state of the RBS.
#   This includes checking that the RBS messages are being received at the appropriate rate
#   This also includes independently detecting the current state of the HRI preemption signal
#   This function will send event messages to the Azure service bus when states have changes
#   It is dependent on pyodbc for connecting to Azure SQL and azure-servicebus for sending messages
#######################################################################################################################
FROM rcvw-cbs-azure-functions-python AS rcvw-cbs-event-mgr-azure

COPY Docker/deploy/rcvw-cbs/rcvw-cbs-event-mgr/requirements.txt /
RUN pip install -r /requirements.txt

COPY Docker/deploy/rcvw-cbs/rcvw-cbs-event-mgr/. /home/site/wwwroot
CMD /usr/local/bin/python3 /home/site/wwwroot/rcvw-cbs-event-mgr.py

#######################################################################################################################
# RCVW CBS Container Apps
#
# These containers are written to be run as a standalone container in Azure
#######################################################################################################################

#######################################################################################################################
# (1) rcvw-cbs-rtcm-azure
#
#   An Azure Container App for forwarding RTCM messages from CORS to the Azure Service Bus subscription.
#   It is dependent on pyodbc for connecting to Azure SQL and pygnssutils for pulling RTCM.
#######################################################################################################################
FROM rcvw-cbs-azure-functions-python AS rcvw-cbs-rtcm-azure

# Need CORS connectivity by State
ENV NTRIP_HOST_FL=40.121.5.206
ENV NTRIP_PORT_FL=10000
ENV NTRIP_MOUNTPOINT_FL=RTCM3_Near
ENV NTRIP_USER_FL=gbaumgardner
ENV NTRIP_PASSWORD_FL=B@ttelle!FDOT
ENV NTRIP_HOST_MI=148.149.0.87
ENV NTRIP_PORT_MI=10010
ENV NTRIP_MOUNTPOINT_MI=VRS_MSM4
ENV NTRIP_USER_MI=battelle
ENV NTRIP_PASSWORD_MI=B@ttelle01!
ENV NTRIP_HOST_MO=rtk3.modot.mo.gov
ENV NTRIP_PORT_MO=2101
ENV NTRIP_MOUNTPOINT_MO=VRS_RTCM31
ENV NTRIP_USER_MO=battelle01
ENV NTRIP_PASSWORD_MO=B@ttelle01!
ENV NTRIP_HOST_OH=156.63.133.115
ENV NTRIP_PORT_OH=2101
ENV NTRIP_MOUNTPOINT_OH=ODOT_VRS_RTCM3
ENV NTRIP_USER_OH=battelle8
ENV NTRIP_PASSWORD_OH=B@ttelle01!

COPY Docker/deploy/rcvw-cbs/rcvw-cbs-rtcm/requirements.txt /
RUN pip install -r /requirements.txt

COPY Docker/deploy/rcvw-cbs/rcvw-cbs-rtcm/. /home/site/wwwroot

CMD /usr/local/bin/python3 /home/site/wwwroot/rcvw-cbs-rtcm.py

#######################################################################################################################
# (2) rcvw-cbs-api-azure
#
#   An Azure Container App for exposing the RCVW CBS database over a REST API.
#   This uses the Azure Data API Builder
#######################################################################################################################
FROM mcr.microsoft.com/dotnet/sdk:6.0-cbl-mariner2.0 AS rcvw-cbs-api-azure-build

WORKDIR /src
COPY [ "Docker/deploy/rcvw-cbs/rcvw-cbs-api/.", "./" ]
RUN dotnet build "./src/Service/Azure.DataApiBuilder.Service.csproj" -c Docker -o /out -r linux-x64

FROM mcr.microsoft.com/dotnet/aspnet:6.0-cbl-mariner2.0 AS rcvw-cbs-api-azure

COPY --from=rcvw-cbs-api-azure-build /out /App
COPY Docker/deploy/rcvw-cbs/rcvw-cbs-api/dab-config.json /App
WORKDIR /App
ENV ASPNETCORE_URLS=http://+:5000
ENTRYPOINT ["dotnet", "Azure.DataApiBuilder.Service.dll"]

