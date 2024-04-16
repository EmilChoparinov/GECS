#include <criterion/criterion.h>
Test(Hello,World)
{
}

// #include <criterion/criterion.h>

// Test(misc, failing) {
//     cr_assert(0);
// }

// Test(misc, passing) {
//     cr_assert(1);
// }

// Test(nmap_tests, create) {
//   nmap_t *map = nmap_make(sizeof(int), sizeof(int), 1);
//   cr_assert(map != NULL, "Expect map to not be null");

//   nmap_free(map);
// }

// Test(nmap_tests, add_no_resize) {
//   nmap_t *map = nmap_make(sizeof(int), sizeof(int), 64);

//   for (int i = 0; i < 64 * NMAP_LOAD_FACTOR; i++) {
//     int stored_value = i + 1;
//     nmap_add(map, &(nmap_keypair_t){.key = &i, .value = &stored_value});
//   }

//   for (int i = 0; i < 64 * NMAP_LOAD_FACTOR; i++) {
//     int *value = nmap_find(map, &i);
//     cr_assert(value != NULL,
//               "Find returned a null value when it expected a find success");
//     cr_assert(
//         *value == i + 1,
//         "nmap find did not return the expected value. Result %d, Expected %d",
//         *value, i + 1);
//   }

//   nmap_free(map);
// }

// Test(nmap_tests, remove_no_resize) {
//   nmap_t *map = nmap_make(sizeof(int), sizeof(int), 64);

//   for (int i = 0; i < 64 * NMAP_LOAD_FACTOR; i++) {
//     int stored_value = i + 1;
//     nmap_add(map, &(nmap_keypair_t){.key = &i, .value = &stored_value});
//   }

//   for (int i = 0; i < 64 * NMAP_LOAD_FACTOR; i++) {
//     int success = nmap_remove(map, &i);
//     cr_assert(success == NMAP_OK, "nmap failed at key removal");
//     int *value = nmap_find(map, &i);
//     cr_assert(value == NULL, "Key was not actually removed. Value of key: %d",
//               *value);
//   }

//   nmap_free(map);
// }