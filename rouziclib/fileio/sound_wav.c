float *load_sound_wav_mem(const uint8_t *data, size_t data_len, size_t *sample_count, int *channels, int *samplerate)
{
	uint8_t *p;
	int chunk_size, bit_depth, byte_depth, compression=0;
	size_t i, full_count;
	float *output, vol;
	int32_t (*func_read)(const uint8_t *, size_t *) = read_byte8s_offset;

	// Common chunk
	p = memmem(data, data_len, "fmt ", 4);
	if (p==NULL)
	{
		fprintf_rl(stderr, "In load_sound_wav_mem(): No fmt tag found, not a valid WAVE file.\n");
		return NULL;
	}
	p = &p[4+4];				// skip subchunk size

	compression = read_LE16(p, &p);		// 1 for integer PCM, 3 for float, 0xFFFE for extensible
	*channels = read_LE16(p, &p);
	*samplerate = read_LE32(p, &p);
	p = &p[4+2];				// skip byte rate, block align
	bit_depth = read_LE16(p, &p);

	// Sound data
	p = memmem(data, data_len, "data", 4);
	if (p==NULL)
	{
		fprintf_rl(stderr, "In load_sound_wav_mem(): No data tag found, not a valid WAVE file.\n");
		return NULL;
	}
	p = &p[4];
	
	// Prepare decoding parameters
	byte_depth = ceil_rshift(bit_depth, 3);		// bytes per sample
	vol = 1.f / (float) (1LL << 8*byte_depth);	// volume multiplier

	*sample_count = read_LE32(p, &p) / (*channels * byte_depth);
	full_count = *sample_count * (size_t) *channels;

	switch (byte_depth)
	{
		case 2:	func_read = read_LE16s;	break;	
		case 3:	func_read = read_LE24s;	break;	
		case 4:	func_read = read_LE32;	break;	
	}

	output = calloc(*sample_count, *channels * sizeof(float));

	switch (compression)
	{
		case 1:		// integer LE
			for (i=0; i < full_count; i++)
				output[i] = func_read(&p[i * byte_depth], NULL) * vol;
			break;

		case 3:		// float LE
			for (i=0; i < full_count; i++)
				output[i] = u32_as_float(read_LE32(&p[i * 4], NULL));
			break;
	}

	return output;
}

float *load_sound_wav_file(const char *path, size_t *sample_count, int *channels, int *samplerate)
{
	uint8_t *data;
	size_t data_len;
	float *output;

	*sample_count = 0;
	data = load_raw_file(path, &data_len);
	if (data==NULL)
		return NULL;

	output = load_sound_wav_mem(data, data_len, sample_count, channels, samplerate);
	free(data);

	return output;
}

void save_sound_wav_fl32_file(const char *path, float *snd, size_t sample_count, int channels, int samplerate)
{
	FILE *file;
	size_t i, full_count = sample_count * (size_t) channels;
	const int bit_depth = 32;
	int byte_depth;

	if (snd==NULL || sample_count==0)
		return ;

	file = fopen_utf8(path, "wb");
	if (file==NULL)
	{
		fprintf_rl(stderr, "save_sound_wav_file() couldn't open file '%s' for writing.\n", path);
		return ;
	}

	byte_depth = ceil_rshift(bit_depth, 3);		// bytes per sample

	fprintf(file, "RIFF");
	fwrite_LE32(file, 36 + full_count*byte_depth);	// size of the whole file
	fprintf(file, "WAVEfmt ");
	fwrite_LE32(file, 16);
	fwrite_LE16(file, 3);				// format (1 for integer, 3 for float)
	fwrite_LE16(file, channels);
	fwrite_LE32(file, samplerate);
	fwrite_LE32(file, samplerate * channels * byte_depth);	// byte rate
	fwrite_LE16(file, channels * byte_depth);		// block align
	fwrite_LE16(file, bit_depth);

	fprintf(file, "data");
	fwrite_LE32(file, full_count*byte_depth);	// size of the data

	// write snd in LE 32-bit float
	for (i=0; i < full_count; i++)
		fwrite_LE32(file, float_as_u32(snd[i]));

	fclose(file);
}
