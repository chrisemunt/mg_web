//
//   ----------------------------------------------------------------------------
//   | Package:     mg_web_node                                                 |
//   | OS:          Unix/Windows                                                |
//   | Description: A Node.js server for mg_web                                 |
//   | Author:      Chris Munt cmunt@mgateway.com                               |
//   |                         chris.e.munt@gmail.com                           |
//   | Copyright(c) 2023 - 2023 MGateway Ltd                                    |
//   | Surrey UK.                                                               |
//   | All rights reserved.                                                     |
//   |                                                                          |
//   | http://www.mgateway.com                                                  |
//   |                                                                          |
//   | Licensed under the Apache License, Version 2.0 (the "License"); you may  |
//   | not use this file except in compliance with the License.                 |
//   | You may obtain a copy of the License at                                  |
//   |                                                                          |
//   | http://www.apache.org/licenses/LICENSE-2.0                               |
//   |                                                                          |
//   | Unless required by applicable law or agreed to in writing, software      |
//   | distributed under the License is distributed on an "AS IS" BASIS,        |
//   | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. |
//   | See the License for the specific language governing permissions and      |
//   | limitations under the License.                                           |
//   |                                                                          |
//   ----------------------------------------------------------------------------
//

const net = require('net');
const cluster = require('node:cluster');
const cpus = require('node:os').cpus().length;
const process = require('node:process');

let port = 7041;
if (process.argv.length > 2) {
   port = parseInt(process.argv[2]);
}
let app = './application.js';
if (process.argv.length > 3) {
   app = process.argv[3];
}
const launch = require(app);

let buffer = "";

if (cluster.isMaster) {
   console.log('mg_web server for Node.js %s; CPUs=%d; pid=%d;', process.version, cpus, process.pid);

   let server = net.createServer();    

   server.on('connection', handle_connection);

   server.listen(port, function() {    
     console.log('mg_web server listening on %j;', server.address());  
   });

   function handle_connection(conn) {    
      let remote_address = conn.remoteAddress + ':' + conn.remotePort;  
      console.log('mg_web new client connection from %s', remote_address);
      conn.on('data', (d) => {     
         wk = cluster.fork();
         wk.send(d, conn);
      });
   }
}
else {
  worker();
}
function worker() {

   let data_properties = { len: 0, type: 0, sort: 0 };
   let buffer = new Uint8Array(2048);

   process.on('message', (dbx, conn) => {

      conn.on('data', onConnData);
      conn.once('close', onConnClose);
      conn.on('error', onConnError);

      let remote_address = conn.remoteAddress + ':' + conn.remotePort;  
      console.log('mg_web new worker process created pid=%d; client=%s', process.pid, remote_address);

      // pretend we're an IRIS server
      let offset = 0;
      let zv = "IRIS for Windows (x86-64) 2022.3 (Build 589U) Fri Jan 6 2023 00:06:23 EST";
      offset = block_add_string(buffer, offset, zv, zv.length, 0, 0);
      conn.write(buffer.slice(0, offset));

      function onConnData(data) {
         let offset = 0;
         let request_no = 0;
         let cgi = [];
         let sys = [];
         let content = "";
         let tlen = get_size(data, offset);
         let cmnd = data[4];
         offset += 5;
         let obufsize = get_size(data, offset);
         let utf16 = data[offset + 5];
         offset += 5;
         let idx = get_size(data, offset);
         offset += 5;
         //console.log('request tlen=%d; cmnd=%d; obufsize=%d; utf16=%d; idx=%d;', tlen, cmnd, obufsize, utf16, idx);

         let len = 0;
         let doffset = 0;
         let dlen = 0;
         let fun = "";
         let ctx = "";
         let param = "";
         for (let argc = 0; argc < 10; argc++) {
            len = block_get_size(data, offset, data_properties)
            //console.log(' >>> item argc=%d; offset=%d; len=%d; type=%d; sort=%d;', argc, offset, len, data_properties.type, data_properties.sort);
            offset += 5;
            if (argc === 0) {
               // dbxweb^%zmgsis
               fun = data.slice(offset, offset + len).toString();
            }
            else if (argc === 1) {
               // arg 1: context
               ctx = data.slice(offset, offset + len).toString();
            }
            else if (argc === 2) {
               // arg 2: HTTP request data
               doffset = offset;
               dlen = len;
            }
            else if (argc === 3) {
               // arg 3: parameters
               param = data.slice(offset, offset + len).toString();
            }
            offset += len;
            if (data_properties.sort === 9) {
               break;
            }
         }
         //console.log('fun=%s; ctx=%s; param=%s;', fun, ctx, param);

         // unpack HTTP request data into cgi array, sys array and content (request payload)
         offset = doffset;
         for (let argc = 0; argc < 1000; argc++) {
            len = block_get_size(data, offset, data_properties)
            //console.log(' >>> web item offset=%d; len=%d; type=%d; sort=%d; data=%s', offset, len, data_properties.type, data_properties.sort, data.slice(offset + 5, offset + 5 + len).toString());
            offset += 5;
            if (data_properties.sort === 5) {
               // CGI environment variable
               let d = data.slice(offset, offset + len).toString().split("=");
               cgi.push(new nvpair(d[0], d[1]));
            }
            if (data_properties.sort === 6) {
               // request payload (if any)
               content = data.slice(offset, offset + len);
            }
            if (data_properties.sort === 8) {
               // system variable
               let d = data.slice(offset, offset + len).toString().split("=");
               if (d[0] === "no") {
                  d[1] = get_size(data, offset + 3);
                  request_no = d[1];
               }
               else if (d[0] === "function") {
                  fun = d[1];
               }
               sys.push(new nvpair(d[0], d[1]));
            }
            offset += len;
            if (data_properties.sort === 9) {
               break;
            }
         }

         // notify mg_web of data framing protocol in use for response
         offset = 0;
         offset = add_head(buffer, offset, 0, 0);
         offset = add_head(buffer, offset, request_no, 1);
         conn.write(buffer.slice(0, offset), 'binary');

         // ******* call-out to application - START *******
         // CGI variables in 'cgi' array; system variables in 'sys' array; request payload in 'content'
         // generate a resonse in variable 'res'
         eval('res = launch.' + fun + '(cgi, content, sys)');
         // ******* call-out to application - END *******

         offset = 0;
         offset = block_add_chunk(buffer, offset, res, res.length);
         offset = set_term(buffer, offset);
         conn.write(buffer.slice(0,offset), 'binary');
      }

      function onConnClose() {
         console.log('connection closed');
      }

      function onConnError(err) {
         console.log('Connection error: %s', err.message);
      }

   });

}

