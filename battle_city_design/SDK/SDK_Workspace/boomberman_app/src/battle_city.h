#ifndef BATTLE_CITY_H_
#define BATTLE_CITY_H_

#define MAP_WIDTH           40  // 40
#define MAP_HEIGHT          30 // 30

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480
#define MAP_BASE_ADDRESS	0x02BF 		// MAP_OFFSET in battle_city.vhd

// ***** 16x16 IMAGES *****
#define IMG_16x16_block			0x017F 		//2 - blok
#define IMG_16x16_enemy			0x023F 		//5 - nepr
#define IMG_16x16_bckgnd		0x027F 		//0 - poz
#define IMG_16x16_door			0x01FF 		//4 - vrata
#define IMG_16x16_bomberman		0x013F 		//1 - bomberm
#define IMG_16x16_brick			0x01BF 		//3 - cigla
#define IMG_16x16_bomb 			0x00FF 		//6 - bomba

#include "maps.h"

// struktura koja sadrzi osobine bombermana
typedef struct {
	unsigned char x;
	unsigned char y;
	unsigned int image;
	unsigned char destroyed;
	signed char lives;

	unsigned int reg_l;
	unsigned int reg_h;
} bomberman_t;

#define BTN_DOWN( b )           ( !( b & 0x01 ) )
#define BTN_UP( b )             ( !( b & 0x10 ) )
#define BTN_LEFT( b )           ( !( b & 0x02 ) )
#define BTN_RIGHT( b )          ( !( b & 0x08 ) )
#define BTN_SHOOT( b )          ( !( b & 0x04 ) )

void battle_city(map_structure_t passed_map);
void draw_map(unsigned char map[30][40]);
void char_spawn(unsigned char map[30][40], bomberman_t * character);
void wait(int wait_time);

#endif /* BATTLE_CITY_H_ */
