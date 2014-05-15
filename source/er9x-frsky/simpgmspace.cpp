/*
 * Authors (alphabetical order)
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 *
 * gruvin9x is based on code named er9x by
 * Author - Erez Raviv <erezraviv@gmail.com>, which is in turn
 * was based on the original (and ongoing) project by Thomas Husterer,
 * th9x -- http://code.google.com/p/th9x/
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


#include <ctype.h>
#include "simpgmspace.h"
#include "lcd.h"
#include "er9x.h"
#include "menus.h"

volatile uint8_t pinb=0, pinc=0xff, pind, pine=0xff, ping=0xff, pinh=0xff, pinj=0xff, pinl=0;
uint8_t portb, portc, porth=0, dummyport;
uint16_t dummyport16;

uint8_t heartbeat;

void setSwitch(int8_t swtch)
{
  switch (swtch) {
    case DSW_ID0:
      ping |=  (1<<INP_G_ID1);  pine &= ~(1<<INP_E_ID2);
      break;
    case DSW_ID1:
      ping &= ~(1<<INP_G_ID1);  pine &= ~(1<<INP_E_ID2);
      break;
    case DSW_ID2:
      ping &= ~(1<<INP_G_ID1);  pine |=  (1<<INP_E_ID2);
    default:
      break;
  }
}

uint8_t eeprom[EESIZE];

void eeWriteBlockCmp(const void *i_pointer_ram, uint16_t pointer_eeprom, size_t size)
{
#if 0
  printf(" eeWriteBlockCmp(%d %d)", size, (int)pointer_eeprom);
  for(uint8_t i=0; i<size; i++)
    printf(" %02X", ((const char*)i_pointer_ram)[i]);
  printf("\n");fflush(stdout);
#endif

  memcpy(&eeprom[pointer_eeprom], i_pointer_ram, size);
}

void eeprom_read_block (void *pointer_ram,
    const void *pointer_eeprom,
    size_t size)
{
  memcpy(pointer_ram, &eeprom[(uint64_t)pointer_eeprom], size);
}


uint8_t main_thread_running = 0;
char * main_thread_error = NULL;
void *main_thread(void *)
{
#ifdef SIMU_EXCEPTIONS
  signal(SIGFPE, sig);
  signal(SIGSEGV, sig);

  try {
#endif
    g_menuStack[0] = menuProc0;
    g_menuStack[1] = menuProcModelSelect;

    eeReadAll(); //load general setup and selected model

    if (main_thread_running == 1) {
      doSplash();
      checkMem();
      checkTHR();
      checkSwitches();
      checkAlarm();
      checkWarnings();
    }

    while (main_thread_running) {
      perMain();
      sleep(1/*ms*/);
    }
#ifdef SIMU_EXCEPTIONS
  }
  catch (...) {
    main_thread_running = 0;
  }
#endif
  return NULL;
}

pthread_t main_thread_pid;
void StartMainThread(bool tests)
{
  frskyStreaming = 1;
  frskyUsrStreaming = 1;

  main_thread_running = (tests ? 1 : 2);
  pthread_create(&main_thread_pid, NULL, &main_thread, NULL);
}

void StopMainThread()
{
  main_thread_running = 0;
  pthread_join(main_thread_pid, NULL);
}

#if 0
static void EeFsDump(){
  for(int i=0; i<EESIZE; i++)
  {
    printf("%02x ",eeprom[i]);
    if(i%16 == 15) puts("");
  }
  puts("");
}
#endif

void sig(int sgn)
{
  main_thread_error = (char *)malloc(2048);
  sprintf(main_thread_error,"Signal %d caught\n", sgn);
  write_backtrace(main_thread_error);
  throw std::exception();
}

