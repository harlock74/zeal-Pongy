#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_keyboard.h>
#include <zos_time.h>
#include <zos_video.h>
#include <zvb_gfx.h>
#include <zvb_hardware.h>
#include "controller.h"
#include "keyboard.h"
#include "utils.h"
#include "pong.h"

gfx_context vctx;
Player player;
Player player2;
Player ball;
static uint8_t controller_mode = 1;
static uint8_t frames = 0;
// static uint16_t buttons = 0;

#define PLAYER_SPEED   1
#define BALL_WIDTH     3
#define PLAYER_HEIGHT  16
#define PLAYER_WIDTH   16
#define REAL_PLAYER_WIDTH  6
#define SCREEN_TOP_BOUNDARY   24
#define SCREEN_BOTTOM_BOUNDARY   232
#define BALL_TOP_BOUNDARY   8
#define BALL_BOTTOM_BOUNDARY   232

static uint8_t tilemap[WIDTH * HEIGHT];

int main(void) {
    init();

    while (input() != 0) {
        update();
        gfx_wait_vblank(&vctx);
        frames++;
        if(frames == PLAYER_SPEED) {
            frames = 0;
            update_hud();
            draw();
        }

        gfx_wait_end_vblank(&vctx);
    }

    deinit();

    return 0;
}

void init(void) {
    zos_err_t err = keyboard_init();
    if(err != ERR_SUCCESS) {
        printf("Failed to init keyboard: %d", err);
        exit(1);
    }
    err = controller_init();
    if(err != ERR_SUCCESS) {
        printf("Failed to init controller: %d", err);
    }
    // verify the controller is actually connected
    uint16_t test = controller_read();
    // if unconnected, we'll get back 0xFFFF (all buttons pressed)
    if(test & 0xFFFF) {
        controller_mode = 0;
    }

    /* Disable the screen to prevent artifacts from showing */
    gfx_enable_screen(0);

    err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    if (err) exit(1);

    // Load the palette
    extern uint8_t _pong_palette_end;
    extern uint8_t _pong_palette_start;
    size_t size = &_pong_palette_end - &_pong_palette_start;
    err = gfx_palette_load(&vctx, &_pong_palette_start, size, 0);
    if (err) exit(1);

    // Load the tiles
    extern uint8_t _pong_tileset_end;
    extern uint8_t _pong_tileset_start;
    size = &_pong_tileset_end - &_pong_tileset_start;
    gfx_tileset_options options = {
        .compression = TILESET_COMP_NONE,
    };
    err = gfx_tileset_load(&vctx, &_pong_tileset_start, size, &options);
    if (err) exit(1);

    extern uint8_t _numbers_tileset_end;
    extern uint8_t _numbers_tileset_start;
    size = &_numbers_tileset_end - &_numbers_tileset_start;
    options.compression = TILESET_COMP_RLE;
    options.from_byte = TILE_SIZE * 44; // 0x6100 // ASCII 44, comma
    err = gfx_tileset_load(&vctx, &_numbers_tileset_start, size, &options);
    if (err) exit(1);


    // Draw the tilemap
    load_tilemap();

    // Setup the player
    player.sprite.x = 16;
    player.sprite.y = 120;
    player.score = 0;
    player.level = 1;
    player.sprite_index = PLAYER_TILE;
    player.sprite.flags = SPRITE_NONE;
    player.sprite.tile = PLAYER_TILE;
    gfx_sprite_set_tile(&vctx, 0, PLAYER_TILE);

    // Setup the ball
    ball.sprite.x = 160;
    ball.sprite.y = 128;
    ball.score = 0;
    ball.level = 1;
    ball.sprite_index = BALL_TILE;
    ball.sprite.flags = SPRITE_NONE;
    ball.sprite.tile = BALL_TILE;
    gfx_sprite_set_tile(&vctx, 1, BALL_TILE);

    // Setup player 2
    player2.sprite.x = 320;
    player2.sprite.y = 180;
    player2.score = 0;
    player2.level = 1;
    player2.sprite_index = PLAYER2_TILE;
    player2.sprite.flags = SPRITE_NONE;
    player2.sprite.tile = PLAYER2_TILE;
    gfx_sprite_set_tile(&vctx, 2, PLAYER2_TILE);

    gfx_enable_screen(1);
}

