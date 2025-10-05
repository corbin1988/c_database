/*
 * C Database Project - Main Entry Point
 * 
 * This program provides a simple database interface that can create
 * and work with database files through command line arguments.
 */

#include <getopt.h>    // For command line argument parsing
#include <stdio.h>     // For standard I/O operations
#include <stdbool.h>   // For boolean data type support

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
    
    printf("Hello world");    // Debug output (TODO: Remove in production)
    
    // Parse command line arguments using getopt
    // Format: "nf:" means 'n' takes no argument, 'f' requires an argument
    // Loop continues until all arguments are processed (getopt returns -1)
    while ((c = getopt(argc, argv, "nf:")) != -1) {
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

    // Display parsed values for debugging/verification
    // TODO: Replace with actual database operations
    printf("New file flag: %d\n", newFile);
    printf("Database filepath: %s\n", filepath);

    // Program completed successfully
    return 0;
}