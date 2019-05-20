#include "battle_city.h"
#include "map.h"
#include "map2.h"
#include "map3.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xio.h"
#include <math.h>

/*
 * GENERATED BY BC_MEM_PACKER
 * DATE: Wed Jul 08 21:00:48 2015
 */

// ***** 16x16 IMAGES *****
#define IMG_16x16_block			0x017F 		//2 - blok
#define IMG_16x16_enemy		0x023F 		//5 - nepr
#define IMG_16x16_bckgnd		0x027F 		//0 - poz
#define IMG_16x16_door			0x01FF 		//4 - vrata
#define IMG_16x16_bomberman		0x013F 		//1 - bomberm
#define IMG_16x16_brick			0x01BF 		//3 - cigla
#define IMG_16x16_bomb 			0x00FF 		//6 - bomba
// ***** MAP *****

#define MAP_BASE_ADDRESS		0x02BF 	// MAP_OFFSET in battle_city.vhd
#define MAP_X					0
#define MAP_X2					640
#define MAP_Y					4
#define MAP_W					64
#define MAP_H					56

#define REGS_BASE_ADDRESS       ( MAP_BASE_ADDRESS + MAP_WIDTH * MAP_HEIGHT )
//#define REGS_BASE_ADDRESS     (5439)

#define BTN_DOWN( b )           ( !( b & 0x01 ) )
#define BTN_UP( b )             ( !( b & 0x10 ) )
#define BTN_LEFT( b )           ( !( b & 0x02 ) )
#define BTN_RIGHT( b )          ( !( b & 0x08 ) )
#define BTN_SHOOT( b )          ( !( b & 0x04 ) )

#define TANK1_REG_L                     8
#define TANK1_REG_H                     9
#define TANK_AI_REG_L                   4
#define TANK_AI_REG_H                   5
#define TANK_AI_REG_L2                  6
#define TANK_AI_REG_H2                  7
#define TANK_AI_REG_L3                  2
#define TANK_AI_REG_H3                  3
#define TANK_AI_REG_L4                  10
#define TANK_AI_REG_H4                  11
#define TANK_AI_REG_L5                  12
#define TANK_AI_REG_H5                  13
#define TANK_AI_REG_L6                  14
#define TANK_AI_REG_H6                  15
#define TANK_AI_REG_L7                  16
#define TANK_AI_REG_H7                  17
#define TANK_AI_REG_L8					18
#define TANK_AI_REG_H8					19
#define BASE_REG_L						0
#define BASE_REG_H	                    1
#define BOMB_TICK_COUNT					25
#define BOMB_MAX_NUMBER					3


// ***** GLOBAL VARIABLES *****
int lives = 0;
int score = 0;
int mapPart = 1;
int map_move = 0;
int brojac = 0;
int udario_u_blok = 0;
int enemyCnt = 0;
int bW;
int lifeDestroyR=-1;
int zameniSaExplosion = 6;

// ***** ENEMY SPAWN LOCATIONS *****
#define enemy1X 19
#define enemy1Y 3

#define enemy2X 27
#define enemy2Y 5

#define enemy3X 20
#define enemy3Y 17

#define enemy4X 8
#define enemy4Y 18

// definicija za true i false
typedef enum {
	b_false, b_true
} bool_t;

// definicija za directione
typedef enum {
	DIR_LEFT = 0, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_STILL
} direction_t;

typedef enum {
	BACKGROUND = 0, BOMBERMAN, BLOCK, BRICK, DOOR, ENEMY, BOMB, EXPLOSION
} game_objects_t;

// struktura koja sadrzi osobine bombermana
typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int image;

	bool_t destroyed;

	unsigned int reg_l;
	unsigned int reg_h;
} characters;

// struktura koja sadrzi osobine protivnika
typedef struct{
	unsigned int x;
	unsigned int y;
	unsigned int type;

	unsigned int destroyed;
} enemy;

characters bomberman = {
		8,	         				 // x trenutni
		3, 		                     // y trenutni
		IMG_16x16_bomberman,  			 // type

		b_false,                		 // destroyed, false znaci da je ziv

		TANK1_REG_L,            		 // reg_l ?
		TANK1_REG_H             		 // reg_h ?
		};

