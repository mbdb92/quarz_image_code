#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] ) {
    FILE *fd;
    char *line = NULL;
    size_t llength = 0;
    ssize_t nread;
    long sum, sline;

    fd = fopen( argv[1], "r" );
    if (fd == NULL ) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    while( (nread = getline( &line, &llength, fd)) != -1 ) {

        sline =strtol( line, NULL, 10 );
        sum = sum + sline;
//        printf("sline: %li, sum: %li\n", sline, sum);

    }

    free(line);
    fclose(fd);
    printf("%li\n",sum);
    return 0;
}
