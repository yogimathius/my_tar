#include "my_tar.h"

void append_archive(const char *archive_name, char *files[], int file_count) {
    int archive_fd = open_archive(archive_name, O_RDWR, 0);

    if (lseek(archive_fd, -TAR_HEADER_SIZE * 2, SEEK_END) == -1) {
        error_msg(STDERR_FILENO, "Failed to seek in archive\n");
        close(archive_fd);
        return;
    }

    struct stat file_stat;
    for (int i = 0; i < file_count; ++i) {
        if (stat(files[i], &file_stat) != 0) {
            char error_message[TAR_HEADER_SIZE] = "my_tar: ";
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

        write_content_to_archive(archive_fd, file_fd);

        close(file_fd);
    }

    int two_blocks_size = TAR_HEADER_SIZE * 2;
    char end_block[two_blocks_size];
    my_memset(end_block, 0, two_blocks_size);
    write(archive_fd, end_block, two_blocks_size);

    close(archive_fd);
}