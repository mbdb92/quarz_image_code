
// Helper function for debugging errors. If the device has a state error,
// this function can check for the state
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

// Central error code resolver, so I can change and edit them at one point
void print_error_code( int errnr ) {
    switch( errnr ) {
            case PCM_OPEN_FAIL:
                fprintf(stderr, "snd_pcm_open() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_ANY_PARAMS_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_any() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_SET_ACCESS_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_set_access() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_SET_FORMAT_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_set_format() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_SET_CHANNELS_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_set_channels() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_SET_RATE_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_set_rate_near() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_SET_PERIOD_FAIL:
                fprintf(stderr, "snd_pcm_hw_params_set_period_size_near() failed: %s\n", snd_strerror(errnr));
                break;
            case HW_PARAMS_ERROR:
                fprintf(stderr, "Setting struct failed\n");
                fprintf(stderr, "snd_pcm_hw_params() failed: %s\n", snd_strerror(errnr));
                break;
            default: 
                break;
    }
}
