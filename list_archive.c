#include "my_tar.h"

void list_archive(const char *archive_name) {
    int archive_fd = open_filepath(archive_name, O_RDONLY, 0);

    tar_header_t header;
    while (read_tar_header(archive_fd, &header) == 0) {
        my_puts(STDOUT_FILENO, header.filename);

        long file_size;
        octal_to_long(header.size, &file_size); 
        lseek(archive_fd, (file_size + 511) & ~511, SEEK_CUR); // Skip to the next header
    }

    close(archive_fd);
}