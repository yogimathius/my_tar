#include "my_tar.h"

void append_archive(const char *archive_name, char *files[], int file_count) {
    int archive_fd = open_archive(archive_name, O_RDWR, 0);

    if (lseek(archive_fd, -TAR_HEADER_SIZE * 2, SEEK_END) == -1) {
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

        char buffer[TAR_HEADER_SIZE];
        ssize_t bytesRead;
        while ((bytesRead = read(file_fd, buffer, TAR_HEADER_SIZE)) > 0) {
            if (write(archive_fd, buffer, bytesRead) != bytesRead) {
                error_msg(STDERR_FILENO, "Failed to write file contents to archive\n");
                close(file_fd);
                break;
            }
            if (bytesRead % TAR_HEADER_SIZE != 0) {
                ssize_t padding = TAR_HEADER_SIZE - (bytesRead % TAR_HEADER_SIZE);
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

    int two_blocks_size = TAR_HEADER_SIZE * 2;
    char end_block[two_blocks_size];
    my_memset(end_block, 0, two_blocks_size);
    write(archive_fd, end_block, two_blocks_size);

    close(archive_fd);
}