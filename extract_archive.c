#include "my_tar.h"

void extract_archive(const char *archive_name) {
    int archive_fd = open_archive(archive_name, O_RDONLY, 0);

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

        char buffer[TAR_HEADER_SIZE];
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

        // Align to the next TAR_HEADER_SIZE-byte boundary, if necessary
        long padding = (TAR_HEADER_SIZE - (file_size % TAR_HEADER_SIZE)) % TAR_HEADER_SIZE;
        if (padding > 0) lseek(archive_fd, padding, SEEK_CUR);
    }

    close(archive_fd);
}