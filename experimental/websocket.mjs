// Application launch point for a mg_web websocket
//
// Example of adding a websocket to the configuration - in mgweb.conf we have:
//
//    <server NodeJS>
//       type IRIS
//       host 127.0.0.1
//       tcp_port 7777
//    </server>
//
//    <location /mgweb/js >
//       function ./application.mjs
//       websocket websocket.mgw ./websocket.mjs
//       servers NodeJS
//    </location >
//
// In the web page, the path to the websocket would be: /mgweb/js/websocket.mgw
//
// simple test websocket

let handler = function (ws_server, cgi, content, sys) {

  // Define a class to represent client data
  class ws_client {

    // Mandatory read method to accept client data 
    read(ws_server, data) {
      // Acknowledge data from client
      ws_server.conn.write('Data received from Client: ' + data);
    }
  }

  // Initialise server side
  ws_server.init(sys, 0, "");

  // Create instance of client class for this connection
  ws_server.client = new ws_client();

  // Send inital message to client
  ws_server.write('Hello from Server');

  return "";
}

export {handler};
