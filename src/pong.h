/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define FREE_TILE           0x01
#define BACKGROUND_TILE     0x11
#define PLAYER_TILE         0x09
#define BALL_TILE           0xA
#define PLAYER2_TILE        0xB
#define WIDTH               20
#define HEIGHT              15
#define TILE_SIZE           (16  * 16)
#define TILEMAP_LAYER       0
#define HUD_LAYER           1

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
