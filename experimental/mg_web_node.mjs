//
//   ----------------------------------------------------------------------------
//   | Package:     mg_web_node                                                 |
//   | OS:          Unix/Windows                                                |
//   | Description: A Node.js server for mg_web                                 |
//   | Author:      Chris Munt cmunt@mgateway.com                               |
//   |                         chris.e.munt@gmail.com                           |
//   | Copyright(c) 2023 - 2024 MGateway Ltd                                    |
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

import net from 'node:net';
import os from 'node:os';
import process from 'node:process';
import child_process from 'node:child_process'

const cpus = os.cpus().length;

const MGWEB_BUFFER_SIZE = 3641145; // or 32768

let port = 7041;
let app = "application.mjs";
let primary = true;
let node_path = process.argv[0];
let mod_name = process.argv[1];

if (process.argv.length > 2) {
  port = parseInt(process.argv[2]);
}
if (port === 1000000) {
   primary = false;
}
if (process.argv.length > 3) {
  app = process.argv[3];
}

if (primary) {
  console.log('mg_web server for Node.js %s; CPUs=%d; pid=%d;', process.version, cpus, process.pid);

  let server = net.createServer();
  let workers = new Map();

  process.on( 'SIGINT', async function() {
    console.log('*** CTRL & C detected: shutting down gracefully...');

    if (workers.size > 0) {
      for (const [key, worker] of workers) {
        console.log('signalling worker ' + worker.pid + ' to stop: key = ' + key);
        worker.send('<<stop>>');
        workers.delete(key);
      }
      setTimeout(() => {
        process.exit();
      }, 1000);
    }
    else {
      process.exit();
    }
  });

  server.on('connection', (conn) => {    
    let remote_address = conn.remoteAddress + ':' + conn.remotePort;  
    console.log('mg_web new client connection from %s', remote_address);

    conn.on('data', (d) => {
      let worker = child_process.fork(mod_name, [1000000], { stdio: ['inherit', 'inherit', 'inherit', 'ipc'] });

      workers.set(worker.pid, worker);

      worker.on('message', message => {
        if (message === 'ready!') {
          worker.send(d, conn);
          return;
        }

        if (message === 'stopping') {
          console.log('master process removing stopped worker from Map');
          workers.delete(worker.pid);
        }

      });
    });
  });

  server.listen(port, () => {    
    console.log('mg_web server listening on %j;', server.address());  
  });
}
else {

  class webserver {
    conn;
    stream_mode;
    request_no;
    buffer;

    stream(sys, binary, options) {
      this.stream_mode = 1;
      return "";
    }

    write(data) {
      let offset = 0;
      offset = set_size(this.buffer, offset, data.length);
      this.conn.write(this.buffer.slice(0, offset), 'binary');
      this.conn.write(data);
    }
  }

  class websocket {
    conn;
    websocket_connection;
    client;

    init(sys, binary, options) {
      let buffer = new Uint8Array(256);
      let offset = 0;
      let res = "HTTP/2 200 OK\r\nBinary: " + binary + "\r\n\r\n";
      let no = sys.get("no");
      offset = block_add_size(buffer, offset, res.length + 5, 0, 0);
      offset = block_add_size(buffer, offset, no, 0, 0);
      for (let i = 0; i < res.length; i++) {
        buffer[offset++] = res.charCodeAt(i);
      }
      this.conn.write(buffer.slice(0, offset), 'binary');
      return "";
    }

    write(data) {
      this.conn.write(data);
    }

    close() {
      // TODO: close the websocket
    }
  }

  process.on( 'SIGINT', function() {
    console.log('*** CTRL & C detected in worker');
  });

  process.on( 'SIGTERM', function() {
    console.log('*** SIGTERM detected in worker');
  });

  const evTarget = new EventTarget();

  let data_properties = { len: 0, type: 0, sort: 0 };
  let handlers = new Map();
  let wsrv = new webserver();
  wsrv.buffer = new Uint8Array(MGWEB_BUFFER_SIZE);
  wsrv.stream_mode = 0;
  let ws = new websocket();
  ws.websocket_connection = false;
  
  function block_add_string(buffer, offset, data, data_len, data_sort, data_type) {
    offset = block_add_size(buffer, offset, data_len, data_sort, data_type);
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

  function block_add_chunk(buffer, offset, data, data_len) {
    offset = set_size(buffer, offset, data_len);
    for (let i = 0; i < data_len; i++) {
      buffer[offset++] = data.charCodeAt(i);
    }
    return offset;
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
    return (offset + 4);
  }

  function set_size(buffer, offset, data_len) {
    buffer[offset + 0] = (data_len >> 0);
    buffer[offset + 1] = (data_len >> 8);
    buffer[offset + 2] = (data_len >> 16);
    buffer[offset + 3] = (data_len >> 24);
    return (offset + 4);
  }

  function get_size(buffer, offset) {
    return ((buffer[offset + 0]) | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24));
  }

  process.on('message', (dbx, conn) => {

    if (dbx === '<<stop>>') {
      console.log('stop child process ' + process.pid);
      evTarget.dispatchEvent(new Event('stop'));
      setTimeout(() => {
        process.exit();
      }, 1000);
      return;
    }

    let remote_address = conn.remoteAddress + ':' + conn.remotePort;  
    console.log('mg_web new worker process created pid=%d; client=%s', process.pid, remote_address);

    // turn the Nagle algorithm off
    conn.setNoDelay();

    // tell the web server what we are
    wsrv.conn = conn;
    let offset = 0;
    let zv = "Node.js " + process.version;
    offset = block_add_string(wsrv.buffer, offset, zv, zv.length, 0, 0);
    conn.write(wsrv.buffer.slice(0, offset));

    conn.on('data', async (data) => {
      let offset = 0;
      let request_no = 0;
      let cgi = new Map();
      let sys = new Map();
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
      let wsfun = "";
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
      //console.log('fun=%s; ctx=%s; param=%s; websocket_connection=%d', fun, ctx, param, websocket_connection);

      // unpack HTTP request data into cgi array, sys array and content (request payload)
      offset = doffset;
      for (let argc = 0; argc < 1000; argc++) {
        len = block_get_size(data, offset, data_properties)
        //console.log(' >>> web item offset=%d; len=%d; type=%d; sort=%d; data=%s', offset, len, data_properties.type, data_properties.sort, data.slice(offset + 5, offset + 5 + len).toString());
        offset += 5;
        if (data_properties.sort === 5) {
          // CGI environment variable
          let d = data.slice(offset, offset + len).toString();
          if (d.startsWith('QUERY_STRING=')) {
            d = d.split('QUERY_STRING=')[1];
            cgi.set('QUERY_STRING', d);
          }
          else {
            d = d.split("=");
            cgi.set(d[0], d[1]);
          }
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
          else if (d[0] === "wsfunction") {
            wsfun = d[1];
          }
          sys.set(d[0], d[1]);
        }
        offset += len;
        if (data_properties.sort === 9) {
          break;
        }
      }
      // websocket client data
      if (ws.websocket_connection === true) {
        // callout to application read method
        ws.client.read(ws, data);
      }
      // websocket initialization
      else if (wsfun != "") {
        if (!handlers.has(wsfun)) {
          try {
            let { handler } = await import(wsfun);
            handlers.set(wsfun, handler);
          }
          catch (err) {
            console.log('Unable to load websocket handler module');
            console.log(err);
          }
        }
        let res = "";
        ws.conn = conn;
        ws.websocket_connection = true;
        try {
          let fn = handlers.get(wsfun);
          sys.set('socket', conn);
          sys.set('evTarget', evTarget);
          if (fn.constructor.name === 'AsyncFunction') {
            res = await fn(ws, cgi, content, sys);
          }
          else {
            res = fn(ws, cgi, content, sys);
          }
        }
        catch (err) {
          console.log('Handler error!');
          console.log(err);
        }
      }
      // regular HTTP request
      else {
        // notify mg_web of data framing protocol in use for response
        offset = 0;
        offset = add_head(wsrv.buffer, offset, 0, 0);
        offset = add_head(wsrv.buffer, offset, request_no, 1);
        conn.write(wsrv.buffer.slice(0, offset), 'binary');

        // ******* call-out to application - START *******
        // CGI variables in 'cgi' array; system variables in 'sys' array; request payload in 'content'
        // generate a resonse in variable 'res'

        if (!handlers.has(fun)) {
          try {
            let { handler } = await import(fun);
            handlers.set(fun, handler);
          }
          catch (err) {
            console.log('Unable to load handler module');
            console.log(err);
            res = "HTTP/1.1 400 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n";
            offset = 0;
            offset = block_add_chunk(wsrv.buffer, offset, res, res.length);
            offset = set_term(wsrv.buffer, offset);
            conn.write(wsrv.buffer.slice(0, offset), 'binary');
          }
        }
        // http://localhost/wsjs.html
        let res = "";
        try {
          let fn = handlers.get(fun);
          sys.set('socket', conn);
          sys.set('evTarget', evTarget);
          if (fn.constructor.name === 'AsyncFunction') {
            res = await fn(wsrv, cgi, content, sys);
          }
          else {
            res = fn(wsrv, cgi, content, sys);
          }
        }
        catch (err) {
          console.log('Handler error!');
          console.log(err);
          res = "HTTP/1.1 400 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n";
        }

        // ******* call-out to application - END *******

        offset = 0;
        if (res.length > 0) {
          offset = block_add_chunk(wsrv.buffer, offset, res, res.length);
        }
        offset = set_term(wsrv.buffer, offset);
        conn.write(wsrv.buffer.slice(0, offset), 'binary');
      }
    });


    //conn.once('close', () => {
    //  console.log('connection closed');
    //});

    conn.on('close', () => {
      console.log('connection closed');
      process.send('stopping');
      evTarget.dispatchEvent(new Event('stop'));
      setTimeout(() => {
        process.exit();
      }, 2000);
    });

    conn.on('error', (err) => {
      console.log('Connection error: %s', err.message);
    });
  });

  process.send('ready!');

}
