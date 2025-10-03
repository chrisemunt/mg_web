# mg_web

A High speed web server extension for InterSystems Cache/IRIS, YottaDB and JavaScript.

Chris Munt <cmunt@mgateway.com>  
3 October 2025, MGateway Ltd [http://www.mgateway.com](http://www.mgateway.com)

* Current Release: Version: 2.8; Revision 43c.
* [Release Notes](#relnotes) can be found at the end of this document.

## Overview

**mg\_web** provides a high-performance minimalistic interface between three popular web servers ( **Microsoft IIS**, **Apache** and **Nginx** ) and a choice of either M-like DB Servers ( **YottaDB**, **InterSystems IRIS** and **Cache** ) or JavaScript.

A longer read on the rationale behind **mg\_web** can be found [here](./why_mg_web.md).  This document was prepared with my colleague Rob Tweed.  Rob has many years' experience in creating and using web application development frameworks.

For those wishing to use **mg\_web** with JavaScript, the essential components can be found in the **mg\_web\_js** repository [here](https://github.com/chrisemunt/mg_web_js/blob/master/README.md).  A complete library of documentation setting out the rationale for using **mg\_web** with JavaScript can be found [here](https://github.com/robtweed/mg-showcase/blob/master/MGWEB.md).  You will also find some fairly staggering benchmark results together with a number of Docker containers showcasing what the **mg\_web** line of products can do.

**mg\_web** is compliant with HTTP version 1.1 and 2.0 and WebSockets are supported.  **mg\_web** can connect to a local DB Server via its high-performance API or to local or remote DB Servers via the network.

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
          Response = "HTTP/1.1 200 OK" + crlf
          Response = Response + "Content-type: text/html" + crlf
          Response = Response + crlf
          //
          // Add the HTML content
          Response = Response + "<html>" + crlf
          Response = Response + "<head><title>" + crlf
          Response = Response + "Hello World" + crlf
          Response = Response + "</title></head>" + crlf
          Response = Response + "<h1>Hello World</h1>" + crlf
          return Response
       }

**mg_web** also provides a mode through which response content can be streamed back to the client using DB Server write statements.

       DBServerFunction(CGI, Content, System)
       {
          stream = startstream(ByRef System)

          // Create HTTP response headers
          Write "HTTP/1.1 200 OK" + crlf
          Write "Content-type: text/html" + crlf
          Write crlf
          //
          // Add the HTML content
          Write "<html>" + crlf
          Write "<head><title>" + crlf
          Write "Hello World" + crlf
          Write "</title></head>" + crlf
          Write "<h1>Hello World</h1>" + crlf
          return stream
       }

In production, the above functions would, of course, be crafted in the scripting language provided by the DB Server.

## Prerequisites

* A supported web server.  Currently **mg\_web** supports **Microsoft IIS**, **Apache** and **Nginx**.

* Node.js version 20 (or later) if JavaScript is used.

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

Copyright (c) 2019-2025 MGateway Ltd,
Surrey UK.                                                      
All rights reserved.

http://www.mgateway.com                                                  
Email: cmunt@mgateway.com
 
 
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.      

## <a name="relnotes"></a>Release Notes

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

### v2.2.20 (30 June 2021)

* Introduce a configuration parameter (**health\_check**) to instruct **mg\_web** to retry connecting to DB Servers marked as 'offline' after the specified period of time.
	* Example: **health_check 600** - with this setting the DB Server will, if marked 'offline', be retried after 600 seconds (i.e. after 10 minutes have elapsed since marking the DB Server 'offline').

### v2.3.21 (18 August 2021)

* Introduce support for TLS-secured connectivity between mg\_web and the DB Superserver.
	* This enhancement is available for InterSystems DB Servers only.
	* This enhancement requires DB Superserver version 4.4; Revision 23 (or later).

### v2.3.22 (25 August 2021)

* Support the renamed TLS libraries introduced with OpenSSL v1.1.
	* libeay32.dll was renamed as libcrypto.dll (or libcrypto-1\_1-x64.dll under x64 Windows and libcrypto-1\_1.dll for x86 Windows).
	* ssleay32.dll was renamed as libssl.dll (or libssl-1\_1-x64.dll under x64 Windows and libssl-1\_1.dll for x86 Windows).
* Correct a memory initialization fault that could occasionally lead to connectivity failures between **mg\_web** and the DB Superserver.
	* Recommend that **mg\_web** is not used with DB Superserver v4.4.23. 

### v2.3.23 (1 September 2021)

* Make DB Server names case-insensitive in any server affinity cookie values (the same change was applied to server affinity variables in v2.1.17).
* Improve the validation of values assigned to server affinity variables.
* Introduce a verbose (v) log level.  If set, the key processing steps involved in extracting DB server affinity variables (and cookies). The DB server name chosen will be recorded for each request. 

### v2.4.24 (27 September 2021)

* Introduce Administrator Facilities, implemented as REST requests.  The following operations are included:
	* Listing the internal status of **mg\_web**.
	* Retrieving the configuration file.
	* Retrieving the event log file.
	* Marking individual servers online/offline.
* Improve the granularity of error reporting.

### v2.4.25 (13 October 2021)

* Introduce a DB Server configuration parameter (**connection\_retries**) to control the number of attempts (and the total time spent) in connecting to a DB Server before marking it offline.
	* connection\_retries number\_of\_connection\_retries/total\_time\_allowed

### v2.4.26 (22 December 2021)

* Introduce a DB Server configuration parameter (**idle\_timeout**) to limit the amount of time that a network connection will remain in the pool without receiving any work.  This parameter should be included in **server** configuration blocks.
	* idle\_timeout timeout\_in\_seconds
	* Example: idle\_timeout 300 (close down network connections that have been idle for more than 5 minutes.
	* This enhancement requires DB Superserver version 4.5; Revision 26 (or later).

### v2.4.27 (4 April 2022)

* Correct a regression that led to request payloads not being correctly transmitted to the DB Server from Nginx-based **mg\_web** installations.
	* This regression was introduced in v2.2.18.
	* This change only affects **mg\_web** for Nginx.

### v2.4.28 (17 January 2023)

* Remove an unnecessary "mg\_web: Bad request" error that was previously recorded in the Apache event log for requests that were destined to be served by modules other than **mg\_web**.
* Correct a fault that led to **mg\_web** connections erroneously closing down if no value was specified for the (optional) **idle\_timeout** configuration parameter. 

### v2.4.29 (29 May 2023)

* Documentation update.

### v2.5.30 (19 July 2023)

* Introduce a configuration parameter (DB Server section) to limit the number of connections created to a DB Server:
	* max_connections
	* When the limit is reached, and all connections in the pool are busy, additional requests will queue for a period up to the time allowed in the **timeout** setting.

### v2.5.31 (3 October 2023)

* Correct a fault in memory management for the Nginx solution.
	* This fault resulted in requests occasionally failing with SIGSEGV errors.

### v2.6.32 (17 January 2024)

* Improve the mechanism through which WebSocket functions are invoked.  Instead of embedding the WebSocket function name and path in the client-side script, a mapping must be created in the appropriate location block of the configuration.  For example:
	* websocket mywebsocket.mgw websocket^webroutine
	* Use '/[location_path]/mywebsocket.mgw' in the client-side script.  When invoked, this URL will map to 'websocket^webroutine' on the DB Server.

### v2.7.33 (3 June 2024)

* Introduce support for Server-Sent Events (SSE).
	* Note that in order to use this facility, DB Superserver v4.5.32 must be installed.
* Ensure that the 'administrator: off' configuration setting is properly honoured.
* Return a HTTP status code of '504 Gateway Timeout' if a request to the DB Server times-out.
	* Previous versions would return '500 Internal Server Error' on response timeout.
	* A custom form can be created to override the default response by defining the form to be returned in parameter: **custompage\_dbserver\_timeout**

### v2.7.34 (7 June 2024)

* Correct a fault in the management of SSE channels that could lead to infinite loops on channel closure - particularly when used with the JavaScript Superserver.

### v2.7.34a (17 June 2024)

* Update the WebLink compatibiity shim code listed [here](./mg_web_weblink_config.md).  This update corrects a fault in the code for processing multi-part requests.  

### v2.7.35 (19 June 2024)

* Record (in the event log) extra error information when connections made through the YottaDB API fail.
* For UNIX systems, record at initialisation-time the user and group under which the hosting web server worker process is running.
	* For example: configuration: /opt/nginx1261/conf/mgweb.conf (user=nobody; group=nogroup).

### v2.7.36 (21 June 2024)

* Correct a fault that resulted in the build for the Apache module failing under UNIX.
	* This regression was introduced in v2.7.35.

### v2.7.36a (9 July 2024)

* Update the WebLink compatibiity shim code listed [here](./mg_web_weblink_config.md).  This update includes an example call-out for the WebLink Event Broker facility.  

### v2.8.37 (28 August 2024)

* Introduce support for chunked request payloads.

### v2.8.38 (7 September 2024)

* Ensure that excessive amounts of memory are not allocated for large response payloads where chunking is disabled in the **mg\_web** configuration.
	* If chunking cannot be used for request payloads over 500K, the payload will be streamed back with a 'Connection: close' response header added.
	* The end of the stream will be marked by the server closing the connection.

### v2.8.39 (28 October 2024)

* Ensure that global resources held by **mg\_web** are explicitly released when hosting web server worker processes are closed down.

### v2.8.40 (11 November 2024)

* Add extra checks to ensure that an application returns a valid HTTP response header with its forms.  If a form's header is found to be faulty (or missing), **mg\_web** will return a basic HTTP header to the client.

### v2.8.41 (9 March 2025)

* Detect the 'client aborted' scenario for SSE channels. Use the following function to detect if the client has aborted from M-based SSE servers.
   * set eof=$$clientgone^%zmgsis(.%system) 
   * This enhancement requires DB Superserver version 4.5; Revision 37 (or later).

### v2.8.42 (19 April 2025)
* Protect against a memory violation that occasionally occurred after a failover event.
   * The symptom of this problem (for Windows hosts) is the following Event Log message: Exception caught in f:mg_web_execute: c0000005:30

### v2.8.43 (23 April 2025)
   * Correct a fault in the buffer allocation for (particularly large) HTTP response headers.
      * The symptom of this problem (for Windows hosts) is the following Event Log message: Exception caught in f:mg_web_process: c0000005:40

### v2.8.43a (18 June 2025):
   * Check that there's enough memory available to service a web request before proceeding.
      * If a web server host is low in memory, an HTTP 500 error will be immediately returned to the client and a "Memory Allocation" error written to the event log.
   * Use a private heap for Windows based web servers to limit the scope for contention between **mg\_web** and other web server addon modules.
   * Ensure that the client side of TCP sockets are closed when the DB Server closes its (server-side) end. 

### v2.8.43b (15 July 2025):
   * Check that there are viable alternative servers before invoking the failover mechanism.
      * With previous builds, a looping condition would occur if all the alternative servers were marked as for "exclusive use". 

### v2.8.43c (3 October 2025):
   * Correct and add protection against the faults leading to the following Windows exceptions.
      * Exception caught in f:mg_find_sa_variable_ex: c0000005:0
      * Exception caught in f:mg_web_http_error: c0000005:0
      * Exception caught in f:mg_submit_headers: c0000005:2
      * Exception caught in f:mg_write_client: c0000005:1 (buffer_size=874524974; total=120000; max=60000; sent=512; result=0)

 
