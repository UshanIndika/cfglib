#include <string.h>
#include <stdbool.h>
#include "wwcfg.h"
#include "platform.h"

#define CONFIGURATION_MAGIC_NUMBER           (0xC04F15A6)
#define CONFIGURATION_MAGIC_NUMBER_SIZE      (4)
#define CONFIGURATION_SECTOR_ADDRESS         (0x0cf50000)
#define CONFIGURATION_FACTORY_SECTOR_ADDRESS (0x0fac0000)
#define CFG_HEADR_SIZE                       (sizeof(((ww_config_t*)0)->type) +  sizeof(((ww_config_t*)0)->index) + sizeof(((ww_config_t*)0)->length))
#define CFG_MAX_SERVER_NAME_BYTES            (128)

enum
{
	CFG_UNVERIFIED = 0,
	CFG_VERIFIED,
	CFG_VERIFIED_FAILED
};
// Globals
static uint8_t configuration_status = CFG_UNVERIFIED;

// Ideally CRC32 is used. however for the moment just a checksum is used just to indicate only the concept
static uint32_t calculate_crc(uint32_t crc, size_t length, uint8_t* data)
{
	for (size_t i = 0; i < length; i++)
	{
		crc += (uint32_t)data[i];
	}

	return crc;
}

static uint32_t read_cfg(uint32_t store_address, ww_config_t* cfg)
{
	uint32_t ret = CFG_FAILURE;
	size_t read = 0;
	ww_config_t tmp_cfg;
	bool found = false;
	uint32_t magic;
	uint32_t index = 0;
	uint32_t crc = 0;
	uint8_t crc_data[4];
	uint16_t crc_len = 0;

	if (NULL != cfg)
	{
		// Read magic number
		index = flash_read(store_address, sizeof(magic), (uint8_t*)&magic);
		if (sizeof(magic) == index && magic == CONFIGURATION_MAGIC_NUMBER)
		{
			do
			{
				// Read header
				read = flash_read(store_address + index, CFG_HEADR_SIZE, (uint8_t*)&tmp_cfg);

				// calculate crc onthe fly
				if (CFG_UNVERIFIED == configuration_status && CFG_TERMINATE != tmp_cfg.type && CONFIGURATION_SECTOR_ADDRESS == store_address)
				{
					crc = calculate_crc(crc, read, (uint8_t*)&tmp_cfg);
				}

				if (CFG_HEADR_SIZE == read && tmp_cfg.type == cfg->type && tmp_cfg.index == cfg->index)
				{
					ret = CFG_LENGTH_NOT_ENOUGH;

					if (tmp_cfg.length <= cfg->length)
					{
						// Read value
						crc_len = flash_read(store_address + index + read, tmp_cfg.length, &cfg->value[0]);
						// TODO: endianess conversion should be handled here
						if (crc_len == tmp_cfg.length)
						{
							found = true;
							cfg->length = tmp_cfg.length;
						}
					}
				}

				if (CFG_UNVERIFIED == configuration_status && CONFIGURATION_SECTOR_ADDRESS == store_address)
				{
					if (CFG_TERMINATE != tmp_cfg.type)
					{
						crc_len = 0;
						for (size_t i = 0; i < tmp_cfg.length; i++)
						{
							crc_len += flash_read(store_address + index + read + crc_len, 1, crc_data);
							crc = calculate_crc(crc, 1, crc_data);
						}

					}
					else
					{
						crc_len = flash_read(store_address + index + read, 4, (uint8_t*)&tmp_cfg.crc);
						configuration_status = CFG_VERIFIED_FAILED;
						if (crc == tmp_cfg.crc)
						{
							configuration_status = CFG_VERIFIED;
						}
					}
				}

				index += read + tmp_cfg.length;
			} while (tmp_cfg.type != CFG_TERMINATE && 0 < read);
		}
	}

	if (found)    ret = CFG_SUCCESS;

	return ret;
}

uint32_t ww_cfg_write_prepare(void)
{
	uint32_t ret = CFG_FAILURE;

	uint32_t magic = CONFIGURATION_MAGIC_NUMBER;
	flash_erase(CONFIGURATION_SECTOR_ADDRESS);
	if (sizeof(magic) == flash_write(CONFIGURATION_SECTOR_ADDRESS, sizeof(magic), (uint8_t*)&magic))
	{
		ret = CFG_SUCCESS;
	}

	return ret;
}

uint32_t ww_cfg_write(ww_config_t* cfg, uint32_t* crc)
{
	uint32_t ret = CFG_FAILURE;
	size_t written = 0;

	// TODO: endianess conversion should be handled here

	written  = flash_write(CONFIGURATION_SECTOR_ADDRESS, sizeof(cfg->type), (uint8_t*)&cfg->type);
	written += flash_write(CONFIGURATION_SECTOR_ADDRESS, sizeof(cfg->index), (uint8_t*)&cfg->index);
	written += flash_write(CONFIGURATION_SECTOR_ADDRESS, sizeof(cfg->length), (uint8_t*)&cfg->length);	
	written += flash_write(CONFIGURATION_SECTOR_ADDRESS, (size_t)cfg->length, (uint8_t*)&cfg->value);
	

	if (written == CFG_HEADR_SIZE + cfg->length )
	{
		*crc = calculate_crc(*crc, sizeof(cfg->type), (uint8_t*)&cfg->type);
		*crc = calculate_crc(*crc, sizeof(cfg->index), (uint8_t*)&cfg->index);
		*crc = calculate_crc(*crc, sizeof(cfg->length), (uint8_t*)&cfg->length);
		*crc = calculate_crc(*crc, (size_t)cfg->length, (uint8_t*)&cfg->value);


		ret = CFG_SUCCESS;
	}

	return ret;
}

