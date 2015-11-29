/*
    ksym_mod.c - functions for building symbol lookup tables for klogd
    Copyright (c) 1995, 1996  Dr. G.W. Wettstein <greg@wind.rmcc.com>
    Copyright (c) 1996 Enjellic Systems Development
    Copyright (c) 1998-2007 Martin Schulze <joey@infodrom.org>

    This file is part of the sysklogd package, a kernel and system log daemon.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
 * This file implements functions which are useful for building
 * a symbol lookup table based on the in kernel symbol table
 * maintained by the Linux kernel.
 *
 * Proper logging of kernel panics generated by loadable modules
 * tends to be difficult.  Since the modules are loaded dynamically
 * their addresses are not known at kernel load time.  A general
 * protection fault (Oops) cannot be properly deciphered with 
 * classic methods using the static symbol map produced at link time.
 *
 * One solution to this problem is to have klogd attempt to translate
 * addresses from module when the fault occurs.  By referencing the
 * the kernel symbol table proper resolution of these symbols is made
 * possible.
 *
 * At least that is the plan.
 *
 * Wed Aug 21 09:20:09 CDT 1996:  Dr. Wettstein
 *	The situation where no module support has been compiled into a
 *	kernel is now detected.  An informative message is output indicating
 *	that the kernel has no loadable module support whenever kernel
 *	module symbols are loaded.
 *
 *	An informative message is printed indicating the number of kernel
 *	modules and the number of symbols loaded from these modules.
 *
 * Sun Jun 15 16:23:29 MET DST 1997: Michael Alan Dorman
 *	Some more glibc patches made by <mdorman@debian.org>.
 *
 * Sat Jan 10 15:00:18 CET 1998: Martin Schulze <joey@infodrom.north.de>
 *	Fixed problem with klogd not being able to be built on a kernel
 *	newer than 2.1.18.  It was caused by modified structures
 *	inside the kernel that were included.  I have worked in a
 *	patch from Alessandro Suardi <asuardi@uninetcom.it>.
 *
 * Sun Jan 25 20:57:34 CET 1998: Martin Schulze <joey@infodrom.north.de>
 *	Another patch for Linux/alpha by Christopher C Chimelis
 *	<chris@classnet.med.miami.edu>.
 *
 * Thu Mar 19 23:39:29 CET 1998: Manuel Rodrigues <pmanuel@cindy.fe.up.pt>
 *	Changed lseek() to llseek() in order to support > 2GB address
 *	space which provided by kernels > 2.1.70.
 *
 * Mon Apr 13 18:18:45 CEST 1998: Martin Schulze <joey@infodrom.north.de>
 *	Removed <sys/module.h> as it's no longer part of recent glibc
 *	versions.  Added prototyp for llseek() which has been
 *	forgotton in <unistd.h> from glibc.  Added more log
 *	information if problems occurred while reading a system map
 *	file, by submission from Mark Simon Phillips <M.S.Phillips@nortel.co.uk>.
 *
 * Sun Jan  3 18:38:03 CET 1999: Martin Schulze <joey@infodrom.north.de>
 *	Corrected return value of AddModule if /dev/kmem can't be
 *	loaded.  This will prevent klogd from segfaulting if /dev/kmem
 *	is not available.  Patch from Topi Miettinen <tom@medialab.sonera.net>.
 *
 * Tue Sep 12 23:11:13 CEST 2000: Martin Schulze <joey@infodrom.ffis.de>
 *	Changed llseek() to lseek64() in order to skip a libc warning.
 *
 * Wed Mar 31 17:35:01 CEST 2004: Martin Schulze <joey@infodrom.org>
 *	Removed references to <linux/module.h> since it doesn't work
 *	anymore with its recent content from Linux 2.4/2.6, created
 *	module.h locally instead.
 *
 * Fri May 25 20:07:30 CEST 2007: Martin Schulze <joey@infodrom.org>
 *	Use new query_module function rather than the old obsolete
 *	hack of stepping through /dev/kmem.
 *
 * Mon May 28 16:46:59 CEST 2007: Martin Schulze <joey@infodrom.org>
 *	Since Linux 2.6 query_module is depricated and no implemented
 *	anymore.  Thus, overhauled symbol import via /proc/kallsyms
 *
 * Thu May 31 12:12:23 CEST 2007: Martin Schulze <joey@infodrom.org>
 *	Only read kernel symbols from /proc/kallsyms if no System.map
 *	has been read as it may contain more symbols.
 *
 * Thu May 31 16:56:26 CEST 2007: Martin Schulze <joey@infodrom.org>
 *	Improved symbol lookup, since symbols are spread over the entire
 *	address space.  Return the symbol that fits best instead of
 *	the first hit.
 */


