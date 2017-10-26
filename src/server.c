#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEFAULT_BACKLOG 5
#define DEFAULT_PORT    1234

void exit_error(char *call)
{
    fprintf(stderr, "server: %s(2 or 3) failed!\n", call);
    if (errno) {
        perror("server");
        exit(errno);
    }
}

int main(int argc, char **argv)
{
    size_t bytes, namelen;
    int sd, con_sd, port = DEFAULT_PORT, backlog = DEFAULT_BACKLOG;
    char buf[BUFSIZ], *filepath, *filename;
    socklen_t addrlen;
    struct sockaddr_in addr;
    FILE *fin;

    switch (argc) {
    case 4:
        backlog = atoi(argv[3]);
    case 3:
        port = atoi(argv[2]);
    case 2: /* intended fall-through */
        if (!(filepath = realpath(argv[1], NULL)))
            exit_error("realpath");

        for (
            filename = filepath + strlen(filepath) - 1;
            *filename != '/';
            --filename
        );
        ++filename;

        namelen = strlen(filename);

        break;

    default:
        fprintf(stderr, "Usage:\n\tserver <filename> [port] [backlog]\n");
        exit(1);
        break;
    }

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit_error("socket");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *) &addr, sizeof(addr)))
        exit_error("bind");

    if (listen(sd, backlog))
        exit_error("listen");

    addrlen = sizeof(addr);

    puts("Press Ctrl + C to exit.");

    for (;;) {
        if ((con_sd = accept(sd, (struct sockaddr *) &addr, &addrlen)) < 0)
            exit_error("accept");

        if (!fork()) {
            close(sd);

            if (!(fin = fopen(filepath, "rb")))
                exit_error("fopen");

            if (
                send(con_sd, &namelen, sizeof(namelen), 0) !=
                    (ssize_t) sizeof(namelen)
            )
                exit_error("send");

            if (send(con_sd, filename, namelen, 0) != (ssize_t) namelen)
                exit_error("send");

            while ((bytes = fread(buf, sizeof(*buf), BUFSIZ, fin))) {
                if (ferror(fin))
                    exit_error("fread");

                if (send(con_sd, buf, bytes, 0) != (ssize_t) bytes)
                    exit_error("send");
            }

            fclose(fin);
            close(con_sd);
            exit(0);
        }

        close(con_sd);
    }

    free(filepath);
    close(sd);
    return 0;
}
