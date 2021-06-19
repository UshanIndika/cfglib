#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "platform.h"
#include "wwcfg.h"

#define SERVER1 "http://ingress-prod1-au.wattwachers.com.au"
#define SERVER2 "https://www.howsmyssl.com/a/check"
#define SERVER3 "prod-26.australiasoutheast.logic.azure.com"

enum
{
    UTEST_STATUS_PASS,
    UTEST_STATUS_FAIL,
    UTEST_STATUS_INCO,
};

void write_default_config(void);
uint32_t cfg_test_001(void);
uint32_t cfg_test_002(void);

int main(int argc, char* argv[], char* envp[])
{
    uint32_t ret = UTEST_STATUS_INCO;
   
#if 0  // Enable only when debugging single test case
    argv[1] = "cfg_test_001";
    argc = 2;
#endif

    if (argc > 1 && argv[1] != NULL)
    {
        if      (0 == strcmp("cfg_test_001", argv[1]))    ret = cfg_test_001();
        else if (0 == strcmp("cfg_test_002", argv[1]))    ret = cfg_test_002();
    }

    printf("\n\r\n\r%s : %s\n\r\n\r", argv[1], (UTEST_STATUS_PASS == ret) ? "PASS" : "FAIL");
    return  ret;
}

uint32_t cfg_test_001()
{
    // Objective: read all the configuration and verify
    uint32_t ret;
    uint8_t buffer[1024];
    ww_golbal_cfg_t* cfg = (ww_golbal_cfg_t*)buffer;
    uint32_t length = sizeof(buffer);

    ret = ww_read_cfg_all(cfg, &length);
    if (CFG_SUCCESS != ret)  return ret;

    // Verify values
    if(!cfg->is_lens)    return UTEST_STATUS_FAIL;
    if (!cfg->is_calibrated)    return UTEST_STATUS_FAIL;
    if(0 != memcmp(cfg->country_code, "AU", 2)) return UTEST_STATUS_FAIL;

    if (0 != strcmp(cfg->servers[0], SERVER1)) return UTEST_STATUS_FAIL;
    if (0 != strcmp(cfg->servers[1], "my server")) return UTEST_STATUS_FAIL;
    if (0 != strcmp(cfg->servers[2], SERVER3)) return UTEST_STATUS_FAIL;

    // TODO check the rest of the configuration

    return UTEST_STATUS_PASS;
}

uint32_t cfg_test_002(void)
{
    // Objective: update couple of configuration and verify
    uint32_t ret;
    uint32_t crc = 0;
    char* test_cfg_str = "my server";

    uint8_t buffer[1024];
    ww_config_t* cfg = (ww_config_t*)buffer;

    ret = ww_cfg_write_prepare();

    if (CFG_SUCCESS != ret)  return ret;
    // Update server name
    cfg->type = CFG_SERVER_NAME;
    cfg->index = 1;
    cfg->length = strlen(test_cfg_str);
    sprintf(cfg->servers, "%s", test_cfg_str);
    ret = ww_cfg_write(cfg, &crc);

    if (CFG_SUCCESS != ret)  return ret;
    // Update a flag
    cfg->type = CFG_CALIBRATED;
    cfg->index = 0;
    cfg->length = 2;
    cfg->is_calibrated = 1;
    ret = ww_cfg_write(cfg, &crc);

    ret = ww_cfg_write_done(crc);

    if (CFG_SUCCESS != ret)  return UTEST_STATUS_FAIL;

    memset(buffer, 0, sizeof(buffer));

    cfg->type = CFG_SERVER_NAME;
    cfg->index = 1;
    cfg->length = 1000;

    ret = ww_read_cfg(cfg);

    if (CFG_SUCCESS != ret)  return UTEST_STATUS_FAIL;

    if (0 != memcmp(test_cfg_str, cfg->servers, strlen(test_cfg_str))) return UTEST_STATUS_FAIL;

    memset(buffer, 0, sizeof(buffer));

    cfg->type = CFG_CALIBRATED;
    cfg->index = 0;
    cfg->length = 1000;

    ret = ww_read_cfg(cfg);

    if(! cfg->is_calibrated) return UTEST_STATUS_FAIL;

    return UTEST_STATUS_PASS;

}

