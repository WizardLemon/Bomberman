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
#define IMG_16x16_enemie		0x023F 		//5 - nepr
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

// ***** GLOBAL VARIABLES *****
int lives = 0;
int score = 0;
int mapPart = 1;
int udario_glavom_skok = 0;
int map_move = 0;
int brojac = 0;
int udario_u_blok = 0;
int enemieCnt;
int bW;
int lifeDestroyR=-1;


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
	DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_STILL, BOMB
} direction_t;

// struktura koja sadrzi osobine bombermana
typedef struct {
	unsigned int x;
	unsigned int y;
	direction_t dir;
	unsigned int type;

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
} enemie;

characters bomberman = {
		128,	         				 // x trenutni
		49, 		                     // y trenutni
		DIR_RIGHT,              		 // dir
		IMG_16x16_bomberman,  			 // type

		b_false,                		 // destroyed, false znaci da je ziv

		TANK1_REG_L,            		 // reg_l ?
		TANK1_REG_H             		 // reg_h ?
		};

enemie enemie1 = {
		enemy1X,						// x
		enemy1Y,						// y
		5,              		        // tip objekta, 5 je za protivnike
		0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
};

enemie enemie2 = {
		enemy2X,			    		// x
		enemy2Y,						// y
		5,              		        // tip objekta, 5 je za protivnike
		0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
};

enemie enemie3 = {
		enemy3X,						// x
		enemy3Y,						// y
		5,              		        // tip objekta, 5 je za protivnike
		0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
};

enemie enemie4 = {
		enemy4X,						// x
		enemy4Y,						// y
		5,              		        // tip objekta, 5 je za protivnike
		0								// destroyed, 0 znaci da je ziv, 1 da je mrtav, unsigned int
};

// random generator, nije za kretanje protivnika, ako ova funkcija ne radi ona se lik ne krece i treperi
unsigned int rand_lfsr113(void) {
	static unsigned int z1 = 12345, z2 = 12345;
	unsigned int b;

	b = ((z1 << 6) ^ z1) >> 13;
	z1 = ((z1 & 4294967294U) << 18) ^ b;
	b = ((z2 << 2) ^ z2) >> 27;
	z2 = ((z2 & 4294967288U) << 2) ^ b;

	return (z1 ^ z2);
}

// chhar promenljiva je tip karaktera koji treba postaviti na mapu
static void chhar_spawn(characters * chhar) {
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_l ),
			(unsigned int )0x8F000000 | (unsigned int )chhar->type);
	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + chhar->reg_h ),
			(chhar->y << 16) | chhar->x);
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
			case 0:
				Xil_Out32(addr, IMG_16x16_bckgnd);
				break;
			case 1:
				Xil_Out32(addr, IMG_16x16_bomberman);
				break;
			case 2:
				Xil_Out32(addr, IMG_16x16_block);
				break;
			case 3:
				Xil_Out32(addr, IMG_16x16_brick);
				break;
			case 4:
				Xil_Out32(addr, IMG_16x16_door);
				break;
			case 5:
				Xil_Out32(addr, IMG_16x16_enemie);
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

int obstackles_detection(int x, int y, int deoMape, unsigned char * map,int dir) {
	unsigned char mario_position_right;
	unsigned char mario_position_left;
	unsigned char mario_position_up;
	unsigned char mario_position_down;

	float Xx = x;
	float Yy = y;

	int roundX = 0;
	int roundY = 0;

	roundX = floor(Xx / 16);
	roundY = floor(Yy / 16);

	mario_position_right = map1[roundY][roundX + 1];
	mario_position_left = map1[roundY][roundX - 1];
	mario_position_up = map1[roundY - 1][roundX];
	mario_position_down = map1[roundY + 1][roundX];
	if (dir == 0) {
		switch (mario_position_right) {
		case 0:
			return 0;
			break;
		case 1:
			return 1;
			break;
		case 2:
			return 2;
			break;
		case 3:
			return 3;
			break;
		case 4:
			return 0;
			break;
		case 5:
			return 5;
			break;
		case 6:
			return 6;
			break;

		}
	} else if (dir == 1) {
		switch (mario_position_left) {
		case 0:
			return 0;
			break;
		case 1:
			return 1;
			break;
		case 2:
			return 2;
			break;
		case 3:
			return 3;
			break;
		case 4:
			return 0;
			break;
		case 5:
			return 5;
			break;
		case 6:
			return 6;
			break;

		}
	} else if (dir == 2) {
		switch (mario_position_up) {
		case 0:
			return 0;
			break;
		case 1:
			return 1;
			break;
		case 2:
			return 2;
			break;
		case 3:
			return 3;
			break;
		case 4:
			return 0;
			break;
		case 5:
			return 5;
			break;
		case 6:
			return 6;
			break;
		}
	}
	else if (dir == 3) {
		switch (mario_position_down) {
		case 0:
			return 0;
			break;
		case 1:
			return 1;
			break;
		case 2:
			return 2;
			break;
		case 3:
			return 3;
			break;
		case 4:
			return 0;
			break;
		case 5:
			return 5;
			break;
		case 6:
			return 6;
			break;
		}
	}
}


