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
    fprintf(stderr, "client: %s(2 or 3) failed!\n", call);
    if (errno) {
        perror("client");
        exit(errno);
    }
}

int main(int argc, char **argv)
{
    ssize_t bytes;
    size_t namelen;
    int sd, port = DEFAULT_PORT;
    char buf[BUFSIZ], filepath[PATH_MAX + 1], *filename, *host = DEFAULT_HOST;
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

    if (!namelen || namelen > PATH_MAX)
        exit_error("fopen");

    if (recv(sd, filepath, namelen, 0) != (ssize_t) namelen)
        exit_error("recv");

    filepath[namelen] = '\0';

    for (
        filename = filepath + namelen - 1;
        filename != filepath && filename[-1] != '/';
        --filename
    );

    if (!(fout = fopen(filename, "wb")))
        exit_error("fopen");

    do {
        if ((bytes = recv(sd, buf, BUFSIZ, 0)) == -1)
            exit_error("recv");

        if (fwrite(buf, sizeof(*buf), bytes, fout) < (size_t) bytes)
            exit_error("fwrite");
    } while (bytes > 0);

    fclose(fout);
    close(sd);
    return 0;
}
