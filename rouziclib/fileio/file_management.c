#ifndef _WIN32
#include <sys/stat.h>
#endif

int create_symlink(const char *oldname, const char *newname, const int is_dir)	// returns 0 if successful
{
	#ifdef _WIN32
	wchar_t oldname_wc[PATH_MAX*2], newname_wc[PATH_MAX*2];

	utf8_to_wchar(oldname, oldname_wc);
	utf8_to_wchar(newname, newname_wc);
	return CreateSymbolicLinkW(newname_wc, oldname_wc, is_dir) == 0;

	#else

	return symlink(oldname, newname);
	#endif
}

int create_dir(const char *path)	// returns 0 if successful
{
	#ifdef _WIN32
	wchar_t wpath[PATH_MAX*2];

	utf8_to_wchar(path, wpath);
	return CreateDirectoryW(wpath, NULL) == 0;

	#else

	return mkdir(path, 0755);
	#endif
}

int create_dir_recursive(const char *path)
{
	char parent_path[PATH_MAX*4];

	if (path==NULL)
		return -1;
	if (path[0]=='\0')
		return -1;

	if (check_dir_exists(path))
		return 0;

	if (create_dir(path) == 0)			// if successful
		return 0;

	remove_name_from_path(parent_path, path);
	if (create_dir_recursive(parent_path))		// if creating the parents failed
		return -1;

	return create_dir(path);			// create the dir after the parents have been created
}

int create_dirs_for_file(const char *filepath)
{
	char parent_path[PATH_MAX*4];

	remove_name_from_path(parent_path, filepath);
	return create_dir_recursive(parent_path);
}

int move_file(const char *path, const char *newpath)	// returns 0 if successful
{
	#ifdef _WIN32
	wchar_t wpath[PATH_MAX*2], wnewpath[PATH_MAX*2];

	utf8_to_wchar(path, wpath);
	utf8_to_wchar(newpath, wnewpath);
	return MoveFileW(wpath, wnewpath)==0;

	#else

	return rename(path, newpath);
	#endif
}

int copy_file(const char *path, const char *newpath, const int overwrite)	// returns 0 if successful
{
	#ifdef _WIN32
	wchar_t wpath[PATH_MAX*2], wnewpath[PATH_MAX*2];

	utf8_to_wchar(path, wpath);
	utf8_to_wchar(newpath, wnewpath);
	return CopyFileW(wpath, wnewpath, overwrite==0)==0;

	#else

	// TODO
	fprintf_rl(stderr, "TODO: implement copy_file() for non-Windows\n");
	return 1;
	#endif
}

int remove_file(const char *path)	// returns 0 if successful
{
	#ifdef _WIN32
	wchar_t wpath[PATH_MAX*2];

	utf8_to_wchar(path, wpath);
	return _wremove(wpath);

	#else

	return remove(path);
	#endif
}

int remove_empty_dir(const char *path)
{
	#ifdef _WIN32
	wchar_t wpath[PATH_MAX*2];

	utf8_to_wchar(path, wpath);
	return RemoveDirectoryW(wpath);

	#else

	return rmdir(path);
	#endif
}

int remove_every_file(const char *path)		// returns 0 if successful on every single file
{
	int i, ret;
	fs_dir_t dir;
	char ffp[PATH_MAX*4];

	memset(&dir, 0, sizeof(fs_dir_t));

	load_dir_depth(path, &dir, 0);

	for (i=0; i<dir.subfile_count; i++)
		if (dir.subfile[i].name)
			ret |= remove_file(append_name_to_path(ffp, path, dir.subfile[i].name));

	free_dir(&dir);

	return ret;
}

int remove_dir(const char *path)
{
	int i, ret;
	fs_dir_t dir;
	char ffp[PATH_MAX*4];

	memset(&dir, 0, sizeof(fs_dir_t));

	load_dir_depth(path, &dir, 0);

	for (i=0; i<dir.subdir_count; i++)
		if (dir.subdir[i].path)
			ret |= remove_dir(dir.subdir[i].path);

	for (i=0; i<dir.subfile_count; i++)
		if (dir.subfile[i].name)
			ret |= remove_file(append_name_to_path(ffp, path, dir.subfile[i].name));

	remove_empty_dir(path);

	free_dir(&dir);

	return ret;
}

void system_open(const char *path)
{
	#ifdef _WIN32
	wchar_t path_w[PATH_MAX*2];

	utf8_to_wchar(path, path_w);
	ShellExecuteW(NULL, L"open", path_w, NULL, NULL, SW_SHOWNORMAL);
	#endif

	#ifdef __APPLE__
	char command[PATH_MAX+8];

	sprintf(command, "open \"%s\"", path);
	system(command);

	/*NSURL *fileURL = [NSURL fileURLWithPath: [NSString stringWithUTF8String:path]];
	
	NSWorkspace *ws = [NSWorkspace sharedWorkspace];
	[ws openURL: fileURL];*/
	#endif
}
