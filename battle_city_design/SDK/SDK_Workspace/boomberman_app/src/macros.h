/*
 * macros.h
 *
 *  Created on: 31.05.2019.
 *      Author: student
 */

#ifndef MACROS_H_
#define MACROS_H_

#define MAP_X							0
#define MAP_X2							640
#define MAP_Y							4
#define MAP_W							64
#define MAP_H							56

#define TANK1_REG_L                     8
#define TANK1_REG_H                     9
#define BASE_REG_L						0
#define BASE_REG_H	                    1
#define BOMB_TICK_COUNT					20
#define STARTING_LIFE_COUNT				3

#define PLUS_BOMB_CHANCE_MOD			10
#define PLUS_EXPLOSION_CHANCE_MOD		6
#define LIVES_STARTING_X				36
#define LIVES_STARTING_Y				2
#define MAP_COUNT 						1
#define BOMBERMAN_STARTING_POSITION_1_X	8
#define BOMBERMAN_STARTING_POSITION_1_Y 3
#define ENEMY_DEFAULT_SPEED 			1
#define ENEMY_DEFAULT_COUNT 			1
#define ENEMY_MAX_NUMBER 				9
#define ENEMY_MAX_SPEED 				9
#define BOMB_MAX_NUMBER					3
#define BOMB_MAX_POWER					6
#define STARTING_BOMB_NUMBER			1
#define STARTING_BOMB_POWER				1

#define MAP_WIDTH						40  // 40
#define MAP_HEIGHT         				30 // 30

#define TANK1_REG_L             		8
#define TANK1_REG_H             		9

#define TANK1_REG_L             		8
#define TANK1_REG_H             		9



// ***** 16x16 IMAGES *****

#define IMG_16x16_1end_down			0x00FF
#define IMG_16x16_1end_left			0x013F
#define IMG_16x16_1end_right			0x017F
#define IMG_16x16_1end_up			0x01BF
#define IMG_16x16_1intersection			0x01FF
#define IMG_16x16_1middle_horizontal			0x023F
#define IMG_16x16_1middle_vertical			0x027F
#define IMG_16x16_2end_down			0x02BF
#define IMG_16x16_2end_left			0x02FF
#define IMG_16x16_2end_right			0x033F
#define IMG_16x16_2end_up			0x037F
#define IMG_16x16_2intersection			0x03BF
#define IMG_16x16_2middle_horizontal			0x03FF
#define IMG_16x16_2middle_vertical			0x043F
#define IMG_16x16_3end_down			0x047F
#define IMG_16x16_3end_left			0x04BF
#define IMG_16x16_3end_right			0x04FF
#define IMG_16x16_3end_up			0x053F
#define IMG_16x16_3intersection			0x057F
#define IMG_16x16_3middle_horizontal			0x05BF
#define IMG_16x16_3middle_vertical			0x05FF
#define IMG_16x16_4end_down			0x063F
#define IMG_16x16_4end_left			0x067F
#define IMG_16x16_4end_right			0x06BF
#define IMG_16x16_4end_up			0x06FF
#define IMG_16x16_4intersection			0x073F
#define IMG_16x16_4middle_horizontal			0x077F
#define IMG_16x16_4middle_vertical			0x07BF
#define IMG_16x16_5end_down			0x07FF
#define IMG_16x16_5end_left			0x083F
#define IMG_16x16_5end_right			0x087F
#define IMG_16x16_5end_up			0x08BF
#define IMG_16x16_5intersection			0x08FF
#define IMG_16x16_5middle_horizontal			0x093F
#define IMG_16x16_5middle_vertical			0x097F
#define IMG_16x16_6end_down			0x09BF
#define IMG_16x16_6end_left			0x09FF
#define IMG_16x16_6end_right			0x0A3F
#define IMG_16x16_6end_up			0x0A7F
#define IMG_16x16_6intersection			0x0ABF
#define IMG_16x16_6middle_horizontal			0x0AFF
#define IMG_16x16_6middle_vertical			0x0B3F
#define IMG_16x16_7end_down			0x0B7F
#define IMG_16x16_7end_left			0x0BBF
#define IMG_16x16_7end_right			0x0BFF
#define IMG_16x16_7end_up			0x0C3F
#define IMG_16x16_7middle_horizontal			0x0C7F
#define IMG_16x16_7middle_vertical			0x0CBF
#define IMG_16x16_background			0x0CFF
#define IMG_16x16_block			0x0D3F
#define IMG_16x16_bomb			0x0D7F
#define IMG_16x16_bomberman			0x0DBF
#define IMG_16x16_bomberman_bomb			0x0DFF
#define IMG_16x16_brick			0x0E3F
#define IMG_16x16_door			0x0E7F
#define IMG_16x16_enemy			0x0EBF
#define IMG_16x16_plus_bomb			0x0EFF
#define IMG_16x16_plus_explosion			0x0F3F


// ***** MAP *****

#define MAP_BASE_ADDRESS			0x0F7F
#define REGS_BASE_ADDRESS       ( MAP_BASE_ADDRESS + MAP_WIDTH * MAP_HEIGHT )

#define SCREEN_WIDTH					640
#define SCREEN_HEIGHT					480

#define BTN_DOWN( b )           ( !( b & 0x01 ) )
#define BTN_UP( b )             ( !( b & 0x10 ) )
#define BTN_LEFT( b )           ( !( b & 0x02 ) )
#define BTN_RIGHT( b )          ( !( b & 0x08 ) )
#define BTN_SHOOT( b )          ( !( b & 0x04 ) )

#define COUNT_OPTION_X 24
#define COUNT_OPTION_Y 18
#define SPEED_OPTION_X 24
#define SPEED_OPTION_Y 24
#define MENU_INDICATOR_COUNT_X COUNT_OPTION_X + 4
#define MENU_INDICATOR_COUNT_Y COUNT_OPTION_Y + 2
#define MENU_INDICATOR_SPEED_X MENUINDICATOR_COUNT_X
#define MENU_INDICATOR_SPEED_Y MENU_INDICATOR_COUNT_Y + 6

#endif /* MACROS_H_ */