enemy enemy1 = {
		enemy1X,						// x
		enemy1Y,						// y
		5,              		        // tip objekta, 5 je za protivnike
		0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
};

enemy enemy2 = {
		enemy2X,			    		// x
		enemy2Y,						// y
		5,              		        // tip objekta, 5 je za protivnike
		0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
};

enemy enemy3 = {
		enemy3X,						// x
		enemy3Y,						// y
		5,              		        // tip objekta, 5 je za protivnike
		0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
};

enemy enemy4 = {
		enemy4X,						// x
		enemy4Y,						// y
		5,              		        // tip objekta, 5 je za protivnike
		0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
};

// random generator, nije za kretanje protivnika, ako ova funkcija ne radi ona se lik ne krece i treperi
static void detonate(int x, int y, unsigned char ** map, unsigned char bomb_power) {
	int i;
	for(i = 1; i <= bomb_power; i++) {
		int obstacle_left = obstacles_detection(x, y, map1, DIR_LEFT, i);
		int obstacle_right = obstacles_detection(x, y, map1, DIR_RIGHT, i);
		int obstacle_up = obstacles_detection(x, y, map1, DIR_UP, i);
		int obstacle_down = obstacles_detection(x, y, map1, DIR_DOWN, i);

		if(!obstacle_left) { //Ako nema prepreke
			map[y][x - i] = zameniSaExplosion;
		} else if (obstacle_left == BOMB){
			place_explosion(x - i, y, map, bomb_power);
		}

		if(!obstacle_right == BOMB) {
			map[y][x + i] = zameniSaExplosion;
		} else if (obstacle_right == BOMB){
			place_explosion(x + i, y, map, bomb_power);
		}

		if(!obstacle_up) {
			map[y - i][x] = zameniSaExplosion;
		} else if (obstacle_up == BOMB){
			place_explosion(x, y - i, map, bomb_power);
		}

		if(!obstacle_down) {
			map[y + i][x] = zameniSaExplosion;
		} else if (obstacle_down == BOMB){
			place_explosion(x, y + i, map, bomb_power);
		}
	}
}

/*unsigned int rand_lfsr113(void) {
	static unsigned int z1 = 12345, z2 = 12345;
	unsigned int b;

	b = ((z1 << 6) ^ z1) >> 13;
	z1 = ((z1 & 4294967294U) << 18) ^ b;
	b = ((z2 << 2) ^ z2) >> 27;
	z2 = ((z2 & 4294967288U) << 2) ^ b;

	return (z1 ^ z2);
}*/

// character promenljiva je tip karaktera koji treba postaviti na mapu
static void char_spawn(characters * character) {
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + character->reg_l ),
			(unsigned int )0x8F000000 | (unsigned int )character->image);
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + character->reg_h ),
			((character->y)*16) << 16 | (character->x*16));
}

