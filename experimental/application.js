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
//       function web
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
function web(cgi, content, sys) {
   let res = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n";
   for (argc = 0; argc < cgi.length; argc++) {
      res = res + "CGI variable " + cgi[argc].name + " : " + cgi[argc].value + "\r\n";
   }
   for (argc = 0; argc < sys.length; argc++) {
      if (sys[argc].name === 'function' || sys[argc].name === 'path' || sys[argc].name === 'no') {
         res = res + "SYS variable " + sys[argc].name + " : " + sys[argc].value + "\r\n";
      }
   }
   res = res + "Request payload: " + content.toString();
   return res;
}

module.exports = {
   web: web
}
