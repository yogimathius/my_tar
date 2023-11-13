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
