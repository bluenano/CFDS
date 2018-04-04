#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef SV_PATH
    #define SV_PATH "/tmp/unix_server"
#endif
const char ServerPathArray[] = SV_PATH;

#ifndef CL_PATH
    #define CL_PATH "/tmp/unix_client"
#endif
const char ClientPathArray[] = CL_PATH;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

template<size_t N>//returns amount not sent
int sendall_literal(int sock, const char(&a)[N])
{
    return N-1 - send(sock, a, N-1, 0);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    static_assert(sizeof(sockaddr_un::sun_path) > 100, "");

    struct sockaddr_un server_addr = {};
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, ServerPathArray); // XXX: should be limited to about 104 characters, system dependent

    struct sockaddr_un client_addr = {};
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, ClientPathArray);

    // get socket
    int const sockfd = socket(PF_UNIX, SOCK_DGRAM, 0);
    {
        int const optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval , sizeof(int)))
            error("setsocketopt()");
    }

    unlink(ClientPathArray);
    // bind client to ClientPathArray
    if (bind(sockfd, (struct sockaddr *) &client_addr, sizeof(client_addr))!=0)
        error("bind()");

    // connect client to ServerPathArray
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr))!=0)
        error("connect()");

    char buf[512];

    if (sendall_literal(sockfd, "greetings")!=0)
        error("send()");

    puts("sent msg, waiting for response");

    int bytes = recv(sockfd, buf, 512, 0);

    if (bytes<=0)
    {
        if (bytes<0)
            error("recv()");
        else
            puts("got zero length recv");
    }
    else
        fwrite(buf, 1, bytes, stdout), putchar('\n');

    close(sockfd);
    return 0;
}
