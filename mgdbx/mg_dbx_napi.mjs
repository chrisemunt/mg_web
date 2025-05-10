//
//   ----------------------------------------------------------------------------
//   | Package:     mg_dbx_napi                                                 |
//   | OS:          Unix/Windows                                                |
//   | Description: An Interface to InterSystems Cache/IRIS and YottaDB         |
//   | Author:      Chris Munt cmunt@mgateway.com                               |
//   |                         chris.e.munt@gmail.com                           |
//   | Copyright(c) 2019 - 2023 MGateway Ltd                                    |
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

const DBX_VERSION_MAJOR     = 1;
const DBX_VERSION_MINOR     = 1;
const DBX_VERSION_BUILD     = 3;

const DBX_DSORT_INVALID     = 0;
const DBX_DSORT_DATA        = 1;
const DBX_DSORT_SUBSCRIPT   = 2;
const DBX_DSORT_GLOBAL      = 3;
const DBX_DSORT_EOD         = 9;
const DBX_DSORT_STATUS      = 10;
const DBX_DSORT_ERROR       = 11;

const DBX_DTYPE_NONE        = 0;
const DBX_DTYPE_STR         = 1;
const DBX_DTYPE_STR8        = 2;
const DBX_DTYPE_STR16       = 3;
const DBX_DTYPE_INT         = 4;
const DBX_DTYPE_INT64       = 5;
const DBX_DTYPE_DOUBLE      = 6;
const DBX_DTYPE_OREF        = 7;
const DBX_DTYPE_NULL        = 10;

const DBX_CMND_OPEN         = 1;
const DBX_CMND_CLOSE        = 2;
const DBX_CMND_NSGET        = 3;
const DBX_CMND_NSSET        = 4;

const DBX_CMND_GSET         = 11;
const DBX_CMND_GGET         = 12;
const DBX_CMND_GNEXT        = 13;
const DBX_CMND_GPREVIOUS    = 14;
const DBX_CMND_GDELETE      = 15;
const DBX_CMND_GDEFINED     = 16;
const DBX_CMND_GINCREMENT   = 17;
const DBX_CMND_GLOCK        = 18;
const DBX_CMND_GUNLOCK      = 19
const DBX_CMND_GMERGE       = 20

const DBX_CMND_FUNCTION     = 31;

const DBX_CMND_CCMETH       = 41;
const DBX_CMND_CGETP        = 42;
const DBX_CMND_CSETP        = 43;
const DBX_CMND_CMETH        = 44;
const DBX_CMND_CCLOSE       = 45;

const DBX_CMND_TSTART       = 61;
const DBX_CMND_TLEVEL       = 62;
const DBX_CMND_TCOMMIT      = 63;
const DBX_CMND_TROLLBACK    = 64;

const DBX_INPUT_BUFFER_SIZE = 3641145; // or 32768

