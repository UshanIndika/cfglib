#ifndef WW_CFG_H_
#define WW_CFG_H_

#include <string.h>
#include <stdint.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: Error zone
#define CFG_ERROR_ZONE (0)
// CFG ERROR codes
#ifndef CFG_ERROR_SPACE
#define CFG_ERROR_SPACE (0)  // TODO: Add systems error space allocated to CFG
#endif

#define CFG_SUCCESS                 (0)
#define CFG_FAILURE                 (CFG_ERROR_SPACE + 0x00000001)
#define CFG_LENGTH_NOT_ENOUGH       (CFG_ERROR_SPACE + 0x00000002)

#define NUMBER_OF_CCTS              (6)  // Each model may support different circuits
#define SIZEOF_COUNTRY_CODE         (2)
#define NUMBER_OF_SERVERS           (3)

// Configuration types
#define CFG_COUNTRY_CODE            (0x0001)
#define CFG_LENS                    (0x0002)
#define CFG_CALIBRATED              (0x0003)
#define CFG_SERVER_NAME             (0x0004)
#define CFG_CIRCUIT                 (0x0005)
#define CFG_TERMINATE               (0x3333)

PACKED_1(
typedef struct ww_cct_cfg
{
	uint16_t current_rating;
	int32_t amperage_calibration_offset;
	int32_t voltage_calibration_offset;
}ww_cct_cfg_t;
)

typedef struct ww_golbal_cfg
{
	uint8_t is_calibrated;
	uint8_t is_lens;
	char country_code[SIZEOF_COUNTRY_CODE];
	ww_cct_cfg_t ccts[NUMBER_OF_CCTS];
	char* servers[NUMBER_OF_SERVERS];
	uint8_t reserved[0];
}ww_golbal_cfg_t;


typedef struct ww_config
{
	uint16_t type;
	uint16_t index;
	uint16_t length;
	union
	{
		uint8_t is_calibrated;
		uint8_t is_lens;
		char country_code[SIZEOF_COUNTRY_CODE];
		char servers[0];
		ww_cct_cfg_t circuit;
		uint32_t crc;
		uint8_t value[0];
	};
}ww_config_t;

uint32_t ww_cfg_write_prepare(void);
uint32_t ww_cfg_write(ww_config_t *cfg, uint32_t *crc);
uint32_t ww_cfg_write_done(uint32_t crc);
uint32_t ww_read_cfg(ww_config_t* cfg);

/*
It is not clear that how frequently configurations are used in the rest of the
program. If configurations are stored in XIP memory high speed NV memory, it
can be directly read. In such cases, read directly from the NV memory is more
sensible as it protects the configuration being accentially modified. However, it may be
read in the begining to a data structure. user provided data structure is given
*/
uint32_t ww_read_cfg_all(ww_golbal_cfg_t* cfg, uint32_t *lengh);

// TODO: load from TLV

#ifdef __cplusplus
}
#endif

#endif
