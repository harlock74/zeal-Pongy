/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define FREE_TILE           1
#define BACKGROUND_TILE     0
#define PLAYER1_TILE         9
#define BALL_TILE           10
#define PLAYER2_TILE        11
#define WIDTH               20
#define HEIGHT              15
#define TILEMAP_LAYER       0
#define HUD_LAYER           1

#define PLAYER_SPEED                1
#define BALL_WIDTH                  3
#define PLAYER_HEIGHT               16
#define PLAYER_WIDTH                16
#define REAL_PLAYER_WIDTH           6
#define SCREEN_TOP_BOUNDARY         24
#define SCREEN_BOTTOM_BOUNDARY      232
#define BALL_TOP_BOUNDARY           8
#define BALL_BOTTOM_BOUNDARY        232

typedef struct {
    gfx_sprite sprite;
    uint8_t sprite_index;

    int8_t h_direction;
    int8_t v_direction;
    uint16_t score;
    uint8_t level;
} Player;

void init(void);
void deinit(void);
void load_tilemap(void);
uint8_t input(void);
void update(void);
void update_hud(void);
void draw(void);
