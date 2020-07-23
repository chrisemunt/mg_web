# mg_web

A High speed web server extension for InterSystems Cache/IRIS and YottaDB.

Chris Munt <cmunt@mgateway.com>  
21 July 2020, M/Gateway Developments Ltd [http://www.mgateway.com](http://www.mgateway.com)

* Current Release: Version: 1.0; Revision 1.
* This is the first release and very much 'a work in progress'.  Proceed with care!
* [Release Notes](#RelNotes) can be found at the end of this document.

## Overview

**mg\_web** provides a high-performance minimalistic interface between three popular web servers ( **Microsoft IIS**, **Apache** and **Nginx** ) and M-like DB Servers ( **YottaDB**, **InterSystems IRIS** and **Cache** ).

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

In production, the above function would, of course, be crafted in the scripting language provided by the DB Server.

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

## Things for the future...

* The ability to handle request and response payloads that exceed the maximum DB Server string size.
* The ability to stream response data using DB Server 'Write' commands.
* The ability to specify more than one DB Server per web path for the purpose of fail-over in the event of the primary DB Server becoming unavailable.

## License

Copyright (c) 2019-2020 M/Gateway Developments Ltd,
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