void deinit(void) {
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    // TODO: clear sprites
    // TODO: clear tilesets
}


void load_tilemap(void) {
    uint8_t line[WIDTH];

    // Load the tilemap
    extern uint8_t _pong_tilemap_start;
    for (uint16_t y = 0; y < HEIGHT; y++) {
        uint16_t offset = y * WIDTH;
        memcpy(&line, &_pong_tilemap_start + offset, WIDTH);
        memcpy(&tilemap[offset], &line, WIDTH);
        gfx_tilemap_load(&vctx, line, WIDTH, TILEMAP_LAYER, 0, y);
    }
}

uint8_t input(void) {
    uint16_t buttons = keyboard_read();
    if(controller_mode == 1) {
        buttons |= controller_read();
    }
    //player.h_direction = 0;
    player.v_direction = 0;
    //if(buttons & SNES_RIGHT) player.h_direction = 1;
    //if(buttons & SNES_LEFT) player.h_direction = -1;
    if(buttons & SNES_DOWN) player.v_direction = 1;
    if(buttons & SNES_UP) player.v_direction = -1;
    if(buttons & SNES_START) return 0;
    return 255;
}
/*
uint16_t get_tile(int8_t x, int8_t y) {
    // TODO: remove this debug display
    char text[10];
    sprintf(text, "%02d", x);
    nprint_string(&vctx, text, strlen(text), 1, HEIGHT - 3);
    sprintf(text, "%02d", y);
    nprint_string(&vctx, text, strlen(text), 5, HEIGHT - 3);

    int16_t offset = (y * WIDTH) + x;
    // out of bounds, assume a wall
    if(offset < 0 || offset > (WIDTH * HEIGHT) - 1) return 1;
    return tilemap[offset];
}
*/
void update(void) {

    static int8_t dx =1;
    static int8_t dy =1;
    int8_t angle = (player.sprite.y - ball.sprite.y)/8;
    int8_t angle2 = (player2.sprite.y - ball.sprite.y)/8;
   
       /*
    // TODO: remove this debug display
    char text[10];
    sprintf(text, "%03d", player.sprite.x);
    nprint_string(&vctx, text, strlen(text), 0, HEIGHT - 1);
    sprintf(text, "%03d", player.sprite.y);
    nprint_string(&vctx, text, strlen(text), 4, HEIGHT - 1);
    sprintf(text, "%03d", ball.sprite.x);
    nprint_string(&vctx, text, strlen(text), 1, HEIGHT - 2);
    sprintf(text, "%03d", ball.sprite.y);
    nprint_string(&vctx, text, strlen(text), 5, HEIGHT - 2);
       */
  

    
    //Ball bouncing physics
    ball.sprite.x += dx;
    ball.sprite.y += dy;
    
    //Player 1 bouncing physics
    player.sprite.y += player.v_direction*2;
    if ((ball.sprite.x - BALL_WIDTH <= player.sprite.x) && 
       (ball.sprite.x - BALL_WIDTH >= player.sprite.x - REAL_PLAYER_WIDTH) && 
       (ball.sprite.y <= player.sprite.y) && 
       (ball.sprite.y >= (player.sprite.y - PLAYER_HEIGHT))) 
       //dx=1;
       
       {
        if (angle == 7) { 
        dx=-dx; 
        dy=2;
       }
       else if (angle == 6) { 
        dx=-dx; 
        dy=1;
       }
       else if (angle == 5 ) { 
        dx=-dx; 
        dy=1;
       }
       else if (angle == 4) { 
        dx=-dx; 
        dy=1;
       }
       else if (angle == 3) { 
        dx=-dx; 
        dy=1;
       }
       else if (angle == 2) { 
        dx=-dx; 
        dy=1;
       }
      else if (angle == 1) { 
        dx=-dx; 
        dy=2;
       }
    
      else if (angle == 0) { 
        dx=-dx; 
        dy=2;
       }

       }

    //Player 2 bouncing physics
    else if ((ball.sprite.x >= player2.sprite.x - PLAYER_WIDTH) &&
    (ball.sprite.x <= player2.sprite.x - 10) &&
    (ball.sprite.y <= player2.sprite.y) && 
    (ball.sprite.y >= (player2.sprite.y - PLAYER_HEIGHT))){
        dx=-dx;

    }

    //AI Vertical Control

    if (ball.sprite.y < player2.sprite.y) player2.sprite.y -= 4;
    else if (ball.sprite.y > player2.sprite.y) player2.sprite.y += 4;

    //Reset ball and players positin when the ball goes out
    if ( ball.sprite.x <=0)
    {
    ball.sprite.x = 160;
    ball.sprite.y = 128;
    player.sprite.x = 16;
    player.sprite.y = 120;
    player2.sprite.x = 320;
    player2.sprite.y = 180;
    player2.score += 1;
    }  

    //Reset ball and players positin when the ball goes out
    if (ball.sprite.x >= 320)
    {
    ball.sprite.x = 160;
    ball.sprite.y = 128;
    player.sprite.x = 16;
    player.sprite.y = 120;
    player2.sprite.x = 320;
    player2.sprite.y = 180;
    player.score += 1;
    }  

    //Ball boundary limits
    if (ball.sprite.y >= BALL_BOTTOM_BOUNDARY) dy=-2;
    if (ball.sprite.y <= BALL_TOP_BOUNDARY) dy=2;
    
    //Player1 boundary limits
    if (player.sprite.y < SCREEN_TOP_BOUNDARY) player.sprite.y = SCREEN_TOP_BOUNDARY;
    if (player.sprite.y > SCREEN_BOTTOM_BOUNDARY) player.sprite.y = SCREEN_BOTTOM_BOUNDARY;
  
    //Player2 boundary limits
    if (player2.sprite.y < SCREEN_TOP_BOUNDARY) player2.sprite.y = SCREEN_TOP_BOUNDARY;
    if (player2.sprite.y > SCREEN_BOTTOM_BOUNDARY) player2.sprite.y = SCREEN_BOTTOM_BOUNDARY;

}

