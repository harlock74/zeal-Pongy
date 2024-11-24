#include <stdio.h>
#include <zvb_gfx.h>
#include "assets.h"

gfx_error load_palette(gfx_context* ctx)
{
    // Load the palette
    const size_t palette_size = &_palette_end - &_palette_start;
    return gfx_palette_load(ctx, &_palette_start, palette_size, 0);
}

gfx_error load_tiles(gfx_context* ctx, gfx_tileset_options* options)
{
    const size_t size = &_tiles_end - &_tiles_start;
    return gfx_tileset_load(ctx, &_tiles_start, size, options);
}

uint8_t* get_tilemap_start(void) {
    return &_tilemap_start;
}

uint8_t* get_tilemap_end(void) {
    return &_tilemap_end;
}

void __assets__(void) {
    __asm__(
    // pong palette
    "__palette_start:\n"
    "    .incbin \"assets/pong.ztp\"\n"
    "__palette_end:\n"
    // pong tiles
    "__tiles_start:\n"
    "    .incbin \"assets/pong.zts\"\n"
    "__tiles_end:\n"

    // tilemap
    "__tilemap_start:\n"
    "    .incbin \"assets/tilemap.ztm\"\n"
    "__tilemap_end:\n"
    );
}