#ifndef FFT_H
#define FFT_H

#define PI 3.14159
#define RANK 1
#define SIZE 360

int create_fft( struct fft_params *fft_p, struct fft_data *fft_d );

int destroy_fft( struct fft_params *fft_p, struct fft_data *fft_d );

int fft_handler( int pipefd[2], void *shmem );
int fft_run();

#endif //FFT_H
