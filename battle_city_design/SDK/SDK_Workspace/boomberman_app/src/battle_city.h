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
typedef enum {
	BACKGROUND = 0, BOMBERMAN, BLOCK, BRICK, DOOR, ENEMY, BOMB, PLUS_BOMB, PLUS_EXPLOSION,
	EXPLOSION_CENTER1 = 11, EXPLOSION_CENTER2, EXPLOSION_CENTER3, EXPLOSION_CENTER4, EXPLOSION_CENTER5, EXPLOSION_CENTER6, EXPLOSION_CENTER7,
	EXPLOSION_HORIZONTAL1 = 21, EXPLOSION_HORIZONTAL2, EXPLOSION_HORIZONTAL3, EXPLOSION_HORIZONTAL4, EXPLOSION_HORIZONTAL5, EXPLOSION_HORIZONTAL6, EXPLOSION_HORIZONTAL7,
	EXPLOSION_VERTICAL1 = 31, EXPLOSION_VERTICAL2, EXPLOSION_VERTICAL3, EXPLOSION_VERTICAL4, EXPLOSION_VERTICAL5, EXPLOSION_VERTICAL6, EXPLOSION_VERTICAL7,
	EXPLOSION_END_LEFT1 = 41, EXPLOSION_END_LEFT2, EXPLOSION_END_LEFT3, EXPLOSION_END_LEFT4, EXPLOSION_END_LEFT5, EXPLOSION_END_LEFT6, EXPLOSION_END_LEFT7,
	EXPLOSION_END_RIGHT1 = 51,  EXPLOSION_END_RIGHT2, EXPLOSION_END_RIGHT3, EXPLOSION_END_RIGHT4, EXPLOSION_END_RIGHT5, EXPLOSION_END_RIGHT6, EXPLOSION_END_RIGHT7,
	EXPLOSION_END_UP1 = 61, EXPLOSION_END_UP2, EXPLOSION_END_UP3, EXPLOSION_END_UP4, EXPLOSION_END_UP5, EXPLOSION_END_UP6, EXPLOSION_END_UP7,
	EXPLOSION_END_DOWN1 = 71, EXPLOSION_END_DOWN2, EXPLOSION_END_DOWN3, EXPLOSION_END_DOWN4, EXPLOSION_END_DOWN5, EXPLOSION_END_DOWN6, EXPLOSION_END_DOWN7
} game_objects_t;

typedef struct bomb {
	unsigned char x; 	//x i y koordinata bombe
	unsigned char y;
	char tick_counter;
	unsigned char available;
	unsigned char placed;
	unsigned char bomb_explosion_stage;
}bomb_t;


void battle_city(map_structure_t * map);
void draw_map(unsigned char map[30][40]);
void char_spawn(bomberman_t * character);
void wait(long int milliseconds);


#endif /* BATTLE_CITY_H_ */
