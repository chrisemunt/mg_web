// Application launch point for mg_web
//
// Example configuration - in mgweb.conf we have:
//
//    <server NodeJS>
//       type IRIS
//       host 127.0.0.1
//       tcp_port 7777
//    </server>
//
//    <location /mgweb/js >
//       function ./application.mjs
//       servers NodeJS
//    </location >
//
// Start the mg_web_node.js server to listen on TCP port 7777:
//
// node mg_web_node.js 7777
//
// Test it with something like:
//
// curl -X POST -d "{'no': 1, 'name': 'Chris Munt'}" http://127.0.0.1/mgweb/js/ABC/DEFG/
//
//
// simple test function
let handler = function(web_server, cgi, content, sys) {

  // applications can either formulate a complete response and return from this function with that
  // or use the 'web_server.write(data)' method to stream response data back to the web server tier

  console.log('*** application.mjs ****');
  console.log('cgi:');
  console.log(cgi);

  console.log('content:');
  console.log(content);

  console.log('sys:');
  console.log(sys);

  //let res = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nIt Works!\r\n";

  let res = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n";
  cgi.forEach((value, name) => {
    res = res + "CGI variable " + name + " : " + value + "\r\n";
  });

  sys.forEach((value, name) => {
    if (name === 'function' || name === 'path' || name === 'no') {
      res = res + "SYS variable " + name + " : " + value + "\r\n";
    }
  });
  res = res + "Request payload: " + content.toString();

  // write out what we have so far
  web_server.write(res);

  // stream a series of messages back to the client
  for (let i = 0; i < 10; i++) {
    res = "\r\nStreaming content back to the web server - line number: " + i;
    web_server.write(res);
  }

  // return with a final message
  res = "\r\n*** The End ***\r\n";
  return res;
}

export {handler};
