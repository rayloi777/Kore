#define STB_TRUETYPE_IMPLEMENTATION
#include <kore3/text.h>
#include <kore3/libs/stb_truetype.h>
#include <kore3/gpu/device.h>
#include <kore3/gpu/texture.h>
#include <kore3/gpu/buffer.h>
#include <kore3/gpu/sampler.h>
#include <kore3/io/filereader.h>
#include <kore3/math/matrix.h>
#include <kore3/window.h>
#include <kore3/log.h>
#include <kore3/metal/commandlist_functions.h>

#include <kong.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TEXT_ATLAS_SIZE 512
#define MAX_STRING_LENGTH 256

typedef struct {
    int codepoint;
    int x, y;
    int width, height;
    float xoffset, yoffset;
    float xadvance;
    float s0, t0, s1, t1;
} glyph_info;

struct draw_font {
    stbtt_fontinfo font_info;
    uint8_t *font_data;
    uint8_t *atlas_pixels;
    float scale;
    float baseline;
    
    glyph_info *glyphs;
    int glyph_count;
    
    kore_gpu_texture texture;
    kore_gpu_texture_view texture_view;
    everything_set set;
};

static kore_gpu_device *g_device = NULL;
static kore_gpu_command_list *g_list = NULL;
static int g_screen_width = 800;
static int g_screen_height = 600;

static kore_gpu_buffer g_uniform_buffer;
static kore_gpu_buffer g_vertex_buffer;
static kore_gpu_buffer g_index_buffer;
static kore_gpu_sampler g_sampler;
static bool g_initialized = false;

static int g_max_vertices = 0;
static int g_max_indices = 0;

