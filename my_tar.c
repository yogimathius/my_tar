#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>


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
#define TAR_BLOCK_SIZE 512

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

void create_archive(const char *archive_name, char *files[], int file_count) {
    int archive_fd = open(archive_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (archive_fd == -1) {
        error_msg(STDERR_FILENO, "Failed to create archive\n");
        exit(EXIT_FAILURE);
    }

    struct stat file_stat;
    for (int i = 0; i < file_count; i++) {
        if (stat(files[i], &file_stat) != 0) {
            char error_message[512] = "my_tar: ";
            my_strncpy(error_message + my_strlen(error_message), files[i], sizeof(error_message) - my_strlen(error_message) - 1);
            my_strncpy(error_message + my_strlen(error_message), ": Cannot stat: No such file or directory\n", sizeof(error_message) - my_strlen(error_message) - 1);
            error_msg(STDERR_FILENO, error_message);
            continue;
        }

        write_tar_header(archive_fd, files[i], &file_stat);

        int file_fd = open(files[i], O_RDONLY);
        if (file_fd == -1) {
            error_msg(STDERR_FILENO, "Failed to open file\n");
            continue;
        }

        char buffer[512];
        ssize_t bytes_read;
        while ((bytes_read = read(file_fd, buffer, 512)) > 0) {
            write(archive_fd, buffer, bytes_read);
            if (bytes_read % 512 != 0) {
                ssize_t padding = 512 - (bytes_read % 512);
                char pad[padding];
                my_memset(pad, 0, padding);
                write(archive_fd, pad, padding);
            }
        }

        close(file_fd);
    }

    // Write two empty blocks at the end of the archive
    int two_blocks_size = TAR_BLOCK_SIZE * 2;
    char end_block[two_blocks_size];
    my_memset(end_block, 0, two_blocks_size);
    write(archive_fd, end_block, two_blocks_size);

    close(archive_fd);
}

void append_archive(const char *filename, char *files[], int file_count) {
    int archive_fd = open(filename, O_RDWR);
    if (archive_fd == -1) {
        error_msg(STDERR_FILENO, "Failed to open the archive for appending\n");
        return;
    }

    if (lseek(archive_fd, -TAR_BLOCK_SIZE * 2, SEEK_END) == -1) {
        error_msg(STDERR_FILENO, "Failed to seek in archive\n");
        close(archive_fd);
        return;
    }

    for (int i = 0; i < file_count; ++i) {
        struct stat file_stat;
        if (stat(files[i], &file_stat) != 0) {
            char error_msg_buff[256] = "Cannot stat file ";
            my_strncpy(error_msg_buff + my_strlen(error_msg_buff), files[i], sizeof(error_msg_buff) - my_strlen(error_msg_buff) - 1);
            my_strncpy(error_msg_buff + my_strlen(error_msg_buff), "\n", sizeof(error_msg_buff) - my_strlen(error_msg_buff) - 1);
            error_msg(STDERR_FILENO, error_msg_buff);
            continue;
        }

        write_tar_header(archive_fd, files[i], &file_stat);

        int file_fd = open(files[i], O_RDONLY);
        if (file_fd == -1) {
            char error_msg_buff[256] = "Cannot open file ";
            my_strncpy(error_msg_buff + my_strlen(error_msg_buff), files[i], sizeof(error_msg_buff) - my_strlen(error_msg_buff) - 1);
            my_strncpy(error_msg_buff + my_strlen(error_msg_buff), " for reading\n", sizeof(error_msg_buff) - my_strlen(error_msg_buff) - 1);
            error_msg(STDERR_FILENO, error_msg_buff);
            continue;
        }

        char buffer[TAR_BLOCK_SIZE];
        ssize_t bytesRead;
        while ((bytesRead = read(file_fd, buffer, TAR_BLOCK_SIZE)) > 0) {
            if (write(archive_fd, buffer, bytesRead) != bytesRead) {
                error_msg(STDERR_FILENO, "Failed to write file contents to archive\n");
                close(file_fd);
                break;
            }
            if (bytesRead % TAR_BLOCK_SIZE != 0) {
                ssize_t padding = TAR_BLOCK_SIZE - (bytesRead % TAR_BLOCK_SIZE);
                char pad[padding];
                my_memset(pad, 0, padding);
                write(archive_fd, pad, padding);
            }
        }

        if (bytesRead == -1) {
            error_msg(STDERR_FILENO, "Failed to read file contents\n");
        }

        close(file_fd);
    }
    
    int two_blocks_size = TAR_BLOCK_SIZE * 2;
    char end_block[two_blocks_size];
    my_memset(end_block, 0, two_blocks_size);
    write(archive_fd, end_block, two_blocks_size);

    close(archive_fd);
}


void list_archive(const char *archive_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1) {
        error_msg(STDERR_FILENO, "Failed to open archive\n");
        exit(EXIT_FAILURE);
    }

    tar_header_t header;
    while (read_tar_header(archive_fd, &header) == 0) {
        my_puts(STDOUT_FILENO, header.filename);

        long file_size;
        octal_to_long(header.size, &file_size); 
        lseek(archive_fd, (file_size + 511) & ~511, SEEK_CUR); // Skip to the next header
    }

    close(archive_fd);
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

void update_archive(const char *archive_name, char *files[], int file_count) {
    char temp_archive_name[256] = "";
    my_strncpy(temp_archive_name, archive_name, sizeof(temp_archive_name) - 6); // -6 for space for ".temp"
    my_strncpy(temp_archive_name + my_strlen(temp_archive_name), ".temp", 6);

    int update_needed = 0;
    int processed[file_count];
    my_memset(processed, 0, sizeof(processed));

    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1) {
        error_msg(STDERR_FILENO, "Failed to open existing archive\n");
        exit(EXIT_FAILURE);
    }

    tar_header_t header;
    while (read_tar_header(archive_fd, &header) > 0) {
        long file_size;
        octal_to_long(header.size, &file_size); 
        long padded_size = (file_size + 511) & ~511;

        for (int i = 0; i < file_count; ++i) {
            if (my_strcmp(header.filename, files[i]) == 0) {
                struct stat file_stat;
                if (stat(files[i], &file_stat) == 0) {
                    long header_mtime;
                    octal_to_long(header.mtime, &header_mtime);
                    if (file_stat.st_mtime > header_mtime) {
                        update_needed = 1;
                    }
                    processed[i] = 1;
                }
                break;
            }
        }

        lseek(archive_fd, padded_size, SEEK_CUR);
    }

    close(archive_fd);

    if (update_needed) {
        int temp_archive_fd = open(temp_archive_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (temp_archive_fd == -1) {
            error_msg(STDERR_FILENO, "Failed to create temporary archive\n");
            exit(EXIT_FAILURE);
        }

        archive_fd = open(archive_name, O_RDONLY);
        while (read_tar_header(archive_fd, &header) > 0) {
            long file_size;
            octal_to_long(header.size, &file_size);
            long padded_size = (file_size + 511) & ~511;

            int should_update = 0;
            for (int i = 0; i < file_count; ++i) {
                if (my_strcmp(header.filename, files[i]) == 0 && processed[i]) {
                    should_update = 1;
                    break;
                }
            }

            if (should_update) {
                struct stat file_stat;
                if (stat(header.filename, &file_stat) == 0) {
                    write_tar_header(temp_archive_fd, header.filename, &file_stat);
                    int file_fd = open(header.filename, O_RDONLY);
                    copy_file_data(file_fd, temp_archive_fd, file_stat.st_size);
                    close(file_fd);
                }
            } else {
                write(temp_archive_fd, &header, sizeof(header));
                copy_file_data(archive_fd, temp_archive_fd, file_size);
            }

            lseek(archive_fd, padded_size, SEEK_CUR);
        }

        close(archive_fd);
        close(temp_archive_fd);
        
        char end_block[1024] = {0};
        write(temp_archive_fd, end_block, TAR_BLOCK_SIZE * 2);

        if (my_rename(temp_archive_name, archive_name) == -1) {
            error_msg(STDERR_FILENO, "Failed to rename temporary archive\n");
            exit(EXIT_FAILURE);
        }
    }
}

void extract_archive(const char *archive_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1) {
        error_msg(STDERR_FILENO, "Failed to open archive for extraction\n");
        exit(EXIT_FAILURE);
    }

    tar_header_t header;
    while (read_tar_header(archive_fd, &header) == 0) {
        long file_size;
        octal_to_long(header.size, &file_size);

        int file_fd = open(header.filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file_fd == -1) {
            error_msg(STDERR_FILENO, "Failed to create file for extraction\n");
            close(archive_fd);
            exit(EXIT_FAILURE);
        }

        char buffer[512];
        while (file_size > 0) {
            ssize_t bytes_read = read(archive_fd, buffer, sizeof(buffer));
            if (bytes_read <= 0) {
                error_msg(STDERR_FILENO, "Failed to read data from archive\n");
                close(file_fd);
                close(archive_fd);
                exit(EXIT_FAILURE);
            }

            ssize_t write_size = (file_size < bytes_read) ? file_size : bytes_read;
            if (write(file_fd, buffer, write_size) != write_size) {
                error_msg(STDERR_FILENO, "Failed to write data to file\n");
                close(file_fd);
                close(archive_fd);
                exit(EXIT_FAILURE);
            }

            file_size -= write_size;
        }

        close(file_fd);

        // Align to the next 512-byte boundary, if necessary
        long padding = (512 - (file_size % 512)) % 512;
        if (padding > 0) {
            lseek(archive_fd, padding, SEEK_CUR);
        }
    }

    close(archive_fd);
}

