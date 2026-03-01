#include <kore3/audio/audio.h>
#include <kore3/gpu/device.h>
#include <kore3/input/keyboard.h>
#include <kore3/log.h>
#include <kore3/mixer/mixer.h>
#include <kore3/mixer/sound.h>
#include <kore3/mixer/soundstream.h>
#include <kore3/system.h>

#include <kong.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static kore_gpu_device       device;
static kore_gpu_command_list list;

static const int width  = 800;
static const int height = 600;

static float time_accumulator = 0.0f;

// Low-level audio generation (sine wave)
static bool use_custom_audio = false;
static float sine_frequency = 440.0f;
static float sine_phase = 0.0f;

// Music stream
static kore_mixer_sound_stream *music_stream = NULL;
static bool music_playing = false;

static void custom_audio_callback(kore_audio_buffer *buffer, uint32_t samples, void *userdata) {
    float sample_rate = (float)kore_audio_samples_per_second();
    
    for (uint32_t i = 0; i < samples; i++) {
        float sample = sinf(sine_phase) * 0.3f;
        sine_phase += 2.0f * 3.14159f * sine_frequency / sample_rate;
        if (sine_phase > 2.0f * 3.14159f) {
            sine_phase -= 2.0f * 3.14159f;
        }
        
        buffer->channels[0][buffer->write_location] = sample;
        buffer->channels[1][buffer->write_location] = sample;
        buffer->write_location = (buffer->write_location + 1) % buffer->data_size;
    }
}

static void update(void *data) {
    time_accumulator += 1.0f / 60.0f;
    
    kore_audio_update();

    kore_gpu_texture *framebuffer = kore_gpu_device_get_framebuffer(&device);

    float t = time_accumulator * 0.5f;
    kore_gpu_color clear_color = {
        .r = 0.5f + 0.5f * sinf(t),
        .g = 0.5f + 0.5f * sinf(t + 2.094f),
        .b = 0.5f + 0.5f * sinf(t + 4.188f),
        .a = 1.0f,
    };

    kore_gpu_render_pass_parameters parameters = {
        .color_attachments_count = 1,
        .color_attachments =
            {
                {
                    .load_op     = KORE_GPU_LOAD_OP_CLEAR,
                    .clear_value = clear_color,
                    .texture =
                        {
                            .texture           = framebuffer,
                            .array_layer_count = 1,
                            .mip_level_count   = 1,
                            .format            = KORE_GPU_TEXTURE_FORMAT_BGRA8_UNORM,
                            .dimension         = KORE_GPU_TEXTURE_VIEW_DIMENSION_2D,
                        },
                },
            },
    };
    kore_gpu_command_list_begin_render_pass(&list, &parameters);

    kore_gpu_command_list_end_render_pass(&list);

    kore_gpu_command_list_present(&list);

    kore_gpu_device_execute_command_list(&device, &list);
}

