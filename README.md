### Introduction

**wpp vs messenger**: delete messages as soon as they are received vs keeps them forever until deleted by user

**end-to-end encryption**: middle manager cannot read messages

### Parts

**database**: Cassandra

**load balancer**: lvl 3,4 or 7 - talks to the server nodes (n1, n2, n3, ...) and the database

**distributed cache**: Keep temporary data for the user - Redis

**conection types**:
- **HTTP**: 
    - HTTP
    - One-directional
    - Isolated requests, no shared context
    - Server cannot initiate a connection by itself
    - Bad for real-time applications
- **Websocket**: 
    - TCP
    - Bi-directional
    - A single long lived TCP connection
    - Optimal for real-time applications
- **Bosh**: 
    - HTTP
    - Bi-directional
    - Simulated bi-directional conection using HTTP
    - Two HTTP requests, one for sending and one for receiveing
    - Utilizes pooling, where the server hold the request until time limit is reached or new data is found. Then, he sends a response and user A can initiate a new request.

    
