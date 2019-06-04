#ifndef BATTLE_CITY_H_
#define BATTLE_CITY_H_

#include "maps.h"

// struktura koja sadrzi osobine bombermana
typedef struct {
	unsigned char x;
	unsigned char y;
	unsigned int image;
	unsigned char destroyed;
	unsigned char bomb_power;
	unsigned char bomb_count;
	signed char lives;
	unsigned char enemies_destroyed;
	unsigned char win_condition;
	unsigned char lose_condition;
	unsigned char active_bombs;
	unsigned char inverbable;

	unsigned int reg_l;
	unsigned int reg_h;
} bomberman_t;

typedef enum {
	DIR_LEFT = 0, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_STILL
} direction_t;

// struktura koja sadrzi osobine protivnika

typedef struct bomb {
	unsigned char x; 	//x i y koordinata bombe
	unsigned char y;
	char tick_counter;
	unsigned char available;
	unsigned char placed;
}bomb_t;

typedef enum {
	BACKGROUND = 0, BOMBERMAN, BLOCK, BRICK, DOOR, ENEMY, BOMB, PLUS_BOMB, PLUS_EXPLOSION
} game_objects_t;

void battle_city(map_structure_t * map);
void draw_map(unsigned char map[30][40]);
void char_spawn(bomberman_t * character);
void wait(long int milliseconds);


#endif /* BATTLE_CITY_H_ */
