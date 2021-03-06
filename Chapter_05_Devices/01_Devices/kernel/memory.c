/*! Memory management */
#define _K_MEMORY_C_

#include "memory.h"

#include "kprint.h"
#include <kernel/errno.h>
#include <arch/processor.h>
#include <lib/string.h>
#include <lib/list.h>
#include <types/bits.h>

/*! Dynamic memory allocator for kernel */
MEM_ALLOC_T *k_mpool = NULL;

/*! Memory segments */
static mseg_t *mseg = NULL;

list_t kobjects; /* list of kernel objects reserved by programs */

/*! Initial memory layout created in arch layer */
void k_memory_init ()
{
	int i;

	k_mpool = NULL;
	mseg = arch_memory_init ();

	/* find kernel heap */
	for ( i = 0; mseg[i].type != MS_END && !k_mpool; i++ )
	{
		if ( mseg[i].type == MS_KHEAP )
		{
			k_mpool = k_mem_init ( mseg[i].start, mseg[i].size );
			break;
		}
	}

	ASSERT ( k_mpool );
}

inline void *k_mem_init ( void *segment, size_t size )
{
	return K_MEM_INIT ( segment, size );
}
inline void *kmalloc ( size_t size )
{
	return KMALLOC ( size );
}
inline int kfree ( void *chunk )
{
	return KFREE ( chunk );
}

/*! Allocate space for kernel object and descriptor of that object */
void *kmalloc_kobject ( size_t obj_size )
{
	kobject_t *kobj;

	kobj = kmalloc ( sizeof (kobject_t) + obj_size );
	ASSERT ( kobj );

	kobj->flags = 0;
	kobj->ptr = NULL;

	if ( obj_size )
		kobj->kobject = kobj + 1;
	else
		kobj->kobject = NULL;

	list_append ( &kobjects, kobj, &kobj->list );

	return kobj;
}

/*! Free space reserved by kernel object */
void *kfree_kobject ( kobject_t *kobj )
{
	ASSERT ( kobj );
#ifndef DEBUG
	list_remove ( &kobjects, 0, &kobj->list );
#else /* DEBUG */
	ASSERT ( list_find_and_remove ( &kobjects, &kobj->list ) );
#endif

	kfree ( kobj );

	return EXIT_SUCCESS;
}

/*! unique system wide id numbers */
#define	WBITS		( sizeof(word_t) * 8 )
#define	ID_ELEMS	( (MAX_RESOURCES-1) / WBITS + 1 )
#define MAX_RES		( ID_ELEMS * WBITS )

static word_t idmask[ ID_ELEMS ] = { 0 };
static id_t last_id = 0;

/*! Allocate and return unique id for new system resource */
id_t k_new_id ()
{
	id_t id = -1;
	uint elem, n;
	word_t mask;

	last_id = ( last_id + 1 < MAX_RES ? last_id + 1 : 1 ); /* skip 0 */

	elem = last_id / WBITS;
	mask = idmask [elem] | ( ( 1 << (last_id % WBITS) ) - 1 );
	/* do not look at lower bits (for now) */

	if ( ~mask ) /* current 'elem' has free ids from last_id forward */
	{
		id = lsb_index ( ~mask );
	}
	else {
		n = elem + 1;
		do {
			if ( ~idmask[n] )
			{
				id = lsb_index ( ~idmask[n] );
				elem = n;
				break;
			}
			n = ( n + 1 ) % ID_ELEMS;
		}
		while ( n != elem + 1 );
	}

	ASSERT ( id != -1 );

	idmask [ elem ] |= ( 1 << id );	/* reserve ID */
	id += elem * WBITS;
	last_id = id;

	return id;
}

/*! Release resource id */
void k_free_id ( id_t id )
{
	ASSERT ( id > 0 && id < MAX_RES &&
		 ( idmask [ id / WBITS ] & ( 1 << ( id % WBITS ) ) ) );

	idmask [ id / WBITS ] &= ~ ( 1 << ( id % WBITS ) );
}

#undef	MAX_RES
#undef	WBITS
#undef	ID_ELEMS

/*! print memory layout */
void k_memory_info ()
{
	int i;

	kprintf ( "Memory segments\n"
		 "===============\n"
		 "Type\tsize\t\tstart addres\tstring\n"
	);

	for ( i = 0; mseg[i].type != MS_END && i < 20; i++ )
	{
		kprintf ( "%d\t%x\t%x\t%s\n", mseg[i].type, mseg[i].size,
					      mseg[i].start, mseg[i].name );
	}
}

/*! Handle memory fault interrupt (and others undefined) */
void k_memory_fault ()
{
	LOG ( ERROR, "Undefined fault (exception)!!!");
	halt ();
}
