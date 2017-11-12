int dir_count(char *path, int *subdir_count, int *subfile_count)
{
	DIR *dirp;
	struct dirent *entry;

	*subdir_count = 0;
	*subfile_count = 0;

	dirp = opendir(path);
	if (dirp == NULL)
		return -1;

	while ((entry = readdir(dirp)))
	{
		if (entry->d_type == DT_DIR)
			(*subdir_count)++;

		if (entry->d_type == DT_REG)
			(*subfile_count)++;
	}

	closedir(dirp);

	return *subdir_count + *subfile_count;
}

void load_dir(char *path, fs_dir_t *dir)
{
	DIR *dirp;
	struct dirent *entry;
	int i_dir=0, i_file=0;

	if (dir->subdir)  free (dir->subdir);	dir->subdir = NULL;
	if (dir->subfile) free (dir->subfile);	dir->subfile = NULL;

	if (dir_count(path, &dir->subdir_count, &dir->subfile_count) < 0)
		return ;

	dir->subdir = calloc(dir->subdir_count, sizeof(fs_dir_t));
	dir->subfile = calloc(dir->subfile_count, sizeof(fs_file_t));

	dirp = opendir(path);
	if (dirp == NULL)
		return ;

	dir->path = calloc(strlen(path)+1, sizeof(char));
	strcpy(dir->path, path);

	while ((entry = readdir(dirp)))
	{
		if (entry->d_type == DT_DIR && i_dir < dir->subdir_count)
		{
			dir->subdir[i_dir].name = calloc(strlen(entry->d_name)+1, sizeof(char));
			strcpy(dir->subdir[i_dir].name, entry->d_name);
			dir->subdir[i_dir].parent = dir;
			i_dir++;
		}

		if (entry->d_type == DT_REG && i_file < dir->subfile_count)
		{
			dir->subfile[i_file].name = calloc(strlen(entry->d_name)+1, sizeof(char));
			strcpy(dir->subfile[i_file].name, entry->d_name);
			dir->subfile[i_file].parent = dir;
			i_file++;
		}
	}

	closedir(dirp);
}

void load_dir_depth(char *path, fs_dir_t *dir, int max_depth)	// a max_depth of -1 means infinite
{
	int i;
	char subdir_path[PATH_MAX*4];

	load_dir(path, dir);

	if (max_depth)
	for (i=0; i<dir->subdir_count; i++)
	{
		if (strcmp(dir->subdir[i].name, ".") && strcmp(dir->subdir[i].name, ".."))
		{
			//fprintf_rl(stdout, "[%s]\n", dir->subdir[i]);
			append_name_to_path(subdir_path, path, dir->subdir[i].name);
			load_dir_depth(subdir_path, &dir->subdir[i], max_depth-1);
		}
	}
}

void print_dir_depth(fs_dir_t *dir, int current_depth)
{
	int i, j;

	for (i=0; i<dir->subdir_count; i++)
	{
		if (strcmp(dir->subdir[i].name, ".") && strcmp(dir->subdir[i].name, ".."))
		{
			for (j=0; j<current_depth; j++)
				fprintf_rl(stdout, "    ");
			fprintf_rl(stdout, "[%s]\n", dir->subdir[i].name);

			print_dir_depth(&dir->subdir[i], current_depth+1);
		}
	}

	for (i=0; i<dir->subfile_count; i++)
	{
		for (j=0; j<current_depth; j++)
			fprintf_rl(stdout, "    ");
		fprintf_rl(stdout, "%s\n", dir->subfile[i].name);
	}
}

char *sprint_dir_depth_fullarg(fs_dir_t *dir, int current_depth, char **string, int *alloc_count)
{
	int i, j, ac=0;
	char *p=NULL;

	if (string==NULL)
		string = &p;
	
	if (alloc_count==NULL)
		alloc_count = &ac;

	for (i=0; i<dir->subdir_count; i++)
	{
		if (strcmp(dir->subdir[i].name, ".") && strcmp(dir->subdir[i].name, ".."))
		{
			for (j=0; j<current_depth; j++)
				sprintf_realloc(string, alloc_count, 1, "    ");
			sprintf_realloc(string, alloc_count, 1, "[%s]\n", dir->subdir[i].name);

			sprint_dir_depth_fullarg(&dir->subdir[i], current_depth+1, string, alloc_count);
		}
	}

	for (i=0; i<dir->subfile_count; i++)
	{
		for (j=0; j<current_depth; j++)
			sprintf_realloc(string, alloc_count, 1, "    ");
		sprintf_realloc(string, alloc_count, 1, "%s\n", dir->subfile[i].name);
	}

	if (string)
		if (*string && current_depth==0)
			*string = realloc(*string, strlen(*string)+1);

	return *string;
}

