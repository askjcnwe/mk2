#include "main.h"
#include "pca9538.h"
#include "kb.h"
#include "sdk_uart.h"

#define KBRD_ADDR 0xE2
#define COL_MASK  0x70      /* столбцы на P4..P6 */

static const uint8_t rows[4] = { ROW1, ROW2, ROW3, ROW4 };

static uint8_t prev_raw = KEY_NONE;
static uint8_t stable   = KEY_NONE;
static uint8_t cnt      = 0;

/* Полярность не инвертируем, в выходной регистр пишем 0:
 * тогда линия, настроенная как выход (бит CONFIG = 0), выдаёт "0". */
HAL_StatusTypeDef Set_Keyboard( void ) {
	HAL_StatusTypeDef ret;
	uint8_t buf = 0;

	ret = PCA9538_Write_Register(KBRD_ADDR, POLARITY_INVERSION, &buf);
	if( ret != HAL_OK ) {
		UART_Transmit((uint8_t*)"Error write polarity\n");
		return ret;
	}

	buf = 0;
	ret = PCA9538_Write_Register(KBRD_ADDR, OUTPUT_PORT, &buf);
	if( ret != HAL_OK ) {
		UART_Transmit((uint8_t*)"Error write output\n");
	}
	return ret;
}

uint8_t KB_Scan( void ) {
	for( uint8_t r = 0; r < 4; r++ ) {
		uint8_t cfg = rows[r];
		uint8_t in  = 0xFF;

		if( PCA9538_Write_Register(KBRD_ADDR, CONFIG, &cfg) != HAL_OK ) return KEY_NONE;
		if( PCA9538_Read_Inputs(KBRD_ADDR, &in) != HAL_OK )             return KEY_NONE;

		uint8_t cols = (uint8_t)(~in) & COL_MASK;   /* нажатая кнопка = 0 на линии столбца */
		if( cols ) {
			if( cols & 0x10 ) return r * 3 + 0;     /* левая   */
			if( cols & 0x20 ) return r * 3 + 1;     /* средняя */
			return r * 3 + 2;                       /* правая  */
		}
	}
	return KEY_NONE;
}

uint8_t KB_GetEvent( void ) {
	uint8_t raw = KB_Scan();

	if( raw != prev_raw ) {         /* состояние изменилось - начинаем отсчёт заново */
		prev_raw = raw;
		cnt = 0;
		return KEY_NONE;
	}
	if( cnt < KB_DEBOUNCE ) {
		cnt++;
		return KEY_NONE;
	}
	if( raw != stable ) {           /* подтверждённое изменение */
		stable = raw;
		return raw;                 /* при отпускании вернётся KEY_NONE */
	}
	return KEY_NONE;
}

uint8_t KB_GetHeld( void ) {
	return stable;
}
