/**                                               _____           ,-.
 * _______       _____       _____                ___   _,.      /  /
 * ___    |__   ____(_)_____ __  /______________  __   ; \____,-==-._  )
 * __  /| |_ | / /_  /_  __ `/  __/  __ \_  ___/  _    //_    `----' {+>
 * _  ___ |_ |/ /_  / / /_/ // /_ / /_/ /  /      _    `  `'--/  /-'`(
 * /_/  |_|____/ /_/  \__,_/ \__/ \____//_/       _          /  /
 *                                                           `='
 *
 * printf.c
 *
 * v1.0
 *
 * vprintf function, support %f
 */
#include "printf.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
//#include "mathlib.h"

#define DEF_PRECISION   6

static const char digit[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                             'A', 'B', 'C', 'D', 'E', 'F'
                            };

static char* print_buf;
static uint16_t print_cnt=0;

int putcf(int c)
{
    print_buf[print_cnt++] = c;

    return print_cnt;
}

static uint8_t get_int_len(long long int value)
{
    uint8_t len = 1;

    while(value > 9) {
        len++;
        value /= 10;
    }

    return len;
}

static uint8_t itoa_dec_unsigned(unsigned long int num)
{
    uint8_t len = 0;

    if(num == 0) {
        putcf('0');
        return 1;
    }

    unsigned long int i = 1;

    while((num / i) > 9) {
        i *= 10;
    }

    do {
        putcf(digit[(num / i) % 10]);
        len++;
    }
    while(i /= 10);

    return len;
}

static uint8_t itoa_dec(long int num, uint8_t width, char pad)
{
    uint8_t len = 0;

    if(num == 0) {
        putcf('0');
        return 1;
    }

    unsigned long int n = num;
    if(num < 0) {
        n = -num;
        putcf('-');
        len++;
    }

    uint8_t num_len = get_int_len(num);
    if(num_len < width) {
        uint8_t fill_num = width - num_len;
        while(fill_num > 0) {
            putcf(pad);
            len++;
            fill_num--;
        }
    }

    return itoa_dec_unsigned(n)+len;
}

static uint8_t itoa_hex(unsigned long int num, uint8_t width, char pad)
{
    uint8_t len = 0;
    bool found_first = false;

    for(int8_t i = (32/4-1); i >= 0; i--) {
        uint8_t shift = i * 4;
        uint32_t mask = (uint32_t)0x0F << shift;
        uint32_t val = (num & mask) >> shift;

        if(val > 0) {
            found_first = true;
        }

        if(found_first || i < width) {
            if(found_first) {
                putcf(digit[val]);
            }
            else {
                putcf(pad);
            }

            len++;
        }
    }

    return len;
}

static uint8_t itoa_dec_ull(unsigned long long int num)
{
    uint8_t len = 0;

    if(num == 0) {
        putcf('0');
        return 1;
    }

    unsigned long int i = 1;

    while((num / i) > 9) {
        i *= 10;
    }

    do {
        putcf(digit[(num / i) % 10]);
        len++;
    }
    while(i /= 10);

    return len;
}

static uint8_t itoa_dec_ll(long long int num, uint8_t width, char pad)
{
    uint8_t len = 0;

    if(num == 0) {
        putcf('0');
        return 1;
    }

    unsigned long long int n = num;
    if(num < 0) {
        n = -num;
        putcf('-');
        len++;
    }

    uint8_t num_len = get_int_len(num);
    if(num_len < width) {
        uint8_t fill_num = width - num_len;
        while(fill_num > 0) {
            putcf(pad);
            len++;
            fill_num--;
        }
    }

    return itoa_dec_unsigned(n)+len;
}

static uint8_t itoa_hex_ll(unsigned long long int num, uint8_t width, char pad)
{
    uint8_t len = 0;
    bool found_first = false;

    for(int8_t i = (64/4-1); i >= 0; i--) {
        uint8_t shift = i * 4;
        uint64_t mask = (uint64_t)0x0F << shift;
        uint64_t val = (num & mask) >> shift;

        if(val > 0) {
            found_first = true;
        }

        if(found_first || i < width) {
            if(found_first) {
                putcf(digit[val]);
            }
            else {
                putcf(pad);
            }

            len++;
        }
    }

    return len;
}

int evsprintf(char* buf, const char* fmt, va_list ap)
{
    int len=0;
    float num;
    char* str;
    uint8_t precision;
    uint8_t width;
    char pad;

    print_buf = buf;
    print_cnt = 0;

    while(*fmt) {
        if(*fmt == '%') {
            precision = DEF_PRECISION;
            pad = ' ';
            width = 0;

            fmt++;
            while('0' == *fmt) {
                pad = '0';
                fmt++;
            }

            while(isdigit((unsigned)*fmt)) {
                width *= 10;
                width += *fmt - '0';
                fmt++;
            }

            while(!isalpha((unsigned) *fmt)) {
                if(*fmt == '.') {
                    fmt++;
                    if(isdigit((unsigned)*fmt)) {
                        precision = *fmt - '0';
                        fmt++;
                    }
                }
            }

            switch(*fmt++) {
            case 'd':
                len += itoa_dec(va_arg(ap, int), width, pad);
                break;
            case 'u':
                len += itoa_dec_unsigned(va_arg(ap, unsigned int));
                break;
            case 'x':
            case 'X':
                len += itoa_hex(va_arg(ap, unsigned int), width, pad);
                break;
            case 'l':
                //len += handle_long(fmt, ap, width, pad);
                switch(*fmt++) {
                case 'd':
                    len += itoa_dec(va_arg(ap, long int), width, pad);
                    break;
                case 'u':
                    len += itoa_dec_unsigned(va_arg(ap, unsigned long int));
                    break;
                case 'x':
                case 'X':
                    len += itoa_hex(va_arg(ap, unsigned long int), width, pad);
                    break;
                case 'l':
                    //len += handle_longlong(fmt, ap, width, pad);
                    switch(*fmt++) {
                    case 'd':
                        len += itoa_dec_ll(va_arg(ap, long long int), width, pad);
                        break;
                    case 'u':
                        len += itoa_dec_ull(va_arg(ap, long long unsigned int));
                        break;
                    case 'x':
                    case 'X':
                        len += itoa_hex_ll(va_arg(ap, long long unsigned int), width, pad);
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                break;
            case 'f':
                num = va_arg(ap, double);
                if(num<0) {
                    putcf('-');
                    num = -num;
                    len++;
                }
                len += itoa_dec((int)num, width, pad);
                putcf('.');
                len++;
                len += itoa_dec((num - (int)num) * powerf(10, precision), precision, pad);
                break;
            case 's':
                str = va_arg(ap, char*);
                while(*str) {
                    putcf(*str++);
                    len++;
                }
                break;
            default:
                break;
            }
        }
        else {
            putcf(*fmt++);
            len++;
        }
    }

    return len;
}

int esprintf(char* buf, const char* fmt, ...)
{
    va_list ap;
    int len;

    va_start(ap, fmt);
    len = evsprintf(buf, fmt, ap);
    va_end(ap);

    return len;
}