class nvpair {
   name;
   value;
   constructor(name, value) {
      this.name = name;
      this.value = value;
   }
}
function block_copy(buffer_to, offset, buffer_from, from, to) {
   for (let i = from; i < to; i++) {
      buffer_to[offset++] = buffer_from[i];
   }
   return offset;
}
function block_add_string(buffer, offset, data, data_len, data_sort, data_type) {
   offset = block_add_size(buffer, offset, data_len, data_sort, data_type);
   for (let i = 0; i < data_len; i++) {
      buffer[offset++] = data.charCodeAt(i);
   }
   return offset;
}
function block_add_chunk(buffer, offset, data, data_len) {
   offset = set_size(buffer, offset, data_len);
   for (let i = 0; i < data_len; i++) {
      buffer[offset++] = data.charCodeAt(i);
   }
   return offset;
}
function block_add_size(buffer, offset, data_len, data_sort, data_type) {
   offset = set_size(buffer, offset, data_len);
   buffer[offset] = ((data_sort * 20) + data_type);
   return (offset + 1);
}
function add_head(buffer, offset, data_len, cmnd) {
   offset = set_size(buffer, offset, data_len);
   buffer[offset] = cmnd;
   return (offset + 1);
}
function block_get_size(buffer, offset, data_properties) {
   data_properties.len = get_size(buffer, offset);
   data_properties.sort = buffer[offset + 4];
   data_properties.type = data_properties.sort % 20;
   data_properties.sort = Math.floor(data_properties.sort / 20);
   return data_properties.len;
}
function set_term(buffer, offset) {
   buffer[offset + 0] = 255;
   buffer[offset + 1] = 255;
   buffer[offset + 2] = 255;
   buffer[offset + 3] = 255;
   return (offset + 4)
}
function set_size(buffer, offset, data_len) {
   buffer[offset + 0] = (data_len >> 0);
   buffer[offset + 1] = (data_len >> 8);
   buffer[offset + 2] = (data_len >> 16);
   buffer[offset + 3] = (data_len >> 24);
   return (offset + 4)
}
function get_size(buffer, offset) {
   return ((buffer[offset + 0]) | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24));
}
