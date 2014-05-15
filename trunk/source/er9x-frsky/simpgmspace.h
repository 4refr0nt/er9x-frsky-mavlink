/*
 * Author - Erez Raviv <erezraviv@gmail.com>
 *
 * Based on th9x -> http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef simpgmspace_h
#define simpgmspace_h

#ifndef __GNUC__
#include <windows.h>
#define sleep(x) Sleep(x)
#else
#include <unistd.h>
#define sleep(x) usleep(1000*x)
#endif

#ifdef SIMU_EXCEPTIONS
extern char * main_thread_error;
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#if defined(WIN32) || !defined(__GNUC__)
#define write_backtrace(output)
#else
#include <execinfo.h>
inline void write_backtrace(char *output)
{

  void *buf[16];
  char **s;
  int n = backtrace(buf,16);
  s = backtrace_symbols(buf, n);
  if (s) {
    for(int i=0; i<n; i++)
      sprintf(output+strlen(output), "%02i: %s\n",i,s[i]);
  }
}
#endif
void sig(int sgn);
#define assert(x) do { if (!(x)) { main_thread_error = (char *)malloc(2048); sprintf(main_thread_error, "Assert failed, %s:%d: %s\n", __FILE__, __LINE__, #x); write_backtrace(main_thread_error); throw std::exception(); } } while(0)
#else
#include <assert.h>
#endif

#include <inttypes.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#undef min
#undef max

#define APM
#define PGM_P prog_char *

typedef unsigned char prog_uchar;
typedef const char prog_char;
typedef const uint16_t prog_uint16_t;
typedef const uint8_t prog_uint8_t;
typedef const int16_t prog_int16_t;
typedef const int8_t prog_int8_t;

extern sem_t eeprom_write_sem;

#define loop_until_bit_is_set( port, bitnum) \
  while ( 0/*! ( (port) & (1 << (bitnum)) )*/ ) ;


#define PROGMEM
#define pgm_read_byte(address_short) (*(uint8_t*)(address_short))
#define pgm_read_word(address_short) (*(uint16_t*)(address_short))
#define pgm_read_adr(address_short) *address_short
#define pgm_read_stringP(adr) ((adr))
#define PSTR(adr) adr
#define _delay_us(a)
#define cli()
#define sei()
#define strcpy_P strcpy
#define strncpy_P strncpy
#define memcpy_P memcpy
#define asm(x)

#define PORTA dummyport
#define PORTB portb
#define PORTC portc
#define PORTD dummyport
#define PORTE dummyport
#define PORTF dummyport
#define PORTG dummyport
#define PORTH porth
#define DDRA  dummyport
#define DDRB  dummyport
#define DDRC  dummyport
#define DDRD  dummyport
#define DDRE  dummyport
#define DDRF  dummyport
#define DDRG  dummyport
#define PINA  ~pinb
#define PINB  ~pinb
#define PINC  ~pinc
#define PIND  ~pind
#define PINE  ~pine
#define PING  ~ping
#define PINH  ~pinh
#define PINJ  ~pinj
#define PINL  ~pinl
#define EEMEM

#define UCSR0B dummyport
#define UDRIE0 dummyport
#define TXEN0 dummyport
#define RXEN0 dummyport
#define DDE0 dummyport
#define PORTE0 dummyport
#define RXCIE0 dummyport
#define OCR0A dummyport
#define OCR4A dummyport
#define OCR1B dummyport16
#define TCCR1A dummyport
#define COM1B0 dummyport

#define SPDR dummyport
#define SPSR dummyport
#define SPIF dummyport
#define SPCR dummyport

#define TCNT0 0

#define TIMSK  dummyport
#define TIMSK1 dummyport

#define UDR0 dummyport
#define OCIE1A dummyport

#define OUT_B_LIGHT   7
#define INP_E_ElevDR  2
#define INP_E_Trainer 5
#define INP_E_Gear    4
#define INP_C_ThrCt   6
#define INP_C_AileDR  7
#define INP_E_ID2     6

#define INP_B_KEY_LFT 6
#define INP_B_KEY_RGT 5
#define INP_B_KEY_UP  4
#define INP_B_KEY_DWN 3
#define INP_B_KEY_EXT 2
#define INP_B_KEY_MEN 1

#define INP_P_SPARE6    7
#define INP_P_SPARE5    6
#define INP_P_KEY_EXT   5
#define INP_P_KEY_MEN   4
#define INP_P_KEY_LFT   3
#define INP_P_KEY_RGT   2
#define INP_P_KEY_UP    1
#define INP_P_KEY_DWN   0

extern volatile unsigned char pinb,pinc,pind,pine,ping,pinh,pinj,pinl;
extern uint8_t portb, portc, porth, dummyport;
extern uint16_t dummyport16;
extern uint8_t main_thread_running;

extern void setSwitch(int8_t swtch);

void StartMainThread(bool tests=true);
void StartEepromThread(const char *filename="eeprom.bin");

extern const char *eepromFile;
 void eeprom_read_block (void *pointer_ram,
                    const void *pointer_eeprom,
                         size_t size);

#undef offsetof
#define offsetof(st, m) ((size_t) ( (char *)&((st *)(0))->m - (char *)0 ))
#define wdt_reset()

#endif
