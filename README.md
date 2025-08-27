# Console Chat Application
### Description 
Contains a server and a client, allowing sending messages through a TCP connection. The server can be opened on any available port and supports an unlimited number of clients connecting simultaneously. The connection is built on Boost.Asio TCP sockets.

---
### Usage
1. Build the program with typing

```bash
cmake -S . -B build
cmake --build build
```

at the project folder.

2. Run the `./server.exe` or `./server.exe 1234` if you want to open a server on a specific port (instead of 1234 could be any number 0-65535).
   
3. Run the `./client.exe` or `./client.exe 1234` if your server has a specific port.

4. After connecting to server, you should type your username to be able to write messages. You can open several client instances to talk to yourself or others.