let mgdbx = function(dbx) {

  class server {
     type = "";
     path = "";
     host = "";
     tcp_port = 0;
     username = "";
     password = "";
     namespace = "";
     env_vars = "";
     debug = "";
     server = "";
     server_software = "";
     timeout = 60;
     init = 0;
     index = 0;
     buffer = [0, 0, 0, 0, 0, 0, 0, 0];
     buffer_size = [0, 0, 0, 0, 0, 0, 0, 0];

     constructor(...args) {
        this.buffer[0] = new Uint8Array(DBX_INPUT_BUFFER_SIZE);
        this.buffer_size[0] = DBX_INPUT_BUFFER_SIZE;
        return;
     }

     get_buffer() {
        let bidx = 0;
        return bidx;
     }

     release_buffer(bidx) {
        return bidx;
     }

     version() {
        return dbx.version();
     }

     dbversion() {
        if (this.init === 0) {
           const ret = dbx.init();
           this.init ++;
        }
        return dbx.dbversion();
     }

     open(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_OPEN, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           const ret = dbx.init();
           this.init ++;
        }

        request.argc = args.length;
        if (request.argc > 1) {
           if (typeof args[request.argc - 1] === "function") {
              request.async = 1;
              request.argc --;
           }
        }

        let bidx = this.get_buffer();
        if (args.length > 0) {
           if (typeof args[0] === 'object') {
              if (args[0].hasOwnProperty('type')) {
                 this.type = args[0].type;
              }
              if (args[0].hasOwnProperty('path')) {
                 this.path = args[0].path;
              }
              if (args[0].hasOwnProperty('host')) {
                 this.host = args[0].host;
              }
              if (args[0].hasOwnProperty('tcp_port')) {
                 this.tcp_port = args[0].tcp_port;
              }
              if (args[0].hasOwnProperty('username')) {
                 this.username = args[0].username;
              }
              if (args[0].hasOwnProperty('password')) {
                 this.password = args[0].password;
              }
              if (args[0].hasOwnProperty('env_vars')) {
                 if (typeof args[0].env_vars === 'object') {
                    let envvars = '';
                    for (const name in args[0].env_vars) {
                       envvars = envvars + name + '=' + args[0].env_vars[name] + '\n';
                    }
                    envvars = envvars + '\n';
                    this.env_vars = envvars;
                 }
                 else {
                    this.env_vars = args[0].env_vars;
                 }
              }
              if (args[0].hasOwnProperty('debug')) {
                 this.debug = args[0].debug;
              }
              if (args[0].hasOwnProperty('timeout')) {
                 this.timeout = args[0].timeout;
              }
           }
        }

        offset = block_add_size(this.buffer[bidx], offset, offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.buffer.length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.index, DBX_DSORT_DATA, DBX_DTYPE_INT);

        offset = block_add_string(this.buffer[bidx], offset, this.type, this.type.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, this.path, this.path.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, this.host, this.host.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, this.tcp_port.toString(), this.tcp_port.toString().length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_string(this.buffer[bidx], offset, this.username, this.username.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, this.password, this.password.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, this.namespace, this.namespace.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, "", 0, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, "", 0, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, this.debug, this.debug.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, this.env_vars, this.env_vars.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, this.server, this.server.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, this.server_software, this.server_software.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
        offset = block_add_string(this.buffer[bidx], offset, this.timeout.toString(), this.timeout.toString().length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_string(this.buffer[bidx], offset, "", 0, DBX_DSORT_EOD, DBX_DTYPE_STR);
        add_head(this.buffer[bidx], 0, offset, request.command);

        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }

        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     close(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_CLOSE, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        request.argc = args.length;
        if (request.argc > 1) {
           if (typeof args[request.argc - 1] === "function") {
              request.async = 1;
              request.argc --;
           }
        }

        let bidx = this.get_buffer();
        offset = block_add_size(this.buffer[bidx], offset, offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.buffer[bidx].length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.index, DBX_DSORT_DATA, DBX_DTYPE_INT);

        offset = block_add_string(this.buffer[bidx], offset, "", 0, DBX_DSORT_EOD, DBX_DTYPE_STR);
        add_head(this.buffer[bidx], 0, offset, request.command);

        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }

        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     current_namespace(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_NSGET, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        request.argc = args.length;
        if (request.argc > 1) {
           if (typeof args[request.argc - 1] === "function") {
              request.async = 1;
              request.argc --;
           }
        }

        let bidx = this.get_buffer();
        offset = block_add_size(this.buffer[bidx], offset, offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.buffer[bidx].length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.index, DBX_DSORT_DATA, DBX_DTYPE_INT);

        if (args.length > 0) {
           request.command = DBX_CMND_NSSET;
           offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
           const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        }

        request.command = DBX_CMND_NSGET;
        offset = 0;
        offset = block_add_size(this.buffer[bidx], offset, offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.buffer[bidx].length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.index, DBX_DSORT_DATA, DBX_DTYPE_INT);

        offset = block_add_string(this.buffer[bidx], offset, "", 0, DBX_DSORT_EOD, DBX_DTYPE_STR);
        add_head(this.buffer[bidx], 0, offset, request.command);

        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }

        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     set(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GSET, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     get(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GGET, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     delete(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GDELETE, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     defined(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GDEFINED, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     next(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GNEXT, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     previous(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GPREVIOUS, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     increment(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GINCREMENT, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     lock(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GLOCK, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     unlock(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GUNLOCK, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     tstart(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_TSTART, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        request.argc = args.length;
        if (request.argc > 1) {
           if (typeof args[request.argc - 1] === "function") {
              request.async = 1;
              request.argc --;
           }
        }

        let bidx = this.get_buffer();
        offset = block_add_size(this.buffer[bidx], offset, offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.buffer[bidx].length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.index, DBX_DSORT_DATA, DBX_DTYPE_INT);

        offset = block_add_string(this.buffer[bidx], offset, "", 0, DBX_DSORT_EOD, DBX_DTYPE_STR);
        add_head(this.buffer[bidx], 0, offset, request.command);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     tlevel(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_TLEVEL, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        request.argc = args.length;
        if (request.argc > 1) {
           if (typeof args[request.argc - 1] === "function") {
              request.async = 1;
              request.argc --;
           }
        }

        let bidx = this.get_buffer();
        offset = block_add_size(this.buffer[bidx], offset, offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.buffer[bidx].length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.index, DBX_DSORT_DATA, DBX_DTYPE_INT);

        offset = block_add_string(this.buffer[bidx], offset, "", 0, DBX_DSORT_EOD, DBX_DTYPE_STR);
        add_head(this.buffer[bidx], 0, offset, request.command);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     tcommit(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_TCOMMIT, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        request.argc = args.length;
        if (request.argc > 1) {
           if (typeof args[request.argc - 1] === "function") {
              request.async = 1;
              request.argc --;
           }
        }

        let bidx = this.get_buffer();
        offset = block_add_size(this.buffer[bidx], offset, offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.buffer[bidx].length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.index, DBX_DSORT_DATA, DBX_DTYPE_INT);

        offset = block_add_string(this.buffer[bidx], offset, "", 0, DBX_DSORT_EOD, DBX_DTYPE_STR);
        add_head(this.buffer[bidx], 0, offset, request.command);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     trollback(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_TROLLBACK, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        request.argc = args.length;
        if (request.argc > 1) {
           if (typeof args[request.argc - 1] === "function") {
              request.async = 1;
              request.argc --;
           }
        }

        let bidx = this.get_buffer();
        offset = block_add_size(this.buffer[bidx], offset, offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.buffer[bidx].length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(this.buffer[bidx], offset, this.index, DBX_DSORT_DATA, DBX_DTYPE_INT);

        offset = block_add_string(this.buffer[bidx], offset, "", 0, DBX_DSORT_EOD, DBX_DTYPE_STR);
        add_head(this.buffer[bidx], 0, offset, request.command);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }

     function(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_FUNCTION, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }
   
     sleep(msecs) {
        let result = 0;

        result = dbx.sleep(msecs);
        return result;
     }

     classmethod(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_CCMETH, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.init === 0) {
           return "";
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        if (request.async) {
           async_command(this, this.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        if (result.type === DBX_DTYPE_OREF) {
           const cls = new mclass(this);
           cls.class_name = args[0];
           cls.oref = request.result_data;
           return cls;
        }

        return request.result_data;
     }

     benchmark(...args) {
        let i = 0;
        let data = "";

        if (this.init === 0) {
           const ret = dbx.init();
           this.init ++;
        }

        let argc = args.length;
        if (argc < 1) {
           return data;
        }
        let context = 0;
        if (argc > 1) {
           context = args[1];
        }

        let bidx = this.get_buffer();
        let istring = args[0];

        for (i = 0; i < istring.length; i ++) {
           this.buffer[bidx][i] = istring.charCodeAt(i);
        }
        this.buffer[bidx][i] = 0;

        if (context === 1) {
           const pdata = dbx.benchmark(this.buffer[bidx], i, 0, 0);
           data = pdata;
        }
        else {
           data = "output string";
        }
        this.release_buffer(bidx);

        return data;
     }

     benchmarkex(...args) {
        let offset = 0;
        let request = { command: 0, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };
        let data = 0;

        if (this.init === 0) {
           return data;
        }

        let argc = args.length;
        if (argc < 1) {
           return data;
        }
        let command = 0;
        if (argc === 1) {
           request.command = DBX_CMND_GGET;
        }
        else if (argc === 3) {
           request.command = DBX_CMND_GSET;
        }
        if (request.command === 0) {
           return data;
        }

        let bidx = this.get_buffer();
        offset = pack_arguments(this.buffer[bidx], offset, this.index, args, request, 0);
        const pdata = dbx.benchmarkex(this.buffer[bidx], offset, request.command, 0);
        get_result(this.buffer[bidx], pdata, request);
        this.release_buffer(bidx);

        return request.result_data;
     }
  }

  class mglobal {
     db;
     global_name = "";
     base_buffer;
     base_offset = 0;

     constructor(db, ...args) {
        let request = { command: 0, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };
        this.db = db;
        this.base_buffer = new Uint8Array(DBX_INPUT_BUFFER_SIZE);
        this.base_offset = 0;
        this.base_offset = pack_arguments(this.base_buffer, this.base_offset, this.db.index, args, request, 0);
        this.base_offset -= 5;
        if (args.length > 0) {
           this.global_name = args[0];
        }
        return;
     }

     set(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GSET, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     get(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GGET, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     delete(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GDELETE, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     defined(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GDEFINED, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     next(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GNEXT, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     previous(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GPREVIOUS, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     increment(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GINCREMENT, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     lock(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GLOCK, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     unlock(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_GUNLOCK, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     merge(...args) {
        let offset = 0;
        let sort = 0;
        let str = "";
        let request = { command: DBX_CMND_GMERGE, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);

        for (let argn = 0; argn < args.length; argn ++) {
           if (typeof args[argn] === "object" && args[argn].constructor.name == "mglobal") {
              offset = block_copy(this.db.buffer[bidx], offset, args[argn].base_buffer, 15, args[argn].base_offset);
           }
           else if (typeof args[argn] === "string") {
              str = args[argn];
              offset = block_add_string(this.db.buffer[bidx], offset, str, str.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
           }
           else {
              str = args[argn].toString();
              offset = block_add_string(this.db.buffer[bidx], offset, str, str.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
           }
        }

        offset = block_add_string(this.db.buffer[bidx], offset, "", 0, DBX_DSORT_EOD, DBX_DTYPE_STR)
        add_head(this.db.buffer[bidx], 0, offset, request.command);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     reset(...args) {
        let request = { command: 0, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };
        this.base_offset = 0;
        this.base_offset = pack_arguments(this.base_buffer, this.base_offset, this.db.index, args, request, 0);
        this.base_offset -= 5;
        if (args.length > 0) {
           this.global_name = args[0];
        }
        return;
     }

  }

  class mclass {
     db;
     class_name = "";
     oref = "";
     base_buffer;
     base_offset = 0;

     constructor(db, ...args) {
        this.db = db;
        this.base_buffer = new Uint8Array(DBX_INPUT_BUFFER_SIZE);
        this.base_offset = 0;

        this.base_offset = block_add_size(this.base_buffer, this.base_offset, this.base_offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        this.base_offset = block_add_size(this.base_buffer, this.base_offset, this.base_buffer.length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        this.base_offset = block_add_size(this.base_buffer, this.base_offset, db.index, DBX_DSORT_DATA, DBX_DTYPE_INT);

        if (args.length > 0) {
           this.class_name = args[0];
           this.base_offset = block_add_string(this.base_buffer, this.base_offset, this.class_name, this.class_name.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
           if (args.length > 1 && this.db.init > 0) {
              let offset = 0;
              let request = { command: DBX_CMND_CCMETH, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };
              let bidx = this.db.get_buffer();

              offset = pack_arguments(this.db.buffer[bidx], offset, db.index, args, request, 0);
              const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
              get_result(this.db.buffer[bidx], pdata, request);
              this.db.release_buffer(bidx);

              if (result.type === DBX_DTYPE_OREF) {
                 this.oref = request.result_data;
              }
           }
        }
        return;
     }

     classmethod(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_CCMETH, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, this.base_offset);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        if (result.type === DBX_DTYPE_OREF) {
           this.oref = request.result_data;
        }
        return request.result_data;
     }

     method(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_CMETH, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, 15);
        offset = block_add_string(this.db.buffer[bidx], offset, this.oref, this.oref.length, DBX_DSORT_DATA, DBX_DTYPE_OREF);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     getproperty(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_CGETP, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, 15);
        offset = block_add_string(this.db.buffer[bidx], offset, this.oref, this.oref.length, DBX_DSORT_DATA, DBX_DTYPE_OREF);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     setproperty(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_CSETP, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, 15);
        offset = block_add_string(this.db.buffer[bidx], offset, this.oref, this.oref.length, DBX_DSORT_DATA, DBX_DTYPE_OREF);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     closeinstance(...args) {
        let offset = 0;
        let request = { command: DBX_CMND_CCLOSE, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };

        if (this.db.init === 0) {
           return "";
        }

        let bidx = this.db.get_buffer();
        offset = block_copy(this.db.buffer[bidx], offset, this.base_buffer, 0, 15);
        offset = block_add_string(this.db.buffer[bidx], offset, this.oref, this.oref.length, DBX_DSORT_DATA, DBX_DTYPE_OREF);
        offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 1);
        if (request.async) {
           async_command(this.db, this.db.buffer[bidx], offset, request, 0, args[request.argc]);
           return null;
        }
        const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
        get_result(this.db.buffer[bidx], pdata, request);
        this.db.release_buffer(bidx);

        return request.result_data;
     }

     reset(...args) {
        this.base_offset = 0;

        this.base_offset = block_add_size(this.base_buffer, this.base_offset, this.base_offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        this.base_offset = block_add_size(this.base_buffer, this.base_offset, this.base_buffer.length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        this.base_offset = block_add_size(this.base_buffer, this.base_offset, this.db.index, DBX_DSORT_DATA, DBX_DTYPE_INT);

        if (args.length > 0) {
           this.class_name = args[0];
           this.base_offset = block_add_string(this.base_buffer, this.base_offset, this.class_name, this.class_name.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
           if (args.length > 1 && this.db.init > 0) {
              let offset = 0;
              let request = { command: DBX_CMND_CCMETH, argc: 0, async: 0, result_data: "", error_message: "", type: 0 };
              let bidx = this.db.get_buffer();

              offset = pack_arguments(this.db.buffer[bidx], offset, this.db.index, args, request, 0);
              const pdata = dbx.command(this.db.buffer[bidx], offset, request.command, 0);
              get_result(this.db.buffer[bidx], pdata, request);
              this.db.release_buffer(bidx);

              if (result.type === DBX_DTYPE_OREF) {
                 this.oref = request.result_data;
              }
           }
        }
        return;
     }
  }

  async function async_command(db, buffer, buffer_size, request, context, callback) {
     let promise = new Promise((resolve, reject) => {
        const pdata = dbx.command(buffer, buffer_size, request.command, context);
        get_result(buffer, pdata, request);
        db.release_buffer(0);
        resolve(request.result_data);
     });
     let result_data = await promise
     if (request.error_message === "") {
        callback(false, result_data);
     }
     else {
        callback(true, result_data);
     }

     return;
  }

  function pack_arguments(buffer, offset, index, args, request, context) {
     let str = "";

     if (context === 0) {
        offset = block_add_size(buffer, offset, offset, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(buffer, offset, buffer.length, DBX_DSORT_DATA, DBX_DTYPE_INT);
        offset = block_add_size(buffer, offset, index, DBX_DSORT_DATA, DBX_DTYPE_INT);
     }

     request.argc = args.length;
     if (request.argc > 1) {
        if (typeof args[request.argc - 1] === "function") {
           request.async = 1;
           request.argc --;
        }
     }

     for (let argn = 0; argn < request.argc; argn ++) {
        //console.log(argn, " = ", args[argn], " : ", typeof args[argn]);
        if (typeof args[argn] === "string")
           str = args[argn];
        else
           str = args[argn].toString();
        if (argn == 0)
           offset = block_add_string(buffer, offset, str, str.length, DBX_DSORT_GLOBAL, DBX_DTYPE_STR);
        else
           offset = block_add_string(buffer, offset, str, str.length, DBX_DSORT_DATA, DBX_DTYPE_STR);
     }

     offset = block_add_string(buffer, offset, "", 0, DBX_DSORT_EOD, DBX_DTYPE_STR)
     add_head(buffer, 0, offset, request.command);

     return offset;
  }

  function get_result(pbuffer, pdata, request) {
     let data_properties = { len: 0, type: 0, sort: 0 };

     block_get_size(pbuffer, 0, data_properties);
     //console.log("mg_dbx_napi.js data_view data properties => ", data_properties);

     if (data_properties.sort === DBX_DSORT_ERROR) {
        if (data_properties.len === 0) {
           request.error_message = ""
        }
        else {
           request.error_message = pdata;
        }
     }
     else {
        if (data_properties.len === 0) {
           request.result_data = ""
        }
        else {
           request.result_data = pdata;
        }
     }
     request.type = data_properties.type;
     return request.result_data;
  }

  function block_copy(buffer_to, offset, buffer_from, from, to) {
     for (let i = from; i < to; i ++) {
        buffer_to[offset ++] = buffer_from[i];
     }
     return offset;
  }

  function block_add_string(buffer, offset, data, data_len, data_sort, data_type) {
     offset = block_add_size(buffer, offset, data_len, data_sort, data_type);
     for (let i = 0; i < data_len; i ++) {
        buffer[offset ++] = data.charCodeAt(i);
     }
     return offset;
  }

  function block_add_size(buffer, offset, data_len, data_sort, data_type) {
     buffer[offset + 0] = (data_len >> 0);
     buffer[offset + 1] = (data_len >> 8);
     buffer[offset + 2] = (data_len >> 16);
     buffer[offset + 3] = (data_len >> 24);
     buffer[offset + 4] = ((data_sort * 20) + data_type);
     return (offset + 5);
  }

  function add_head(buffer, offset, data_len, cmnd) {
     buffer[offset + 0] = (data_len >> 0);
     buffer[offset + 1] = (data_len >> 8);
     buffer[offset + 2] = (data_len >> 16);
     buffer[offset + 3] = (data_len >> 24);
     buffer[offset + 4] = cmnd;
     return (offset + 5);
  }

  function block_get_size(buffer, offset, data_properties) {
     data_properties.len = ((buffer[offset + 0]) | (buffer[offset + 1] << 8) | (buffer[offset + 2] << 16) | (buffer[offset + 3] << 24));
     data_properties.sort = buffer[4];
     data_properties.type = data_properties.sort % 20;
     data_properties.sort = Math.floor(data_properties.sort / 20);
     return data_properties;
  }

  return {
     server,
     mglobal,
     mclass
  };
}
export {mgdbx};