static void map_update(characters * mario) {
	int x, y;
	long int addr;

	if(lifeDestroyR<5 && bW==0){
		for (y = 0; y < MAP_HEIGHT; y++) {
			for (x = 0; x < MAP_WIDTH; x++) {
				addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR
						+ 4 * (MAP_BASE_ADDRESS + y * MAP_WIDTH + x);
				switch (map1[y][x]) {								//ovde menjam mapu
				case BACKGROUND:
					Xil_Out32(addr, IMG_16x16_bckgnd);
					break;
				case BOMBERMAN:
					Xil_Out32(addr, IMG_16x16_bomberman);
					break;
				case BLOCK:
					Xil_Out32(addr, IMG_16x16_block);
					break;
				case BRICK:
					Xil_Out32(addr, IMG_16x16_brick);
					break;
				case DOOR:
					Xil_Out32(addr, IMG_16x16_door);
					break;
				case ENEMY:
					Xil_Out32(addr, IMG_16x16_enemy);
					break;
				case BOMB:
					Xil_Out32(addr, IMG_16x16_bomb);
					break;
				default:
					Xil_Out32(addr, IMG_16x16_bckgnd);
					break;
				}
			}
		}
	}else if(lifeDestroyR==5){
		for (y = 0; y < MAP_HEIGHT; y++) {
			for (x = 0; x < MAP_WIDTH; x++) {
				addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR
						+ 4 * (MAP_BASE_ADDRESS + y * MAP_WIDTH + x);
				switch (map0[y][x]) {								//ovde menjam mapu
				case 0:
					Xil_Out32(addr, IMG_16x16_bckgnd);
					break;
				case 1:
					Xil_Out32(addr, IMG_16x16_bomberman);
					break;
				case 6:
					Xil_Out32(addr, IMG_16x16_bomb);
					break;
				default:
					Xil_Out32(addr, IMG_16x16_bckgnd);
					break;
				}
			}
		}
	}
	else if(lifeDestroyR<5 && bW==1){
		for (y = 0; y < MAP_HEIGHT; y++) {
			for (x = 0; x < MAP_WIDTH; x++) {
				addr = XPAR_BATTLE_CITY_PERIPH_0_BASEADDR
						+ 4 * (MAP_BASE_ADDRESS + y * MAP_WIDTH + x);
				switch (map3[y][x]) {								//ovde menjam mapu
				case 0:
					Xil_Out32(addr, IMG_16x16_bckgnd);
					break;
				case 1:
					Xil_Out32(addr, IMG_16x16_bomberman);
					break;
				default:
					Xil_Out32(addr, IMG_16x16_bckgnd);
					break;
				}
			}
		}
	}
}

static void map_reset(unsigned char * map) {
	unsigned int i;

	for (i = 0; i <= 20; i += 2) {
		Xil_Out32(
				XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + i ),
				(unsigned int )0x0F000000);
	}
}

int obstacles_detection(int x, int y, unsigned char ** map, direction_t dir, int position_distance) {
	if(position_distance > 0) {

		unsigned char position_right = map1[y][x + position_distance];
		unsigned char position_left = map1[y][x - position_distance];
		unsigned char position_up = map1[y - position_distance][x];
		unsigned char position_down = map1[y + position_distance][x];
		if (dir == DIR_LEFT) {
			return position_left;
		} else if (dir == DIR_RIGHT) {
			return position_right;
		} else if (dir == DIR_UP) {
			return position_up;
		} else if (dir == DIR_DOWN) {
			return position_down;
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}


static bool_t mario_move(unsigned char ** map, characters * mario, direction_t dir) {
	unsigned int x = mario->x;
	unsigned int y = mario->y;

	int i;
	int obstacle = 0;

	obstacle = obstacles_detection(x, y, map, dir, 1);
	if(obstacle == 0) {
		switch(dir) {
		case DIR_LEFT:
			x -= 1;
			break;
		case DIR_RIGHT:
			x += 1;
			break;
		case DIR_UP:
			y -= 1;
			break;
		case DIR_DOWN:
			y += 1;
			break;
		default:
			break;
		}
	}

	mario->x = x;
	mario->y = y;

	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + mario->reg_h ),
			((y*16) << 16) | (x*16));

	for (i = 0; i < 1000000; i++) {
	}

	return b_false;
}

static void find_enemy(int x, int y){
	if(enemy1.x == x && enemy1.y == y){
		enemy1.type = 0;
	}else if(enemy2.x == x && enemy2.y == y){
		enemy2.type= 0;
	}else if(enemy3.x == x && enemy3.y == y){
		enemy3.type = 0;
	}else if(enemy4.x == x && enemy4.y == y){
		enemy4.type = 0;
	}
}

static int bomberman_and_enemy(characters *mario)
{
	unsigned int xX;
	unsigned int yY;

	xX=mario->x;
	yY=mario->y;

	if(enemy1.x==xX && enemy1.y==yY){
		return 1;
	}else if(enemy2.x==xX && enemy2.y==yY){
		return 1;
	}else if(enemy3.x==xX && enemy3.y==yY){
		return 1;
	}else if(enemy4.x==xX && enemy4.y==yY){
		return 1;
	}else {
		return 0;
	}

}