/* Includes. */
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <malloc.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include "module.h"
#if !defined(__GLIBC__)
#include <linux/time.h>

#include <linux/linkage.h>
#else /* __GLIBC__ */
#include <linux/linkage.h>

#endif /* __GLIBC__ */
#include <stdarg.h>
#include <paths.h>
#include <linux/version.h>

#include "klogd.h"
#include "ksyms.h"

#define KSYMS  "/proc/kallsyms"

static int num_modules = 0;
struct Module *sym_array_modules = (struct Module *) 0;

static int have_modules = 0;

#if defined(TEST)
static int debugging = 1;
#else
extern int debugging;
#endif


/* Function prototypes. */
static void FreeModules(void);
static int AddSymbol(const char *);
struct Module *AddModule(const char *);
static int symsort(const void *, const void *);

/* Imported from ksym.c */
extern int num_syms;

/**************************************************************************
 * Function:	InitMsyms
 *
 * Purpose:	This function is responsible for building a symbol
 *		table which can be used to resolve addresses for
 *		loadable modules.
 *
 * Arguments:	Void
 *
 * Return:	A boolean return value is assumed.
 *
 *		A false value indicates that something went wrong.
 *
 *		True if loading is successful.
 **************************************************************************/

extern int InitMsyms()

{
	auto int	rtn,
			tmp;

	FILE *ksyms;

	char buf[128];
	char *p;

	/* Initialize the kernel module symbol table. */
	FreeModules();

	ksyms = fopen(KSYMS, "r");

	if ( ksyms == NULL )
	{
		if ( errno == ENOENT )
			Syslog(LOG_INFO, "No module symbols loaded - "
			       "kernel modules not enabled.\n");
		else
			Syslog(LOG_ERR, "Error loading kernel symbols " \
			       "- %s\n", strerror(errno));
		fclose(ksyms);
		return(0);
	}

	if ( debugging )
		fprintf(stderr, "Loading kernel module symbols - "
			"Source: %s\n", KSYMS);

	while ( fgets(buf, sizeof(buf), ksyms) != NULL )
	{
		if (num_syms > 0 && index(buf, '[') == NULL)
			continue;

		p = index(buf, ' ');

		if ( p == NULL )
			continue;

		if ( buf[strlen(buf)-1] == '\n' )
			buf[strlen(buf)-1] = '\0';
		/* overlong lines will be ignored above */

		AddSymbol(buf);
	}

	fclose(ksyms);

	have_modules = 1;

	/* Sort the symbol tables in each module. */
	for (rtn = tmp = 0; tmp < num_modules; ++tmp)
	{
		rtn += sym_array_modules[tmp].num_syms;
		if ( sym_array_modules[tmp].num_syms < 2 )
			continue;
		qsort(sym_array_modules[tmp].sym_array, \
		      sym_array_modules[tmp].num_syms, \
		      sizeof(struct sym_table), symsort);
	}

	if ( rtn == 0 )
		Syslog(LOG_INFO, "No module symbols loaded.");
	else
		Syslog(LOG_INFO, "Loaded %d %s from %d module%s", rtn, \
		       (rtn == 1) ? "symbol" : "symbols", \
		       num_modules, (num_modules == 1) ? "." : "s.");

	return(1);
}


static int symsort(p1, p2)

     const void *p1;

     const void *p2;

{
	auto const struct sym_table	*sym1 = p1,
					*sym2 = p2;

	if ( sym1->value < sym2->value )
		return(-1);
	if ( sym1->value == sym2->value )
		return(0);
	return(1);
}


