#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <stdio.h>


void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}


int broadcast(const char* filename)
{
    ma_result result;
#if 0
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;

    result = ma_decoder_init_file(filename, NULL, &decoder);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Could not load file: %s\n", filename);
        return -2;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -4;
    }

    printf("Press Enter to quit...");
    getchar();

    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);

#else

    ma_engine engine;
    ma_engine_config engine_config;
    /* ma_sound sound; */

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
	fprintf(stderr, "ma_engine_init failed (%d)\n", result);
	return -1;
    }

    /* result = ma_sound_init_from_file(&engine, filename, 0, NULL, NULL, &sound); */
    /* if (result != MA_SUCCESS) { */
    /* 	fprintf(stderr, "ma_sound_init_from_file (%d)\n", result); */
    /* 	return -1; */
    /* } */

    /* ma_sound_start(&sound); */
    ma_engine_play_sound(&engine, filename, NULL);

    printf("Press Enter to quit...");
    getchar();

    /* ma_sound_uninit(&sound); */
    ma_engine_uninit(&engine);


#endif	/* 0 */

    return 0;
}