uint32_t ww_cfg_write_done(uint32_t crc)
{
	uint32_t ret = CFG_FAILURE;
	ww_config_t cfg = { .type = CFG_TERMINATE , .length = sizeof(((ww_config_t*)0)->crc), .index = 0 , .crc = 0};
	cfg.crc = crc;

	ret = ww_cfg_write(&cfg, &crc);

	return ret;
}

uint32_t ww_read_cfg(ww_config_t* cfg)
{
	uint32_t ret1 = CFG_FAILURE;
	uint32_t ret2 = CFG_FAILURE;
	uint16_t lenbkup = cfg->length;
	uint16_t swap = cfg->length;

	// Read the default configuration
	ret1 = read_cfg(CONFIGURATION_FACTORY_SECTOR_ADDRESS, cfg);
	// Override the on-field configuration if available
	if (CFG_VERIFIED_FAILED!= configuration_status)
	{
		swap = cfg->length;
		cfg->length = lenbkup;
		ret2 = read_cfg(CONFIGURATION_SECTOR_ADDRESS, cfg);
	}

	if (CFG_SUCCESS == ret2 || CFG_LENGTH_NOT_ENOUGH == ret2)
	{
		return ret2;
	}

	cfg->length = swap;

	return ret1;
}

uint32_t ww_read_cfg_all(ww_golbal_cfg_t* cfg, uint32_t* length)
{
	uint32_t ret = CFG_FAILURE;
	uint32_t expected_length = sizeof(ww_golbal_cfg_t);
	uint32_t tmp_ret = CFG_SUCCESS;

	uint8_t tmparray[CFG_MAX_SERVER_NAME_BYTES + CFG_HEADR_SIZE];
	ww_config_t *tmp_cfg = (ww_config_t*)tmparray;
	uint8_t* server_ptr;

	if (NULL == cfg || NULL == length || *length < expected_length)
	{
		return CFG_FAILURE;
	}
	// Read server names
	server_ptr = ((uint8_t*)cfg) + sizeof(ww_golbal_cfg_t);

	for (int i = 0; i < NUMBER_OF_SERVERS; i++)
	{
		tmp_cfg->type = CFG_SERVER_NAME;
		tmp_cfg->index = i;
		tmp_cfg->length = sizeof(tmparray) - (CFG_HEADR_SIZE);

		tmp_ret += ww_read_cfg(tmp_cfg);
		expected_length += tmp_cfg->length + 1;
		
		if (CFG_SUCCESS == tmp_ret)
		{
			memcpy(server_ptr, tmp_cfg->value, tmp_cfg->length);
			server_ptr[tmp_cfg->length] = '\0';
			cfg->servers[i] = server_ptr;
			server_ptr = server_ptr + tmp_cfg->length + 1;
		}

		cfg->ccts[i] = tmp_cfg->circuit;
	}

	*length = expected_length;

	if(CFG_SUCCESS != tmp_ret)    return CFG_FAILURE;

	// Reaf calibrated
	tmp_cfg->type = CFG_CALIBRATED;
	tmp_cfg->index = 0;
	tmp_cfg->length = sizeof(tmparray) - CFG_HEADR_SIZE;

	ret = ww_read_cfg(tmp_cfg);
	if (CFG_SUCCESS != ret)    return ret;

	cfg->is_calibrated = tmp_cfg->is_calibrated;

	// Reaf lens
	tmp_cfg->type = CFG_LENS;
	tmp_cfg->index = 0;
	tmp_cfg->length = sizeof(tmparray) - (CFG_HEADR_SIZE);

	ret = ww_read_cfg(tmp_cfg);
	if (CFG_SUCCESS != ret)    return ret;

	cfg->is_lens = tmp_cfg->is_lens;

	// Reaf country code
	tmp_cfg->type = CFG_COUNTRY_CODE;
	tmp_cfg->index = 0;
	tmp_cfg->length = sizeof(tmparray) - (CFG_HEADR_SIZE);

	ret = ww_read_cfg(tmp_cfg);
	if (CFG_SUCCESS != ret)    return ret;

	memcpy(cfg->country_code, tmp_cfg->country_code, SIZEOF_COUNTRY_CODE);

	// read circuits
	for (int i = 0; i < NUMBER_OF_CCTS; i++)
	{
		tmp_cfg->type = CFG_CIRCUIT;
		tmp_cfg->index = i;
		tmp_cfg->length = sizeof(tmparray) - (CFG_HEADR_SIZE);

		ret = ww_read_cfg(tmp_cfg);
		if (CFG_SUCCESS != ret)    return ret;

		cfg->ccts[i] = tmp_cfg->circuit;
	}

	
	return ret;
}