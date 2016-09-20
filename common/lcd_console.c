/*
 * (C) Copyright 2001-2015
 * DENX Software Engineering -- wd@denx.de
 * Compulab Ltd - http://compulab.co.il/
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <video_font.h>		/* Get font data, width and height */
#if defined(CONFIG_LCD_LOGO)
#include <bmp_logo.h>
#endif

#ifdef CONFIG_LCD_CONSOLE_ANSI
static char ansi_buf[10];
static int ansi_buf_size;
static int ansi_colors_need_revert;
static int ansi_cursor_hidden;
#endif


static struct console_t cons;

void lcd_set_col(short col)
{
	cons.curr_col = col;
}

void lcd_set_row(short row)
{
	cons.curr_row = row;
}

void lcd_position_cursor(unsigned col, unsigned row)
{
	cons.curr_col = min_t(short, col, cons.cols - 1);
	cons.curr_row = min_t(short, row, cons.rows - 1);
}

int lcd_get_screen_rows(void)
{
	return cons.rows;
}

int lcd_get_screen_columns(void)
{
	return cons.cols;
}

static void lcd_putc_xy0(struct console_t *pcons, ushort x, ushort y, char c)
{
	int fg_color = lcd_getfgcolor();
	int bg_color = lcd_getbgcolor();
	int i, row;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  y * pcons->lcdsizex +
				  x;

	for (row = 0; row < VIDEO_FONT_HEIGHT; row++) {
		uchar bits = video_fontdata[c * VIDEO_FONT_HEIGHT + row];
		for (i = 0; i < VIDEO_FONT_WIDTH; ++i) {
			*dst++ = (bits & 0x80) ? fg_color : bg_color;
			bits <<= 1;
		}
		dst += (pcons->lcdsizex - VIDEO_FONT_WIDTH);
	}
}

static inline void console_setcol0(struct console_t *pcons, u32 row, int begin, int end, int clr)
{
	int i;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  row * VIDEO_FONT_HEIGHT *
				  begin;

	for (i = 0; i < (VIDEO_FONT_HEIGHT * end); i++)
		*dst++ = clr;
}

static inline void console_setrow0(struct console_t *pcons, u32 row, int clr)
{
	int i;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  row * VIDEO_FONT_HEIGHT *
				  pcons->lcdsizex;

	for (i = 0; i < (VIDEO_FONT_HEIGHT * pcons->lcdsizex); i++)
		*dst++ = clr;
}

static inline void console_moverow0(struct console_t *pcons,
				    u32 rowdst, u32 rowsrc)
{
	int i;
	fbptr_t *dst = (fbptr_t *)pcons->fbbase +
				  rowdst * VIDEO_FONT_HEIGHT *
				  pcons->lcdsizex;

	fbptr_t *src = (fbptr_t *)pcons->fbbase +
				  rowsrc * VIDEO_FONT_HEIGHT *
				  pcons->lcdsizex;

	for (i = 0; i < (VIDEO_FONT_HEIGHT * pcons->lcdsizex); i++)
		*dst++ = *src++;
}

static inline void console_back(void)
{
	if (--cons.curr_col < 0) {
		cons.curr_col = cons.cols - 1;
		if (--cons.curr_row < 0)
			cons.curr_row = 0;
	}

	cons.fp_putc_xy(&cons,
			cons.curr_col * VIDEO_FONT_WIDTH,
			cons.curr_row * VIDEO_FONT_HEIGHT, ' ');
}

#ifdef CONFIG_LCD_CONSOLE_ANSI

static void console_clear_line(int line, int begin, int end)
{
	int bg_color = lcd_getbgcolor();
	console_setcol0(&cons, line, begin, end, bg_color);
}

static void console_cursor_fix(void)
{
	if (cons.curr_row < 0)
		cons.curr_row = 0;
	if (cons.curr_row >= cons.rows)
		cons.curr_row = cons.rows - 1;
	if (cons.curr_col < 0)
		cons.curr_col = 0;
	if (cons.curr_col >= cons.cols)
		cons.curr_col = cons.cols - 1;
}

static void console_cursor_up(int n)
{
	cons.curr_row -= n;
	console_cursor_fix();
}

static void console_cursor_down(int n)
{
	cons.curr_row += n;
	console_cursor_fix();
}

static void console_cursor_left(int n)
{
	cons.curr_col -= n;
	console_cursor_fix();
}

static void console_cursor_right(int n)
{
	cons.curr_col += n;
	console_cursor_fix();
}

static void console_cursor_set_position(int row, int col)
{
	if (cons.rows != -1)
		cons.curr_row = row;
	if (cons.cols != -1)
		cons.curr_col = col;
	console_cursor_fix();
}

static void console_previousline(int n)
{
	/* FIXME: also scroll terminal ? */
	cons.curr_row -= n;
	console_cursor_fix();
}

