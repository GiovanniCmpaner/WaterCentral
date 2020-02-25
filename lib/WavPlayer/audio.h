#ifndef AUDIO_H
#define AUDIO_H

typedef struct {
    uint16_t num_channels;
    uint16_t sample_rate;
    uint8_t bits_per_sample;
    uint32_t length;
    uint8_t data[];
} audio_t;

extern const audio_t audio_cursor_move;

void audio_initialize();
void audio_play(const audio_t* audio);
void audio_set_volume( uint8_t new_percentage );
uint8_t audio_get_volume();

#endif