static void keyboard_down(int key, void *data) {
    switch (key) {
    case KORE_KEY_1:
        // Toggle custom audio
        use_custom_audio = !use_custom_audio;
        if (use_custom_audio) {
            if (music_stream && music_playing) {
                kore_mixer_stop_sound_stream(music_stream);
                music_playing = false;
            }
            kore_audio_set_callback(custom_audio_callback, NULL);
            kore_log(KORE_LOG_LEVEL_INFO, "Custom audio: ON (440Hz sine wave)");
        }
        else {
            kore_mixer_init();
            kore_log(KORE_LOG_LEVEL_INFO, "Custom audio: OFF (Mixer mode)");
        }
        break;
    case KORE_KEY_2:
        // Increase frequency
        sine_frequency *= 1.059463094f;
        kore_log(KORE_LOG_LEVEL_INFO, "Frequency: %.1f Hz", sine_frequency);
        break;
    case KORE_KEY_3:
        // Decrease frequency
        sine_frequency /= 1.059463094f;
        kore_log(KORE_LOG_LEVEL_INFO, "Frequency: %.1f Hz", sine_frequency);
        break;
    case KORE_KEY_P:
        // Play/Pause music
        if (music_stream) {
            if (music_playing) {
                kore_mixer_stop_sound_stream(music_stream);
                music_playing = false;
                kore_log(KORE_LOG_LEVEL_INFO, "Music paused");
            } else {
                kore_mixer_play_sound_stream(music_stream);
                music_playing = true;
                kore_log(KORE_LOG_LEVEL_INFO, "Music playing");
            }
        }
        break;
    case KORE_KEY_S:
        // Stop music and reset
        if (music_stream) {
            kore_mixer_stop_sound_stream(music_stream);
            kore_mixer_sound_stream_reset(music_stream);
            music_playing = false;
            kore_log(KORE_LOG_LEVEL_INFO, "Music stopped");
        }
        break;
    case KORE_KEY_L:
        // Toggle loop
        if (music_stream) {
            bool looping = !kore_mixer_sound_stream_looping(music_stream);
            kore_mixer_sound_stream_set_looping(music_stream, looping);
            kore_log(KORE_LOG_LEVEL_INFO, "Looping: %s", looping ? "ON" : "OFF");
        }
        break;
    case KORE_KEY_I:
        // Show music info
        if (music_stream) {
            kore_log(KORE_LOG_LEVEL_INFO, "=== Music Info ===");
            kore_log(KORE_LOG_LEVEL_INFO, "  Length: %.1f sec", kore_mixer_sound_stream_length(music_stream));
            kore_log(KORE_LOG_LEVEL_INFO, "  Position: %.1f sec", kore_mixer_sound_stream_position(music_stream));
            kore_log(KORE_LOG_LEVEL_INFO, "  Channels: %d", kore_mixer_sound_stream_channels(music_stream));
            kore_log(KORE_LOG_LEVEL_INFO, "  Sample rate: %d", kore_mixer_sound_stream_sample_rate(music_stream));
            kore_log(KORE_LOG_LEVEL_INFO, "  Looping: %s", kore_mixer_sound_stream_looping(music_stream) ? "YES" : "NO");
            kore_log(KORE_LOG_LEVEL_INFO, "  Ended: %s", kore_mixer_sound_stream_ended(music_stream) ? "YES" : "NO");
        }
        break;
    case KORE_KEY_SPACE:
        kore_log(KORE_LOG_LEVEL_INFO, "Audio Test - Controls:");
        kore_log(KORE_LOG_LEVEL_INFO, "  1: Toggle custom audio (sine wave)");
        kore_log(KORE_LOG_LEVEL_INFO, "  2/3: Increase/Decrease frequency");
        kore_log(KORE_LOG_LEVEL_INFO, "  P: Play/Pause music");
        kore_log(KORE_LOG_LEVEL_INFO, "  S: Stop music");
        kore_log(KORE_LOG_LEVEL_INFO, "  L: Toggle loop");
        kore_log(KORE_LOG_LEVEL_INFO, "  I: Show music info");
        break;
    }
}

int kickstart(int argc, char **argv) {
    kore_init("Audio Test", width, height, NULL, NULL);
    kore_set_update_callback(update, NULL);
    kore_keyboard_set_key_down_callback(keyboard_down, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);

    kong_init(&device);

    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &list);

    kore_mixer_init();
    
    // Load and play OGG music
    music_stream = kore_mixer_sound_stream_create("file_example_OOG_1MG.ogg", true);
    if (music_stream) {
        kore_mixer_sound_stream_set_volume(music_stream, 0.7f);
        kore_mixer_play_sound_stream(music_stream);
        music_playing = true;
        kore_log(KORE_LOG_LEVEL_INFO, "Loaded music: file_example_OOG_1MG.ogg");
        kore_log(KORE_LOG_LEVEL_INFO, "  Length: %.1f sec", kore_mixer_sound_stream_length(music_stream));
        kore_log(KORE_LOG_LEVEL_INFO, "  Sample rate: %d Hz", kore_mixer_sound_stream_sample_rate(music_stream));
    } else {
        kore_log(KORE_LOG_LEVEL_ERROR, "Failed to load music: file_example_OOG_1MG.ogg");
    }
    
    kore_log(KORE_LOG_LEVEL_INFO, "Audio Test - Controls:");
    kore_log(KORE_LOG_LEVEL_INFO, "  1: Toggle custom audio (sine wave)");
    kore_log(KORE_LOG_LEVEL_INFO, "  2/3: Increase/Decrease frequency");
    kore_log(KORE_LOG_LEVEL_INFO, "  P: Play/Pause music");
    kore_log(KORE_LOG_LEVEL_INFO, "  S: Stop music");
    kore_log(KORE_LOG_LEVEL_INFO, "  L: Toggle loop");
    kore_log(KORE_LOG_LEVEL_INFO, "  I: Show music info");
    kore_log(KORE_LOG_LEVEL_INFO, "  System sample rate: %d Hz", kore_audio_samples_per_second());

    kore_start();

    return 0;
}
