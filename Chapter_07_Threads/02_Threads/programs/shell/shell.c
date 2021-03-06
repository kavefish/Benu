/*! simple shell interpreter */

#include <stdio.h>
#include <lib/string.h>
#include <time.h>
#include <kernel/memory.h>
#include <arch/processor.h>
#include <pthread.h>

char PROG_HELP[] = "Simple command shell";

#include PROGRAMS

typedef struct _cmd_t_
{
	int (*func) ( char *argv[] );
	char *name;
	char *descr;
}
cmd_t;

#define MAXCMDLEN	72
#define MAXARGS		10
#define INFO_SIZE	1000

static int help ();
static int clear ();
static int sysinfo ( char *args[] );
static int turn_off ( char *args[] );

static cmd_t sh_cmd[] =
{
	{ help, "help", "help - list available commands" },
	{ clear, "clear", "clear - clear screen" },
	{ sysinfo, "sysinfo", "system information; usage: sysinfo [options]" },
	{ turn_off, "poweroff", "poweroff - use ACPI to power off" },
	{ NULL, "" }
};

static cmd_t prog[] = PROGRAMS_FOR_SHELL;

int shell ( char *args[] )
{
	char cmd[MAXCMDLEN + 1];
	int i, key, rv;
	timespec_t t;
	int argnum;
	char *argval[MAXARGS + 1];
	pthread_t thr;

	printf ( "\n*** Simple shell interpreter ***\n\n" );
	help ();

	t.tv_sec = 0;
	t.tv_nsec = 100000000; /* 100 ms */

	while (1)
	{
		new_cmd:
		printf ( "\n> " );

		i = 0;
		memset ( cmd, 0, MAXCMDLEN );

		/* get command - get chars until new line is received */
		while ( i < MAXCMDLEN )
		{
			key = get_char ();

			if ( !key )
			{
				nanosleep ( &t, NULL );
				continue;
			}

			if ( key == '\n' || key == '\r')
			{
				if ( i > 0 )
					break;
				else
					goto new_cmd;
			}

			switch ( key )
			{
			case '\b':
				if ( i > 0 )
				{
					cmd[--i] = 0;
					printf ( "%c", key );
				}
				break;

			default:
				printf ( "%c", key );
				cmd[i++] = (char) key;
				break;
			}
		}
		printf ( "\n" );

		/* parse command line */
		argnum = 0;
		for(i = 0; i < MAXCMDLEN && cmd[i]!=0 && argnum < MAXARGS; i++)
		{
			if ( cmd[i] == ' ' || cmd[i] == '\t')
				continue;

			argval[argnum++] = &cmd[i];
			while ( cmd[i] && cmd[i] != ' ' && cmd[i] != '\t'
				&& i < MAXCMDLEN )
				i++;

			cmd[i] = 0;
		}
		argval[argnum] = NULL;

		/* match command to shell command */
		for ( i = 0; sh_cmd[i].func != NULL; i++ )
		{
			if ( strcmp ( argval[0], sh_cmd[i].name ) == 0 )
			{
				if ( sh_cmd[i].func ( argval ) )
					printf ( "\nProgram returned error!\n" );

				goto new_cmd;
			}
		}

		/* not shell command; start given program within new thread */
		/* match command to program */
		for ( i = 0; prog[i].func != NULL; i++ )
		{
			if ( strcmp ( argval[0], prog[i].name ) == 0 )
			{
				rv = pthread_create ( &thr, NULL,
						      (void *) prog[i].func,
						      argval );
				if ( !rv )
				{
					if ( argnum < 2 ||
						argval[argnum-1][0] != '&' )
						pthread_join ( thr, NULL );

					goto new_cmd;
				}
			}
		}

		if ( strcmp ( argval[0], "quit" ) == 0 ||
			strcmp ( argval[0], "exit" ) == 0 )
			break;

		/* not program kernel or shell knows about it - report error! */
		printf ( "Invalid command!" );
	}

	printf ( "Exiting from shell\n" );

	return 0;
}

static int help ()
{
	int i;

	printf ( "Shell commands: " );
	for ( i = 0; sh_cmd[i].func != NULL; i++ )
		printf ( "%s ", sh_cmd[i].name );
	printf ( " quit/exit\n" );

	printf ( "Programs: " );
	for ( i = 0; prog[i].func != NULL; i++ )
		printf ( "%s ", prog[i].name );
	printf ( "\n" );

	return 0;
}

static int clear ()
{
	return clear_screen ();
}

static int sysinfo ( char *args[] )
{
	char info[INFO_SIZE];

	sys__sysinfo ( info, INFO_SIZE, args );

	printf ( "%s\n", info );

	return 0;
}

static int turn_off ( char *args[] )
{
	printf ( "Powering off\n\n" );
	power_off ();

	return -1;
}