static bool_t mario_move(unsigned char * map, characters * mario,direction_t dir, int start_jump) {
	unsigned int x;
	unsigned int y;
	int i;

	int obstackle = 0;

	x = mario->x;
	y = mario->y;

	if (dir == DIR_LEFT) {
		obstackle = obstackles_detection(x, y, mapPart, map, 1);
		if(obstackle == 0){
			x-=16;
		}
	} else if (dir == DIR_RIGHT) {
		obstackle = obstackles_detection(x, y, mapPart, map, 0);
		if(obstackle == 0){
			x+=16;
		}
	} else if (dir == DIR_UP) {
		obstackle = obstackles_detection(x, y, mapPart, map, 2);
		if(obstackle == 0){
			y-=16;
		}
	} else if (dir == DIR_DOWN){
		obstackle = obstackles_detection(x, y, mapPart, map, 3);
		if(obstackle == 0){
			y+=16;
		}
	}


	mario->x = x;
	mario->y = y;

	Xil_Out32(
			XPAR_BATTLE_CITY_PERIPH_0_BASEADDR + 4 * ( REGS_BASE_ADDRESS + mario->reg_h ),
			(mario->y << 16) | mario->x);

	for (i = 0; i < 1000000; i++) {
	}

	return b_false;
}

static void find_enemie(int x, int y){
	if(enemie1.x == x && enemie1.y == y){
		enemie1.type = 0;
	}else if(enemie2.x == x && enemie2.y == y){
		enemie2.type= 0;
	}else if(enemie3.x == x && enemie3.y == y){
		enemie3.type = 0;
	}else if(enemie4.x == x && enemie4.y == y){
		enemie4.type = 0;
	}
}

static int bombermanAndEnemie(characters *mario)
{
	unsigned int xX;
	unsigned int yY;

	xX=mario->x;
	yY=mario->y;

	if(enemie1.x==xX && enemie1.y==yY){
		return 1;
	}else if(enemie2.x==xX && enemie2.y==yY){
		return 1;
	}else if(enemie3.x==xX && enemie3.y==yY){
		return 1;
	}else if(enemie4.x==xX && enemie4.y==yY){
		return 1;
	}else {
		return 0;
	}

}


static int find_bomberman(characters *mario, int x, int y)
{
	unsigned int xX1=mario->x;
	unsigned int yY1=mario->y;

	unsigned int xX=xX1/16;
	unsigned int yY=yY1/16;

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

	if(obstacle == 3){										// cigla
		return 1;
	}else if(obstacle == 5){								// enemie
		find_enemie(x, y);
		return 3;
	}else if(obstacle == 2){								// blok
		return -1;
	}else{
		return 0;
	}
}


