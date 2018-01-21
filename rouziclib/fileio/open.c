#ifdef _WIN32
FILE *fopen_utf8(const char *filename, const char *mode)
{
	wchar_t wfilename[PATH_MAX*4], wmode[8];
	FILE *file;

	if (utf8_to_wchar(filename, wfilename)==0)
		return NULL;

	if (utf8_to_wchar(mode, wmode)==0)
		return NULL;

	file = _wfopen(wfilename, wmode);
	
	return file;
}
#else
FILE *fopen_utf8(const char *filename, const char *mode)
{
	return fopen(filename, mode);
}
#endif

uint8_t *load_raw_file(const char *path, size_t *size)	// path is UTF-8
{
	FILE *in_file;
	uint8_t *data;
	size_t fsize;
	
	in_file = fopen_utf8(path, "rb");

	if (in_file==NULL)
	{
		fprintf_rl(stderr, "File '%s' not found.\n", path);
		return NULL;
	}

	fseek(in_file, 0, SEEK_END);
	fsize = ftell(in_file);
	rewind (in_file);

	data = calloc (fsize+1, sizeof(uint8_t));
	fread(data, 1, fsize, in_file);

	fclose(in_file);

	if (size)
		*size = fsize;

	return data;	
}

uint8_t *load_raw_file_dos_conv(const char *path, size_t *size)	// path is UTF-8, loads raw file but converts "\r\n" to "\n"
{
	FILE *in_file;
	size_t i, offset=0, fsize;
	uint8_t *data, byte0, byte1=0;
	
	in_file = fopen_utf8(path, "rb");

	if (in_file==NULL)
	{
		fprintf_rl(stderr, "File '%s' not found.\n", path);
		return NULL;
	}

	fseek(in_file, 0, SEEK_END);
	fsize = ftell(in_file);
	rewind (in_file);

	data = calloc (fsize+1, sizeof(uint8_t));

	for (i=0; i < fsize; i++)
	{
		byte0 = byte1;
		fread(&byte1, 1, 1, in_file);

		if (byte0=='\r' && byte1=='\n')
			offset++;

		data[i-offset] = byte1;
	}

	fclose(in_file);

	data = realloc(data, fsize-offset+1);

	if (size)
		*size = fsize - offset;

	return data;	
}

int save_string_to_file(const char *path, const char *mode, char *string)
{
	FILE *file;

	file = fopen_utf8(path, mode);

	if (file==NULL)
	{
		fprintf_rl(stderr, "File '%s' not found.\n", path);
		return 0;
	}

	fprintf(file, "%s", string);

	fclose(file);

	return 1;
}

int32_t count_linebreaks(FILE *file)
{
	int32_t i=0;
	uint8_t b;

	while (fread(&b, 1, 1, file))
		if (b=='\n')
			i++;

	rewind(file);

	return i;
}

int check_file_is_readable(char *path)
{
	FILE *file;
	int ret=0;

	file = fopen_utf8(path, "rb");
	if (file)
	{
		ret = 1;
		fclose(file);
	}

	return ret;
}