static void console_swap_colors(void)
{
	int bgcolor, fgcolor;

    bgcolor = lcd_getbgcolor();
    fgcolor = lcd_getfgcolor();

	lcd_setbgcolor(fgcolor);
    lcd_setfgcolor(bgcolor);
} 

static inline int console_cursor_is_visible(void)
{
	return !ansi_cursor_hidden;
}
#else
static inline int console_cursor_is_visible(void)
{
	return 1;
}
#endif

static inline void console_newline(void)
{
	const int rows = CONFIG_CONSOLE_SCROLL_LINES;
	int bg_color = lcd_getbgcolor();
	int i;

	cons.curr_col = 0;

	/* Check if we need to scroll the terminal */
	if (++cons.curr_row >= cons.rows) {
		for (i = 0; i < cons.rows-rows; i++)
			cons.fp_console_moverow(&cons, i, i+rows);
		for (i = 0; i < rows; i++)
			cons.fp_console_setrow(&cons, cons.rows-i-1, bg_color);
		cons.curr_row -= rows;
	}
	lcd_sync();
}

void console_calc_rowcol(struct console_t *pcons, u32 sizex, u32 sizey)
{
	pcons->cols = sizex / VIDEO_FONT_WIDTH;
#if defined(CONFIG_LCD_LOGO) && !defined(CONFIG_LCD_INFO_BELOW_LOGO)
	pcons->rows = (pcons->lcdsizey - BMP_LOGO_HEIGHT);
	pcons->rows /= VIDEO_FONT_HEIGHT;
#else
	pcons->rows = sizey / VIDEO_FONT_HEIGHT;
#endif
}

void __weak lcd_init_console_rot(struct console_t *pcons)
{
	return;
}

void lcd_init_console(void *address, int vl_cols, int vl_rows, int vl_rot)
{
	memset(&cons, 0, sizeof(cons));
	cons.fbbase = address;

	cons.lcdsizex = vl_cols;
	cons.lcdsizey = vl_rows;
	cons.lcdrot = vl_rot;

	cons.fp_putc_xy = &lcd_putc_xy0;
	cons.fp_console_moverow = &console_moverow0;
	cons.fp_console_setrow = &console_setrow0;
    /*cons.fp_console_setcol = &console_setcol0;*/
	console_calc_rowcol(&cons, cons.lcdsizex, cons.lcdsizey);

	lcd_init_console_rot(&cons);

	debug("lcd_console: have %d/%d col/rws on scr %dx%d (%d deg rotated)\n",
	      cons.cols, cons.rows, cons.lcdsizex, cons.lcdsizey, vl_rot);
}

