#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/parse.h"
#include "include/common.h"

int main() {
    printf("=== Test Framework Mimic ===\n");
    
    // Mimic exactly what the test framework does
    struct dbheader_t *dbhdr = NULL;
    struct employee_t *employees = NULL;
    
    // Create database header
    printf("Creating database header...\n");
    if (create_db_header(&dbhdr) != STATUS_SUCCESS) {
        printf("FAIL: Could not create database header\n");
        return -1;
    }
    printf("SUCCESS: Database header created, count = %d\n", dbhdr->count);
    
    // Test data (like what the test framework uses)
    const char *test_employee = "John Doe,123 Main St,40";
    
    printf("About to call add_employee with:\n");
    printf("  dbhdr = %p (count = %d)\n", (void*)dbhdr, dbhdr->count);
    printf("  employees = %p\n", (void*)employees);
    printf("  test_employee = \"%s\"\n", test_employee);
    
    // This is exactly how the test framework calls it
    printf("Calling add_employee...\n");
    int add_result = add_employee(dbhdr, &employees, (char*)test_employee);
    
    printf("add_employee returned: %d\n", add_result);
    
    if (add_result == STATUS_SUCCESS) {
        printf("SUCCESS: Employee added\n");
        printf("New count: %d\n", dbhdr->count);
        
        if (employees != NULL && dbhdr->count > 0) {
            printf("Employee data:\n");
            printf("  Name: %s\n", employees[0].name);
            printf("  Address: %s\n", employees[0].address);
            printf("  Hours: %d\n", employees[0].hours);
        }
    } else {
        printf("FAIL: add_employee returned error\n");
    }
    
    // Cleanup
    free(employees);
    free(dbhdr);
    
    printf("Test completed successfully\n");
    return 0;
}