void write_default_config(void)
{
    uint32_t ret;
    uint32_t crc = 0;

    uint8_t buffer[1024];
    ww_config_t* cfg = (ww_config_t*)buffer;
    ww_cct_cfg_t cct0 = { .current_rating = 60,  .amperage_calibration_offset = 0, .voltage_calibration_offset = 0 };
    ww_cct_cfg_t cct1 = { .current_rating = 60,  .amperage_calibration_offset = -6, .voltage_calibration_offset = 17 };
    ww_cct_cfg_t cct2 = { .current_rating = 120, .amperage_calibration_offset = -5, .voltage_calibration_offset = 23 };
    ww_cct_cfg_t cct3 = { .current_rating = 200, .amperage_calibration_offset = -4, .voltage_calibration_offset = 47 };
    ww_cct_cfg_t cct4 = { .current_rating = 400, .amperage_calibration_offset = -3, .voltage_calibration_offset = 91 };
    ww_cct_cfg_t cct5 = { .current_rating = 600, .amperage_calibration_offset = -2, .voltage_calibration_offset = 117 };

    ret = ww_cfg_write_prepare();


    cfg->type = CFG_COUNTRY_CODE;
    cfg->index = 0;
    cfg->length = 2;
    cfg->country_code[0] = 'A';
    cfg->country_code[1] = 'U';
    ret = ww_cfg_write(cfg, &crc); 

    cfg->type = CFG_CALIBRATED;
    cfg->index = 0;
    cfg->length = 2;
    cfg->is_calibrated = 0;
    ret = ww_cfg_write(cfg, &crc);

    cfg->type = CFG_LENS;
    cfg->index = 0;
    cfg->length = 2;
    cfg->is_lens = 1;
    ret = ww_cfg_write(cfg, &crc);

    cfg->type = CFG_SERVER_NAME;
    cfg->index = 0;
    cfg->length = strlen(SERVER1);
    sprintf(cfg->servers, "%s", SERVER1);
    ret = ww_cfg_write(cfg, &crc);

    cfg->type = CFG_SERVER_NAME;
    cfg->index = 1;
    cfg->length = strlen(SERVER2);
    sprintf(cfg->servers, "%s", SERVER2);
    ret = ww_cfg_write(cfg, &crc);


    cfg->type = CFG_SERVER_NAME;
    cfg->index = 2;
    cfg->length = strlen(SERVER3);
    sprintf(cfg->servers, "%s", SERVER3);
    ret = ww_cfg_write(cfg, &crc);

    cfg->type = CFG_CIRCUIT;
    cfg->index = 0;
    cfg->length = sizeof(cct0);
    cfg->circuit = cct0;
    ret = ww_cfg_write(cfg, &crc);

    cfg->type = CFG_CIRCUIT;
    cfg->index = 1;
    cfg->length = sizeof(cct1);
    cfg->circuit = cct1;
    ret = ww_cfg_write(cfg, &crc);

    cfg->type = CFG_CIRCUIT;
    cfg->index = 2;
    cfg->length = sizeof(cct1);
    cfg->circuit = cct2;
    ret = ww_cfg_write(cfg, &crc);

    cfg->type = CFG_CIRCUIT;
    cfg->index = 3;
    cfg->length = sizeof(cct1);
    cfg->circuit = cct3;
    ret = ww_cfg_write(cfg, &crc);

    cfg->type = CFG_CIRCUIT;
    cfg->index = 4;
    cfg->length = sizeof(cct1);
    cfg->circuit = cct4;
    ret = ww_cfg_write(cfg, &crc);

    cfg->type = CFG_CIRCUIT;
    cfg->index = 5;
    cfg->length = sizeof(cct1);
    cfg->circuit = cct5;
    ret = ww_cfg_write(cfg, &crc);

    ww_cfg_write_done(crc);
}