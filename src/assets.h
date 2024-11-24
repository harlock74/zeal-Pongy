
extern uint8_t _palette_end;
extern uint8_t _palette_start;
extern uint8_t _tiles_end;
extern uint8_t _tiles_start;
extern uint8_t _tilemap_start;
extern uint8_t _tilemap_end;

gfx_error load_palette(gfx_context* ctx);
gfx_error load_tiles(gfx_context* ctx, gfx_tileset_options* options);

uint8_t* get_tilemap_start(void);
uint8_t* get_tilemap_end(void);