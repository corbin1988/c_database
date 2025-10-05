#include <stdio.h>
#include <stdlib.h>
#include "include/parse.h"
#include "include/common.h"

int main() {
    printf("=== Testing list_employees with NULL inputs ===\n");
    
    // Test 1: NULL header (this should not crash)
    printf("Test 1: Calling list_employees with NULL header...\n");
    list_employees(NULL, NULL);
    printf("Test 1 passed - no crash!\n\n");
    
    // Test 2: Valid header with 0 count
    printf("Test 2: Calling list_employees with empty database...\n");
    struct dbheader_t *dbhdr = NULL;
    if (create_db_header(&dbhdr) == STATUS_SUCCESS) {
        list_employees(dbhdr, NULL);
        printf("Test 2 passed!\n");
        free(dbhdr);
    }
    
    printf("All tests completed successfully!\n");
    return 0;
}