/**************************************************************************
 * Function:	FreeModules
 *
 * Purpose:	This function is used to free all memory which has been
 *		allocated for the modules and their symbols.
 *
 * Arguments:	None specified.
 *
 * Return:	void
 **************************************************************************/

static void FreeModules()

{
	auto int	nmods,
			nsyms;

	auto struct Module *mp;


	/* Check to see if the module symbol tables need to be cleared. */
	have_modules = 0;
	if ( num_modules == 0 )
		return;

	if ( sym_array_modules == NULL )
		return;

	for (nmods = 0; nmods < num_modules; ++nmods)
	{
		mp = &sym_array_modules[nmods];
		if ( mp->num_syms == 0 )
			continue;
	       
		for (nsyms= 0; nsyms < mp->num_syms; ++nsyms)
			free(mp->sym_array[nsyms].name);
		free(mp->sym_array);
		if ( mp->name != NULL )
			free(mp->name);
	}

	free(sym_array_modules);
	sym_array_modules = (struct Module *) 0;
	num_modules = 0;
	return;
}


/**************************************************************************
 * Function:	AddModule
 *
 * Purpose:	This function is responsible for adding a module to
 *		the list of currently loaded modules.
 *
 * Arguments:	(const char *) module
 *
 *		module:->	The name of the module.
 *
 * Return:	struct Module *
 **************************************************************************/

struct Module *AddModule(module)

     const char *module;

{
	struct Module *mp;

	if ( num_modules == 0 )
	{
		sym_array_modules = (struct Module *)malloc(sizeof(struct Module));

		if ( sym_array_modules == NULL )
		{
			Syslog(LOG_WARNING, "Cannot allocate Module array.\n");
			return NULL;
		}
		mp = sym_array_modules;
	}
	else
	{
		/* Allocate space for the module. */
		mp = (struct Module *) \
			realloc(sym_array_modules, \
				(num_modules+1) * sizeof(struct Module));

		if ( mp == NULL )
		{
			Syslog(LOG_WARNING, "Cannot allocate Module array.\n");
			return NULL;
		}

		sym_array_modules = mp;
		mp = &sym_array_modules[num_modules];
	}

	num_modules++;
	mp->sym_array = NULL;
	mp->num_syms = 0;

	if ( module != NULL )
		mp->name = strdup(module);
	else
		mp->name = NULL;

	return mp;
}


/**************************************************************************
 * Function:	AddSymbol
 *
 * Purpose:	This function is responsible for adding a symbol name
 *		and its address to the symbol table.
 *
 * Arguments:	(struct Module *) mp, (unsigned long) address, (char *) symbol
 *
 *		mp:->	A pointer to the module which the symbol is
 *			to be added to.
 *
 *		address:->	The address of the symbol.
 *
 *		symbol:->	The name of the symbol.
 *
 * Return:	int
 *
 *		A boolean value is assumed.  True if the addition is
 *		successful.  False if not.
 **************************************************************************/

static int AddSymbol(line)

	const char *line;
	
{
	char *module;
	unsigned long address;
	char *p;
	static char *lastmodule = NULL;
	struct Module *mp;

	module = index(line, '[');

	if ( module != NULL )
	{
		p = index(module, ']');

		if ( p != NULL )
			*p = '\0';

		p = module++;

		while ( isspace(*(--p)) );
		*(++p) = '\0';
	}

	p = index(line, ' ');

	if ( p == NULL )
		return(0);

	*p = '\0';

	address  = strtoul(line, (char **) 0, 16);

	p += 3;

	if ( num_modules == 0 ||
	     ( lastmodule == NULL && module != NULL ) ||
	     ( module == NULL && lastmodule != NULL) ||
	     ( module != NULL && strcmp(module, lastmodule)))
	{
		mp = AddModule(module);

		if ( mp == NULL )
			return(0);
	}
	else
		mp = &sym_array_modules[num_modules-1];

	lastmodule = mp->name;

	/* Allocate space for the symbol table entry. */
	mp->sym_array = (struct sym_table *) realloc(mp->sym_array, \
        	(mp->num_syms+1) * sizeof(struct sym_table));

	if ( mp->sym_array == (struct sym_table *) 0 )
		return(0);

	mp->sym_array[mp->num_syms].name = strdup(p);
	if ( mp->sym_array[mp->num_syms].name == (char *) 0 )
		return(0);
	
	/* Stuff interesting information into the module. */
	mp->sym_array[mp->num_syms].value = address;
	++mp->num_syms;

	return(1);
}


