# EtherFlyer Exchange Server

EtherFlyer Exchange Server is a trading backend with high-speed performance, designed for cryptocurrency exchanges. It can support up to 10000 trades every second and real-time user/market data notification though websocket.

## Architecture

![architecture](https://user-images.githubusercontent.com/33028500/32134960-134bcb4e-bc32-11e7-9dbf-1017adc4e893.jpg)

For this project, it is marked as Server in this picture.

## code structure

**Require system**

* MySQL: For saving operation log, user balance history, order history and trade history.

* Redis: A redis sentinel group is for saving market data.

* Kafka: A message system.

**Base library**

* network: An event base and high performance network programming library, easily supporting [1000K TCP connections](http://www.kegel.com/c10k.html). Include TCP/UDP/UNIX SOCKET server and client implementation, a simple timer, state machine, thread pool. 

* utils: Some basic library, including log, config parse, some data structure and http/websocket/rpc server implementation.

**Modules**

* matchengine: This is the most important part for it records user balance and executes user order. It is in memory database, saves operation log in MySQL and redoes the operation log when start. It also writes user history into MySQL, push balance, orders and deals message to kafka.

* marketprice: Reads message(s) from kafka, and generates k line data.

* readhistory: Reads history data from MySQL.

* accesshttp: Supports a simple HTTP interface and hides complexity for upper layer.

* accwssws: A websocket server that supports query and pushes for user and market data. By the way, you need nginx in front to support wss.

* alertcenter: A simple server that writes FATAL level log to redis list so we can send alert emails.
