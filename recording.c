#define ALSA_PCM_NEW_HW_PARAMS_API
#define MONO 0
#define RATE 44100u
#define FRAMES 32

#include <alsa/asoundlib.h>

void check_state( snd_pcm_t *handle ) {
    switch( (unsigned int) snd_pcm_state(handle) ) {
            case SND_PCM_STATE_OPEN:
                printf("State is OPEN\n");
                break;
            case SND_PCM_STATE_SETUP:
                printf("State is SETUP\n");
                break;
            case SND_PCM_STATE_PREPARED:
                printf("State is PREPARED\n");
                break;
            case SND_PCM_STATE_RUNNING:
                printf("State is RUNNING\n");
                break;
            case SND_PCM_STATE_XRUN:
                printf("State is XRUN\n");
                break;
            case SND_PCM_STATE_DRAINING:
                printf("State is DRAINING\n");
                break;
            case SND_PCM_STATE_PAUSED:
                printf("State is PAUSED\n");
                break;
            case SND_PCM_STATE_SUSPENDED:
                printf("State is SUSPENDED\n");
                break;
            case SND_PCM_STATE_DISCONNECTED:
                printf("State is DISCONNECTED\n");
                break;
            default: 
                break;
    }
}

int main() {
    // Für die einzelnen Return Codes und deren evaluation
    int rc;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_format_t *format;
    int dir;
    // Diese Zuweisungen sehen sinnfrei aus, sie dienen dazu, das ein Pointer auf den Wert erzeugt werden kann
    unsigned int rate = RATE;
    snd_pcm_uframes_t frames = FRAMES;
    char *buffer;

    // Öffnet das device "dafault" zum aufnehmen. Dabei soll es nicht blockieren (?)
    // Die Speicheradresse vom Pointer wird übergeben, da snd_pcm_open einen pointer auf einen pointer braucht
    rc = snd_pcm_open( &handle, "default", SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK );
    if (rc < 0){
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }
    check_state( handle );

    // Speicherreservierung im Stack. Alternativ kann man malloc nutzen
    snd_pcm_hw_params_alloca(&params);
    // CHECK! Füllen des Parameterstructs mit Basisconfig
    rc = snd_pcm_hw_params_any(handle, params);
    if (rc < 0){
        fprintf(stderr, "unable to get basic struct: %s\n", snd_strerror(rc));
        exit(1);
    }
    check_state( handle );
    // Hier kann alternativ mmap angewendet werden. Dies kann mit der fft sinnvoll sein,
    // erstmal sollte aber normales rw verwendet werden
    // NONINTERLEAVED da sowieso Mono verwendet werden soll. Bei Stereo sollte Interleaved
    // verwendet werden, da hier kein Handle für die verschiedenen Speicherquews verwendet wird
    rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0){
        fprintf(stderr, "unable to set access mode: %s\n", snd_strerror(rc));
        exit(1);
    }
    check_state( handle );
    // Setzen des formats auf den Standart bei normalen CPUs
    rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    if (rc < 0){
        fprintf(stderr, "unable to set format mode: %s\n", snd_strerror(rc));
        exit(1);
    }
    check_state( handle );
    // Verwendung von einem Monochannel. Ich brauch eigentlich kein Stereo für die Erkennung
    rc = snd_pcm_hw_params_set_channels(handle, params, MONO);
    check_state( handle );

    // &RATE mit #define RATE geht btw. auch nicht, deswegen wird die int definiert. Wird zwar erst 
    // im nächsten Befehl notwendig, aber dann braucht man es nur an einer Stelle ändern
    rc = snd_pcm_hw_params_test_rate(handle, params, rate, dir);
    if (rc < 0)
        fprintf(stderr, "rate isn't available: %i\n", rate);

    check_state( handle );
    // Setzt die Rate so nahe wie möglich an die gewünschte. Der Test dient nur zur Information
    // die Funktion hier sollte auch funktionieren, wenn der Test abstruse ist
    rc = snd_pcm_hw_params_set_rate_near(handle, params, &rate, &dir);
    if (rc < 0){
        fprintf(stderr, "unable to set format mode: %s\n", snd_strerror(rc));
        exit(1);
    }
    check_state( handle );
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    if (rc < 0){
        fprintf(stderr, "unable to set period size: %s\n", snd_strerror(rc));
        exit(1);
    }
    check_state( handle );

    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf (stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(2);
    }
    check_state( handle );

    // Der Wert ist in frames gespeichert, das hier der gesetzte überschrieben wird, kann uns egal sein
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
//    printf("Framescount: %i\n", frames);

    // Copied Code
    int size;
    long loops;
    unsigned int val;
    size = frames * 4;
    buffer = (char *) malloc(size);
    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    loops = 100000 / val;
    while (loops > 0) {
        loops--;
        rc = snd_pcm_readi(handle, buffer, frames);
        if (rc == -EPIPE) {
            fprintf(stderr, "overrun occured\n");
            snd_pcm_prepare(handle);
        }else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        }else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
        rc = write(1, buffer, size);
        if (rc != size) {
            fprintf(stderr, "short write: wrote %d bytes\n", rc);
        }
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);

    return 0;
}
