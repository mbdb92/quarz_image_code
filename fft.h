#ifndef FFT_H
#define FFT_H

int create_fft( struct fft_params *fft_p, struct fft_data *fft_d );

int destroy_fft( struct fft_params *fft_p, struct fft_data *fft_d );

#ifdef PIPE
int fft_handler( int pipefd[2], void *shmem );
#endif /* PIPE */
#ifdef RINGBUFFER
int fft_handler( int *ringbuffer, int *wrote_size, void *shmem );
#endif /* RINGBUFFER */
int fft_run( char *filename);

#endif //FFT_H
