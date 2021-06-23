# mg_web

A High speed web server extension for InterSystems Cache/IRIS and YottaDB.

Chris Munt <cmunt@mgateway.com>  
23 June 2021, M/Gateway Developments Ltd [http://www.mgateway.com](http://www.mgateway.com)

* Current Release: Version: 2.2; Revision 19.
* [Release Notes](#RelNotes) can be found at the end of this document.

## Overview

**mg\_web** provides a high-performance minimalistic interface between three popular web servers ( **Microsoft IIS**, **Apache** and **Nginx** ) and M-like DB Servers ( **YottaDB**, **InterSystems IRIS** and **Cache** ).  It is compliant with HTTP version 1.1 and 2.0 and WebSockets are supported.  **mg\_web** can connect to a local DB Server via its high-performance API or to local or remote DB Servers via the network.

A longer read on the rationale behind **mg\_web** can be found [here](./why_mg_web.md).  This document was prepared with my colleague Rob Tweed.  Rob has many years' experience in creating and using web application development frameworks.

Full documentation for installing and configuring **mg\_web** can be found [here](https://github.com/chrisemunt/mg_web/blob/master/doc/mg_web.pdf).

If you are familiar with **WebLink**, **WebLink Developer** or **EWD** then [this document](./mg_web_weblink_config.md) will help you get started with **mg\_web** and will explain how existing applications created with those technologies can be run through **mg\_web**.  Thanks are due to Rob Tweed and Mike Clayton for designing this interface.

## Technical Summary

HTTP requests passed to the DB Server via **mg\_web** are processed by a simple function of the form:

       Response = DBServerFunction(CGI, Content, System)

Where **CGI** represents an array of CGI Environment Variables, **Content** represents the request payload and **System** is reserved for **mg\_web** use.

A simple 'Hello World' function would look something like the following pseudo-code:

       DBServerFunction(CGI, Content, System)
       {
          // Create HTTP response headers
          Response = ”HTTP/1.1 200 OK” + crlf
          Response = Response + ”Content-type: text/html” + crlf
          Response = Response + crlf
          //
          // Add the HTML content
          Response = Response + ”<html>" + crlf
          Response = Response + ”<head><title>” + crlf
          Response = Response + ”Hello World” + crlf
          Response = Response + ”</title></head>” + crlf
          Response = Response + ”<h1>Hello World</h1>” + crlf
          return Response
       }

**mg_web** also provides a mode through which response content can be streamed back to the client using DB Server write statements.

       DBServerFunction(CGI, Content, System)
       {
          stream = startstream(ByRef System)

          // Create HTTP response headers
          Write ”HTTP/1.1 200 OK” + crlf
          Write ”Content-type: text/html” + crlf
          Write crlf
          //
          // Add the HTML content
          Write ”<html>" + crlf
          Write ”<head><title>” + crlf
          Write ”Hello World” + crlf
          Write ”</title></head>” + crlf
          Write ”<h1>Hello World</h1>” + crlf
          return stream
       }

In production, the above functions would, of course, be crafted in the scripting language provided by the DB Server.

## Pre-requisites

* A supported web server.  Currently **mg\_web** supports **Microsoft IIS**, **Apache** and **Nginx**.

* A database. InterSystems **Cache/IRIS** or **YottaDB** (or similar M DB Server):

       https://www.intersystems.com/
       https://yottadb.com/

* A suitable C compiler if building from the source code.

**mg\_web** is written in standard C (or C++ for IIS).  The GNU C compiler (gcc) can be used for Linux systems:

Ubuntu:

       apt-get install gcc

Red Hat and CentOS:

       yum install gcc

Apple OS X can use the freely available **Xcode** development environment.

Windows can use the free "Microsoft Visual Studio Community" edition of Visual Studio for building the **SIG**:

* Microsoft Visual Studio Community: [https://www.visualstudio.com/vs/community/](https://www.visualstudio.com/vs/community/)

There are built Windows binaries available from:

* [https://github.com/chrisemunt/mg_web/blob/master/bin](https://github.com/chrisemunt/mg_web/blob/master/bin)

Currently, you will find built solutions for Windows IIS (x64) and Apache (x86 and x64).

Full documentation for building, deploying and using **mg\_web** will be found in the package: **/doc/mg\_web.pdf**


## License

Copyright (c) 2019-2021 M/Gateway Developments Ltd,
Surrey UK.                                                      
All rights reserved.
 
http://www.mgateway.com                                                  
Email: cmunt@mgateway.com
 
 
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.      

## <a name="RelNotes"></a>Release Notes

### v1.0.1 (21 July 2020)

* Initial Release

### v1.0.2 (31 July 2020)

* Improve the parsing and validation of the **mg\_web** configuration file (mgweb.conf).

### v1.0.3 (3 August 2020)

* Correct a fault in the network connectivity code between **mg\_web** and InterSystems databases under UNIX.
* Correct a fault in the API-based connectivity code between **mg\_web** and YottaDB under UNIX.
* Introduce an Event Log facility that can be controlled by the **log\_level** configuration parameter.  See the documentation (section: 'General **mg\_web** configuration').

### v1.1.4 (17 August 2020)

* Introduce the ability to stream response content back to the client using M Write statements (InterSystems Databases) or by using the supplied write^%zmgsis() procedure (YottaDB and InterSystems Databases).
* Include the configuration path and server names used for the request in the DB Server function's %system array.

### v1.1.5 (19 August 2020)

* Introduce HTTP version 2 compliance.

### v2.0.6 (29 August 2020)

* Introduce WebSocket support.
* Introduce a "stream ASCII" mode.  This mode will enable both InterSystems DB Servers and YottaDB to return web response content using the embedded DB Server **write** commands.
* Reset the UCI/Namespace after completing each web request (and before re-using the same DB server process for processing the next web request). 
* Insert a default HTTP response header (Content-type: text/html) if the application doesn't return one.

### v2.0.7 (2 September 2020)

* Correct a fault in WebSocket connectivity for Nginx under UNIX.

### v2.0.8 (30 October 2020)

* Introduce a configuration parameter ('**chunking**') to control the level at which HTTP chunked transfer is used. Chunking can be completely disabled ('chunking off'), or set to only be used if the response payload exceeds a certain size (e.g. 'chunking 250KB').
* Introduce the ability to define custom HTML pages to be returned on **mg\_web** error conditions.  Custom pages (specified as full URLs) can be defined for the following error conditions.
	* DB Server unavailable (parameter: **custompage\_dbserver\_unavailable**)
	* DB Server busy (parameter: **custompage\_dbserver\_busy**).
	* DB Server disabled (parameter: **custompage\_dbserver\_disabled**)

### v2.1.9 (6 November 2020)

* Introduce the functionality to support load balancing and fail-over.
	* Web Path Configuration Parameters:
		* **load\_balancing** [on|off] (default is off)
		* **server\_affinity** variable:[variable(s)] cookie:[name]
* Correct a fault that led to response payloads being truncated when connecting to YottaDB via its API.

### v2.1.10 (12 November 2020)

* Correct a fault that led to web server worker processes crashing if an error occurred while processing a request via a DB Server API.  Such error conditions will now be handled gracefully and an appropriate HTTP error code returned to the client.
* Correct a memory leak that could potentially occur when using Cookies to implement server affinity in a multi-server configuration.
* Introduce a global-level configuration parameter to allow administrators to set the size of the working buffer for handling requests and their response (parameter: **request\_buffer\_size**).

### v2.1.11 (20 November 2020)

* Correct a fault that led to **mg\_web** not working correctly under Nginx on the Raspberry Pi.

### v2.1.12 (23 November 2020)

* Ensure that API bindings to the DB Server are gracefully closed when the web server terminates the hosting worker process.  This correction does not affect configurations using network based connectivity between **mg\_web** and the DB Server.

### v2.1.13 (29 January 2021)

* Improved error reporting (to the event log).
* Miscellaneous minor bug fixes.

### v2.1.14 (6 February 2021)

* Correct a regression in the generation of chunked responses (regression introduced in v2.1.13).

### v2.1.15 (20 April 2021)

* Add functionality to parse multipart MIME content.  In addition to the inclusion of 'helper functions' to parse multipart content in the DB Superserver code base, this update will also parse such content on the web server side to extract variables used for Server Affinity.
	* This enhancement requires DB Superserver version 4.2; Revision 21 (or later).

### v2.1.16 (3 May 2021)

* Introduce a mechanism through which DB Servers can be excluded from Load-Balancing and Failover.  DB Servers marked in this way are usually reserved to enable specific applications to be accessed through the hosting path.

### v2.1.17 (26 May 2021)

* Make DB Server names case-insensitive in the configuration and in any server affinity variables.

### v2.2.18 (18 June 2021)

* Introduce support for request payloads that exceed the maximum string length of the target DB Server.
	* This enhancement requires DB Superserver version 4.3; Revision 22 (or later).

### v2.2.19 (23 June 2021)

* Mark all DB Servers for a path as being 'online' after a request fails on account of all servers being being marked 'offline'.  This will allow subsequent requests to succeed if, in the meantime, a participating DB Server becomes available.