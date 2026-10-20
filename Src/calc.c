#include "calc.h"
#include "oled.h"
#include "fonts.h"
#include <string.h>

#define MAX_DIGITS  8                       /* максимум цифр в операнде */
#define MAX_ABS     99999999999999999LL     /* 17 цифр: вместе со знаком = 18 символов Font_7x10 */
#define CHARS_LINE  18                      /* 128 / 7 */
#define LINE_H      11                      /* высота строки (шрифт 10 px + 1 px зазор) */

/* Раскладка: индекс = строка*3 + столбец (строка 0 = ROW1).
 * 'O' - кнопка выбора операции, '=' - равно.
 * Если физическая раскладка стенда другая - меняйте только эту таблицу. */
static const char keymap[12] = {
	'7', '8', '9',
	'4', '5', '6',
	'1', '2', '3',
	'O', '0', '='
};

#define OPS_COUNT 4
static const char ops[OPS_COUNT] = { '+', '-', '*', '/' };

typedef enum { ST_A, ST_B, ST_RESULT, ST_ERROR } state_t;

static state_t state;
static int64_t a, b, res;
static uint8_t opIdx;
static uint8_t nd;              /* сколько цифр введено в текущий операнд (0 = ещё ничего не вводили) */
static uint8_t showHelp = 1;    /* 1 = показывать экран с раскладкой и подсказками */
static char   *errText = "Error";

static void i64_to_str( int64_t v, char *buf ) {
	char tmp[24];
	int n = 0, i = 0;
	uint64_t u = (v < 0) ? (uint64_t)(-(v + 1)) + 1u : (uint64_t)v;

	do { tmp[n++] = (char)('0' + (u % 10)); u /= 10; } while( u );
	if( v < 0 ) buf[i++] = '-';
	while( n ) buf[i++] = tmp[--n];
	buf[i] = 0;
}

static void put_right( uint8_t y, char *s ) {
	int len = (int)strlen(s);
	if( len > CHARS_LINE ) len = CHARS_LINE;
	oled_SetCursor((uint8_t)((CHARS_LINE - len) * 7), y);
	oled_WriteString(s, Font_7x10, White);
}

void calc_Init( void ) {
	state = ST_A;
	a = b = res = 0;
	opIdx = 0;
	nd = 0;
	errText = "Error";
}

void calc_Clear( void ) {
	calc_Init();
	showHelp = 1;               /* после сброса снова показываем раскладку */
}

uint8_t calc_IsEquals( uint8_t keyIndex ) {
	return (keyIndex < 12 && keymap[keyIndex] == '=');
}

static void do_digit( int d ) {
	if( state == ST_RESULT || state == ST_ERROR ) {
		calc_Init();                         /* цифра после результата = новый ввод */
	}
	int64_t *cur = (state == ST_A) ? &a : &b;

	if( *cur == 0 && nd > 0 ) {              /* на экране уже "0": следующая цифра его заменяет */
		*cur = d;                            /* (0 после 0 остаётся нулём, ведущие нули не копим) */
		return;
	}
	if( nd >= MAX_DIGITS ) return;

	*cur = *cur * 10 + d;                    /* первая цифра (в т.ч. 0) */
	nd++;                                    /* nd > 0 => операнд "введён" и будет отображён */
}

static void do_op( void ) {
	switch( state ) {
	case ST_A:
		state = ST_B; opIdx = 0; b = 0; nd = 0;
		break;
	case ST_B:
		opIdx = (uint8_t)((opIdx + 1) % OPS_COUNT);  /* циклическая смена + - * / */
		break;
	case ST_RESULT:                          /* продолжаем вычисления с результата */
		a = res; state = ST_B; opIdx = 0; b = 0; nd = 0;
		break;
	default:
		break;
	}
}

static void do_equals( void ) {
	if( state != ST_B ) return;

	int64_t r = 0;
	int ovf = 0;
	switch( ops[opIdx] ) {
	case '+': r = a + b; break;
	case '-': r = a - b; break;
	case '*': ovf = __builtin_mul_overflow(a, b, &r); break;
	default:                                 /* '/' - целочисленное деление (с отбрасыванием остатка) */
		if( b == 0 ) {
			errText = "Div by 0";
			state = ST_ERROR;
			return;
		}
		r = a / b;
		break;
	}

	if( ovf || r > MAX_ABS || r < -MAX_ABS ) {
		errText = "Error";
		state = ST_ERROR;
	} else {
		res = r;
		state = ST_RESULT;
	}
}

void calc_OnKey( uint8_t keyIndex ) {
	if( keyIndex >= 12 ) return;
	char k = keymap[keyIndex];

	showHelp = 0;                            /* любая кнопка закрывает экран-подсказку */

	if( k >= '0' && k <= '9' ) do_digit(k - '0');
	else if( k == 'O' )        do_op();
	else if( k == '=' )        do_equals();
}

/* Стартовый экран: слева раскладка клавиатуры (берётся из keymap), справа - гайд */
static void draw_help( void ) {
	char s[2] = { 0, 0 };

	oled_Fill(Black);

	for( uint8_t r = 0; r < 4; r++ ) {
		for( uint8_t c = 0; c < 3; c++ ) {
			s[0] = keymap[r * 3 + c];
			oled_SetCursor((uint8_t)(c * 14), (uint8_t)(r * LINE_H));
			oled_WriteString(s, Font_7x10, White);
		}
	}

	oled_SetCursor(56, 0);          oled_WriteString("O: + - * /", Font_7x10, White);
	oled_SetCursor(56, LINE_H);     oled_WriteString("=: result",  Font_7x10, White);
	oled_SetCursor(56, 2*LINE_H);   oled_WriteString("Hold '=':",  Font_7x10, White);
	oled_SetCursor(56, 3*LINE_H);   oled_WriteString("clear all",  Font_7x10, White);

	oled_DrawHLine(0, 127, 46, White);
	oled_SetCursor(0, 52);
	oled_WriteString("Press any key", Font_7x10, White);

	oled_UpdateScreen();
}

void calc_Draw( void ) {
	char num[24], line[24];

	if( showHelp ) {
		draw_help();
		return;
	}

	oled_Fill(Black);

	/* строка 1: первый операнд */
	i64_to_str(a, num);
	put_right(0, num);

	/* строка 2: операция и второй операнд */
	if( state != ST_A ) {
		line[0] = ops[opIdx];
		line[1] = 0;
		if( state != ST_B || nd > 0 ) {
			i64_to_str(b, num);
			strcat(line, num);
		}
		put_right(LINE_H, line);
	}

	/* результат */
	if( state == ST_RESULT || state == ST_ERROR ) {
		oled_DrawHLine(0, 127, 23, White);
		if( state == ST_RESULT ) {
			i64_to_str(res, num);
			put_right(26, num);
		} else {
			put_right(26, errText);
		}
	}

	/* подсказки */
	oled_SetCursor(0, 41);
	oled_WriteString("OP: change + - * /", Font_7x10, White);
	oled_SetCursor(0, 52);
	oled_WriteString("Hold '=': clear", Font_7x10, White);

	oled_UpdateScreen();
}
