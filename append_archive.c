#include "my_tar.h"

void append_archive(const char *archive_name, char *files[], int file_count) {
    int archive_fd = open_filepath(archive_name, O_RDWR, 0);

    if (lseek(archive_fd, -TAR_HEADER_SIZE * 2, SEEK_END) == -1) {
        error_msg(STDERR_FILENO, "Failed to seek in archive\n");
        close(archive_fd);
        return;
    }

    write_to_archive(files, file_count, archive_fd);

    close(archive_fd);
}