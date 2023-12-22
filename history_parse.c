/*************************************************************************
    > File Name: history_parse.c
    > Author: jimmy
    > Mail: 1074833353@qq.com 
    > Created Time: 2023年11月30日 星期二 14时48分41秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "time.h"
#include "cJSON.h"
#ifdef __WIN32__
#include <windows.h>
#endif

static bool atoh(uint8_t *buffer_in, uint8_t size_in, uint8_t *buffer_out, uint8_t size_out);
static void data_parse_and_dump(cJSON *root, uint8_t *buffer, uint8_t *num_info, uint8_t type);
static void format_convert(uint8_t sign, uint8_t byte_offset, uint8_t bit_offset, uint8_t bit_length, uint32_t accuracy, uint8_t *buffer, cJSON *element);
static void str_trim_space(char *str);

int main(int argc, char **argv)
{
    char *pdata = NULL;
    char *pconf = NULL;
    FILE *fp = NULL;
    cJSON *jsonroot = NULL;

#ifdef __WIN32__
    SetConsoleOutputCP(65001); //设置输出编码为UTF-8
#endif

    if(argc != 2) {
        printf("Usage: %s <raw_data>\r\n", argv[0]);
        return -1;
    }

    fp = fopen("config.json", "r");
    if(fp) {
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        pconf = malloc(size+1);
        if(pconf) {
            memset(pconf, 0, size + 1);
            size_t length = fread(pconf, 1, size, fp);
            if(length) {
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

    //去除空格
    str_trim_space(argv[1]);

    do {//20ce解析
        //计算总条数
        uint16_t total_count = 0;
        pdata = argv[1];
        do {
            pdata = strstr(pdata, "20ce");
            if(pdata) {
                total_count++;
                pdata = pdata+2;
            }
        } while(pdata);

        //逐条解析数据
        uint16_t count = 0;
        pdata = argv[1];
        do {
            char buffer[128] = {0};
            pdata = strstr(pdata, "20ce");
            if(pdata) {
                count++;
                snprintf(buffer, sizeof(buffer), "%d/%d", count, total_count);
                data_parse_and_dump(jsonroot, pdata+4, buffer, 0);
                pdata = pdata+2;
            }
        } while(pdata);
    } while(0);

    do {//20cd解析
        //计算总条数
        uint16_t total_count = 0;
        pdata = argv[1];
        do {
            pdata = strstr(pdata, "20cd");
            if(pdata) {
                total_count++;
                pdata = pdata+2;
            }
        } while(pdata);

        //逐条解析数据
        uint16_t count = 0;
        pdata = argv[1];
        do {
            char buffer[128] = {0};
            pdata = strstr(pdata, "20cd");
            if(pdata) {
                count++;
                snprintf(buffer, sizeof(buffer), "%d/%d", count, total_count);
                data_parse_and_dump(jsonroot, pdata+4, buffer, 1);
                pdata = pdata+2;
            }
        } while(pdata);
    } while(0);

    if(pconf) {
        free(pconf);
    }
    cJSON_Delete(jsonroot);
    return 0;
}

static void data_parse_and_dump(cJSON *root, uint8_t *buffer, uint8_t *num_info, uint8_t type)
{
    cJSON *parser_root = cJSON_CreateObject();
    cJSON *item = NULL; 
    cJSON *unit = NULL;
    cJSON *temp = NULL;
    char *json_out = NULL;
    char timbuf[128] = {0};
    uint8_t enable = 0;
    int8_t sign = 0;
    int8_t auto_sign = 0;
    uint8_t bit_offset = 0;
    uint8_t bit_length = 0;
    uint8_t byte_offset = 0;
    uint32_t accuracy = 0;
    time_t timestamp = 0;

    if(!parser_root) {
        return;
    }

    cJSON_AddStringToObject(parser_root, "number", num_info);
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
    cJSON_AddStringToObject(parser_root, "localtime", timbuf);

    item = cJSON_GetObjectItem(root, "format_enable");
    if(item) {
        enable = item->valueint;
    }
    if(root && enable) {
        if(type == 0) {
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
                            if(sign == -1) {
                                sign = auto_sign;
                            } 
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
                        temp = cJSON_GetObjectItem(element, "name");
                        if(temp) {
                            if(!strcmp(temp->valuestring, "sign")) {
                                temp = cJSON_GetObjectItem(element, "value");
                                if(temp) {
                                    auto_sign = temp->valueint;
                                }
                            }
                        }
                    }
                }
            }
        } else if(type == 1) {
            int count = 0;
            for(count = 0; count < sizeof(timbuf)-1; count++) {
                if(!buffer[8+count]) {
                    break;
                }
            }
            memset(timbuf, 0, sizeof(timbuf));
            atoh(buffer+8, count, (uint8_t *)timbuf, sizeof(timbuf));
            cJSON_AddStringToObject(parser_root, "data", timbuf);
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
    // printf("sign:%d, byte_offset:%d, bit_offset:%d, bit_length:%d, accuracy:%d, bytes:%d\r\n", sign, byte_offset, bit_offset, bit_length, accuracy, bytes);
    if(bytes == 1 || bytes == 2 || bytes == 4) {
        switch(bytes) {
            case 1:
            {
                uint8_t temp = 0;
                atoh(buffer+byte_offset*2, 2, (uint8_t *)&temp, sizeof(temp));
                temp = (temp >> bit_offset) & ((1 << bit_length) - 1);
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
                temp = (temp >> bit_offset) & ((1 << bit_length) - 1);
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
                if(bit_length < 32) {
                    temp = (temp >> bit_offset) & ((1 << bit_length) - 1);
                } else {
                    temp = (temp >> bit_offset);
                }
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

static void str_trim_space(char *str)
{
    char *ptmp = str;

    while(*str != 0) {
        if(*str != ' ') {
            *ptmp++ = *str;
        }
        ++str;
    }
    *ptmp = 0;
}