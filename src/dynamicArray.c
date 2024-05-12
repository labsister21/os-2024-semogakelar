// #include <stdio.h>
// #include <stdlib.h>

// #define INITIAL_CAPACITY 1

// typedef struct {
//     int *array;
//     int size;
//     int capacity;
// } DynamicArray;

// DynamicArray* createDynamicArray() {
//     DynamicArray *dynArray = (DynamicArray*)malloc(sizeof(DynamicArray));
//     dynArray->array = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
//     dynArray->size = 0;
//     dynArray->capacity = INITIAL_CAPACITY;
//     return dynArray;
// }

// void resize(DynamicArray *dynArray, int newCapacity) {
//     int *newArray = (int*)malloc(newCapacity * sizeof(int));
//     for (int i = 0; i < dynArray->size; i++) {
//         newArray[i] = dynArray->array[i];
//     }
//     free(dynArray->array);
//     dynArray->array = newArray;
//     dynArray->capacity = newCapacity;
// }

// void add(DynamicArray *dynArray, int element) {
//     if (dynArray->size == dynArray->capacity) {
//         resize(dynArray, 2 * dynArray->capacity);
//     }
//     dynArray->array[dynArray->size] = element;
//     dynArray->size++;
// }

// void removeElement(DynamicArray *dynArray, int element) {
//     int index = -1;
//     for (int i = 0; i < dynArray->size; i++) {
//         if (dynArray->array[i] == element) {
//             index = i;
//             break;
//         }
//     }
//     if (index != -1) {
//         for (int i = index; i < dynArray->size - 1; i++) {
//             dynArray->array[i] = dynArray->array[i + 1];
//         }
//         dynArray->size--;
//         if (dynArray->size <= dynArray->capacity / 4) {
//             resize(dynArray, dynArray->capacity / 2);
//         }
//     }
// }

// void printDynamicArray(DynamicArray *dynArray) {
//     printf("[");
//     for (int i = 0; i < dynArray->size; i++) {
//         printf("%d", dynArray->array[i]);
//         if (i < dynArray->size - 1) {
//             printf(", ");
//         }
//     }
//     printf("]\n");
// }

// void freeDynamicArray(DynamicArray *dynArray) {
//     free(dynArray->array);
//     free(dynArray);
// }

// void removeElementByIndex(DynamicArray *dynArray, int index) {
//     // Check if the index is valid
//     if (index < 0 || index >= dynArray->size) {
//         printf("Error: Invalid index.\n");
//         return;
//     }

//     // Shift elements after the removed index to the left
//     for (int i = index; i < dynArray->size - 1; i++) {
//         dynArray->array[i] = dynArray->array[i + 1];
//     }

//     // Decrease the size of the array
//     dynArray->size--;

//     // Check if resizing is necessary
//     if (dynArray->size <= dynArray->capacity / 4) {
//         resize(dynArray, dynArray->capacity / 2);
//     }
// }