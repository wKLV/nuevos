#if !defined(__cplusplus)
#include <stdbool.h> /* C doesn't have booleans by default. */
#endif
#include <stddef.h>
#include <stdint.h>
 
/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
/* Hardware text mode color constants. */
enum vga_color {
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};
 
uint8_t make_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}
 
uint16_t make_vgaentry(char c, uint8_t color) {
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}
 
size_t strlen(const char* str) {
	size_t ret = 0;
	while ( str[ret] != 0 )
		ret++;
	return ret;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
void terminal_initialize() {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = make_color(COLOR_GREEN, COLOR_WHITE);
	terminal_buffer = (uint16_t*) 0xB8000;

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = make_vgaentry(' ', terminal_color);
		}
	}

}
 
void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    if (c != '\n') {
        terminal_buffer[index] = make_vgaentry(c, color);
    }
    else {
        terminal_buffer[index] = make_vgaentry('n', make_color(COLOR_WHITE, COLOR_LIGHT_GREEN));
    }
}

void terminal_putchar(char c) {
    terminal_putentryat(c, terminal_color, terminal_column++, terminal_row);
    if (terminal_column+1 == VGA_WIDTH) {
        terminal_column = 0;
    }
    if (terminal_row+1 == VGA_HEIGHT) {
        terminal_row = 0;
    }
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    /* TODO: Is it wrong to use 'N' for the port? It's not a 8-bit constant. */
    /* TODO: Should %1 be %w1? */
    return ret;
}
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* TODO: Is it wrong to use 'N' for the port? It's not a 8-bit constant. */
    /* TODO: Should %1 be %w1? */
}

/* void update_cursor(int row, int col)
 * by Dark Fiber
 */
void update_cursor(int row, int col)
{
    unsigned short position=(row*80) + col;

    // cursor LOW port to vga INDEX register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position&0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char )((position>>8)&0xFF));
}
 
void terminal_writestring(const char* data) {
	size_t datalen = strlen(data); //Note: Don't write /0
	for (size_t i = 0; i < datalen; i++)
		terminal_putchar(data[i]);
}


/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */
unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '#', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   '\\',
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};	
unsigned char kbdusMayus[128] =
{
    0,  27, '!', '"', '$', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '\'', '~',   0,		/* Left shift */
 '~', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
    'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   '|',
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};	
typedef enum {
    SHIFT = 0x1,
    CONTROL = 0x2,
    ALT = 0x4,
    CAPS = 0x8,
    MAYUS = SHIFT | CAPS,

} KeyState;
static uint8_t state;

char get_key_scan() {
    int c = inb(0x60);
    if(c & 0x80) {
        //Release
        if(c == (42 | 0x80)) state &= !SHIFT;
        if(c == (58 | 0x80)) {
            if(state & CAPS) state &= !CAPS;
            else state |= CAPS;
        }
        return -1;
    }
    else {
        if(c == 42) state |= SHIFT;
        if(state & MAYUS) {
            //if(kbdusMayus[c] == 0) return c;
            return kbdusMayus[c];
        }
        else {
            //if(kbdus[c] == 0) return c;
            return kbdus[c];
        }
        
    }
}

//static char text[1<<10];
char editText[1<<20];

void insert_mode();
 
#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kernel_main() {
	/* Initialize terminal interface */
	terminal_initialize();
 
	/* Since there is no support for newlines in terminal_putchar
         * yet, '\n' will produce some VGA specific character instead.
         * This is normal.
         */
        /*
	terminal_writestring("Hello, kernel World!\n Hello Kernel World\nHola mundonnapa");
        terminal_column = 10; terminal_row = 10;
        terminal_writestring("I'm in 10,10\n\nline 12 I believe\n");
        */
        //terminal_writestring("I\nt'\ns\n\ng\nr\ne\na\nt\n!\n\nA\ni\nn'\nt\n\ni\nt\n");
        insert_mode(0,0,0);
}
void print_file();

void cursor_char(char c, int* posP, int* posxP, int* posyP) {
    if(c) {
        int pos = *posP, posx = *posxP, posy = *posyP;
        if(c=='\n') { 
            posx =0;
            posy++;
            editText[pos++] = '\n';
        }
        else if(c=='\t') {
            editText[pos++] = ' ';
            editText[pos++] = ' ';
            posx += 2;
        }
        else if(c=='\b') {
            c = editText[pos-1];
            editText[--pos] = 0;
            if(c != '\n') {
                posx --;
                terminal_putentryat(' ', make_color(COLOR_GREEN, COLOR_WHITE), posx, posy);
            }
            else {
                print_file(&pos, &posx, &posy);
            }
        }
        else{
            terminal_putentryat(c, make_color(COLOR_GREEN, COLOR_WHITE), posx, posy); 
            posx++;
            editText[pos++] = c; 
        }
        update_cursor(posy, posx);
        *posP = pos, *posxP = posx, *posyP = posy;
    }
}

void print_file(int* posP, int* posxP, int* posyP) {
    char c; unsigned int i;
    int pos = 0, posx = 0, posy = 0;
    for(i =0; i<700; ++i) {
        c = editText[i];
        cursor_char(c, &pos, &posx, &posy);
    }
    *posP = pos, *posxP = posx, *posyP = posy;
}

void normal_mode() {
    char c;
    int pos = 0;
    int posx =0, posy = 0;

    while((c=get_key_scan())) {
        switch(c) {
            case 'w': {
                for(; editText[pos] != ' '; pos++, posx++); 
                update_cursor(posy, posx);
            }; break;
            case 'b': {
                for(; editText[pos] != ' '; pos--, posx--); 
                update_cursor(posy, posx);
            }; break;
            case 'i': {
                insert_mode(pos, posx, posy);
            };
        };
    }
}

void insert_mode(int pos, int posx, int posy) {
    char c;
    char lastKeyPressed = 0;

    while((c=get_key_scan()) != 27) if(c) {
        if(c == -1) {
            lastKeyPressed = -1;
            continue;
        }
        else if(c == lastKeyPressed) continue; 
        lastKeyPressed = c;
        cursor_char(c, &pos, &posx, &posy);
        /*
        terminal_putentryat(editText[pos-2], make_color(COLOR_RED, COLOR_WHITE), 40, 0);
        terminal_putentryat(editText[pos-1], make_color(COLOR_RED, COLOR_WHITE), 41, 0);
        terminal_putentryat(editText[pos], make_color(COLOR_RED, COLOR_WHITE), 42, 0);
        */
    }
    normal_mode();
}