void free_dir(fs_dir_t *dir)
{
	int i;

	for (i=0; i<dir->subdir_count; i++)
	{
		if (strcmp(dir->subdir[i].name, ".") && strcmp(dir->subdir[i].name, ".."))
			free_dir(&dir->subdir[i]);
		else if (dir->subdir[i].name)
			free (dir->subdir[i].name);
	}

	for (i=0; i<dir->subfile_count; i++)
		if (dir->subfile[i].name)
			free (dir->subfile[i].name);

	if (dir->name)  free (dir->name);	dir->name = NULL;
	if (dir->subdir)  free (dir->subdir);	dir->subdir = NULL;
	if (dir->subfile) free (dir->subfile);	dir->subfile = NULL;
}

void export_subfiles_to_file(FILE *file, fs_dir_t *dir, const int indent)
{
	int i, j;

	for (i=0; i<dir->subfile_count; i++)
		if (dir->subfile[i].name)
		{
			for (j=0; j<indent; j++)
				fprintf(file, "\t");

			fprintf(file, "%s\n", dir->subfile[i].name);
		}
}

int export_subfiles_to_path(char *path, fs_dir_t *dir)
{
	FILE *file;

	file = fopen_utf8(path, "wb");
	if (file == NULL)
	{
		fprintf_rl(stderr, "Couldn't open file %s for writing in export_subfiles_to_path()\n", path);
		return 0;
	}

	export_subfiles_to_file(file, dir, 0);

	fclose(file);

	return 1;
}

void export_whole_dir_flat_to_file(FILE *file, fs_dir_t *dir, const int show_dirs, const int path_start)
{
	int i;
	char *printed_path, dir_char[2];

	sprintf(dir_char, "%c", DIR_CHAR);
	printed_path = &dir->path[path_start];
	if (printed_path[0] == '\0')		// hide the DIR_CHAR if the printed_path is to be empty
		dir_char[0] = '\0';
	else
		printed_path++;			// else remove the remaining DIR_CHAR

	for (i=0; i<dir->subdir_count; i++)
	{
		if (strcmp(dir->subdir[i].name, ".") && strcmp(dir->subdir[i].name, ".."))
		{
			if (show_dirs)
				fprintf(file, "%s%s%s\n", printed_path, dir_char, dir->subdir[i].name);

			export_whole_dir_flat_to_file(file, &dir->subdir[i], show_dirs, path_start);
		}
	}

	for (i=0; i<dir->subfile_count; i++)
		fprintf(file, "%s%s%s\n", printed_path, dir_char, dir->subfile[i].name);
}

int export_whole_dir_flat_to_path(char *path, fs_dir_t *dir, const int show_dirs, const int remove_path)
{
	FILE *file;

	file = fopen_utf8(path, "wb");
	if (file == NULL)
	{
		fprintf_rl(stderr, "Couldn't open file %s for writing in export_whole_dir_to_path()\n", path);
		return 0;
	}

	export_whole_dir_flat_to_file(file, dir, show_dirs, remove_path ? strlen(dir->path) : 0);

	fclose(file);

	return 1;
}

int dirent_test(char *path)
{
	int i;
	fs_dir_t dir;

	memset(&dir, 0, sizeof(fs_dir_t));

	load_dir_depth(path, &dir, 2);
	print_dir_depth(&dir, 0);
	free_dir(&dir);

	return 0;
}

void get_volumes_path()
{
#ifdef _WIN32
	int i, type, drivecount;
	uint32_t bits;
	char drive[]="A:\\", name[256], *list;
	
/*	bits = GetLogicalDrives();

	for (i=0; i<32; i++)
	{
		if ((bits >> i) & 1)
		{
			drive[0] = 'A'+i;
			type = GetDriveType(drive);
			name[0] = '\0';
			if (GetVolumeInformationA(drive, name, sizeof(name), NULL, NULL, NULL, NULL, 0) == 0)
			{
				//fprintf_rl(stderr, "GetLastError(): %d\n", GetLastError());
			}
			fprintf_rl(stdout, "%s (%c:), type %d\n", name, 'A'+i, type);
		}
	}

	arrayise_text(list, &drivecount);*/
#else
#endif
}

#ifndef _WIN32
#include <sys/statvfs.h>
#endif

int64_t get_volume_free_space(char *path)
{
#ifdef _WIN32
	uint64_t free_space=0;
	wchar_t wpath[PATH_MAX*4];
	FILE *file;

	utf8_to_wchar(path, wpath);

	//if (GetDiskFreeSpaceExW(wpath, &free_space, NULL, NULL)==0)
	if (GetDiskFreeSpaceExA(path, &free_space, NULL, NULL)==0)
	{
		fprintf_rl(stderr, "GetLastError() in get_volume_free_space(): %d\n", GetLastError());
		return -1;
	}

	return free_space;
#else
	struct statvfs stats;

	if (statvfs(path, &stats))
		return -1;

	return (int64_t) stats.f_bsize * stats.f_bavail;
#endif
}
