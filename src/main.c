/*
 * C Database Project - Main Entry Point
 * 
 * This program provides a simple database interface that can create
 * and work with database files through command line arguments.
 */

#include <getopt.h>    // For command line argument parsing
#include <stdio.h>     // For standard I/O operations
#include <stdbool.h>   // For boolean data type support
#include <stdlib.h>    // For memory allocation and process control

#include "file.h"
#include "parse.h"    // For database file operations
#include <common.h>

/**
 * print_usage - Display program usage information
 * @argv: Array of command line arguments (argv[0] is program name)
 * 
 * Prints helpful usage information showing how to properly invoke
 * the program with its required and optional arguments.
 */
void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database filepath>\n", argv[0]);
    printf("\t -n - create new database file\n");  // Fixed typo: "creatte" -> "create"
    printf("\t -f - (required) path to database file\n");
    return;
}

/**
 * main - Program entry point
 * @argc: Number of command line arguments
 * @argv: Array of command line argument strings
 * 
 * Parses command line arguments to determine the database file path
 * and whether to create a new database file. Validates required
 * arguments and provides user feedback.
 * 
 * Return: 0 on success, -1 on error
 */
int main(int argc, char *argv[]) {
    // Initialize variables to store parsed command line options
    char *filepath = NULL;    // Path to the database file
    bool newFile = false;     // Flag indicating whether to create new file
    int c;                    // Variable to store current option character
    char *addstring = NULL;  // String for adding employee
    
    // Database file operations variables
    int dbfd = -1;                    // File descriptor for database file
    struct dbheader_t *dbhr = NULL;   // Pointer to database header structure
    struct employee_t *employees = NULL; // Pointer to employee records array
    
    // Parse command line arguments using getopt
    // Format: "nf:" means 'n' takes no argument, 'f' requires an argument
    // Loop continues until all arguments are processed (getopt returns -1)
    while ((c = getopt(argc, argv, "nf:a:")) != -1) {
        switch (c) {
        case 'n':
            // User specified -n flag to create new database file
            newFile = true;
            break;
            
        case 'f':
            // User specified -f flag with filepath argument
            // optarg contains the argument value provided after -f
            filepath = optarg;
            break;
        case 'a':
            addstring = optarg;
            break;
        case '?': 
            // getopt encountered an unknown option or missing argument
            printf("Unknown option -%c\n", optopt);
            break;
            
        default:
            // Unexpected return value from getopt (should not happen)
            return -1;
        }
    }

    // Validate that required filepath argument was provided
    if(filepath == NULL) {
        printf("Filepath is required argument\n");
        print_usage(argv);  // Show usage information to help user
        return 0;
    }

    // Handle database file operations based on user request
    if(newFile) {
        // Create a new database file
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create database file \n");
            return -1;
        }

        // Initialize the database header for the new file
        if(create_db_header(&dbhr) == STATUS_ERROR) {
            printf("Unable to create database header \n");
            return -1;
        }

    } else {
        // Open existing database file
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to open database file \n");
            return -1;  // Make sure this actually exits
        }

        // Read and validate the existing database header
        if(validate_db_header(dbfd, &dbhr) == STATUS_ERROR) {
            printf("Failed to validate database header\n");
            return -1;  // Make sure this actually exits
        }
    }

    // Display parsed values for debugging/verification
    printf("New file flag: %d\n", newFile);
    printf("Database filepath: %s\n", filepath);

    if(read_employees(dbfd, dbhr, &employees) == STATUS_ERROR) {
        printf("Failed to read employees from database\n");
        return -1;
    }

    if(addstring != NULL) {
        if(add_employee(dbhr, &employees, addstring) == STATUS_ERROR) {
            printf("Failed to add employee\n");
            return -1;
        }
    }

    // Write the database header and employee records to file
    // This ensures the file is properly formatted and can be read later
    output_file(dbfd, dbhr, employees);

    // Program completed successfully
    return 0;
}