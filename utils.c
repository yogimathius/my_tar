#include "my_tar.h"

int open_archive(const char *archive_name, int flags, int set_mode) {
    int archive_fd;
    if (set_mode == 1) {
        archive_fd = open(archive_name, flags, 0644);
    } else {
        archive_fd = open(archive_name, flags);
    }

    if (archive_fd == -1) {
        error_msg(STDERR_FILENO, "Failed to open archive\n");
        exit(EXIT_FAILURE);
    }
    return archive_fd;
}

size_t my_strlen(const char *str) {
    const char *s;
    for (s = str; *s; ++s) {}
    return (s - str);
}

int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void error_msg(int fd, const char *msg) { write(fd, msg, my_strlen(msg)); }

void my_strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
}

int my_rename(const char *oldpath, const char *newpath) {
    if (link(oldpath, newpath) == -1) return -1;
    if (unlink(oldpath) == -1) return -1;
    return 0;
}

int contains_dir(char *pathname) {
    for (int i = 0; i < my_strlen(pathname); i++) {
        if (pathname[i] == '/') {
            return i;
        }
    }
    return 0;
}

void my_memset(void *s, int c, size_t n) {
    unsigned char* p = s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
}

void num_to_octal(unsigned long num, char *str, int size) {
    char temp[32];
    int i = 30;

    temp[31] = '\0';

    do {
        temp[i--] = '0' + (num % 8);
        num /= 8;
    } while (num != 0 && i >= 0);

    while (i >= 31 - size) {
        temp[i--] = '0';
    }

    i++;
    int j = 0;
    while (temp[i]) {
        str[j++] = temp[i++];
    }
    str[j] = '\0';
}

void my_puts(int fd, const char *msg) {
    write(fd, msg, my_strlen(msg));
    write(fd, "\n", 1);
}

void octal_to_long(const char *str, long *num) {
    *num = 0;
    while (*str) {
        *num = (*num << 3) + (*str - '0');
        str++;
    }
}

void write_tar_header(int fd, const char *filename, const struct stat *st) {
    tar_header_t header;
    my_memset(&header, 0, sizeof(header));

    my_strncpy(header.filename, filename, TAR_FILENAME_LEN - 1);
    num_to_octal(st->st_mode & 0777, header.mode, TAR_MODE_LEN - 1);
    num_to_octal(st->st_uid, header.uid, TAR_UID_LEN - 1);
    num_to_octal(st->st_gid, header.gid, TAR_GID_LEN - 1);
    num_to_octal(st->st_size, header.size, TAR_SIZE_LEN - 1);
    num_to_octal(st->st_mtime, header.mtime, TAR_MTIME_LEN - 1);

    // Calculate and set the checksum
    my_memset(header.chksum, ' ', TAR_CHKSUM_LEN);
    unsigned int chksum = 0;
    for (int i = 0; i < TAR_HEADER_SIZE; ++i) {
        chksum += ((unsigned char*)&header)[i];
    }
    num_to_octal(chksum, header.chksum, TAR_CHKSUM_LEN - 1);

    write(fd, &header, sizeof(header));
}

int read_tar_header(int fd, tar_header_t *header) {
    ssize_t bytes_read = read(fd, header, sizeof(*header));
    if (bytes_read != sizeof(*header)) {
        return -1; // Error or end of archive
    }

    // Check for an empty block (end of archive)
    for (int i = 0; i < TAR_HEADER_SIZE; ++i) {
        if (((unsigned char*)header)[i] != 0) {
            return 0; // Not an empty block
        }
    }

    return 1; // Empty block found
}

void copy_file_data(int source_fd, int dest_fd, long size) {
    char buffer[512];
    long total_bytes_read = 0;

    while (total_bytes_read < size) {
        ssize_t bytes_read = read(source_fd, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            error_msg(STDOUT_FILENO, "Failed to read data for copying\n");
            exit(EXIT_FAILURE);
        }
        write(dest_fd, buffer, bytes_read);
        total_bytes_read += bytes_read;
    }

    long padding = (512 - (size % 512)) % 512;
    if (padding > 0) {
        char pad[512] = {0};
        write(dest_fd, pad, padding);
    }
}

void write_content_to_archive(int archive_fd, int file_fd) {
    char buffer[TAR_HEADER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, TAR_HEADER_SIZE)) > 0) {
        if (write(archive_fd, buffer, bytes_read) != bytes_read) {
            error_msg(STDERR_FILENO, "Failed to write file contents to archive\n");
            close(file_fd);
            break;
        }
        if (bytes_read % TAR_HEADER_SIZE != 0) {
            ssize_t padding = TAR_HEADER_SIZE - (bytes_read % TAR_HEADER_SIZE);
            char pad[padding];
            my_memset(pad, 0, padding);
            write(archive_fd, pad, padding);
        }
    }

    if (bytes_read == -1) {
        error_msg(STDERR_FILENO, "Failed to read file contents\n");
    }
}

void print_failed_stat(char *file) {
    char error_message[TAR_HEADER_SIZE] = "my_tar: ";
    my_strncpy(error_message + my_strlen(error_message), file, sizeof(error_message) - my_strlen(error_message) - 1);
    my_strncpy(error_message + my_strlen(error_message), ": Cannot stat: No such file or directory\n", sizeof(error_message) - my_strlen(error_message) - 1);
    error_msg(STDERR_FILENO, error_message);
}

void write_to_archive(char *files[], int file_count, int archive_fd) {
    struct stat file_stat;
    for (int i = 0; i < file_count; ++i) {
        char *file = files[i];
        if (stat(file, &file_stat) != 0) {
            print_failed_stat(file);
            continue;
        }

        write_tar_header(archive_fd, file, &file_stat);

        int file_fd = open(file, O_RDONLY);
        if (file_fd == -1) {
            error_msg(STDERR_FILENO, "Failed to open file\n");
            continue;
        }

        write_content_to_archive(archive_fd, file_fd);

        close(file_fd);
    }
}