static void destroy(unsigned char * map, int x, int y, characters * mario){
	int obstackle = 0;
	int mapPart = 0;
	int fB=0;
	int bE=0;



	//Right destroy
	obstackle = obstackles_detection(x*16, y*16, mapPart, map, 0);
	fB = find_bomberman(mario, x,y);
	bE=bombermanAndEnemie(mario);
	bW=bomberman_win(mario);
	if(bW==1){
		map_update(mario);
	}


	if(destroy_direction(x+1, y, obstackle, mario, fB, bE) == 1){
		map1[y][x+1] = 0;
	}else if (destroy_direction(x+1, y, obstackle, mario, fB, bE) == 3){
		map1[y][x+1]=0;
		if(++enemieCnt==4){
			map1[12][18]=4;
		}
	}else if(destroy_direction(x+1,y,obstackle, mario, fB, bE )==2 || bE==1 ){
		lifeDestroyR++;
		if(lifeDestroyR==3){
		map1[2][35]=0;
		}else if(lifeDestroyR==4){
			map1[2][36]=0;
		}else if(lifeDestroyR==5){
			map_update(mario);
		}

	}
	else if(destroy_direction(x+1, y, obstackle, mario, fB, bE) == 0) {
		obstackle = obstackles_detection(x*16+16, y*16, mapPart, map, 0);
		fB = find_bomberman(mario, x,y);
		bE=bombermanAndEnemie(mario);
		if(destroy_direction(x+2, y, obstackle, mario, fB, bE) == 1){
				map1[y][x+2] = 0;
		}else if (destroy_direction(x+2, y, obstackle,mario, fB, bE) == 3){
				map1[y][x-1]=0;
				if(++enemieCnt==4){
					map1[12][18]=4;
				}
		}else if(destroy_direction(x+2,y,obstackle, mario, fB, bE )==2 || bE==1){
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
	obstackle = obstackles_detection(x*16, y*16, mapPart, map, 1);
	fB = find_bomberman(mario, x,y);
	bE=bombermanAndEnemie(mario);
	if(destroy_direction(x-1, y, obstackle,mario, fB, bE) == 1){
		map1[y][x-1] = 0;
	}
	else if (destroy_direction(x-1, y, obstackle,mario, fB, bE) == 3){
			map1[y][x-1]=0;
			if(++enemieCnt==4){
				map1[12][18]=4;
			}
	}else if(destroy_direction(x-1,y,obstackle, mario, fB, bE )==2 || bE==1){
		lifeDestroyR++;
		if(lifeDestroyR==3){
		map1[2][35]=0;
		}else if(lifeDestroyR==4){
			map1[2][36]=0;
		}else if(lifeDestroyR==5){
			map_update(mario);
		}
	}
	else if(destroy_direction(x-1, y, obstackle,mario, fB, bE) == 0) {
		map1[mario->y+1][mario->x]=4;
		obstackle = obstackles_detection(x*16-16, y*16, mapPart, map, 1);
		fB = find_bomberman(mario, x,y);
		bE=bombermanAndEnemie(mario);
		if(destroy_direction(x-2, y, obstackle,mario, fB, bE) == 1){
				map1[y][x-2] = 0;
		}else if (destroy_direction(x-2, y, obstackle,mario, fB, bE) == 3){
				map1[y][x-1]=0;
				if(++enemieCnt==4){
					map1[12][18]=4;
				}
		}else if(destroy_direction(x-2,y,obstackle, mario, fB, bE )==2  || bE==1){
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
	obstackle = obstackles_detection(x*16, y*16, mapPart, map, 2);
	fB = find_bomberman(mario, x,y);
	bE=bombermanAndEnemie(mario);
	if(destroy_direction(x, y-1, obstackle,mario, fB, bE) == 1){
		map1[y-1][x] = 0;
	}else if (destroy_direction(x, y-1, obstackle,mario, fB, bE) == 3){
		map1[y-1][x]=0;
		if(++enemieCnt==4){
			map1[12][18]=4;
		}
	}else if(destroy_direction(x,y-1,obstackle, mario, fB, bE )==2 || bE==1){
		lifeDestroyR++;
		if(lifeDestroyR==3){
		map1[2][35]=0;
		}else if(lifeDestroyR==4){
			map1[2][36]=0;
		}else if(lifeDestroyR==5){
			map_update(mario);
		}
	}
	else if(destroy_direction(x, y-1, obstackle,mario, fB, bE) == 0) {
		obstackle = obstackles_detection(x*16, y*16-16, mapPart, map, 2);
		fB = find_bomberman(mario, x,y);
		bE=bombermanAndEnemie(mario);
		if(destroy_direction(x, y-2, obstackle,mario, fB, bE) == 1){
				map1[y-2][x] = 0;
		}else if (destroy_direction(x, y-2, obstackle,mario, fB, bE) == 3){
				map1[y][x-1]=0;
				if(++enemieCnt==4){
					map1[12][18]=4;
				}
		}else if(destroy_direction(x,y-2,obstackle, mario, fB, bE )==2 || bE==1){
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
	obstackle = obstackles_detection(x*16, y*16, mapPart, map, 3);
	fB = find_bomberman(mario, x,y);
	bE=bombermanAndEnemie(mario);
	if(destroy_direction(x, y+1, obstackle,mario, fB, bE) == 1){
		map1[y+1][x] = 0;
	}else if (destroy_direction(x, y+1, obstackle,mario, fB, bE) == 3){
		map1[y+1][x]=0;
		if(++enemieCnt==4){
			map1[12][18]=4;
		}
	}else if(destroy_direction(x,y+1,obstackle, mario, fB, bE )==2  || bE==1){
		lifeDestroyR++;
		if(lifeDestroyR==3){
		map1[2][35]=0;
		}else if(lifeDestroyR==4){
			map1[2][36]=0;
		}else if(lifeDestroyR==5){
			map_update(mario);
		}
	}
	else if(destroy_direction(x, y+1, obstackle,mario, fB, bE) == 0) {
		obstackle = obstackles_detection(x*16, y*16+16, mapPart, map, 3);
		fB = find_bomberman(mario, x,y);
		bE=bombermanAndEnemie(mario);
		if(destroy_direction(x, y+2, obstackle,mario, fB, bE) == 1){
			map1[y+2][x] = 0;
		}else if (destroy_direction(x, y+2, obstackle,mario, fB, bE) == 3){
				map1[y][x-1]=0;
				if(++enemieCnt==4){
					map1[12][18]=4;
			}
		}else if(destroy_direction(x,y+2,obstackle, mario, fB, bE )==2 || bE==1){
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





static void random_move_enemy(unsigned int *x, unsigned int *y, unsigned int type)
{
	int i = 0;
	int t = rand() % 4;
	int obstacle = obstackles_detection(*x*16, *y*16, 0, map1, t);

	if(obstacle == 0){
		if(t == 0){
			map1[*y][*x] = 0;
			*x = *x + 1;
			map1[*y][*x] = type;
		}else if(t == 1){
			map1[*y][*x] = 0;
			*x = *x - 1;
			map1[*y][*x] = type;
		}else if(t == 2){
			map1[*y][*x] = 0;
			*y = *y - 1;
			map1[*y][*x] = type;
		}else{
			map1[*y][*x] = 0;
			*y =*y + 1;
			map1[*y][*x] = type;
		}
	}

	for (i = 0; i < 100000; i++) {
			}
}

void battle_city() {
	unsigned int buttons;
	int i;
	int mapChange;

	map_reset(map1);
	map_update(&bomberman);
	chhar_spawn(&bomberman);

	int bomb_act = 0;
	float xX;
	float yY;
	int roundX;
	int roundY;
	int cnt = 15;

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
		}else if(BTN_SHOOT(buttons) && bomb_act == 0){
			xX = bomberman.x;
			yY = bomberman.y;
			roundX = floor(xX/16);
			roundY = floor(yY/16);
			map1[roundY][roundX] = 6;
			bomb_act = 1;
		}

		if(--cnt == 0){
			map1[roundY][roundX] = 0;
			cnt = 25;
			destroy(map1, roundX, roundY, &bomberman);
			bomb_act = 0;
			roundX=0;
			roundY=0;
		}

		random_move_enemy(&enemie1.x, &enemie1.y, enemie1.type);
		random_move_enemy(&enemie2.x, &enemie2.y, enemie2.type);
		random_move_enemy(&enemie3.x, &enemie3.y, enemie3.type);
		random_move_enemy(&enemie4.x, &enemie4.y, enemie4.type);

		int start_jump = 0;
		mario_move(map1, &bomberman, d, start_jump);


		map_update(&bomberman);

		for (i = 0; i < 100000; i++) {
		}

	}
}

