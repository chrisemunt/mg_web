/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: An abstraction of the InterSystems Cache/IRIS API           |
   |              and YottaDB API                                             |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2019-2025 MGateway Ltd                                     |
   | Surrey UK.                                                               |
   | All rights reserved.                                                     |
   |                                                                          |
   | http://www.mgateway.com                                                  |
   |                                                                          |
   | Licensed under the Apache License, Version 2.0 (the "License"); you may  |
   | not use this file except in compliance with the License.                 |
   | You may obtain a copy of the License at                                  |
   |                                                                          |
   | http://www.apache.org/licenses/LICENSE-2.0                               |
   |                                                                          |
   | Unless required by applicable law or agreed to in writing, software      |
   | distributed under the License is distributed on an "AS IS" BASIS,        |
   | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. |
   | See the License for the specific language governing permissions and      |
   | limitations under the License.                                           |
   |                                                                          |
   ----------------------------------------------------------------------------
*/

#ifndef MG_WEBSYS_H
#define MG_WEBSYS_H

/* Set this symbol to 1 to include TLS functionality */
#define DBX_WITH_TLS             0
#if defined(_WIN32)
/* Set this symbol to 1 to allocate memory from a private heap */
#define MG_PRIVATE_HEAP          1
#endif

#define MAJORVERSION             2
#define MINORVERSION             8
#define MAINTVERSION             43
#define BUILDNUMBER              0

#define DBX_VERSION_MAJOR        "2"
#define DBX_VERSION_MINOR        "8"
#define DBX_VERSION_BUILD        "43b"

#define DBX_VERSION              DBX_VERSION_MAJOR "." DBX_VERSION_MINOR "." DBX_VERSION_BUILD
#define DBX_COMPANYNAME          "MGateway Ltd\0"
#define DBX_FILEDESCRIPTION      "HTTP Gateway for InterSystems IRIS/Cache and YottaDB\0"
#define DBX_FILEVERSION          DBX_VERSION
#define DBX_INTERNALNAME         "mg_web_iis\0"
#define DBX_LEGALCOPYRIGHT       "Copyright 2017-2025, MGateway Ltd\0"
#define DBX_ORIGINALFILENAME     "mg_web_iis\0"
#define DBX_PLATFORM             PROCESSOR_ARCHITECTURE
#define DBX_PRODUCTNAME          "mg_web_iis\0"
#define DBX_PRODUCTVERSION       DBX_VERSION
#define DBX_BUILD                DBX_VERSION

#endif

