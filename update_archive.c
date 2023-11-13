#include "my_tar.h"

void update_archive(const char *archive_name, char *files[], int file_count) {
    char temp_archive_name[256] = "";
    my_strncpy(temp_archive_name, archive_name, sizeof(temp_archive_name) - 6); // -6 for space for ".temp"
    my_strncpy(temp_archive_name + my_strlen(temp_archive_name), ".temp", 6);

    int update_needed = 0;
    int processed[file_count];
    my_memset(processed, 0, sizeof(processed));

    int archive_fd = open_archive(archive_name, O_RDONLY, 0);

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

        archive_fd = open_archive(archive_name, O_RDONLY, 0);
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
        write(temp_archive_fd, end_block, TAR_HEADER_SIZE * 2);

        if (my_rename(temp_archive_name, archive_name) == -1) {
            error_msg(STDERR_FILENO, "Failed to rename temporary archive\n");
            exit(EXIT_FAILURE);
        }
    }
}