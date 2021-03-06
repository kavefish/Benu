/*! Print on console using ARM PrimeCell UART (PL011) [very simple mode!] */

#ifdef UART0

#include <types/io.h>
#include <lib/string.h>
#include <types/basic.h>
#include <ARCH/config.h>

#define UART0_FR	(UART0_BASE + 0x18)
#define UART0_DR	(UART0_BASE + 0x00)
#define UART0_IMSC	(UART0_BASE + 0x38)

static void uart0_putchar ( char c );
static int uart0_init ();
static int uart0_clear ();
static int uart0_gotoxy ( int x, int y );
static int uart0_printf ( int attr, char *text );


/*!
 * Send character on uart0
 * \param c Single character
 */
static void uart0_putchar ( char c )
{
	volatile unsigned int *uart_dr = (unsigned int *) UART0_DR;
	volatile unsigned int *uart_fr = (unsigned int *) UART0_FR;

	/* Wait for UART to become ready to transmit */
	while ( (*uart_fr) & (1 << 5) )
		;

	/* Transmit char */
	*uart_dr = (unsigned int) c;

}

#if 1 /* test UART receive irqs with echo */
#include <arch/interrupt.h>
/*!
 * Get character from uart0
 */
static int uart0_getchar ()
{
	volatile unsigned int *uart_dr = (unsigned int *) UART0_DR;
	volatile unsigned int *uart_fr = (unsigned int *) UART0_FR;

	if ( (*uart_fr) & (1 << 4) ) /* empty */
		return 0;
	else
		return *uart_dr;
}

static void uart_echo ()
{
	uart0_putchar ( uart0_getchar() );
}

/*! Init console with interrupt enable */
void uart0_echo_test_start ()
{
	volatile unsigned int *uart_imsc = (unsigned int *) UART0_IMSC;

	/* enable RXIM interrupt (interrupt on data receive) */
	*uart_imsc =  1 << 4;

	/* register UART interrupt */
	arch_register_interrupt_handler ( IRQ_OFFSET + UART0IRQL, uart_echo );

	/* enable UART interrupt in VIC */
	arch_irq_enable ( IRQ_OFFSET + UART0IRQL );
}
#endif

/*! Init console */
int uart0_init ()
{
	return 0;
}

/*!
 * Clear console
 * (just go to new line - send \n on serial port)
 */
int uart0_clear ()
{
	uart0_putchar ( '\n' );
	return 0;
}

/*!
 * Move cursor to specified location
 * (just go to new line - send \n on serial port)
 */
static int uart0_gotoxy ( int x, int y )
{
	uart0_putchar ( '\n' );
	return 0;
}

/*!
 * Print text string on console, starting at current cursor position
 * \param attr Attributes to apply (ignored)
 * \param text String to print
 */
static int uart0_printf ( int attr, char *text )
{
	if ( text == NULL )
		return 0;


	while ( *text != '\0' ) /* Loop until end of string */
		uart0_putchar ( *text++ );

	return strlen ( text );
}


/*! uart0 as console */
console_t uart0 = (console_t)
{
	.init	= uart0_init,
	.clear	= uart0_clear,
	.gotoxy	= uart0_gotoxy,
	.print	= uart0_printf
};

static int _do_nothing_ ()
{
	return 0;
}

/*! dev_null */
console_t dev_null = (console_t)
{
	.init	= (void *) _do_nothing_,
	.clear	= (void *) _do_nothing_,
	.gotoxy	= (void *) _do_nothing_,
	.print	= (void *) _do_nothing_
};

#endif /* UART0 */
