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
let handler = function(cgi, content, sys) {

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
  return res;
}

export {handler};