int main(int argc, char **argv) {

    const char *mode = argv[1];
    const char *filename = argv[2];
    if (my_strcmp(mode, "-cf") == 0) {
        create_archive(filename, &argv[3], argc - 3);
    } else if (my_strcmp(mode, "-rf") == 0) {
        append_archive(filename, &argv[3], argc - 3);
    } else if (my_strcmp(mode, "-tf") == 0) {
        list_archive(filename);
    } else if (my_strcmp(mode, "-uf") == 0) {
        update_archive(filename, &argv[3], argc - 3);
    } else if (my_strcmp(mode, "-xf") == 0) {
        extract_archive(filename);
    } else {
        error_msg(STDERR_FILENO, "Invalid mode: ");
        error_msg(STDERR_FILENO, mode);
        error_msg(STDERR_FILENO, "\n");
        return 1;
    }

    return 0;
}

/*
Can you run with ./my_tar -cf file.tar source.c source.h?
Can inspect the content of the tarball with  ./my_tar -tf file.tar
Can you run with  ./my_tar -rf file.tar new_source.c
Can you extract the tarball with ./my_tar -xf file.tar
Does the command ./my_tar -cf file.tar source.h i_don_t_exist handles file that
doesn't exist?
Can you use my_tar to untar an archive created by the real tar?
*/

/*
-c Create a new archive containing the specified items.
-r Like -c, but new entries are appended to the archive. The -f option is
required. -t List archive contents to stdout. -u Like -r, but new entries are
added only if they have a modification date newer than the corresponding entry
in the archive. The -f option is required. -x Extract to disk from the archive.
If a file with the same name appears more than once in the archive, each copy
will be extracted, with later copies overwriting (replacing) earlier copies.
*/
