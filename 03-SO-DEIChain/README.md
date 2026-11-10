# DEIChain

C simulation of a blockchain with a controller, transaction generator, miners, validators and statistics. The implementation explores process coordination and interprocess communication.

## Requirements

Linux, GCC, Make, POSIX threads and OpenSSL development libraries. The Makefile links libcrypto.

## Run

Review config.cfg and build from this directory.

```sh
make
./controller
```

In a second terminal, submit transactions with a reward from 1 to 3 and a delay in milliseconds.

```sh
./transactionGenerator 1 1000
```

## Project notes

The controller expects config.cfg in the working directory. The simulation produces runtime logs and uses operating-system IPC resources.
