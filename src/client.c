#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 1234

void exit_error(char *call)
{
    fprintf(stderr, "client: %s(2) failed!\n", call);
    perror("client");
    exit(errno);
}

int main(int argc, char **argv)
{
    ssize_t bytes;
    size_t namelen;
    int sd, port = DEFAULT_PORT;
    char buf[BUFSIZ], filename[NAME_MAX + 1], *host = DEFAULT_HOST;
    struct sockaddr_in addr;
    FILE *fout;

    switch (argc) {
    case 3:
        port = atoi(argv[2]);
    case 2:
        host = argv[1];
    case 1: /* intended fall-through */
        break;

    default:
        fprintf(stderr, "Usage:\n\tclient [host] [port]\n");
        exit(1);
        break;
    }

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit_error("socket");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(host, &addr.sin_addr);

    if (connect(sd, (struct sockaddr *) &addr, sizeof(addr)))
        exit_error("connect");

    if ((bytes = recv(sd, &namelen, sizeof(namelen), 0)) == -1)
        exit_error("recv");

    if (namelen > NAME_MAX)
        exit_error("open");

    if (recv(sd, filename, namelen, 0) != (ssize_t) namelen)
        exit_error("recv");

    filename[namelen] = '\0';

    if (!(fout = fopen(filename, "wb")))
        exit_error("open");

    do {
        if ((bytes = recv(sd, buf, BUFSIZ, 0)) == -1)
            exit_error("recv");

        if (fwrite(buf, sizeof(*buf), bytes, fout) < (size_t) bytes)
            exit_error("write");
    } while (bytes > 0);

    fclose(fout);
    close(sd);
    return 0;
}
