#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef SV_PATH
    #define SV_PATH "/tmp/unix_server"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char PathArray[] = SV_PATH;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

template<size_t N>//returns amount not sent
int sendallto_literal(int sock, const char(&a)[N], const struct sockaddr *addr, socklen_t alen)
{
    return N-1 - sendto(sock, a, N-1, 0, addr, alen);
}

int main()
{
    int const svsock = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (svsock < 0)
        error("socket()");
    #if 1
    /* setsockopt: Handy debugging trick that lets
    * us rerun the server immediately after we kill it;
    * otherwise we have to wait about 20 secs.
    * Eliminates "ERROR on binding: Address already in use" error.
    */
    {
        int const optval = 1;
        if (setsockopt(svsock, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval , sizeof(int)))
            error("setsocketopt()");
    }
    #endif // 0

    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, PathArray);
    static_assert(sizeof(address.sun_path) >= sizeof(PathArray), "");

    unlink(PathArray);
    if (bind(svsock, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0)
        error("bind()");

    struct sockaddr_un fromAddr;//in, not store?
    socklen_t fromLen = sizeof(fromAddr);

    enum{InCap=4096, OutCap=4096};

    char inbuf[InCap];
    //char outbuf[OutCap];

    for (;;)
    {

        int ngot = recvfrom(svsock, inbuf, 4096, 0, (struct sockaddr *)&fromAddr, &fromLen);

        if (ngot>512)
            error("Message received too big");

        if (ngot<=0)
        {
            if (ngot<0)
                error("recvfrom()");
            else
            {
                puts("recvfrom returned 0");
                continue;
            }
        }

        fwrite(inbuf, 1, ngot, stdout);
        putchar('\n');


        if (sendallto_literal(svsock, "its ready", (const struct sockaddr *)&fromAddr, fromLen)!=0)
            error("sendto()");

        puts("exiting loop");
        break;
    }

    close(svsock);
    return 0;
}
