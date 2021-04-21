#include <Arduino.h>
#include <flashdb.h>
#include <drv_spi_flash.h>

#define FDB_LOG_TAG "[main]"

static uint32_t boot_count = 0;
static time_t boot_time[10] = {0, 1, 2, 3};
/* default KV nodes */
static struct fdb_default_kv_node default_kv_table[] = {
    {"username", (void *)"armink", 0},                       /* string KV */
    {"password", (void *)"123456", 0},                       /* string KV */
    {"boot_count", (void *)&boot_count, sizeof(boot_count)}, /* int type KV */
    {"boot_time", (void *)&boot_time, sizeof(boot_time)},    /* int array type KV */
};
/* KVDB object */
static struct fdb_kvdb kvdb = {0};
/* TSDB object */
struct fdb_tsdb tsdb = {0};
/* counts for simulated timestamp */
static int counts = 0;

extern "C" {
extern void kvdb_basic_sample(fdb_kvdb_t kvdb);
extern void kvdb_type_string_sample(fdb_kvdb_t kvdb);
extern void kvdb_type_blob_sample(fdb_kvdb_t kvdb);
extern void tsdb_sample(fdb_tsdb_t tsdb);
}

static void lock(fdb_db_t db)
{
  __disable_irq();
}

static void unlock(fdb_db_t db)
{
  __enable_irq();
}

static fdb_time_t get_time(void)
{
  /* Using the counts instead of timestamp.
     * Please change this function to return RTC time.
     */
  return ++counts;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  spi_flash_init();
  fdb_err_t result;

#ifdef FDB_USING_KVDB
  { /* KVDB Sample */
    struct fdb_default_kv default_kv;

    default_kv.kvs = default_kv_table;
    default_kv.num = sizeof(default_kv_table) / sizeof(default_kv_table[0]);
    /* set the lock and unlock function if you want */
    fdb_kvdb_control(&kvdb, FDB_KVDB_CTRL_SET_LOCK, (void *)lock);
    fdb_kvdb_control(&kvdb, FDB_KVDB_CTRL_SET_UNLOCK, (void *)unlock);
    /* Key-Value database initialization
         *
         *       &kvdb: database object
         *       "env": database name
         * "fdb_kvdb1": The flash partition name base on FAL. Please make sure it's in FAL partition table.
         *              Please change to YOUR partition name.
         * &default_kv: The default KV nodes. It will auto add to KVDB when first initialize successfully.
         *        NULL: The user data if you need, now is empty.
         */
    result = fdb_kvdb_init(&kvdb, "env", "fdb_kvdb1", &default_kv, NULL);

    if (result != FDB_NO_ERR)
    {
      return;
    }

    /* run basic KV samples */
    kvdb_basic_sample(&kvdb);
    /* run string KV samples */
    kvdb_type_string_sample(&kvdb);
    /* run blob KV samples */
    kvdb_type_blob_sample(&kvdb);
  }
#endif /* FDB_USING_KVDB */

#ifdef FDB_USING_TSDB
  { /* TSDB Sample */
    /* set the lock and unlock function if you want */
    fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_SET_LOCK, (void *)lock);
    fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_SET_UNLOCK, (void *)unlock);
    /* Time series database initialization
         *
         *       &tsdb: database object
         *       "log": database name
         * "fdb_tsdb1": The flash partition name base on FAL. Please make sure it's in FAL partition table.
         *              Please change to YOUR partition name.
         *    get_time: The get current timestamp function.
         *         128: maximum length of each log
         *        NULL: The user data if you need, now is empty.
         */
    result = fdb_tsdb_init(&tsdb, "dev_data", "fdb_tsdb1", get_time, 128, NULL);
    /* read last saved time for simulated timestamp */
    fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_GET_LAST_TIME, &counts);

    if (result != FDB_NO_ERR)
    {
      return;
    }

    /* run TSDB sample */
    tsdb_sample(&tsdb);
  }
#endif /* FDB_USING_TSDB */
}

void loop()
{
  // put your main code here, to run repeatedly:
}