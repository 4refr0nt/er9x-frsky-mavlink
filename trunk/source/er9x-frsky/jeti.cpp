/*
 * Author - Karl Szmutny <shadow@privy.de>
 *
 * Based on the Code from Peter "woggle" Mack, mac@denich.net
 * -> http://svn.mikrokopter.de/filedetails.php?repname=Projects&path=/Transportables_Koptertool/trunk/jeti.c
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

#include "jeti.h"
#include "er9x.h"

uint16_t jeti_keys = JETI_KEY_NOCHANGE;
uint8_t JetiBuffer[32]; // 32 characters
uint8_t JetiBufferReady;


ISR (USART0_RX_vect)
{
        uint8_t stat;
        uint8_t rh;
        uint8_t rl;
        static uint8_t jbp;
        
        stat = UCSR0A;
        rh = UCSR0B;
        rl = UDR0;

        
        if (stat & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))
        {       // discard buffer and start new on any error
                JetiBufferReady = 0;
                jbp = 0;
        }
        else if ((rh & (1 << RXB80)) == 0)
        {       // control
                if (rl == 0xfe)
                {       // start condition
                        JetiBufferReady = 0;
                        jbp = 0;
                }
                else if (rl == 0xff)
                {       // stop condition
                        JetiBufferReady = 1;
                }
        }
        else
        {       // data
                if (jbp < 32)
                {
                    if(rl==0xDF) JetiBuffer[jbp++] = '@'; //@ => °  Issue 163
                    else         JetiBuffer[jbp++] = rl;
                }
        }

}



void JETI_Init (void)
{

   jeti_keys = JETI_KEY_NOCHANGE;

   DDRE  &= ~(1 << DDE0);          // set RXD0 pin as input
   PORTE &= ~(1 << PORTE0);        // disable pullup on RXD0 pin

#undef BAUD
#define BAUD 9600
#include <util/setbaud.h>
   UBRR0H = UBRRH_VALUE;
   UBRR0L = UBRRL_VALUE;

   UCSR0A &= ~(1 << U2X0); // disable double speed operation

   // set 9O1
   UCSR0C = (1 << UPM01) | (1 << UPM00) | (1 << UCSZ01) | (1 << UCSZ00);
   UCSR0B = (1 << UCSZ02);
        
   // flush receive buffer
   while ( UCSR0A & (1 << RXC0) ) UDR0;

}


void JETI_DisableTXD (void)
{
        UCSR0B &= ~(1 << TXEN0);        // disable TX
}


void JETI_EnableTXD (void)
{
        UCSR0B |=  (1 << TXEN0);        // enable TX
}


void JETI_DisableRXD (void)
{
        UCSR0B &= ~(1 << RXEN0);        // disable RX
        UCSR0B &= ~(1 << RXCIE0);       // disable Interrupt
}


void JETI_EnableRXD (void)
{
        UCSR0B |=  (1 << RXEN0);        // enable RX
        UCSR0B |=  (1 << RXCIE0);       // enable Interrupt
}


void JETI_putw (uint16_t c)
{
        loop_until_bit_is_set(UCSR0A, UDRE0);
        UCSR0B &= ~(1 << TXB80);
        if (c & 0x0100)
        {
                UCSR0B |= (1 << TXB80);
        }
        UDR0 = c;
}

void JETI_putc (uint8_t c)
{
        loop_until_bit_is_set(UCSR0A, UDRE0);
        //      UCSRB &= ~(1 << TXB8);
        UCSR0B |= (1 << TXB80);
        UDR0 = c;
}

void JETI_puts (char *s)
{
        while (*s)
        {
                JETI_putc (*s);
                s++;
        }
}

void JETI_put_start (void)
{
        loop_until_bit_is_set(UCSR0A, UDRE0);
        UCSR0B &= ~(1 << TXB80);
        UDR0 = 0xFE;
}

void JETI_put_stop (void)
{
        loop_until_bit_is_set(UCSR0A, UDRE0);
        UCSR0B &= ~(1 << TXB80);
        UDR0 = 0xFF;
}

