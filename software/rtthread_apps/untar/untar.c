#include <stdlib.h>
#include <dfs_posix.h>

/* supported file type */
#define REGTYPE		'0'		/* regular file */
#define REGTYPE0	'\0'    /* regular file (ancient bug compat)*/
#define DIRTYPE		'5'     /* directory */
#define GNULONGNAME 'L'     /* GNU long (>100 chars) file name */

#define NAME_SIZE	100

struct TarHeader
{
	/* byte offset */
	char name[NAME_SIZE];         /*   0-99 */
	char mode[8];                 /* 100-107 */
	char uid[8];                  /* 108-115 */
	char gid[8];                  /* 116-123 */
	char size[12];                /* 124-135 */
	char mtime[12];               /* 136-147 */
	char chksum[8];               /* 148-155 */
	char typeflag;                /* 156-156 */
	char linkname[NAME_SIZE];     /* 157-256 */
	char magic[6];                /* 257-262 */
	char version[2];              /* 263-264 */
	char uname[32];               /* 265-296 */
	char gname[32];               /* 297-328 */
	char devmajor[8];             /* 329-336 */
	char devminor[8];             /* 337-344 */
	char prefix[155];             /* 345-499 */
	char padding[12];             /* 500-512 (pad to exactly the TAR_BLOCK_SIZE) */
};

/* A few useful constants */
#define TAR_MAGIC          "ustar"        /* ustar and a null */
#define TAR_VERSION        "  "           /* Be compatable with GNU tar format */
#define TAR_MAGIC_LEN		6
#define TAR_VERSION_LEN		2
#define TAR_BLOCK_SIZE		512
#define TAR_BUFSZ			(1024 * 8)

static void usage(void)
{
	rt_kprintf("untar [-v] file.tar\n");
	return;
}


int make_directory(const char *dir)
{
    char *s = rt_strdup(dir);
    char *path = s;
    char c;
    int ret = 0;
	DIR *d;

    do
    {
        c = 0;
        /* Bypass leading non-'/'s and then subsequent '/'s. */
        while (*s)
        {
            if (*s == '/')
            {
                do{++s;}while (*s == '/');
                c = *s;                /* Save the current char */
                *s = 0;                /* and replace it with nul. */
                break;
            }
            ++s;
        }

		d = opendir(path);
		if (d != RT_NULL)
		{
			/* directory exist */
			closedir(d);
			*s = c;
			continue;
		}

		/* create this directory */
        ret = mkdir(path, 0777);
        if (ret < 0) break;

		if (!c) goto out;
        /* Remove any inserted nul from the path (recursive mode). */
        *s = c;
    }while (1);

out:
    rt_free(path);
    return ret;
}

int tar_check_header(struct TarHeader *header)
{
	int index;
	long chksum, sum = 0;
	unsigned char *s = (unsigned char *)header;

	/* Check the checksum */
	/* Get checksum value */
	chksum = strtol(header->chksum, NULL, 8);

	for (index = sizeof(struct TarHeader); index-- != 0;)
		sum += *s++;
	/* Remove the effects of the checksum field (replace
	 * with blanks for the purposes of the checksum) */
	s = header->chksum;
	for (index = sizeof(header->chksum) ; index-- != 0;)
		sum -= *s++;
	sum += ' ' * sizeof(header->chksum);
	if (sum != chksum )
	{
		rt_kprintf("bad header!\n");
		return -1;
	}

	return 0;
}

int untar(char* tar_fn, char* target_dir, int verbose)
{
	int file_sz;
	char *fn, *target_fn = RT_NULL;
	int tar_fd = -1, fd = -1;
	unsigned char* buf = RT_NULL;
	struct TarHeader *header = RT_NULL;

	tar_fd = open(tar_fn, O_RDONLY, 0);
	if (tar_fd < 0)
	{
		rt_kprintf("open tarball file:%s failed!\n", tar_fn);
		return -1;
	}

	if (target_dir != RT_NULL)
	{
		/* allocate target filename */
		target_fn = (char*)rt_malloc(256);
		if (target_fn == RT_NULL) goto __exit;
	}

	buf = rt_malloc(TAR_BUFSZ);
	if (buf == RT_NULL) goto __exit;
	header = rt_malloc(sizeof(struct TarHeader));
	if (header == RT_NULL) goto __exit;

	while (1)
	{
		/* read raw header */
		if (read(tar_fd, header, sizeof(struct TarHeader)) != sizeof(struct TarHeader))
		{
			/* read data failed */
			goto __exit;
		}
		if (header->name[0] == '\0') break; /* end of tarball file */

		/* Check checksum */
		if (tar_check_header(header) != 0) goto __exit;

		/* get file name */
		fn = header->name;
		/* remove /// */
		while (*fn == '/') fn ++;
		if (target_dir != RT_NULL)
		{
			rt_snprintf(target_fn, 255, "%s/%s", target_dir, fn);
			fn = target_fn;
		}

		/* get type and size of file */
		if (header->typeflag == REGTYPE || header->typeflag == REGTYPE0)
		{
			int fd;

			/* extract file */
			file_sz  = strtol(header->size, NULL, 8);

			if (verbose) rt_kprintf("=> %s\n", fn);

			/* open target file for write */
			fd = open(fn, O_CREAT|O_TRUNC|O_WRONLY, 0);
			if (fd < 0) goto __exit;

			/* read data and write data to a new file */
			while (file_sz)
			{
				int read_length, length;

				/* get the length for one chunk read */
				read_length = file_sz > TAR_BUFSZ? TAR_BUFSZ:(file_sz + TAR_BLOCK_SIZE - 1)/TAR_BLOCK_SIZE * TAR_BLOCK_SIZE;

				/* read data and write it to target file */
				length = read(tar_fd, buf, read_length);
				if (file_sz < TAR_BUFSZ)
				{
					write(fd, buf, file_sz);
					break;
				}
				else
				{
					write(fd, buf, length);
					file_sz -= length;
				}
			}

			close(fd);
		}
		else if (header->typeflag == DIRTYPE)
		{
			/* make directory */
			make_directory(fn);
		}
		else if (header->typeflag == GNULONGNAME)
		{
			/* long file name */
			file_sz  = strtol(header->size, NULL, 8);

			/* TODO */
		}
	}

__exit:
	if (tar_fd >= 0) close(tar_fd);
	if (fd >= 0) close(fd);
	if (target_fn != RT_NULL) rt_free(target_fn);
	if (buf != RT_NULL) rt_free(buf);
	if (header != RT_NULL) rt_free(header);

	return 0;
}

int main(int argc, char** argv)
{
	int verbose;
	char* tar_fn;
	char* target_dir = RT_NULL;

	if (argc < 2) {usage(); return 0;}
	if (argv[1][0] == '-' && argv[1][1] == 'v')
	{
		verbose = 1;
		tar_fn = argv[2];
		if (argc == 4) target_dir = argv[3];
	}
	else
	{
		verbose = 0;
		tar_fn = argv[1];
		if (argc == 3) target_dir = argv[2];
	}

	return untar(tar_fn, target_dir, verbose);
}
