# Messaging System

C messaging application with network clients, a server and an administrative client. The project uses socket communication and a text file for user records.

## Requirements

Linux or a compatible Unix environment with GCC, Make and pthread support.

## Run

Prepare a user file in the format parsed by udpserver.c. Build the programs using make. The server receives two ports and the user-file path.

```sh
make
./udpserver 5000 5001 user.txt
```

udpclient and admin_client each receive a server IP address and port. Match the ports to the corresponding server service.

## Project notes

The portfolio copy omits the original user.txt. Port values above are examples. Consult the server setup before starting clients.
