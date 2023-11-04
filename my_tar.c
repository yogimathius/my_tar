#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  char filename[256];
  char size[12];
  time_t mod_time;
} FileEntry;

#define TAR_BLOCK_SIZE 512

void create_archive(const char *filename, char *files[], int file_count) {
  int archive_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (archive_fd == -1) {
    perror("Cannot create archive");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < file_count; ++i) {
    int file_fd = open(files[i], O_RDONLY);
    if (file_fd == -1) {
      perror("Cannot open file");
      exit(EXIT_FAILURE);
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) != 0) {
      perror("Cannot stat file");
      exit(EXIT_FAILURE);
    }

    FileEntry entry;
    strncpy(entry.filename, files[i], 255);
    snprintf(entry.size, sizeof(entry.size), "%lo", file_stat.st_size);
    entry.mod_time = file_stat.st_mtime;

    write(archive_fd, &entry, sizeof(FileEntry));
    char *endPtr;
    long int fileSize = strtol(entry.size, &endPtr, 8);
    char *buffer = malloc(fileSize);

    if (buffer == NULL) {
      perror("Memory allocation failed");
      exit(EXIT_FAILURE);
    }

    read(file_fd, buffer, fileSize);
    write(archive_fd, buffer, fileSize);

    free(buffer);
    close(file_fd);
  }

  close(archive_fd);
}

// DEPENDS ON CREATE!!!
void append_archive(const char *filename, char *files[], int file_count) {
  // TODO: Implement
  int i = 0;
  int file_descriptor = open(filename, O_RDONLY);

  if (file_descriptor == -1) {
    perror("Failed to open the file");
    exit(EXIT_FAILURE);
  }

  while (i < file_count) {
    printf("iterating over: %s\n", files[i]);
    i++;
  }
}

int octalToDecimal(const char *octal) {
  int result = 0;
  while (*octal >= '0' && *octal <= '7') {
    result = result * 8 + (*octal - '0');
    octal++;
  }
  return result;
}

void list_archive(const char *filename) {
  // TODO: Implement
  FILE *tarfile = fopen(filename, "rb");

  if (tarfile == NULL) {
    perror("Failed to open the file");
    exit(EXIT_FAILURE);
  }

  char header[TAR_BLOCK_SIZE];
  while (fread(header, 1, TAR_BLOCK_SIZE, tarfile) == TAR_BLOCK_SIZE) {
    char filename[101];
    strncpy(filename, header, 100);

    int filesize = octalToDecimal(header + 124);

    if (filename[0] == '\0' && filesize == 0) {
      break;
    }

    printf("%s\n", filename);

    fseek(tarfile,
          (filesize + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE * TAR_BLOCK_SIZE,
          SEEK_CUR);
  }

  fclose(tarfile);
}

// DEPENDS ON CREATE!!!
void update_archive(const char *filename, char *files[], int file_count) {
  // TODO: Implement
}

void extract_archive(const char *filename) {
  // TODO: Implement
}

int main(int argc, char **argv) {

  const char *mode = argv[1];
  const char *filename = argv[2];
  if (strcmp(mode, "-cf") == 0) {
    create_archive(filename, &argv[3], argc - 3);
  } else if (strcmp(mode, "-rf") == 0) {
    append_archive(filename, &argv[3], argc - 3);
  } else if (strcmp(mode, "-tf") == 0) {
    list_archive(filename);
  } else if (strcmp(mode, "-uf") == 0) {
    update_archive(filename, &argv[3], argc - 3);
  } else if (strcmp(mode, "-xf") == 0) {
    extract_archive(filename);
  } else {
    fprintf(stderr, "Invalid mode: %s\n", mode);
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