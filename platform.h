#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


#if defined (UNIT_TEST)
/* UNIT test stubs */

static inline void flash_erase(uint32_t sector_address)
{
	FILE* stream = NULL;

	char path[32];
	sprintf(path, "%04X.tlv\0", sector_address & 0xffff0000);
	stream = fopen(path, "wb");

	if (stream)
	{
		fclose(stream);
	}
}

static inline size_t flash_write(uint32_t address, size_t length, uint8_t *buffer)
{
	FILE* stream = NULL;
	size_t ret = 0;

	char path[32];
	sprintf(path, "%04X.tlv\0", address & 0xffff0000);
	stream = fopen(path, "a+b");
		
	if (stream)
	{
		ret = fwrite(buffer, 1, length, stream);
		fclose(stream);
	}

	return ret;
}

static inline size_t flash_read(uint32_t address, size_t length, uint8_t* buffer)
{
	FILE* stream = NULL;
	size_t ret = 0;

	char path[32];
	sprintf(path, "%04X.tlv\0", address & 0xffff0000);
	stream = fopen(path, "rb");

	if (stream)
	{
		fseek(stream, address & 0x0000ffff, SEEK_SET);
		ret = fread(buffer, 1, length, stream);
		fclose(stream);
	}

	return ret;
}
#else
// TODO: add platform specific flash write APIs should be added here
#endif

#if defined(__GNUC__)
#define PACKED_1( struct2pack ) _Pragma("pack(push, 1)") struct2pack _Pragma("pack(pop)")
#elif defined(_WIN32)
#define PACKED_1( struct2pack ) _Pragma("pack(push, 1)") struct2pack _Pragma("pack(pop)")
#endif

#ifdef __cplusplus
}
#endif


#endif
