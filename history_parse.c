/*************************************************************************
    > File Name: history_parse.c
    > Author: jimmy
    > Mail: 1074833353@qq.com 
    > Created Time: 2021年08月31日 星期二 19时48分41秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "time.h"
#include "cJSON.h"

static bool atoh(uint8_t *buffer_in, uint8_t size_in, uint8_t *buffer_out, uint8_t size_out);
static void data_parse_and_dump(cJSON *root, uint8_t *buffer, uint16_t num);
static void format_convert(uint8_t sign, uint8_t byte_offset, uint8_t bit_offset, uint8_t bit_length, uint32_t accuracy, uint8_t *buffer, cJSON *element);

int main(int arvc, char **argv)
{
    char *pdata = argv[1];
    char *pconf = NULL;
    FILE *fp = NULL;
    cJSON *jsonroot = NULL;
    uint16_t count = 0;

    fp = fopen("config.json", "r");
    if(fp) {
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        pconf = malloc(size+1);
        if(pconf) {
            memset(pconf, 0, size + 1);
            size_t length = fread(pconf, 1, size, fp);
            if(length == size) {
                jsonroot = cJSON_Parse(pconf);
                if (jsonroot != NULL) {
                    // char *json_out = NULL;
                    // json_out = cJSON_Print(jsonroot);
                    // char *json_out = data_parse(jsonroot, pdata + 12);
                    // printf("json_out:%s\r\n", json_out);
                    // if(json_out) {
                    //     free(json_out);
                    // }
                }
            }
        }
        fclose(fp);
    }

    do {
        pdata = strstr(pdata, "20ce");
        if(pdata) {
            // char time_strb[9] = {0};
            // char time_strl[9] = {0};
            // unsigned int timestamp = 0;
            // struct tm *ptm = NULL;
            // memset(time_strb, 0, sizeof(time_strb));
            // memset(time_strl, 0, sizeof(time_strl));
            // memcpy(time_strb, pdata+4, 8);
            // time_strl[0] = time_strb[6];
            // time_strl[1] = time_strb[7];
            // time_strl[2] = time_strb[4];
            // time_strl[3] = time_strb[5];
            // time_strl[4] = time_strb[2];
            // time_strl[5] = time_strb[3];
            // time_strl[6] = time_strb[0];
            // time_strl[7] = time_strb[1];
            // sscanf((char *)time_strl, "%x", &timestamp);
            // time_t temp = timestamp;
            // ptm = gmtime(&temp);
            // printf("timestamp = %u, local:%s\r\n", timestamp, ctime(&temp));
            count++;
            data_parse_and_dump(jsonroot, pdata+4, count);
            pdata = pdata+2;
        }
    } while(pdata);

    if(pconf) {
        free(pconf);
    }
    cJSON_Delete(jsonroot);
    return 0;
}

static void data_parse_and_dump(cJSON *root, uint8_t *buffer, uint16_t num)
{
    cJSON *parser_root = cJSON_CreateObject();
    cJSON *item = NULL; 
    cJSON *unit = NULL;
    cJSON *temp = NULL;
    char *json_out = NULL;
    char timbuf[128] = {0};
    uint8_t enable = 0;
    uint8_t sign = 0;
    uint8_t bit_offset = 0;
    uint8_t bit_length = 0;
    uint8_t byte_offset = 0;
    uint32_t accuracy = 0;
    time_t timestamp = 0;

    if(!parser_root) {
        return;
    }

    cJSON_AddNumberToObject(parser_root, "number", num);
    atoh(buffer, 8, (uint8_t *)&timestamp, sizeof(timestamp));
    item = cJSON_GetObjectItem(root, "timezone");
    if(item) {
        timestamp += item->valueint*60;
    }
    snprintf(timbuf, sizeof(timbuf), "%s", ctime(&timestamp));
    if(strlen(timbuf)) {
        timbuf[strlen(timbuf)-1] = 0;
    }
    cJSON_AddNumberToObject(parser_root, "timestamp", timestamp);
    cJSON_AddStringToObject(parser_root, "unix time", timbuf);

    item = cJSON_GetObjectItem(root, "format_enable");
    if(item) {
        enable = item->valueint;
    }
    if(root && enable) {
        cJSON *array = cJSON_CreateArray();
        item = cJSON_GetObjectItem(root, "format");
        if (item != NULL && array != NULL) {
            cJSON_AddItemToObject(parser_root, "data", array);
            int count = cJSON_GetArraySize(item);
            for (int j = 0; j < count; j++) {
                unit = cJSON_GetArrayItem(item, j);
                if(unit) {
                    cJSON *element = cJSON_CreateObject();
                    cJSON_AddItemToArray(array, element);
                    temp = cJSON_GetObjectItem(unit, "name");
                    if(temp) {
                        cJSON_AddStringToObject(element, "name", temp->valuestring);
                    }
                    temp = cJSON_GetObjectItem(unit, "sign");
                    if(temp) {
                        sign = temp->valueint;
                    }
                    temp = cJSON_GetObjectItem(unit, "accuracy");
                    if(temp) {
                        accuracy = temp->valueint;
                    }
                    temp = cJSON_GetObjectItem(unit, "offset");
                    if(temp) {
                        byte_offset = temp->valueint;
                    }
                    temp = cJSON_GetObjectItem(unit, "bitoffset");
                    if(temp) {
                        bit_offset = temp->valueint;
                    }
                    temp = cJSON_GetObjectItem(unit, "bitsize");
                    if(temp) {
                        bit_length = temp->valueint;
                    }
                    format_convert(sign, byte_offset, bit_offset, bit_length, accuracy, buffer+8, element);
                }
            }
        }
    }

    json_out = cJSON_Print(parser_root);
    cJSON_Delete(parser_root);
    if(json_out) {
        printf("%s\r\n", json_out);
        free(json_out);
    }
}

static void format_convert(uint8_t sign, uint8_t byte_offset, uint8_t bit_offset, uint8_t bit_length, uint32_t accuracy, uint8_t *buffer, cJSON *element)
{
    uint8_t bytes = (bit_length%8)?(bit_length/8+1):(bit_length/8);
    if(bytes == 1 || bytes == 2 || bytes == 4) {
        switch(bytes) {
            case 1:
            {
                uint8_t temp = 0;
                atoh(buffer+byte_offset*2, 2, (uint8_t *)&temp, sizeof(temp));
                temp = (temp >> bit_offset);
                if(!bit_offset && sign) {
                    char temp1 = temp;
                    if(accuracy > 1) {
                        float f = temp1;
                        cJSON_AddNumberToObject(element, "value", f/(float)accuracy);
                    } else {
                        cJSON_AddNumberToObject(element, "value", temp1);
                    }
                } else {
                    if(accuracy > 1) {
                        float f = temp;
                        cJSON_AddNumberToObject(element, "value", f/(float)accuracy);
                    } else {
                        cJSON_AddNumberToObject(element, "value", temp);
                    }
                }
                break;
            }
            case 2:
            {
                uint16_t temp = 0;
                atoh(buffer+byte_offset*2, 4, (uint8_t *)&temp, sizeof(temp));
                temp = (temp >> bit_offset);
                if(!bit_offset && sign) {
                    short temp1 = temp;
                    if(accuracy > 1) {
                        float f = temp1;
                        cJSON_AddNumberToObject(element, "value", f/(float)accuracy);
                    } else {
                        cJSON_AddNumberToObject(element, "value", temp1);
                    }
                } else {
                    if(accuracy > 1) {
                        float f = temp;
                        cJSON_AddNumberToObject(element, "value", f/(float)accuracy);
                    } else {
                        cJSON_AddNumberToObject(element, "value", temp);
                    }
                }
                break;
            }
            case 4:
            {
                uint32_t temp = 0;
                atoh(buffer+byte_offset*2, 8, (uint8_t *)&temp, sizeof(temp));
                temp = (temp >> bit_offset);
                if(!bit_offset && sign) {
                    int temp1 = temp;
                    if(accuracy > 1) {
                        float f = temp1;
                        cJSON_AddNumberToObject(element, "value", f/(float)accuracy);
                    } else {
                        cJSON_AddNumberToObject(element, "value", temp1);
                    }
                } else {
                    if(accuracy > 1) {
                        float f = temp;
                        cJSON_AddNumberToObject(element, "value", f/(float)accuracy);
                    } else {
                        cJSON_AddNumberToObject(element, "value", temp);
                    }
                }
                break;
            }
            default:
                break;
        }
    } else {
        cJSON_AddStringToObject(element, "value", "unsupported");
    }
}

static bool atoh(uint8_t *buffer_in, uint8_t size_in, uint8_t *buffer_out, uint8_t size_out)
{
    uint8_t i, high, low;

    if((size_in%2) || (size_out < size_in/2)) {
        return false;
    }

    for(i = 0; i < size_in/2; i++) {
        if(buffer_in[2*i] >= '0' && buffer_in[2*i] <= '9') {
            high = buffer_in[2*i] - '0';
        } else if(buffer_in[2*i] >= 'A' && buffer_in[2*i] <= 'F') {
            high = buffer_in[2*i] - 'A' + 10;
        } else if(buffer_in[2*i] >= 'a' && buffer_in[2*i] <= 'f') {
            high = buffer_in[2*i] - 'a' + 10;
        } else {
            return false;
        }
        if(buffer_in[2*i + 1] >= '0' && buffer_in[2*i + 1] <= '9') {
            low = buffer_in[2*i + 1] - '0';
        } else if(buffer_in[2*i + 1] >= 'A' && buffer_in[2*i + 1] <= 'F') {
            low = buffer_in[2*i + 1] - 'A' + 10;
        } else if(buffer_in[2*i + 1] >= 'a' && buffer_in[2*i + 1] <= 'f') {
            low = buffer_in[2*i + 1] - 'a' + 10;
        } else {
            return false;
        }
        buffer_out[i] = high*16 + low;
    }

    return true;
}