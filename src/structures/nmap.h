/* =========================================================================
  Author: E.D Choparinov, Amsterdam
  Related Files: nmap.h nmap.c
  Created On: April 16 2024
  Purpose:
    Stand-alone open-address map provider. Include nmap.h to access the
    data structure.
========================================================================= */

#ifndef __HEADER_NMAP_H__
#define __HEADER_NMAP_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define NMAP_LOAD_FACTOR 0.75f
#define NMAP_OK 0
#define NMAP_FAIL -1

#define NMAP_MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct nmap_t         nmap_t;
typedef struct nmap_keypair_t nmap_keypair_t;

struct nmap_keypair_t {
  void *key;
  void *value;
};

nmap_t *nmap_make(size_t key_size, size_t value_size, size_t initial_map_size);
int     nmap_free(nmap_t *map);

void *nmap_find(nmap_t *map, void *key);
int   nmap_add(nmap_t *map, nmap_keypair_t *pair);
int   nmap_remove(nmap_t *map, void *key);

#endif