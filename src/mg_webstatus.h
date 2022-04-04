/*
   ----------------------------------------------------------------------------
   | mg_web.so|dll                                                            |
   | Description: HTTP Gateway for InterSystems Cache/IRIS and YottaDB        |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2019-2022 M/Gateway Developments Ltd,                      |
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

#ifndef MG_WEBSTATUS_H
#define MG_WEBSTATUS_H

typedef struct tagMGADM {
   short chunking_allowed;
   int chunkno;
   int len_alloc;
   int len_used;
   char *buf_addr;
   char *filename;
} MGADM, *LPMGADM;


int mg_admin      (MGWEB *pweb);
int mg_status     (MGWEB *pweb, MGADM *padm, int context);
int mg_get_file   (MGWEB *pweb, MGADM *padm, int context);
int mg_status_add (MGWEB *pweb, MGADM *padm, char *data, int data_len, int context);
int mg_get_value  (char *json, char *name, char *value);

#endif

