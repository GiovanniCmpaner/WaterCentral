#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2s.h"

#include "audio.h"
//-----------------------------------------------------------------------------
void audio_initialize();
void audio_play(const audio_t* audio);

static void audio_process();
//-----------------------------------------------------------------------------
static const char* tag = "audio";
static uint8_t volume_percentage = 100;


static QueueHandle_t audio_queue = NULL;
//-----------------------------------------------------------------------------
void audio_initialize(){
    ESP_LOGD(tag, "initialize");
    
	const i2s_config_t i2s_config = {
       .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
       .sample_rate =  16000,
       .bits_per_sample = 16,
	   .communication_format = I2S_COMM_FORMAT_I2S_MSB,
	   .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
	   .dma_buf_count = 4,
	   .dma_buf_len = 1024,
	   .use_apll = 1,
       .tx_desc_auto_clear = 1 // Apaga o buffer DMA quando não houver mais data sendo fornecidos, evita que fique playndo o trecho final
	};
	i2s_driver_install(0, &i2s_config, 0, NULL);
	i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
    i2s_set_clk(0, 16000, 16, 1);
    
    audio_queue = xQueueCreate( 1, sizeof(const audio_t*) );
    xTaskCreatePinnedToCore(audio_process, "audio_process", 3072, NULL, 1, NULL, 0);
}
//-----------------------------------------------------------------------------
static void audio_process(){
    const size_t write_buffer_length = 8184; // Tamanho maximo do buffer DMA usado pelo I2S
    uint8_t* const write_buffer = pvPortMalloc(write_buffer_length); // Aloca o buffer
    
    const audio_t* audio = NULL;
    uint32_t data_pos = 0;
    
    TickType_t interval = pdMS_TO_TICKS( 10 );
    TickType_t last_wake_time = xTaskGetTickCount();
    while (1) {
        const audio_t* new_audio;
        if(xQueueReceive( audio_queue, &new_audio, 0 )){
            audio = new_audio;
            data_pos = 0;
        }
        if(audio != NULL){
            const size_t write_block_length = ( write_buffer_length / 2 ) < ( audio->length - data_pos ) ? ( write_buffer_length / 2 ) : ( audio->length - data_pos );
            
            for(uint32_t n = 0; n < write_block_length; n++){
                write_buffer[n * 2 + 1] = ( audio->data[data_pos] * volume_percentage ) / 100;
                data_pos++;
            }

            memset(&write_buffer[write_block_length * 2],0, write_buffer_length - ( write_block_length * 2 ));
            
            // Sempre envia o tamanho inteiro to buffer DMA, pois ele espera estar completo antes de começar a enviar
            // Se enviar menos, ele vai ter atraso ou não vai playr o fim, pois o fim dificilmente vai casar o tamanho inteiro
            size_t bytes_written;
            i2s_write(0, write_buffer, write_buffer_length, &bytes_written, portMAX_DELAY);
            
            if(data_pos == audio->length){
                audio = NULL;
            }
        }
        else {
            vTaskDelayUntil( &last_wake_time, interval );
        }
    }
}
//-----------------------------------------------------------------------------
void audio_play(const audio_t* audio){
    ESP_LOGD(tag, "play");
    
    if(audio_queue != NULL){
        xQueueOverwrite( audio_queue, &audio );    
    }
}
//-----------------------------------------------------------------------------
void audio_set_volume( uint8_t new_percentage )
{
    if( new_percentage > 100 ){
        new_percentage = 100;
    }
    volume_percentage = new_percentage;
}
//-----------------------------------------------------------------------------
uint8_t audio_get_volume()
{
    return volume_percentage;
}
//-----------------------------------------------------------------------------