# uLog

uLog is simple log system based on UDP socket. When it works, it always include two entities:
1. ulog client generates log message and uses UDP socket to send the message stream to another host, i.e., ulog server
2. ulog server receive message stream from UDP socket. Then it parses the stream to get actual log and prints the log  on screen or outputs to a text file.

# Sample and Usage
## ulog client
[ulog_client.cpp](./sample/ulog_client.cpp) shows how to write a client code. It first initilizes ulog as below:
```
    uLog ulog;
    ulog.set_client(uLogDebug);
```

Then it calls `uLOG` marco to generate and send log message by UDP socket like this:
```
uLOG(uLogDebug, "Module", "s8=", s8);
```
`uLogDebug` is log level. `"Module"` is a user defined string to seperate different modules in a big project. `"s8=", s8` is log message main body.

**Notes:** A log message main body of `uLOG` marco can accept at most **16** input arguments.

### sync()
In order to reduce ulog overhead, when a log message is generated by calling `uLOG`, the generated message can still store in ulog internal buffer until the buffer size is over a threshold or the message is a `uLogError` level log.

Sometimes, if it is necessary to send out current message without the buffering, then `ulog.sync()` can be called to force ulog send all buffered message immediately.

**Notes:** `sync()` should be called before exiting client code, else there may be some logs are not sent to ulog server.

**Notes:** ulog uses UDP port `5400` and `127.0.0.1` as default ulog server.

## ulog server
[ulog_server.cpp](./sample/ulog_server.cpp) shows how to write a server code. It is very simple and can be used to receive and parse ulog message stream from any ulog client.

When an output file specified by `ulog_server` exists already, `ulog_server` will append new log message to the tail of the file.




