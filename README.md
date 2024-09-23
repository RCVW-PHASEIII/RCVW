# Project Description

Rail Crossing Violation Warning 

Plugins for V2X Hub version 4.0 that implement both Roadside and In-Vehicle operations of RCVW.

Vehicle-to-Everything (V2X) software applications provided by the US Department of Transportation (USDOT) Federal Highway Administration (FHWA) are built upon a reference implementation known as V2X-Hub. The V2X-Hub is itself a collection of intercommunicating software components for an Intelligent Transportation System (ITS) that uses Battelle’s Transportation Message eXchange (TMX®) platform. 

The TMX® software architecture provides a robust, flexible, scalable, and extensible interface allowing multiple connected vehicle (CV) applications to co-exist in a single platform, while sharing critical resources between applications. TMX® is hardware-agnostic and can be modified to work with current roadside unit (RSU) or after market on board unit (OBU) hardware to facilitate CV message transmission and receipt.

The critical operations of OBUs and RSUs include sending and receiving SAE J2735 messages over the Cellular Vehicle-to-Everything (C-V2X) wireless protocols, and many CV applications require interfaces to external systems such as Global Navigation Satellite System (GNSS) positioning information, vehicle controller area network (CAN) data, traffic signals , and cloud services. The TMX® middleware provides CV applications with the ability to exchange messages and external data efficiently and effectively across software components using a publish-subscribe model. The critical communication functions required for developing comprehensive safety and mobility applications are all implemented natively within the TMX® platform, leaving the application programmers to focus on coding the business logic. This ultimately leads to rapid, productive development of CV safety and mobility applications.

The goals of the TMX® platform and its V2X-Hub derivative is to provide:

* A ubiquitous means of message Tx and Rx over wired and wireless transport, including the licensed C-V2X bands and unlicensed spectrum such as cellular and Wi-Fi.

* A built-in means for efficient encoding and decoding of critical information without the need for custom data structures and code generation.

* A prescribed component architecture to maximize scalability and reusability.

* Native support for common CV and ITS functions such as simple network management protocol (SNMP), CAN, SAE J2735, National Transportation Communications for ITS Protocol (NTCIP), Global Navigation Satellite System (GNSS), and specialized functions such as geofencing and driver alerting.

The FHWA maintains a version of V2X-Hub that was originally built on the release 3.0 of TMX®, which is limited to a singular custom communication protocol built specifically for the US DOT prototype projects. While new components have been added to V2X-Hub, the underlying platform was never enhanced to take advantage of compiler evolution, or to include additional communication protocols and associated performance improvements. In order to meet more advanced requirements of specific US DOT V2X projects, Battelle has developed release 4.0 of the TMX® architecture and partial port of the necessary V2X-Hub components. 

# Prerequisites

To compile the RCVW Roadside-based Subsystem (RBS) component with cloud connectivity, you must compile Apache Qpid™ Proton version 0.39 or higher:

https://qpid.apache.org/releases/qpid-proton-0.39.0/

For other prerequisites, see the system requirements for V2X Hub

# Usage

## Building
```
$ cd TMX/build
$ mkdir debug
$ cd debug
$ cmake -DTMX_PROJECT=rcvw ..

```
## RBS Component
The Roadside-based Subsystem compiles into a single Docker image.

```
$ make docker-image-rcvw-rbs
```

## VBS Component
The Vehicle-based Subsystem compiles into a single Docker image.

```
$ make docker-image-rcvw-vbs
```

## CBS Components
The Cloud-based Subsystem has multiple Docker images.

```
$ make docker-image-rcvw-cbs-api
$ make docker-image-rcvw-cbs-event-mgr
$ make docker-image-rcvw-cbs-ingest
$ make docker-image-rcvw-cbs-rtcm
```

## Execution
Each Docker image for an RCVW component comes with an associated docker-compose.yaml file under the build directory to help execute each component.

For example:
```
$ docker compose up -d rcvw-rbs
$ docker compose up -d rcvw-vbs
$ docker compose up -d rcvw-cbs-api
$ docker compose up -d rcvw-cbs-event-mgr
$ docker compose up -d rcvw-cbs-ingest
$ docker compose up -d rcvw-cbs-rtcm
```
# Version History and Retention
Version 2.0 - September, 2024 - RCVW Phase III

# License
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# Contact Information

Contact Name: Jared Withers (FRA) 
Contact Information: Jared.Withers@dot.gov, 202-493-6014

