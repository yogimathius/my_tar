#include "my_tar.h"

void create_archive(const char *archive_name, char *files[], int file_count) {
    int archive_fd = open_filepath(archive_name, O_WRONLY | O_CREAT | O_TRUNC, 1);

    write_to_archive(files, file_count, archive_fd);

    close(archive_fd);
}