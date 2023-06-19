README

# Concert Ticket Sales Server

This is a simple client-server application that allows clients to purchase concert tickets. The server manages the ticket inventory and handles client requests. The server can handle multiple client connections concurrently using threads.

## Instructions

### Compilation

To compile the client and server codes, use the following commands:

$ make

### Execution

1. Run the server first by executing the following command:

$ ./server

2. Run the client by executing the following command:

$ ./client

3. Follow the prompts on the client-side to interact with the server.

## Server Code (server.c)

The `server.c` file contains the server-side code. It listens for incoming client connections, handles each connection in a separate thread, and manages the ticket inventory.

The server uses semaphores to limit the number of concurrent connections and a mutex to protect the treasury variable, which stores the total amount collected from ticket sales.

The server receives requests from clients, checks the ticket availability, calculates the price, and prompts the client to confirm the purchase. After the purchase is confirmed, the server updates the ticket inventory, adds the price to the treasury, and creates a receipt file for the client.

## Client Code (client.c)

The `client.c` file contains the client-side code. It establishes a connection with the server, sends requests to purchase concert tickets, and receives responses from the server.

The client prompts the user to select a category of seat, enter the number of seats, provide their name, and confirm their purchase. The client communicates this information to the server, which processes the request and sends a response back to the client.

## Authors

This project have been developped by Maxime Travaillard and Antoine Wyplosz.

