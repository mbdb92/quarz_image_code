#ifndef FFT_H
#define FFT_H

int create_fft( struct fft_params *fft_p, struct fft_data *fft_d );

int destroy_fft( struct fft_params *fft_p, struct fft_data *fft_d );

int run_fft( struct fft_params *fft_p, struct fft_data *fft_d );

#endif //FFT_H
