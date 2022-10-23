#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "broadcast.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    ma_result result;
    ma_engine engine;
    ma_sound sound;

    if (argc < 2) {
        printf("No input file.");
        return -1;
    }

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
      printf("Failed to initialize audio engine.");
      return -1;
    }

    result = ma_sound_init_from_file(&engine, argv[1], 0, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
	fprintf(stderr, "ma_sound_init_from_file (%d)\n", result);
	return -1;
    }

    ma_sound_start(&sound);

    /* printf("get_time: %lld\n", ma_engine_get_time(&engine)); */

    /* ma_engine_play_sound(&engine, argv[1], NULL); */

    printf("Press Enter to quit...");
    getchar();

    ma_sound_uninit(&sound);
    ma_engine_uninit(&engine);

    return 0;
}

