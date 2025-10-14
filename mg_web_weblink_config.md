# Setting Up *mg_web* to Replace *WebLink*

## Background

One of the primary design aims of the [*mg_web*](https://github.com/chrisemunt/mg_web)
Web gateway was to provide a modern alternative
to the *WebLink* gateway interface that preceded *Cach&eacute; Server Pages* (*CSP*) on
InterSystems' Cach&eacute;.

Many InterSystems customers have built their own Web Applications and REST APIs 
manually on top of *WebLink*.  Others have built Web Applications using the
*WebLink Developer* (*WLD*) framework, and some have also used our *Enterprise
Web Developer* (*EWD*) framework and compiled their *EWD* applications to run
with *WebLink*.

This document will describe in detail how you can set up *mg_web* to replace
WebLink for all the above scenarios.

It is assumed that you will have also installed either the Cach&eacute; or IRIS
database on one of your servers.


## *mg_web* Installation

*mg_web* can run on all the major Web Servers (IIS, NGINX and Apache).  Its installation
and configuration for each of these Web Servers is described in detail
in the [*mg_web* documentation](https://github.com/chrisemunt/mg_web/blob/master/doc/mg_web.pdf).

Please follow the instructions in the above document as follows:

- IIS: Chapter 2.1
- Apache: Chapter 2.2
- NGINX: Chapter 2.3

**NOTE**: If you're using IIS, you might want to edit the Handler Mappings configuration, 
and change the path from ".mgw" to simply "*" - ie an asterisk.  If you make this change,
it means your URL paths just need to be prefixed */mgweb* in order to invoke *mg_web*. Remember
to restart the *World Wide Publishing Service* if you make this change.


## Installing the Database Server Routines

First, make sure you've set *RW* permissions for the *CACHELIB* or *IRISLIB* namespaces on
your Cach&eacute; or IRIS system respectively.

You can do this in the System Management Portal:

        System > Configuration > Local Databases

Select *IRISLIB* or *CACHELIB* and set it to *RW*.  On IRIS this is done by un-checking
the *Mount Read Only* option. Make sure you click the *Save* button to effect the change.

Then see Chapters 3.1 and 3.3.1 of the *mg_web* documentation, which explains how to
install the Database Server Routines.


## Configuring *mg_web* to Emulate *WebLink*

*mg_web* is configured via a file named *mgweb.conf*.  You'll have already seen this discussed
in the *mg_web* documentation, and its location will depend on which Web Server you're
using.

Edit your *mgweb.conf* file to contain the following:

        timeout 30
        log_level eft
        <cgi>
          HTTP*
          AUTH_PASSWORD       
          AUTH_TYPE     
          CONTENT_TYPE
          GATEWAY_INTERFACE
          PATH_TRANSLATED
          REMOTE_ADDR
          REMOTE_HOST
          REMOTE_IDENT            
          REMOTE_USER              
          PATH_INFO
          REQUEST_METHOD
          SERVER_NAME
          SERVER_PORT
          SERVER_PROTOCOL
          SERVER_SOFTWARE
        </cgi>
        <server local>
          type IRIS
          host localhost
          tcp_port 7041
          username _SYSTEM
          password SYS
          namespace USER
        </server>
        <location />
          function web^%zmgweb
          servers local
        </location>



This configuration is suitable for connecting *mg_web* to a single IRIS server over a 
networked connection.  

You should modify the *<server>* section to match your own
specific requirements, eg if you want to use Cach&eacute; instead of IRIS.  

Notice that if you are happy to run your Web Server on the same physical machine as your
IRIS or Cach&eacute; database, you can opt to connect *mg_web* to the very high-performance API
interface of Cach&eacute; or IRIS.

The options you have available are described in detail in the *mg_web* documentation:
see Chapter 4.1.


## The DB Server Interface Function

Notice these lines in the configuration shown in the previous section:

        <location />
         function web^%zmgweb

The location path here is specified as */*.  It's important to understand that this path
is relative to the virtual path/alias you configured for the *mgweb* module.  If you
followed the documentation for *mg_web*, this will be */mgweb*.

So the configuration above instructs *mg_web* to invoke an ObjectScript function - *web^%zmgweb* - 
for all incoming requests with a URL path starting with */mgweb* 

We now, therefore, need to create this function on your Cach&eacute; or IRIS system.

Using *Studio*, create a new ObjectScript routine named *%zmgweb*.

Paste the code below into it:

           ; WebLink/WLD Interface
           ;
        web(%CGIEVAR,%content,%sys)
           ; Normalise the incoming request information to match
           ; how WebLink would hold it
           ;
           new %s,%rc,%def,%ct,%KEY,%KEYHEAD,%MPC
           kill ^MGW("MPC",$J)
           set %s=$$streamascii^%zmgsis(.%sys)
           set %rc=$$nvpair^%zmgsis(.%KEY,$get(%CGIEVAR("QUERY_STRING")))
           set %rc=$$content^%zmgsis(.%KEY,.%KEYHEAD,.%content,.%CGIEVAR)
           set %KEY("MGWLPN")=$get(%sys("server"))
           set %KEY("MGWCHD")=$get(%sys("server_no"))
           set %KEY("MGWUCI")=$$getuci^%zmgsis()
           set %KEY("MGWNSP")=%KEY("MGWUCI")
           set %KEY("MGWLIB")=$get(%CGIEVAR("SCRIPT_NAME"))
           set %def=$data(%content),%ct=$get(%CGIEVAR("CONTENT_TYPE"))
           if %def,%ct'="application/x-www-form-urlencoded",%ct'["boundary=" do
           . if %def=1 set ^MGW("MPC",$job,"CONTENT",1)=%content QUIT
           . merge ^MGW("MPC",$job,"CONTENT")=%content
           . QUIT
           set name="" for  set name=$order(%KEYHEAD(name)) quit:name=""  do
           . new filename,len,data,lendata,n
           . set filename=$piece($piece($get(%KEYHEAD(name,"Content-Disposition")),"filename=""",2),"""",1)
           . set len=$length(filename)
           . set data=$get(%KEY(name))
           . if 'len set %KEY(name)=data
           . if len do
           . . set %MPC(name)=""
           . . set lendata=$length(data)
           . . if '$data(%KEY(name,1)) set ^MGW("MPC",$J,name,1)=data
           . . for n=1:1 quit:'$data(%KEY(name,n))  set data=$get(%KEY(name,n)),lendata=lendata+$length(data),^MGW("MPC",$J,name,n)=data
           . . kill %KEY(name)
           . . set %KEY(name)="1#1~0~"_lendata_"~"_$get(%KEYHEAD(name,"Content-Type"))_"~"_filename
           . . QUIT
           . QUIT
           do weblink(.%CGIEVAR,.%KEY,.%MPC)
           QUIT %s
           ;
        weblink(%CGIEVAR,%KEY,%MPC)
           new (%CGIEVAR,%KEY,%MPC)
           ;
           set %METHOD=$get(%CGIEVAR("REQUEST_METHOD"))
           ;
           ; Intercept Event Broker calls (XMLHTTP calls fired from mgwx.js)
           ; 
           if $data(%KEY("MGWreq")) do eventbroker(.%CGIEVAR,.%KEY) QUIT
           ;
           ; Intercept URL paths for WebLink Developer Applications
           ;
           if $data(%KEY("wlapp")) do ^%wld QUIT
           ;
           ; Intercept URL paths using WLD to EWD Bridge
           ;
           i $get(%KEY("MGWAPP"))="ewdwl" do runPage^%zewdWLD QUIT
           ;
           ; Otherwise fallback to WebLink custom routine
           ;
           do ^%ZMGW2
           QUIT
           ;
        eventbroker(%CGIEVAR,%KEY)
           set %REQUEST(0)=0 if $data(%KEY("MGWmod")) set %REQUEST(0)=%KEY("MGWmod")
           set req=$get(%KEY("MGWreq"))
           set %KEY("MGWreq")=$piece(req,"^",1,3),req=$piece(req,"^",4,9999)
           set %REQUEST(-1)=%KEY("MGWreq")
           set %REQUEST=$piece(req,"(",1),req=$piece(req,"(",2,9999),req=$extract(req,1,$length(req)-1)
           for i=1:1:$length(req,$char(1)) set %REQUEST(i)=$piece(req,$char(1),i)
           do HTTPA^%mgwj
           do eventbroker1(.%CGIEVAR,.%KEY,.%REQUEST)
           do END^%mgwj
           do HTTPZ^%mgwj
           QUIT
           ;
        eventbroker1(%CGIEVAR,%KEY,%REQUEST)
           new (%CGIEVAR,%KEY,%REQUEST)
           if %REQUEST["wlapp." do ^%wldja QUIT
           do JAVA^%ZMGW2
           QUIT
           ;

Save and compile this ObjectScript routine.  This routine calls procedures in routine %mgwj. If you don't have this routine, the code for the required procedures is as follows.

        HTTPA ; HTML wrapper - top
           write "HTTP/1.1 200 OK"_$char(13,10)
           write "Content-type: text/html"_$char(13,10)
           write "Connection: close"_$char(13,10)
           write $char(13,10)
           write "<HTML>"_$char(13,10)
           write "<HEAD><TITLE>WebLink - Private Command Shell</TITLE></HEAD>"_$char(13,10)
           write "<BODY>"_$char(13,10)
           QUIT
           ;
        HTTPZ ; HTML wrapper - tail
           write "</BODY>"_$char(13,10)
           write "</HTML>"_$char(13,10)
           QUIT
           ;
        END ; Send the end-of-data marker to the client
           write $char(2,2,10)
           QUIT
           ;
        sendhead(head) ; Send record header to the client
           new i,del
           if $data(%TXT(1)) set head="",del="" for i=1:1 quit:'$data(%TXT(i))  set head=head_del_%TXT(i),del=$char(1)
           write $char(2)_$get(head)_$char(10)
           QUIT
           ;
        sendline(data) ; Send a line of data to the client
           new i,del
           if $data(%TXT(1)) set data="",del="" for i=1:1 quit:'$data(%TXT(i))  set data=data_del_%TXT(i),del=$char(1)
           write $get(data)_$char(10)
           QUIT
           ;


If you already have a custom ^%ZMGW2 routine, you should leave it alone and *mg_web* will invoke it
for you.

However, if you don't already have a *^%ZMGW2* ObjectScript routine, create one using
Studio and paste the following into it.  This will ensure that you have a
default "fallback" page appearing if the URL doesn't match anything else.  Feel free
to modify the page generation logic:

        %ZMGW2 ; WebLink launch routine
           write "HTTP/1.1 200 OK"_$char(13,10)
           write "Content-type: text/html"_$char(13,10)
           write "Connection: close"_$char(13,10)
           write $char(13,10)
           write "<html>"_$char(13,10)
           write "<head><title>It Works</title></head>"_$char(13,10)
           write "<body>"_$char(13,10)
           write "<h1>It Works!</h1>"_$char(13,10)
           set ver=$$v^%zmgsis()
           write "mg_web Version: v"_$piece(ver,".",1,3)_" ("_$piece(ver,".",4)_")"_"<br /><br />"_$char(13,10)
           write "Database: "_$zversion_"<br />"_$char(13,10)
           write "Namespace: "_$$getuci^%zmgsis()_"<br />"_$char(13,10)
           write "Date/time: "_$$getdatetime^%zmgsis($horolog)_"<br />"_$char(13,10)
           write "</body>"_$char(13,10)
           write "</html>"_$char(13,10)
           QUIT
           ;
        JAVA ; Process in-form M Method (Invoked by the mgwx.js AJAX module)
           ; Receive request in %REQUEST
           ;    Associated parameters in %REQUEST(1->n)
           ;
           ; Send each line of response as sendline^%mgwj(<Data>)
           ;    Finally, quit from this code module
           ;
           do sendline^%mgwj("This is the response to '"_%REQUEST_"' from the M System")
           QUIT



Save and compile this ObjectScript routine.


## *EWD* Configuration

If you have used *EWD* with a *compilation target* of *WebLink* (ie *wl*), you're probably
using the default configuration that assumes that the incoming URL paths start with:

        /scripts/mgwms32.dll

You need to change the EWD configuration to generate *mg_web* URL paths that start with:

        /mgweb


To do this, open a Terminal session and type:

        s ^zewd("config","RootURL","wl")="/mgweb"

Then recompile your EWD application(s) using:

        d compileAll^%zewdAPI(appName,,"wl")

         where appName is the name of your EWD application


## That Should be It!

*mg_web* should now be set up to emulate WebLink for any and all of your *WebLink*, *WLD*
and *EWD* applications.

Note that although *EWD* applications must be recompiled, *WLD* applications should run
without any changes being needed:

To start the *Weblink Developer* Management Console:

        http://localhost/mgweb?wlapp=wldev

To start a WLD application named *test*:

        http://localhost/mgweb?wlapp=test


To start an *EWD* application named *test* using its first page named *page1*:

        http://localhost/mgweb?MGWAPP=ewdwl&app=test&page=page1


