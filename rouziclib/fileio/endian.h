#ifndef RL_DONT_ASSUME_LITTLE_ENDIAN
#define ASS_LE
#endif

extern uint8_t fread_byte8(FILE *file);
extern uint16_t fread_LE16(FILE *file);
extern uint16_t fread_BE16(FILE *file);
extern uint32_t fread_LE32(FILE *file);
extern uint32_t fread_BE32(FILE *file);
extern void fwrite_byte8(FILE *file, uint8_t s);
extern void fwrite_LE16(FILE *file, uint16_t s);
extern void fwrite_BE16(FILE *file, uint16_t s);
extern void fwrite_LE32(FILE *file, uint32_t w);
extern void fwrite_BE32(FILE *file, uint32_t w);
extern void fwrite_LE64(FILE *file, uint64_t w);
extern void fwrite_BE64(FILE *file, uint64_t w);

extern uint8_t read_byte8(const uint8_t *buf, size_t *index);
extern int32_t read_byte8s(const uint8_t *buf, size_t *index);
extern int32_t read_byte8s_offset(const uint8_t *buf, size_t *index);
extern uint16_t read_LE16(const uint8_t *buf, size_t *index);
extern uint16_t read_BE16(const uint8_t *buf, size_t *index);
extern int32_t read_LE16s(const uint8_t *buf, size_t *index);
extern int32_t read_BE16s(const uint8_t *buf, size_t *index);
extern uint32_t read_LE24(const uint8_t *buf, size_t *index);
extern uint32_t read_BE24(const uint8_t *buf, size_t *index);
extern int32_t read_LE24s(const uint8_t *buf, size_t *index);
extern int32_t read_BE24s(const uint8_t *buf, size_t *index);
extern uint32_t read_LE32(const uint8_t *buf, size_t *index);
extern uint32_t read_BE32(const uint8_t *buf, size_t *index);
extern uint64_t read_LE64(const uint8_t *buf, size_t *index);
extern uint64_t read_BE64(const uint8_t *buf, size_t *index);

extern void print_LE16(uint8_t *buf, uint16_t data);
extern void print_LE24(uint8_t *buf, uint32_t data);
extern void print_LE32(uint8_t *buf, uint32_t data);
extern void print_LE64(uint8_t *buf, uint64_t data);
extern void print_BE16(uint8_t *buf, uint16_t data);
extern void print_BE24(uint8_t *buf, uint32_t data);
extern void print_BE32(uint8_t *buf, uint32_t data);
extern void print_BE64(uint8_t *buf, uint64_t data);