static int find_bomberman(characters *mario, int x, int y)
{

	unsigned int xX = mario->x;
	unsigned int yY = mario->y;

	//xil_printf("MAR: x=%d\ty=%d\n\r", xX, yY);
	//xil_printf("BMB: x=%d\ty=%d\n\r\n\r", x, y);

	if(xX==x && yY==y){
		return 1;
	}else if(xX==x+1 && yY==y){
		return 1;
	}else if(xX==x+2 && yY==y){
		return 1;
	}else if(xX==x && yY==y+1){
		return 1;
	}else if(xX==x && yY==y+2){
		return 1;
	}
	else if(xX==x && yY==y-1){
			return 1;
		}
	else if(xX==x && yY==y-2){
			return 1;
		}
	else if(xX==x-1 && yY==y){
			return 1;
		}
	else if(xX==x-2 && yY==y){
			return 1;
		}
	else {
		return 0;
	}
}

static int bomberman_win(characters *mario){
	unsigned int xX;
	unsigned int yY;

	xX=(mario->x)/16;
	yY=(mario->y)/16;

	if(yY==12 && xX==18){
		return 1;
	}else {
		return 0;
	}
}

static int destroy_direction(int x, int y, int obstacle, characters * mario, int fB, int bE){
	if(fB == 1){									// bomberman
			return 2;
		}
		if(bE==1){
			return 5;
		}
	if(obstacle == BRICK){										// cigla
		return 1;
	}else if(obstacle == ENEMY){								// enemy
		find_enemy(x, y);
		return 3;
	}else if(obstacle == BLOCK){								// blok
		return -1;
	}else{
		return 0;
	}
}


