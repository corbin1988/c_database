/*
 * C Database Project - Database Parsing and Validation Module
 * 
 * This module handles database file operations including:
 * - Database header creation and validation
 * - File output operations with proper byte ordering
 * - Employee record management
 * 
 * The database uses network byte order (big-endian) for cross-platform compatibility.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>    // For htonl, htons, ntohl, ntohs byte order functions
#include <sys/types.h>
#include <sys/stat.h>     // For file statistics (fstat)
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {
	// Validate inputs
	if (dbhdr == NULL || employees == NULL || addstring == NULL) {
		return STATUS_ERROR;
	}
	
	// Make a copy of the string since strtok modifies it
	char *addstring_copy = strdup(addstring);
	if (addstring_copy == NULL) {
		return STATUS_ERROR;
	}
	
	printf("%s\n", addstring);
	
	// Parse the employee data
	char *name = strtok(addstring_copy, ",");
	char *addr = strtok(NULL, ",");
	char *hours = strtok(NULL, ",");
	
	if (name == NULL || addr == NULL || hours == NULL) {
		free(addstring_copy);
		return STATUS_ERROR;
	}
	
	printf("%s %s %s\n", name, addr, hours);
	
	// Increment the employee count
	dbhdr->count++;
	
	// Reallocate memory for the new employee
	*employees = realloc(*employees, dbhdr->count * sizeof(struct employee_t));
	if (*employees == NULL) {
		printf("Failed to allocate memory for employee\n");
		free(addstring_copy);
		return STATUS_ERROR;
	}
	
	// Copy the parsed data to the new employee
	strncpy((*employees)[dbhdr->count-1].name, name, sizeof((*employees)[dbhdr->count-1].name) - 1);
	strncpy((*employees)[dbhdr->count-1].address, addr, sizeof((*employees)[dbhdr->count-1].address) - 1);
	(*employees)[dbhdr->count-1].name[sizeof((*employees)[dbhdr->count-1].name) - 1] = '\0';
	(*employees)[dbhdr->count-1].address[sizeof((*employees)[dbhdr->count-1].address) - 1] = '\0';
	(*employees)[dbhdr->count-1].hours = atoi(hours);
	
	free(addstring_copy);
	return STATUS_SUCCESS;
}

int parse_employee_string(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {
	// Just call add_employee since it now handles everything
	return add_employee(dbhdr, employees, addstring);
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
	if (fd < 0) {
		printf("Got a bad FD from the user\n");
		return STATUS_ERROR;
	}


	int count = dbhdr->count;

	struct employee_t *employees = calloc(count, sizeof(struct employee_t));
	if (employees == NULL) {
		printf("Malloc failed\n");
		return STATUS_ERROR;
	}

	read(fd, employees, count*sizeof(struct employee_t));

	int i = 0;
	for (; i < count; i++) {
		employees[i].hours = ntohl(employees[i].hours);
	}

	*employeesOut = employees;
	return STATUS_SUCCESS;

}

/**
 * output_file - Write database header and employee records to file
 * @fd: File descriptor for the database file
 * @dbhdr: Pointer to database header structure
 * @employees: Array of employee records to write (can be NULL for header-only writes)
 * 
 * Writes the database header and employee records to the file in network byte order
 * for cross-platform compatibility. Updates the header with correct file size.
 * 
 * Return: STATUS_SUCCESS on success, STATUS_ERROR on failure
 */
int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
	// Validate file descriptor
	if (fd < 0) {
		printf("Got a bad FD from the user\n");
		return STATUS_ERROR;
	}

	// Store original count before converting to network byte order
	int realcount = dbhdr->count;

	// Convert header fields to network byte order (big-endian)
	// This ensures the database file works across different architectures
	dbhdr->magic = htonl(dbhdr->magic);           // Convert 32-bit magic number
	dbhdr->filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));
	dbhdr->count = htons(dbhdr->count);           // Convert 16-bit employee count
	dbhdr->version = htons(dbhdr->version);       // Convert 16-bit version number

	// Seek to beginning of file to overwrite existing data
	lseek(fd, 0, SEEK_SET);

	// Write the database header to file
	write(fd, dbhdr, sizeof(struct dbheader_t));

	// Write employee records if they exist
	int i = 0;
	for (; i < realcount; i++) {
		// Convert employee hours to network byte order before writing
		employees[i].hours = htonl(employees[i].hours);
		write(fd, &employees[i], sizeof(struct employee_t));
	}

	return STATUS_SUCCESS;

}	

/**
 * validate_db_header - Read and validate database header from existing file
 * @fd: File descriptor for the database file
 * @headerOut: Pointer to store the validated header structure
 * 
 * Reads the database header from an existing file, converts from network byte order,
 * and validates the magic number, version, and file integrity.
 * 
 * Return: STATUS_SUCCESS on success, STATUS_ERROR on failure
 */
int validate_db_header(int fd, struct dbheader_t **headerOut) {
	// Validate input parameters
	if (fd < 0) {
		printf("Got a bad FD from the user\n");
		return STATUS_ERROR;
	}
	
	if (headerOut == NULL) {
		printf("Invalid headerOut parameter\n");
		return STATUS_ERROR;
	}

	// Allocate memory for header structure
	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Malloc failed create a db header\n");
		return STATUS_ERROR;
	}

	// Read header from file - must read exactly the header size
	if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
		perror("read");
		free(header);
		return STATUS_ERROR;
	}

	// Convert header fields from network byte order to host byte order
	header->version = ntohs(header->version);     // Convert 16-bit version
	header->count = ntohs(header->count);         // Convert 16-bit employee count
	header->magic = ntohl(header->magic);         // Convert 32-bit magic number
	header->filesize = ntohl(header->filesize);   // Convert 32-bit file size

	// Validate magic number - ensures this is a valid database file
	if (header->magic != HEADER_MAGIC) {
		printf("Impromper header magic\n");
		free(header);
		return -1;
	}

	// Validate version - ensures compatibility with current code
	if (header->version != 1) {
		printf("Impromper header version\n");
		free(header);
		return -1;
	}

	// Validate file size integrity - ensures file hasn't been corrupted
	struct stat dbstat = {0};
	fstat(fd, &dbstat);  // Get actual file size from filesystem
	if (header->filesize != dbstat.st_size) {
		printf("Corrupted database\n");
		free(header);
		return -1;
	}

	*headerOut = header;
    return STATUS_SUCCESS;
}

/**
 * create_db_header - Initialize a new database header structure
 * @headerOut: Pointer to store the newly created header structure
 * 
 * Creates and initializes a new database header with default values:
 * - Version 1 (current database format version)
 * - Count 0 (no employee records initially)
 * - Standard magic number for file identification
 * - File size set to header size only
 * 
 * Return: STATUS_SUCCESS on success, STATUS_ERROR on failure
 */
int create_db_header(struct dbheader_t **headerOut) {
	// Validate input parameter
	if (headerOut == NULL) {
		printf("Invalid headerOut parameter\n");
		return STATUS_ERROR;
	}

	// Allocate and zero-initialize memory for new header
	struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
	if (header == NULL) {
		printf("Malloc failed to create db header\n");
		return STATUS_ERROR;
	}

	// Initialize header fields with default values
	header->version = 0x1;                    // Database format version 1
	header->count = 0;                        // No employee records initially
	header->magic = HEADER_MAGIC;             // Magic number for file identification
	header->filesize = sizeof(struct dbheader_t);  // File contains only header initially

	// Return the created header to caller
	*headerOut = header;

	return STATUS_SUCCESS;
}