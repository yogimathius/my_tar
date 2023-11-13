#include "my_tar.h"

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