static void destroy(unsigned char ** map, int x, int y, characters * mario){
	int obstacle = 0;
	int fB=0;
	int bE=0;

	//Right destroy
	obstacle = obstacles_detection(x, y, map, 0, 1);
	fB = find_bomberman(mario, x, y);
	bE=bomberman_and_enemy(mario);
	bW=bomberman_win(mario);
	if(bW==1){
		map_update(mario);
	}


	if(destroy_direction(x+1, y, obstacle, mario, fB, bE) == 1){
		map1[y][x+1] = 0;
	}else if (destroy_direction(x+1, y, obstacle, mario, fB, bE) == ENEMY){
		map1[y][x+1]=0;
		if(++enemyCnt==4){
			map1[12][18]=4;
		}
	}else if(destroy_direction(x+1,y,obstacle, mario, fB, bE )==2 || bE==1 ){
		lifeDestroyR++;
		if(lifeDestroyR==3){
		map1[2][35]=0;
		}else if(lifeDestroyR==4){
			map1[2][36]=0;
		}else if(lifeDestroyR==5){
			map_update(mario);
		}

	}
	else if(destroy_direction(x+1, y, obstacle, mario, fB, bE) == 0) {
		obstacle = obstacles_detection(x+1, y, map, 0, 1);
		fB = find_bomberman(mario, x,y);
		bE=bomberman_and_enemy(mario);
		if(destroy_direction(x+2, y, obstacle, mario, fB, bE) == 1){
				map1[y][x+2] = 0;
		} else if (destroy_direction(x+2, y, obstacle,mario, fB, bE) == ENEMY){
				map1[y][x-1]=0;
				if(++enemyCnt==4){
					map1[12][18]=DOOR;
				}
		}else if(destroy_direction(x+2,y,obstacle, mario, fB, bE )==2 || bE==1){
			lifeDestroyR++;
			if(lifeDestroyR==3){
			map1[2][35]=0;
			}else if(lifeDestroyR==4){
				map1[2][36]=0;
			}else if(lifeDestroyR==5){
				map_update(mario);
			}
		}
	}

	//Left destroy
	obstacle = obstacles_detection(x, y, map, 1, 1);
	fB = find_bomberman(mario, x,y);
	bE=bomberman_and_enemy(mario);
	if(destroy_direction(x-1, y, obstacle,mario, fB, bE) == 1){
		map1[y][x-1] = 0;
	}
	else if (destroy_direction(x-1, y, obstacle,mario, fB, bE) == ENEMY){
			map1[y][x-1]=0;
			if(++enemyCnt==4){
				map1[12][18]=4;
			}
	}else if(destroy_direction(x-1,y,obstacle, mario, fB, bE )==2 || bE==1){
		lifeDestroyR++;
		if(lifeDestroyR==3){
		map1[2][35]=0;
		}else if(lifeDestroyR==4){
			map1[2][36]=0;
		}else if(lifeDestroyR==5){
			map_update(mario);
		}
	}
	else if(destroy_direction(x-1, y, obstacle,mario, fB, bE) == 0) {
		map1[mario->y+1][mario->x]=4;
		obstacle = obstacles_detection(x-1, y, map, 1, 1);
		fB = find_bomberman(mario, x,y);
		bE=bomberman_and_enemy(mario);
		if(destroy_direction(x-2, y, obstacle,mario, fB, bE) == 1){
				map1[y][x-2] = 0;
		}else if (destroy_direction(x-2, y, obstacle,mario, fB, bE) == ENEMY){
				map1[y][x-1]=0;
				if(++enemyCnt==4){
					map1[12][18]=4;
				}
		}else if(destroy_direction(x-2,y,obstacle, mario, fB, bE )==2  || bE==1){
			lifeDestroyR++;
			if(lifeDestroyR==3){
			map1[2][35]=0;
			}else if(lifeDestroyR==4){
				map1[2][36]=0;
			}else if(lifeDestroyR==5){
				map_update(mario);
			}
		}
	}

	//Up destroy
	obstacle = obstacles_detection(x, y, map, 2, 1);
	fB = find_bomberman(mario, x,y);
	bE=bomberman_and_enemy(mario);
	if(destroy_direction(x, y-1, obstacle,mario, fB, bE) == 1){
		map1[y-1][x] = 0;
	}else if (destroy_direction(x, y-1, obstacle,mario, fB, bE) == ENEMY){
		map1[y-1][x]=0;
		if(++enemyCnt==4){
			map1[12][18]=4;
		}
	}else if(destroy_direction(x,y-1,obstacle, mario, fB, bE )==2 || bE==1){
		lifeDestroyR++;
		if(lifeDestroyR==3){
		map1[2][35]=0;
		}else if(lifeDestroyR==4){
			map1[2][36]=0;
		}else if(lifeDestroyR==5){
			map_update(mario);
		}
	}
	else if(destroy_direction(x, y-1, obstacle,mario, fB, bE) == 0) {
		obstacle = obstacles_detection(x, y-1, map, 2, 1);
		fB = find_bomberman(mario, x,y);
		bE=bomberman_and_enemy(mario);
		if(destroy_direction(x, y-2, obstacle,mario, fB, bE) == 1){
				map1[y-2][x] = 0;
		}else if (destroy_direction(x, y-2, obstacle,mario, fB, bE) == ENEMY){
				map1[y][x-1]=0;
				if(++enemyCnt==4){
					map1[12][18]=4;
				}
		}else if(destroy_direction(x,y-2,obstacle, mario, fB, bE )==2 || bE==1){
			lifeDestroyR++;
			if(lifeDestroyR==3){
			map1[2][35]=0;
			}else if(lifeDestroyR==4){
				map1[2][36]=0;
			}else if(lifeDestroyR==5){
				map_update(mario);
			}
		}
	}

	//Down destroy
	obstacle = obstacles_detection(x, y, map, 3, 1);
	fB = find_bomberman(mario, x,y);
	bE=bomberman_and_enemy(mario);
	if(destroy_direction(x, y+1, obstacle,mario, fB, bE) == 1){
		map1[y+1][x] = 0;
	}else if (destroy_direction(x, y+1, obstacle,mario, fB, bE) == ENEMY){
		map1[y+1][x]=0;
		if(++enemyCnt==4){
			map1[12][18]=4;
		}
	}else if(destroy_direction(x,y+1,obstacle, mario, fB, bE )==2  || bE==1){
		lifeDestroyR++;
		if(lifeDestroyR==3){
		map1[2][35]=0;
		}else if(lifeDestroyR==4){
			map1[2][36]=0;
		}else if(lifeDestroyR==5){
			map_update(mario);
		}
	}
	else if(destroy_direction(x, y+1, obstacle,mario, fB, bE) == 0) {
		obstacle = obstacles_detection(x, y+1, map, 3, 1);
		fB = find_bomberman(mario, x,y);
		bE=bomberman_and_enemy(mario);
		if(destroy_direction(x, y+2, obstacle,mario, fB, bE) == 1){
			map1[y+2][x] = 0;
		}else if (destroy_direction(x, y+2, obstacle,mario, fB, bE) == ENEMY){
				map1[y][x-1]=0;
				if(++enemyCnt==4){
					map1[12][18]=4;
			}
		}else if(destroy_direction(x,y+2,obstacle, mario, fB, bE )==2 || bE==1){
			lifeDestroyR++;
			if(lifeDestroyR==3){
			map1[2][35]=0;
			}else if(lifeDestroyR==4){
				map1[2][36]=0;
			}else if(lifeDestroyR==5){
				map_update(mario);
			}
		}
	}

}