void update_hud(void) {
    char text[10];
    sprintf(text, "%02d", player.score);
    nprint_string(&vctx, text, strlen(text), 7, 1);

    sprintf(text, "%02d", player2.score);
    nprint_string(&vctx, text, strlen(text), 11, 1);
}

void draw(void) {
    uint8_t err = gfx_sprite_render(&vctx, 0, &player.sprite);
    err = gfx_sprite_render(&vctx, 1, &ball.sprite);
    err = gfx_sprite_render(&vctx, 2, &player2.sprite);
    if(err != 0) {
        printf("graphics error: %d", err);
        exit(1);
    }
}

void __assets__(void) {
    __asm__(
    // pong palette
    "__pong_palette_start:\n"
    "    .incbin \"assets/pong.ztp\"\n"
    "__pong_palette_end:\n"
    // pong tiles
    "__pong_tileset_start:\n"
    "    .incbin \"assets/pong.zts\"\n"
    "__pong_tileset_end:\n"

    // numbers palette
    "__numbers_palette_start:\n"
    "    .incbin \"assets/numbers.ztp\"\n"
    "__numbers_palette_end:\n"

    // numbers tiles
    "__numbers_tileset_start:\n"
    "    .incbin \"assets/numbers.zts\"\n"
    "__numbers_tileset_end:\n"

    // tilemap
    "__pong_tilemap_start:\n"
    "    .incbin \"assets/pong.ztm\"\n"
    "__pong_tilemap_end:\n"
    );
}