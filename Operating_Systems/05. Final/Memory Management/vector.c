#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4

typedef struct {
    void* data;
    int size;
    int capacity;
    int elemSize;
} Vector;

int vector_init(Vector* vec, int elemSize) {
    vec->elemSize = elemSize;
    vec->data = malloc(elemSize * INITIAL_CAPACITY);
    if (vec->data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }
    vec->size = 0;
    vec->capacity = INITIAL_CAPACITY;
    return 0;
}

void vector_destroy(Vector* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
    vec->elemSize = 0;
}

int vector_push_back(Vector* vec, void* value) {
    // Check if resizing is needed
    if (vec->size == vec->capacity) {
        vec->capacity *= 2;
        vec->data = (void **)realloc(vec->data, vec->elemSize * vec->capacity);
        if (vec->data == NULL) {
            fprintf(stderr, "Memory reallocation failed\n");
            return -1;
        }
    }
    // Add the new element to the end of the vector
    memcpy((char*)vec->data + vec->size * vec->elemSize, value, vec->elemSize);
    vec->size++;
    return 0;
}

int vector_pop(Vector* vec, void* out) {
    if (vec->size == 0) {
        fprintf(stderr, "Vector is empty\n");
        return -1; 
    }
    vec->size--;
    memcpy(out, (char*)vec->data + vec->size * vec->elemSize, vec->elemSize);
    // Resize the memory block if necessary
    if (vec->size < vec->capacity / 2 && vec->capacity > INITIAL_CAPACITY) {
        vec->capacity /= 2;
        vec->data = realloc(vec->data, vec->elemSize * vec->capacity);
        if (vec->data == NULL) {
            fprintf(stderr, "Memory reallocation failed\n");
            return -1;
        }
    }
    return 0; 
}

int vector_remove(Vector* vec, int index) {
    if (index < 0 || index >= vec->size) {
        fprintf(stderr, "Invalid index\n");
        return -1; 
    }
    // Move elements after the specified index to fill the gap
    memmove((char*)vec->data + index * vec->elemSize, 
            (char*)vec->data + (index + 1) * vec->elemSize, 
            (vec->size - index - 1) * vec->elemSize);
    vec->size--;
    // Resize the memory block if necessary
    if (vec->size < vec->capacity / 2 && vec->capacity > INITIAL_CAPACITY) {
        vec->capacity /= 2;
        vec->data = realloc(vec->data, vec->elemSize * vec->capacity);
        if (vec->data == NULL) {
            fprintf(stderr, "Memory reallocation failed\n");
            return -1;
        }
    }
    return 0; 
}