static void random_move_enemy(enemy * enm, unsigned char ** map)
{
	int i = 0;
	direction_t dir = (direction_t)(rand()%4);
	int x = enm->x;
	int y = enm->y;
	int obstacle = obstacles_detection(x, y, map, dir, 1);

	if(!obstacle) {
		map1[y][x] = 0;
		switch(dir) {
		case DIR_LEFT:
			x -= 1;
			break;
		case DIR_RIGHT:
			x += 1;
			break;
		case DIR_UP:
			y -= 1;
			break;
		case DIR_DOWN:
			y += 1;
			break;
		default:;
		}

		map1[y][x] = ENEMY;
		enm->x = x;
		enm->y = y;
	}

	for (i = 0; i < 100000; i++) {
	}
}

void battle_city() {
	unsigned int buttons;
	int i;

	map_reset(map1);
	map_update(&bomberman);
	char_spawn(&bomberman);

	int active_bombs = 0;
	int x, y;
	int bombs_coordinates[3][2], active_bombs_index = 0, exploding_bombs_index = 0;
	int bombs_tick_counter[3] = {-1, -1, -1}; //nema postavljenih
	int pom;
	int available_bombs = 3;

	map1[enemy1Y][enemy1X] = 5;

	map1[enemy2Y][enemy2X] = 5;

	map1[enemy3Y][enemy3X] = 5;

	map1[enemy4Y][enemy4X] = 5;

	while (1) {
		buttons = XIo_In32( XPAR_IO_PERIPH_BASEADDR );

		direction_t d = DIR_STILL;
		if (BTN_LEFT(buttons)) {
			d = DIR_LEFT;
		} else if (BTN_RIGHT(buttons)) {
			d = DIR_RIGHT;
		} else if (BTN_UP(buttons)){
			d = DIR_UP;
		} else if (BTN_DOWN(buttons)){
			d = DIR_DOWN;
		} else if(BTN_SHOOT(buttons) && active_bombs < available_bombs){
			x = bomberman.x;
			y = bomberman.y;

			pom = (active_bombs_index++)%3; //cirkularni buffer za ove sto nisu eksplodirale
			bombs_coordinates[pom][0] = y; //KOD NJIH PRVO IDE Y KOORDINATA IZ NEKOG RAZLOGA
			bombs_coordinates[pom][1] = x;
			bombs_tick_counter[pom] = BOMB_TICK_COUNT;
			map1[y][x] = BOMB;
			active_bombs++;
		}
		//Ovde se ulazi kad istekne bomba
		if(active_bombs > 0){
			for(i = 0; i < 3; i++) {
				if(bombs_tick_counter[i]-- == 0){
					pom = (exploding_bombs_index++)%3; //Cirkularni buffer za eksplodirajuce

					map1[bombs_coordinates[pom][0]][bombs_coordinates[pom][1]] = BACKGROUND;
					destroy(map1, bombs_coordinates[pom][1], bombs_coordinates[pom][0], &bomberman);

					active_bombs--;
				}
			}
		}

		random_move_enemy(&enemy1, map1);
		random_move_enemy(&enemy2, map1);
		random_move_enemy(&enemy3, map1);
		random_move_enemy(&enemy4, map1);

		mario_move(map1, &bomberman, d);

		map_update(&bomberman);

		for (i = 0; i < 100000; i++) {
		}

	}
}

