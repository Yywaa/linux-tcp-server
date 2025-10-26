# Linux TCP Server

A lightweight, modular TCP server written in **C/C++** using **POSIX sockets** on **Linux**.  
This project was built as part of my journey to learn and practice network programming, focusing on socket communication, multithreading, and client management.


## 🚀 Features

- **TCPNewConnectionService** – handles new client connections
- **TCPClient** – manages individual client sockets (read/write)
- **TCPServerController** – controls server start, stop, and thread coordination
- **TcpClientDBManager** – maintains a thread-safe list of connected clients
- **TcpMsgDemarcar** – performs message framing and boundary detection

## 🧠 Key Concepts Practiced

- POSIX **socket APIs** (`socket`, `bind`, `listen`, `accept`, `recv`, `send`)
- **Multithreading** with `pthread` and read-write locks
- **Concurrent client handling** and thread-safe data structures
- Basic **TCP message framing and demarcation**
- Organized, modular C++ class design for network systems

🏷️ Tags

C++ Linux TCP POSIX Sockets Networking Multithreading Server
