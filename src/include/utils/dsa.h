/*-------------------------------------------------------------------------
 *
 * dsa.h
 *	  Dynamic shared memory areas.
 *
 * Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/include/utils/dsa.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef DSA_H
#define DSA_H

#include "postgres.h"

#include "port/atomics.h"
#include "storage/dsm.h"

/* The opaque type used for an area. */
struct dsa_area;
typedef struct dsa_area dsa_area;

/*
 * If this system doesn't support atomic operations on 64 bit values then
 * we fall back to 32 bit dsa_pointer.  For testing purposes,
 * USE_SMALL_DSA_POINTER can be defined to force the use of 32 bit
 * dsa_pointer even on systems that support 64 bit atomics.
 */
#ifndef PG_HAVE_ATOMIC_U64_SUPPORT
#define SIZEOF_DSA_POINTER 4
#else
#ifdef USE_SMALL_DSA_POINTER
#define SIZEOF_DSA_POINTER 4
#else
#define SIZEOF_DSA_POINTER 8
#endif
#endif

/*
 * The type of 'relative pointers' to memory allocated by a dynamic shared
 * area.  dsa_pointer values can be shared with other processes, but must be
 * converted to backend-local pointers before they can be dereferenced.  See
 * dsa_get_address.  Also, an atomic version and appropriately sized atomic
 * operations.
 */
#if SIZEOF_DSA_POINTER == 4
typedef uint32 dsa_pointer;
typedef pg_atomic_uint32 dsa_pointer_atomic;
#define dsa_pointer_atomic_init pg_atomic_init_u32
#define dsa_pointer_atomic_read pg_atomic_read_u32
#define dsa_pointer_atomic_write pg_atomic_write_u32
#define dsa_pointer_atomic_fetch_add pg_atomic_fetch_add_u32
#define dsa_pointer_atomic_compare_exchange pg_atomic_compare_exchange_u32
#else
typedef uint64 dsa_pointer;
typedef pg_atomic_uint64 dsa_pointer_atomic;
#define dsa_pointer_atomic_init pg_atomic_init_u64
#define dsa_pointer_atomic_read pg_atomic_read_u64
#define dsa_pointer_atomic_write pg_atomic_write_u64
#define dsa_pointer_atomic_fetch_add pg_atomic_fetch_add_u64
#define dsa_pointer_atomic_compare_exchange pg_atomic_compare_exchange_u64
#endif

/* A sentinel value for dsa_pointer used to indicate failure to allocate. */
#define InvalidDsaPointer ((dsa_pointer) 0)

/* Check if a dsa_pointer value is valid. */
#define DsaPointerIsValid(x) ((x) != InvalidDsaPointer)

/*
 * The type used for dsa_area handles.  dsa_handle values can be shared with
 * other processes, so that they can attach to them.  This provides a way to
 * share allocated storage with other processes.
 *
 * The handle for a dsa_area is currently implemented as the dsm_handle
 * for the first DSM segment backing this dynamic storage area, but client
 * code shouldn't assume that is true.
 */
typedef dsm_handle dsa_handle;

extern void dsa_startup(void);

extern dsa_area *dsa_create(int tranche_id, const char *tranche_name);
extern dsa_area *dsa_create_in_place(void *place, Size size,
					int tranche_id, const char *tranche_name,
					dsm_segment *segment);
extern dsa_area *dsa_attach(dsa_handle handle);
extern dsa_area *dsa_attach_in_place(void *place, dsm_segment *segment);
extern void dsa_release_in_place(void *place);
extern void dsa_on_dsm_detach_release_in_place(dsm_segment *, Datum);
extern void dsa_on_shmem_exit_release_in_place(int, Datum);
extern void dsa_pin_mapping(dsa_area *area);
extern void dsa_detach(dsa_area *area);
extern void dsa_pin(dsa_area *area);
extern void dsa_unpin(dsa_area *area);
extern void dsa_set_size_limit(dsa_area *area, Size limit);
extern Size dsa_minimum_size(void);
extern dsa_handle dsa_get_handle(dsa_area *area);
extern dsa_pointer dsa_allocate(dsa_area *area, Size size);
extern void dsa_free(dsa_area *area, dsa_pointer dp);
extern void *dsa_get_address(dsa_area *area, dsa_pointer dp);
extern void dsa_trim(dsa_area *area);
extern void dsa_dump(dsa_area *area);

#endif   /* DSA_H */