void lcd_putc(const char c)
{
#ifdef CONFIG_LCD_CONSOLE_ANSI
	int i;

	if (c == 27) {
		for (i = 0; i < ansi_buf_size; ++i)
			parse_putc(ansi_buf[i]);
		ansi_buf[0] = 27;
		ansi_buf_size = 1;
		return;
	}

	if (ansi_buf_size > 0) {
		/*
		 * 0 - ESC
		 * 1 - [
		 * 2 - num1
		 * 3 - ..
		 * 4 - ;
		 * 5 - num2
		 * 6 - ..
		 * - cchar
		 */
		int next = 0;

		int flush = 0;
		int fail = 0;

		int num1 = 0;
		int num2 = 0;
		int cchar = 0;

		ansi_buf[ansi_buf_size++] = c;

		if (ansi_buf_size >= sizeof(ansi_buf))
			fail = 1;

		for (i = 0; i < ansi_buf_size; ++i) {
			if (fail)
				break;

			switch (next) {
			case 0:
				if (ansi_buf[i] == 27)
					next = 1;
				else
					fail = 1;
				break;

			case 1:
				if (ansi_buf[i] == '[')
					next = 2;
				else
					fail = 1;
				break;

			case 2:
				if (ansi_buf[i] >= '0' && ansi_buf[i] <= '9') {
					num1 = ansi_buf[i]-'0';
					next = 3;
				} else if (ansi_buf[i] != '?') {
					--i;
					num1 = 1;
					next = 4;
				}
				break;

			case 3:
				if (ansi_buf[i] >= '0' && ansi_buf[i] <= '9') {
					num1 *= 10;
					num1 += ansi_buf[i]-'0';
				} else {
					--i;
					next = 4;
				}
				break;

			case 4:
				if (ansi_buf[i] != ';') {
					--i;
					next = 7;
				} else
					next = 5;
				break;

			case 5:
				if (ansi_buf[i] >= '0' && ansi_buf[i] <= '9') {
					num2 = ansi_buf[i]-'0';
					next = 6;
				} else
					fail = 1;
				break;

			case 6:
				if (ansi_buf[i] >= '0' && ansi_buf[i] <= '9') {
					num2 *= 10;
					num2 += ansi_buf[i]-'0';
				} else {
					--i;
					next = 7;
				}
				break;

			case 7:
				if ((ansi_buf[i] >= 'A' && ansi_buf[i] <= 'H')
					|| ansi_buf[i] == 'J'
					|| ansi_buf[i] == 'K'
					|| ansi_buf[i] == 'h'
					|| ansi_buf[i] == 'l'
					|| ansi_buf[i] == 'm') {
					cchar = ansi_buf[i];
					flush = 1;
				} else
					fail = 1;
				break;
			}
		}

		if (fail) {
			for (i = 0; i < ansi_buf_size; ++i)
				parse_putc(ansi_buf[i]);
			ansi_buf_size = 0;
			return;
		}

		if (flush) {
			ansi_buf_size = 0;
			switch (cchar) {
			case 'A':
				/* move cursor num1 rows up */
				console_cursor_up(num1);
				break;
			case 'B':
				/* move cursor num1 rows down */
				console_cursor_down(num1);
				break;
			case 'C':
				/* move cursor num1 columns forward */
				console_cursor_right(num1);
				break;
			case 'D':
				/* move cursor num1 columns back */
				console_cursor_left(num1);
				break;
			case 'E':
				/* move cursor num1 rows up at begin of row */
				console_previousline(num1);
				break;
			case 'F':
				/* move cursor num1 rows down at begin of row */
				console_newline();
				break;
			case 'G':
				/* move cursor to column num1 */
				console_cursor_set_position(-1, num1-1);
				break;
			case 'H':
				/* move cursor to row num1, column num2 */
				console_cursor_set_position(num1-1, num2-1);
				break;
			case 'J':
				/* clear console and move cursor to 0, 0 */
				lcd_clear();
				console_cursor_set_position(0, 0);
				break;
			case 'K':
				/* clear line */
				if (num1 == 0)
					console_clear_line(cons.curr_row,
							cons.curr_col,
							cons.cols-1);
				else if (num1 == 1)
					console_clear_line(cons.curr_row,
							0, cons.curr_col);
				else
					console_clear_line(cons.curr_row,
							0, cons.cols-1);
				break;
			case 'h':
				ansi_cursor_hidden = 0;
				break;
			case 'l':
				ansi_cursor_hidden = 1;
				break;
			case 'm':
				if (num1 == 0) { /* reset swapped colors */
					if (ansi_colors_need_revert) {
						console_swap_colors();
						ansi_colors_need_revert = 0;
					}
				} else if (num1 == 7) { /* once swap colors */
					if (!ansi_colors_need_revert) {
						console_swap_colors();
						ansi_colors_need_revert = 1;
					}
				}
				break;
			}
		}
	} else {
		parse_putc(c);
	}
#else
	parse_putc(c);
#endif
}

void parse_putc(const char c)
{
	if (!lcd_is_enabled) {
		serial_putc(c);

		return;
	}

	switch (c) {
	case '\r':
		cons.curr_col = 0;
		break;
	case '\n':
		console_newline();

		break;
	case '\t':	/* Tab (8 chars alignment) */
		cons.curr_col +=  8;
		cons.curr_col &= ~7;

		if (cons.curr_col >= cons.cols)
			console_newline();

		break;
	case '\b':
		console_back();

		break;
	default:
		cons.fp_putc_xy(&cons,
				cons.curr_col * VIDEO_FONT_WIDTH,
				cons.curr_row * VIDEO_FONT_HEIGHT, c);
		if (++cons.curr_col >= cons.cols)
			console_newline();
	}

	if (console_cursor_is_visible())
		lcd_position_cursor(cons.curr_col, cons.curr_row);
}

void lcd_puts(const char *s)
{
	if (!lcd_is_enabled) {
		serial_puts(s);

		return;
	}

	while (*s)
		lcd_putc(*s++);

	lcd_sync();
}

void lcd_printf(const char *fmt, ...)
{
	va_list args;
	char buf[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	lcd_puts(buf);
}

static int do_lcd_setcursor(cmd_tbl_t *cmdtp, int flag, int argc,
			    char *const argv[])
{
	unsigned int col, row;

	if (argc != 3)
		return CMD_RET_USAGE;

	col = simple_strtoul(argv[1], NULL, 10);
	row = simple_strtoul(argv[2], NULL, 10);
	lcd_position_cursor(col, row);

	return 0;
}

static int do_lcd_puts(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	lcd_puts(argv[1]);

	return 0;
}

U_BOOT_CMD(
	setcurs, 3,	1,	do_lcd_setcursor,
	"set cursor position within screen",
	"    <col> <row> in character"
);

U_BOOT_CMD(
	lcdputs, 2,	1,	do_lcd_puts,
	"print string on lcd-framebuffer",
	"    <string>"
);

