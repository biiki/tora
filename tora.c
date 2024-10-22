#include <stdio.h>
#include "tora.h"

// Function to create a SOCKS5 request
S5Req *create_socks5_request(const char *dstip, const int dstport) {
    S5Req *req;
    req = malloc(reqsize);

    req->version = SOCKS5_VERSION;
    req->command = SOCKS5_CMD_CONNECT;
    req->reserved = 0x00;
    req->address_type = SOCKS5_ATYP_IPV4;
    req->dstip = inet_addr(dstip);  // Set the destination IP address
    req->dstport = htons(dstport);  // Set the destination port number

    return req;
}

// Function to execute an external command with torsocks
void execute_with_torsocks(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: tora <command> [command options]\n");
        exit(1);
    }

    // Allocate memory for the new arguments array
    char **new_argv = malloc((argc + 1) * sizeof(char *));
    if (new_argv == NULL) {
        perror("malloc");
        exit(1);
    }

    // Set "torsocks" as the first argument
    new_argv[0] = "torsocks";

    // Copy the remaining arguments from argv
    for (int i = 1; i < argc; i++) {
        new_argv[i] = argv[i];
    }

    // Null-terminate the new_argv array
    new_argv[argc] = NULL;

    // Execute the command with torsocks using execvp
    execvp(new_argv[0], new_argv);

    // If execvp returns, an error occurred
    perror("execvp");
    free(new_argv);
    exit(1);
}

int main(int argc, char *argv[]) {
    // If the first argument is "exec", treat it as a command to be run with torsocks
    if (argc > 1 && strcmp(argv[1], " ") == 0) {
        // Call the function to execute a command with torsocks
        execute_with_torsocks(argc - 1, &argv[1]);
    }

    // Original functionality for Tor connection handling
    char *host;
    int port, s;
    struct sockaddr_in sock;
    S5Req *req;
    S5Res *res;
    char buf[ressize];
    int success;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        return -1;
    }

    host = argv[1];  // AWS web server host
    port = atoi(argv[2]);  // AWS web server port

    // Create a socket
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return -1;
    }

    // Set up the proxy server's address (Tor proxy)
    sock.sin_family = AF_INET;
    sock.sin_port = htons(PROXYPORT);
    sock.sin_addr.s_addr = inet_addr(PROXY);

    // Connect to the SOCKS5 proxy
    if (connect(s, (struct sockaddr *)&sock, sizeof(sock)) < 0) {
        perror("connect");
        return -1;
    }
    printf("Connected to the proxy\n");

    // Send SOCKS5 greeting to proxy (no authentication)
    unsigned char socks5_greeting[] = { SOCKS5_VERSION, 0x01, SOCKS5_AUTH_NOAUTH };  // SOCKS5, 1 method, no auth
    write(s, socks5_greeting, sizeof(socks5_greeting));

    // Read proxy's response to the greeting
    read(s, buf, 2);  // Read the first two bytes
    if (buf[1] != SOCKS5_AUTH_NOAUTH) {
        fprintf(stderr, "SOCKS5 proxy requires authentication\n");
        close(s);
        return -1;
    }
    printf("SOCKS5 proxy authentication successful\n");

    // Send the connection request to the SOCKS5 proxy
    req = create_socks5_request(host, port);
    write(s, req, reqsize);

    // Read the proxy's response to the connection request
    memset(buf, 0, ressize);
    if (read(s, buf, ressize) < 1) {
        perror("read");
        free(req);
        close(s);
        return -1;
    }

    // Parse the proxy's response
    res = (S5Res *)buf;
    success = (res->reply == 0x00);  // Check if reply is success (0x00)
    if (!success) {
        fprintf(stderr, "Unable to traverse the proxy, error code: %d\n", res->reply);
        free(req);
        close(s);
        return -1;
    }

    printf("Successfully connected through the SOCKS5 proxy to %s:%d\n", host, port);

    // Cleanup
    free(req);
    close(s);
    return 0;
}
