how to use :
1. The server of functioning WAS is listening to 3001, so you can't change port on html document
2. The server of functioning WebContainer Server is listening to 3010 and it can be able to change port number
3. If you want to try this, you just execute project 'SimpleHttpServer.WebApp'. open a browser and input http://127.0.0.1:3010

Key Projects
1. was_dll (for 'WebSocketServer' project)
2. WebSocketServer
3. SimpleHttpServer.WebApp


Referenced List
1. DARKCODE (YOUTUBE) : https://www.youtube.com/channel/UCD3KVjbb7aq2OiOffuungzw
2. MDN : https://developer.mozilla.org/
3. SimpleHttpServer-master : https://github.com/jeske/SimpleHttpServer
4. StackOverflow : https://stackoverflow.com/
5. Goole Search



Used Language : C++(used for C# Engine at WebSocketServer), C#


My Project
1. WebContainerServer : https://github.com/jangsungLee/SimpleHttpServer-master/tree/master/SimpleHttpServer.WebApp
2. WebSocketServer : https://github.com/jangsungLee/SimpleHttpServer-master/tree/master/WebSocketServer
3. WebPage : https://github.com/jangsungLee/SimpleHttpServer-master/tree/master/SimpleHttpServer/Resources
4. Canceled to make project --> WebContainerServer Service (Because i do not have the experience to make service
and i have to do the others like my study)



Revised Referenced Library : SimpleHttpServer-master
-->Revised Content
   1. add to be able to find resource file(add Resource Class)
   2. add the SimpleHttpServer.dll which was 'in' bin/Debug to 'in' bin/Release
   3. others
   


Note.
When you make the packet including TCP/UDP(like DNS, WebSocket, ...), You Must Calculate all of Checksum(IP,TCP or UDP) yourself
even though you input and the value is calculated (when the checksum is 0). Because I only focus to concern the ICMP Checksum right.
The ICMP Packet and pure IP Packet will be calculated correctly.
