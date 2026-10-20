#ifndef CALC_H_
#define CALC_H_

#include "main.h"

void    calc_Init( void );                 /* сброс состояния */
void    calc_Clear( void );                /* то же, что Init */
void    calc_OnKey( uint8_t keyIndex );    /* обработка нажатия (индекс из KB_GetEvent) */
void    calc_Draw( void );                 /* перерисовка экрана */
uint8_t calc_IsEquals( uint8_t keyIndex ); /* это кнопка "="? (для долгого нажатия) */

#endif /* CALC_H_ */
