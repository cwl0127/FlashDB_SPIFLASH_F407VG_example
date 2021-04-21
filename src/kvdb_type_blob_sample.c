/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief blob KV samples.
 *
 * Key-Value Database blob type KV feature samples
 */

#include <flashdb.h>
#include <string.h>

#ifdef FDB_USING_KVDB

#define FDB_LOG_TAG "[sample][kvdb][blob]"

#define SSID_MAXLEN 16
#define PWD_MAXLEN 16
#define LOCALIP_MAXLEN 16
#define SUBNET_MAXLEN 16
#define GATEWAY_MAXLEN 16
#define DNS_MAXLEN 16
#define SERVERIP_MAXLEN 32
#define SERVERPORT_MAXLEN 5

typedef struct
{
    struct
    {
        volatile uint8_t data[SSID_MAXLEN + 1]; //WIFI名称
        volatile uint8_t len;
    } ssid;

    struct
    {
        volatile uint8_t data[PWD_MAXLEN + 1]; //WIFI密码
        volatile uint8_t len;
    } pwd;

    struct
    {
        volatile uint8_t data[LOCALIP_MAXLEN + 1]; //本地IP
        volatile uint8_t len;
    } localip;

    struct
    {
        volatile uint8_t data[SUBNET_MAXLEN + 1]; //子网掩码
        volatile uint8_t len;
    } subnet;

    struct
    {
        volatile uint8_t data[GATEWAY_MAXLEN + 1]; //网关
        volatile uint8_t len;
    } gateway;

    struct
    {
        volatile uint8_t data[DNS_MAXLEN + 1]; //DNS
        volatile uint8_t len;
    } dns;

    struct
    {
        volatile uint8_t data[SERVERIP_MAXLEN + 1]; //服务器IP
        volatile uint8_t len;
    } server_ip;

    struct
    {
        volatile uint8_t data[SERVERPORT_MAXLEN + 1]; //服务器端口
        volatile uint8_t len;
    } server_port;

    volatile uint8_t backlight;       //屏幕背光
    volatile uint8_t wlan_power;      //WIFI开关
    volatile uint8_t wallpaper_index; //壁纸的编号
    volatile uint8_t theme_index;     //主题的编号
    volatile uint8_t sysclk;          //系统时钟
    volatile uint8_t system_update;   //系统更新标志位
    volatile uint8_t flag;
} PARAM_SETTING;

static PARAM_SETTING my_setting = {
    {"TP-LINK456", 11},    //ssid
    {"147258369.", 11},    //pwd
    {"192.168.5.178", 14}, //localip
    {"255.255.255.0", 14}, //subnet
    {"192.168.5.1", 12},   //gateway
    {"192.168.5.1", 12},   //dns
    {"192.168.0.110", 14}, //server_ip
    {"1500", 5},           //server_port
    16,                    //backlight
    1,                     //wlan_power
    0,                     //wallpaper_index
    0,                     //theme_index
    1,                     //sysclk
    0,                     //system_update
    0                      //flag
};

void kvdb_type_blob_sample(fdb_kvdb_t kvdb)
{
    struct fdb_blob blob;

    FDB_INFO("==================== kvdb_type_blob_sample ====================\n");

    /* CREATE new Key-Value */

    PARAM_SETTING read_setting = {0};

    fdb_kv_get_blob(kvdb, "setting_param", fdb_blob_make(&blob, &read_setting, sizeof(read_setting)));
    if (blob.saved.len > 0)
    {
        if (read_setting.flag == 0xA5)
        {
            FDB_INFO("setting_param is exists：\n");
            FDB_INFO("ssid: %s\n", read_setting.ssid.data);
            FDB_INFO("pwd: %s\n", read_setting.pwd.data);
            FDB_INFO("local_ip: %s\n", read_setting.localip.data);
            FDB_INFO("subnet: %s\n", read_setting.subnet.data);
            FDB_INFO("gateway: %s\n", read_setting.gateway.data);
            FDB_INFO("dns: %s\n", read_setting.dns.data);
            FDB_INFO("server_ip: %s\n", read_setting.server_ip.data);
            FDB_INFO("server_port: %s\n", read_setting.server_port.data);
            FDB_INFO("wlan_power:%d\n", read_setting.wlan_power);
            FDB_INFO("backlight:%d\n", read_setting.backlight);
            FDB_INFO("flag: %d\n", read_setting.flag);
        }
        else
        {
            FDB_INFO("setting_param is not saved：\n");
            FDB_INFO("flag: %d\n", read_setting.flag);
        }
    }
    else
    {
        FDB_INFO("setting_param is not exists\n");
        FDB_INFO("create the 'setting_param' blob KV\n");
        /* It will create new KV node when "temp" KV not in database.
        * fdb_blob_make: It's a blob make function, and it will return the blob when make finish.
        */
        fdb_kv_set_blob(kvdb, "setting_param", fdb_blob_make(&blob, &my_setting, sizeof(my_setting)));
    }
    
    memset(&read_setting, 0, sizeof(read_setting));
    /* GET the KV value */
    /* get the "setting_param" KV value */
    fdb_kv_get_blob(kvdb, "setting_param", fdb_blob_make(&blob, &read_setting, sizeof(read_setting)));
    if (blob.saved.len > 0)
    {
        FDB_INFO("get the 'setting_param': \n");
        FDB_INFO("ssid: %s\n", read_setting.ssid.data);
        FDB_INFO("pwd: %s\n", read_setting.pwd.data);
        FDB_INFO("local_ip: %s\n", read_setting.localip.data);
        FDB_INFO("subnet: %s\n", read_setting.subnet.data);
        FDB_INFO("gateway: %s\n", read_setting.gateway.data);
        FDB_INFO("dns: %s\n", read_setting.dns.data);
        FDB_INFO("server_ip: %s\n", read_setting.server_ip.data);
        FDB_INFO("server_port: %s\n", read_setting.server_port.data);
        FDB_INFO("wlan_power:%d\n", read_setting.wlan_power);
        FDB_INFO("my_setting.backlight:%d\n", read_setting.backlight);
        FDB_INFO("my_setting.flag: %d\n", read_setting.flag);
        if (read_setting.flag != 0xA5)
        {
            /* CHANGE the KV value */
            read_setting.flag = 0xA5;
            read_setting.ssid.len = sizeof("tp_link");
            memcpy((void *)read_setting.ssid.data, "tp_link", read_setting.ssid.len);

            /* change the "temp" KV's value to 38 */
            fdb_kv_set_blob(kvdb, "setting_param", fdb_blob_make(&blob, &read_setting, sizeof(read_setting)));
            FDB_INFO("set 'setting_param.ssid.data' value to %s\n", "tp_link");
        }
    }

    // { /* DELETE the KV by name */
    //     FDB_INFO("delete the 'setting_param' finish\n");
    //     fdb_kv_del(kvdb, "setting_param");
    // }

    FDB_INFO("===========================================================\n");
}

#endif /* FDB_USING_KVDB */
