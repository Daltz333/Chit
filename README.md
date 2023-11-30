# Chit

COSC 423 Project #2

## Project Goals

- Chit Client:
  - Provides a user interface that implements secure realtime UDP communication with other Psst! users.
  - Performs basic message encryption and decryption using RSA PKE.
- Address Server: 
  - TCP server for realtime communication
- Public Key Server:
  - TCP server for storing users public keys.

## Run Instructions

Run the following in separate bash instances:

- Run `./bin/pk_server PORT`
- Run `./bin/chit_client PORT`
- Run `./bin/address_server PORT`

### Valid client commands

TODO

## Build Instructions

Project uses a standard `Makefile` for building.

```bash
make clean
make all
```

### Targets

- `make chit_client` - Builds the terminal client
- `make address_server` - Builds the address server for UDP lookup
- `make pk_server` - Builds the public key server for encrypted message transmission
- `make all` - Builds all of the above
- `make clean` - Cleans the `bin` and `obj` directories

## Directory Structure

The project is composed of four directories under `src`. 

- `address_server` - code specific to the address server
- `chit_client` code specific to the chit client
- `pk_server` - code specific to the public key server
- `shared` - shared code that should be duplicated among targets

## Binaries

Binaries are located in `bin` with their respective target names. Binaries are also available via GitHub actions artifacts.

## Notice for Consumers

This application isn't really intended for public consumption. There are many encrypted terminal chat clients out there to use. This project has many issues that have been "hand-waved" away as this is just a school project. Issues such as:

- Limited to 100 clients, and subsequent clients are denied
- RSA encryption is very slow
- Not very user intuitive (ideally we'd generate keys for the user!)
- Breaks on long messages, special characters, etc.
