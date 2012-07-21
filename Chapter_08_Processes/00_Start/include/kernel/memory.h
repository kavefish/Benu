/*! Memory management */
#pragma once

/*! interface to threads */
int sys__sysinfo ( char *buffer, size_t buf_size, char **param )

#ifdef _KERNEL_ /* (for kernel and arch layer) */

#include <types/basic.h>

extern inline void *k_mem_init ( void *segment, size_t size );
extern inline void *kmalloc ( size_t size );
extern inline int kfree ( void *chunk );

struct _kobject_t_; typedef struct _kobject_t_ kobject_t;

#endif /* _KERNEL_ */