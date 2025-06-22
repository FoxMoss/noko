#include <stddef.h>
#include <stdlib.h>

// https://stackoverflow.com/questions/5256313/c-c-macro-string-concatenation
#define PPCAT_NX(A, B) A##B
#define PPCAT(A, B) PPCAT_NX(A, B)

#ifndef KEY_TYPE
#define KEY_TYPE size_t
#endif
#ifndef VALUE_TYPE
#define VALUE_TYPE size_t
#endif
#ifndef ID
#define ID SizeSize
#endif
#define Pair PPCAT(ID, Pair)
#define Map PPCAT(ID, Map)
#define create_map PPCAT(ID, _create_map)
#define appened PPCAT(ID, appened)

struct Pair {
  KEY_TYPE key;
  VALUE_TYPE pair;
};

struct Map {
  struct Pair *map;
  size_t len;
};

struct Map create_map() {
  struct Map ret;
  ret.map = malloc(0);
  ret.len = 0;
  return ret;
}
void appened(struct Map *map) {
  map->len++;
  map->map = realloc(map->map, map->len * sizeof(struct Pair));
}
