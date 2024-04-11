#include "small_vector.h"

struct small_vector_t {
    size_t len;
    size_t el_size;
    size_t capacity;
    void *data;
};

// static void reserve(small_vector *s);

small_vector_t *stack_make(size_t el_size) {
    small_vector_t *s;
    if((s = malloc(sizeof(*s))) == NULL) return NULL;
    if((s->data = malloc(el_size * SV_START_SIZE)) == NULL) return NULL;

    s->len = 0;
    s->el_size = el_size;
    s->capacity = SV_START_SIZE;

    return s;
}

void stack_destroy(small_vector_t *s) {
    free(s->data);
    free(s);
}

void stack_push(small_vector_t *s, void *el);
void *stack_pop(small_vector_t *s);

// static void reserve(small_vector *s) {
//     if(s->len == s->capacity) {
        
//     }
// }