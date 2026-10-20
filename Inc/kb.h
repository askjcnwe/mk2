#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_

#include "main.h"

#define ROW1 0xFE
#define ROW2 0xFD
#define ROW3 0xFB
#define ROW4 0xF7

/* Индекс кнопки = строка*3 + столбец (строка 0 = ROW1/P0, столбец 0 = левый) */
#define KEY_NONE      0xFF
#define KB_DEBOUNCE   2      /* сколько подряд одинаковых опросов нужно для подтверждения */

HAL_StatusTypeDef Set_Keyboard( void );   /* однократная настройка PCA9538 */
uint8_t KB_Scan( void );                  /* "сырой" опрос: индекс 0..11 или KEY_NONE */
uint8_t KB_GetEvent( void );              /* событие нажатия (один раз за нажатие) с антидребезгом */
uint8_t KB_GetHeld( void );               /* какая кнопка сейчас удерживается (после антидребезга) */

#endif /* INC_KEYBOARD_H_ */