static uint32_t utf8_decode(const char *s, int *len) {
    uint32_t c = (unsigned char)s[0];
    if (c < 0x80) {
        *len = 1;
        return c;
    }
    if ((c & 0xe0) == 0xc0) {
        *len = 2;
        return ((c & 0x1f) << 6) | (s[1] & 0x3f);
    }
    if ((c & 0xf0) == 0xe0) {
        *len = 3;
        return ((c & 0x0f) << 12) | ((s[1] & 0x3f) << 6) | (s[2] & 0x3f);
    }
    if ((c & 0xf8) == 0xf0) {
        *len = 4;
        return ((c & 0x07) << 18) | ((s[1] & 0x3f) << 12) | ((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
    }
    *len = 1;
    return '?';
}

static glyph_info *find_glyph(draw_font *font, int codepoint) {
    for (int i = 0; i < font->glyph_count; i++) {
        if (font->glyphs[i].codepoint == codepoint) {
            return &font->glyphs[i];
        }
    }
    return NULL;
}

void draw_init(void *device, void *command_list) {
    g_device = (kore_gpu_device *)device;
    g_list = (kore_gpu_command_list *)command_list;
    
    if (g_device) {
        g_screen_width = kore_window_width(0);
        g_screen_height = kore_window_height(0);
    }
    
    kore_log(KORE_LOG_LEVEL_INFO, "draw_init: screen size %d x %d", g_screen_width, g_screen_height);
    
    constants_type_buffer_create(g_device, &g_uniform_buffer, 1);
    
    kore_gpu_sampler_parameters sam_params = {
        .address_mode_u = KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
        .address_mode_v = KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
        .address_mode_w = KORE_GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mag_filter     = KORE_GPU_FILTER_MODE_LINEAR,
        .min_filter     = KORE_GPU_FILTER_MODE_LINEAR,
        .mipmap_filter  = KORE_GPU_MIPMAP_FILTER_MODE_NEAREST,
        .lod_min_clamp  = 0,
        .lod_max_clamp  = 1,
        .compare        = KORE_GPU_COMPARE_FUNCTION_UNDEFINED,
        .max_anisotropy = 1,
    };
    kore_gpu_device_create_sampler(g_device, &sam_params, &g_sampler);
    
    g_initialized = true;
}

void draw_set_viewport(int width, int height) {
    g_screen_width = width;
    g_screen_height = height;
}

draw_font *draw_font_create(const char *ttf_path, int *glyphs, int glyph_count, float size) {
    draw_font *font = (draw_font *)malloc(sizeof(draw_font));
    if (!font) return NULL;
    
    memset(font, 0, sizeof(draw_font));
    
    kore_file_reader reader;
    kore_log(KORE_LOG_LEVEL_INFO, "Trying to open font: %s", ttf_path);
    if (!kore_file_reader_open(&reader, ttf_path, KORE_FILE_TYPE_ASSET)) {
        kore_log(KORE_LOG_LEVEL_ERROR, "Failed to open font file: %s", ttf_path);
        free(font);
        return NULL;
    }
    
    font->font_data = (uint8_t *)malloc(reader.size);
    kore_file_reader_read(&reader, font->font_data, reader.size);
    kore_file_reader_close(&reader);
    
    if (!stbtt_InitFont(&font->font_info, font->font_data, 0)) {
        kore_log(KORE_LOG_LEVEL_ERROR, "Failed to init font");
        free(font->font_data);
        free(font);
        return NULL;
    }
    
    font->scale = stbtt_ScaleForPixelHeight(&font->font_info, size);
    
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font->font_info, &ascent, &descent, &line_gap);
    font->baseline = (float)(ascent - descent + line_gap) * font->scale;
    
    font->glyph_count = glyph_count;
    font->glyphs = (glyph_info *)malloc(sizeof(glyph_info) * glyph_count);
    
    font->atlas_pixels = (uint8_t *)calloc(TEXT_ATLAS_SIZE * TEXT_ATLAS_SIZE, 1);
    
    int pw = TEXT_ATLAS_SIZE;
    int ph = TEXT_ATLAS_SIZE;
    int x = 1, y = 1, max_h = 0;
    int loaded = 0;
    
    for (int i = 0; i < glyph_count; i++) {
        int codepoint = glyphs[i];
        
        int w, h, xoff, yoff;
        uint8_t *bitmap = stbtt_GetCodepointBitmap(&font->font_info, 0, font->scale, codepoint, &w, &h, &xoff, &yoff);
        
        if (!bitmap) {
            w = 0; h = 0; xoff = 0; yoff = 0;
        }
        
        if (x + w + 1 >= pw) {
            x = 1;
            y += max_h + 1;
            max_h = 0;
        }
        
        if (y + h >= ph) {
            if (bitmap) stbtt_FreeBitmap(bitmap, NULL);
            font->glyphs[i].width = 0;
            font->glyphs[i].height = 0;
            continue;
        }
        
        if (h > max_h) max_h = h;
        
        font->glyphs[i].codepoint = codepoint;
        font->glyphs[i].x = x;
        font->glyphs[i].y = y;
        font->glyphs[i].width = w;
        font->glyphs[i].height = h;
        font->glyphs[i].xoffset = (float)xoff;
        font->glyphs[i].yoffset = (float)yoff;
        
        int advance;
        stbtt_GetCodepointHMetrics(&font->font_info, codepoint, &advance, NULL);
        font->glyphs[i].xadvance = (float)advance * font->scale;
        
        font->glyphs[i].s0 = (float)x / (float)pw;
        font->glyphs[i].t0 = (float)y / (float)ph;
        font->glyphs[i].s1 = (float)(x + w) / (float)pw;
        font->glyphs[i].t1 = (float)(y + h) / (float)ph;
        
        if (bitmap && w > 0 && h > 0) {
            for (int py = 0; py < h; py++) {
                for (int px = 0; px < w; px++) {
                    font->atlas_pixels[(y + py) * pw + (x + px)] = bitmap[py * w + px];
                }
            }
            stbtt_FreeBitmap(bitmap, NULL);
            loaded++;
        }
        
        x += w + 1;
    }
    
    kore_log(KORE_LOG_LEVEL_INFO, "Font loaded: %d/%d glyphs, atlas %dx%d", loaded, glyph_count, pw, ph);
    
    kore_gpu_texture_parameters tex_params = {
        .width = pw,
        .height = ph,
        .depth_or_array_layers = 1,
        .mip_level_count = 1,
        .sample_count = 1,
        .dimension = KORE_GPU_TEXTURE_DIMENSION_2D,
        .format = KORE_GPU_TEXTURE_FORMAT_RGBA8_UNORM,
        .usage = KORE_GPU_TEXTURE_USAGE_SAMPLED | KORE_GPU_TEXTURE_USAGE_COPY_DST,
    };
    kore_gpu_device_create_texture(g_device, &tex_params, &font->texture);
    
    // 转换为 RGBA
    uint8_t *rgba_pixels = (uint8_t *)malloc(pw * ph * 4);
    for (int i = 0; i < pw * ph; i++) {
        rgba_pixels[i * 4 + 0] = font->atlas_pixels[i];
        rgba_pixels[i * 4 + 1] = font->atlas_pixels[i];
        rgba_pixels[i * 4 + 2] = font->atlas_pixels[i];
        rgba_pixels[i * 4 + 3] = font->atlas_pixels[i];
    }
    kore_gpu_texture_upload(g_device, &font->texture, rgba_pixels, pw, ph);
    free(rgba_pixels);
    
    kore_gpu_texture_view_create(g_device, &font->texture, &font->texture_view);
    
    everything_parameters params = {
        .constants = &g_uniform_buffer,
        .tex = font->texture_view,
        .sam = &g_sampler,
    };
    kong_create_everything_set(g_device, &params, &font->set);
    
    return font;
}

void draw_string(draw_font *font, const char *utf8_text, float x, float y, float r, float g, float b, float a) {
    if (!font || !utf8_text || !g_list || !g_initialized) return;
    
    int char_count = 0;
    const char *p = utf8_text;
    while (*p) {
        int len;
        uint32_t codepoint = utf8_decode(p, &len);
        p += len;
        
        glyph_info *gi = find_glyph(font, codepoint);
        if (gi && gi->width > 0) {
            char_count++;
        }
    }
    
    if (char_count == 0) return;
    
    int total_vertices = char_count * 4;
    int total_indices = char_count * 6;
    
    if (total_vertices > g_max_vertices) {
        if (g_vertex_buffer.metal.buffer) {
            kore_gpu_buffer_destroy(&g_vertex_buffer);
        }
        kore_gpu_buffer_parameters vb_params = {
            .size = sizeof(vertex_in) * total_vertices,
            .usage_flags = KORE_GPU_BUFFER_USAGE_VERTEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
        };
        kore_gpu_device_create_buffer(g_device, &vb_params, &g_vertex_buffer);
        g_max_vertices = total_vertices;
    }
    
    if (total_indices > g_max_indices) {
        if (g_index_buffer.metal.buffer) {
            kore_gpu_buffer_destroy(&g_index_buffer);
        }
        kore_gpu_buffer_parameters ib_params = {
            .size = sizeof(uint16_t) * total_indices,
            .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
        };
        kore_gpu_device_create_buffer(g_device, &ib_params, &g_index_buffer);
        g_max_indices = total_indices;
    }
    
    vertex_in *all_verts = (vertex_in *)malloc(sizeof(vertex_in) * total_vertices);
    uint16_t *all_indices = (uint16_t *)malloc(sizeof(uint16_t) * total_indices);
    
    float xpos = x;
    float ypos = y + font->baseline;
    float screen_w = (float)g_screen_width;
    float screen_h = (float)g_screen_height;
    
    int vidx = 0;
    int iidx = 0;
    int char_idx = 0;
    
    p = utf8_text;
    while (*p) {
        int len;
        uint32_t codepoint = utf8_decode(p, &len);
        p += len;
        
        glyph_info *gi = find_glyph(font, codepoint);
        if (!gi || gi->width == 0) continue;
        
        float w = (float)gi->width;
        float h = (float)gi->height;
        
        float x0 = (xpos + gi->xoffset) / screen_w * 2.0f - 1.0f;
        float y0 = (ypos + gi->yoffset) / screen_h * 2.0f - 1.0f;
        float x1 = x0 + w / screen_w * 2.0f;
        float y1 = y0 + h / screen_h * 2.0f;
        
        uint16_t base = (uint16_t)(char_idx * 4);
        
        all_indices[iidx++] = base + 0;
        all_indices[iidx++] = base + 1;
        all_indices[iidx++] = base + 2;
        all_indices[iidx++] = base + 0;
        all_indices[iidx++] = base + 2;
        all_indices[iidx++] = base + 3;
        
        all_verts[vidx + 0].pos = (kore_float3){x0, y0, 0.5f};
        all_verts[vidx + 0].uv = (kore_float2){gi->s0, gi->t1};
        all_verts[vidx + 1].pos = (kore_float3){x1, y0, 0.5f};
        all_verts[vidx + 1].uv = (kore_float2){gi->s1, gi->t1};
        all_verts[vidx + 2].pos = (kore_float3){x1, y1, 0.5f};
        all_verts[vidx + 2].uv = (kore_float2){gi->s1, gi->t0};
        all_verts[vidx + 3].pos = (kore_float3){x0, y1, 0.5f};
        all_verts[vidx + 3].uv = (kore_float2){gi->s0, gi->t0};
        
        vidx += 4;
        char_idx++;
        xpos += gi->xadvance;
    }
    
    void *vb_ptr = kore_gpu_buffer_lock_all(&g_vertex_buffer);
    memcpy(vb_ptr, all_verts, sizeof(vertex_in) * total_vertices);
    kore_gpu_buffer_unlock_all(&g_vertex_buffer);
    
    void *ib_ptr = kore_gpu_buffer_lock_all(&g_index_buffer);
    memcpy(ib_ptr, all_indices, sizeof(uint16_t) * total_indices);
    kore_gpu_buffer_unlock_all(&g_index_buffer);
    
    kore_matrix4x4 mvp = kore_matrix4x4_identity();
    {
        constants_type *ptr = constants_type_buffer_lock(&g_uniform_buffer, 0, 1);
        if (ptr) {
            ptr->mvp = mvp;
            ptr->color = (kore_float4){r, g, b, a};
            constants_type_buffer_unlock(&g_uniform_buffer);
        }
    }
    
    everything_set_update updates[1];
    updates[0].kind = EVERYTHING_SET_UPDATE_CONSTANTS;
    updates[0].constants = &g_uniform_buffer;
    kong_update_everything_set(&font->set, updates, 1);
    
    kore_log(KORE_LOG_LEVEL_INFO, "draw_string: calling draw, indices=%d", total_indices);
    
    kong_set_render_pipeline_text_pipeline(g_list);
    kore_metal_command_list_set_vertex_buffer(g_list, 0, &g_vertex_buffer.metal, 0, sizeof(vertex_in) * total_vertices, sizeof(vertex_in));
    kore_gpu_command_list_set_index_buffer(g_list, &g_index_buffer, KORE_GPU_INDEX_FORMAT_UINT16, 0);
    kong_set_descriptor_set_everything(g_list, &font->set);
    
    kore_gpu_command_list_draw_indexed(g_list, total_indices, 1, 0, 0, 0);
    
    free(all_verts);
    free(all_indices);
}

float draw_string_width(draw_font *font, const char *utf8_text) {
    if (!font || !utf8_text) return 0.0f;
    
    float width = 0.0f;
    const char *p = utf8_text;
    
    while (*p) {
        int len;
        uint32_t codepoint = utf8_decode(p, &len);
        p += len;
        
        glyph_info *gi = find_glyph(font, codepoint);
        if (gi) {
            width += gi->xadvance;
        }
    }
    
    return width;
}

void draw_shutdown(void) {
    if (g_vertex_buffer.metal.buffer) {
        kore_gpu_buffer_destroy(&g_vertex_buffer);
    }
    if (g_index_buffer.metal.buffer) {
        kore_gpu_buffer_destroy(&g_index_buffer);
    }
    g_initialized = false;
}

void draw_font_destroy(draw_font *font) {
    if (!font) return;
    
    if (font->font_data) free(font->font_data);
    if (font->atlas_pixels) free(font->atlas_pixels);
    if (font->glyphs) free(font->glyphs);
    kore_gpu_texture_view_destroy(&font->texture_view);
    kore_gpu_texture_destroy(&font->texture);
    free(font);
}
