/* primary_header.h */
#ifndef MY_TAR_H
#define MY_TAR_H

#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define TAR_HEADER_SIZE 512
#define TAR_FILENAME_LEN 100
#define TAR_MODE_LEN 8
#define TAR_UID_LEN 8
#define TAR_GID_LEN 8
#define TAR_SIZE_LEN 12
#define TAR_MTIME_LEN 12
#define TAR_CHKSUM_LEN 8
#define TAR_LINK_INDICATOR_LEN 1
#define TAR_LINKNAME_LEN 100

typedef struct {
    char filename[TAR_FILENAME_LEN];
    char mode[TAR_MODE_LEN];
    char uid[TAR_UID_LEN];
    char gid[TAR_GID_LEN];
    char size[TAR_SIZE_LEN];
    char mtime[TAR_MTIME_LEN];
    char chksum[TAR_CHKSUM_LEN];
    char link[TAR_LINK_INDICATOR_LEN];
    char linkname[TAR_LINKNAME_LEN];
    char padding[TAR_HEADER_SIZE - TAR_FILENAME_LEN - TAR_MODE_LEN - TAR_UID_LEN - TAR_GID_LEN - TAR_SIZE_LEN - TAR_MTIME_LEN - TAR_CHKSUM_LEN - TAR_LINK_INDICATOR_LEN - TAR_LINKNAME_LEN];
} tar_header_t;

int open_archive(const char *archive_name, int flags, int set_mode);

size_t my_strlen(const char *str);

int my_strcmp(const char *s1, const char *s2);

void error_msg(int fd, const char *msg);

void my_strncpy(char *dest, const char *src, size_t n);

int my_rename(const char *oldpath, const char *newpath);

int contains_dir(char *pathname);

void my_memset(void *s, int c, size_t n);

void num_to_octal(unsigned long num, char *str, int size);

void my_puts(int fd, const char *msg);

void octal_to_long(const char *str, long *num) ;

void write_tar_header(int fd, const char *filename, const struct stat *st);

int read_tar_header(int fd, tar_header_t *header);

void copy_file_data(int source_fd, int dest_fd, long size);

void write_content_to_archive(int archive_fd, int file_fd);

void print_failed_stat(char *file);

void create_archive(const char *archive_name, char *files[], int file_count);

void append_archive(const char *archive_name, char *files[], int file_count);

void list_archive(const char *archive_name);

void update_archive(const char *archive_name, char *files[], int file_count);

void extract_archive(const char *archive_name);

#endif /* MY_TAR_H */