/**************************************************************************
 * Function:	LookupModuleSymbol
 *
 * Purpose:	Find the symbol which is related to the given address from
 *		a kernel module.
 *
 * Arguments:	(long int) value, (struct symbol *) sym
 *
 *		value:->	The address to be located.
 * 
 *		sym:->		A pointer to a structure which will be
 *				loaded with the symbol's parameters.
 *
 * Return:	(char *)
 *
 *		If a match cannot be found a diagnostic string is printed.
 *		If a match is found the pointer to the symbolic name most
 *		closely matching the address is returned.
 **************************************************************************/

extern char * LookupModuleSymbol(value, sym)

	unsigned long value;

	struct symbol *sym;
	
{
	auto int	nmod,
			nsym;

	auto struct sym_table *last;

	auto struct Module *mp;

	static char ret[100];


	sym->size = 0;
	sym->offset = 0;
	if ( num_modules == 0 )
		return((char *) 0);
	
	for (nmod = 0; nmod < num_modules; ++nmod)
	{
		mp = &sym_array_modules[nmod];

		/*
		 * Run through the list of symbols in this module and
		 * see if the address can be resolved.
		 */
		for(nsym = 1, last = &mp->sym_array[0];
		    nsym < mp->num_syms;
		    ++nsym)
		{
			if ( mp->sym_array[nsym].value > value )
			{
			    if ( sym->size == 0 ||
				 (value - last->value) < sym->offset ||
				 ( (sym->offset == (value - last->value)) &&
				   (mp->sym_array[nsym].value-last->value) < sym->size ) )
			    {
				sym->offset = value - last->value;
				sym->size = mp->sym_array[nsym].value - \
					last->value;
				ret[sizeof(ret)-1] = '\0';
				if ( mp->name == NULL )
					snprintf(ret, sizeof(ret)-1,
						 "%s", last->name);
				else
					snprintf(ret, sizeof(ret)-1,
						 "%s:%s", mp->name, last->name);
			    }
			    break;
			}
			last = &mp->sym_array[nsym];
		}
	}

	if ( sym->size > 0 )
		return(ret);

	/* It has been a hopeless exercise. */
	return((char *) 0);
}


/*
 * Setting the -DTEST define enables the following code fragment to
 * be compiled.  This produces a small standalone program which will
 * dump the current kernel symbol table.
 */
#if defined(TEST)

#include <stdarg.h>


extern int main(int, char **);


int main(argc, argv)

	int argc;

	char *argv[];

{
	auto int lp, syms;


	if ( !InitMsyms() )
	{
		fprintf(stderr, "Cannot load module symbols.\n");
		return(1);
	}

	printf("Number of modules: %d\n\n", num_modules);

	for(lp= 0; lp < num_modules; ++lp)
	{
		printf("Module #%d = %s, Number of symbols = %d\n", lp + 1, \
		       sym_array_modules[lp].name == NULL
		       ?"kernel space"
		       :sym_array_modules[lp].name, \
		       sym_array_modules[lp].num_syms);

		for (syms= 0; syms < sym_array_modules[lp].num_syms; ++syms)
		{
			printf("\tSymbol #%d\n", syms + 1);
			printf("\tName: %s\n", \
			       sym_array_modules[lp].sym_array[syms].name);
			printf("\tAddress: %lx\n\n", \
			       sym_array_modules[lp].sym_array[syms].value);
		}
	}

	FreeModules();
	return(0);
}

extern void Syslog(int priority, char *fmt, ...)

{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stdout, "Pr: %d, ", priority);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fputc('\n', stdout);

	return;
}

#endif
/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 *  tab-width: 8
 * End:
 */
