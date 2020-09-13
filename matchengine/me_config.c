/*
 * Description: 
 *     History: yang@haipo.me, 2017/03/16, create
 */

# include "me_config.h"

struct settings settings;
const char *g_path;

static int load_assets(json_t *root, const char *key)
{
    json_t *node = json_object_get(root, key);
    if (!node || !json_is_array(node)) {
        return -__LINE__;
    }

    settings.asset_num = json_array_size(node);
    settings.assets = malloc(sizeof(struct asset) * MAX_ASSET_LIST);
    for (size_t i = 0; i < settings.asset_num; ++i) {
        json_t *row = json_array_get(node, i);
        if (!json_is_object(row))
            return -__LINE__;
        ERR_RET_LN(read_cfg_str(row, "name", &settings.assets[i].name, NULL));
        ERR_RET_LN(read_cfg_int(row, "prec_save", &settings.assets[i].prec_save, true, 0));
        ERR_RET_LN(read_cfg_int(row, "prec_show", &settings.assets[i].prec_show, false, settings.assets[i].prec_save));
        if (strlen(settings.assets[i].name) > ASSET_NAME_MAX_LEN)
            return -__LINE__;
    }

    return 0;
}

static int load_markets(json_t *root, const char *key)
{
    json_t *node = json_object_get(root, key);
    if (!node || !json_is_array(node)) {
        return -__LINE__;
    }

    settings.market_num = json_array_size(node);
    settings.markets = malloc(sizeof(struct market) * MAX_MARKET_LIST);
    for (size_t i = 0; i < settings.market_num; ++i) {
        json_t *row = json_array_get(node, i);
        if (!json_is_object(row))
            return -__LINE__;
        ERR_RET_LN(read_cfg_str(row, "name", &settings.markets[i].name, NULL));
        ERR_RET_LN(read_cfg_int(row, "fee_prec", &settings.markets[i].fee_prec, false, 4));
        ERR_RET_LN(read_cfg_mpd(row, "min_amount", &settings.markets[i].min_amount, "0.01"));

        json_t *stock = json_object_get(row, "stock");
        if (!stock || !json_is_object(stock))
            return -__LINE__;
        ERR_RET_LN(read_cfg_str(stock, "name", &settings.markets[i].stock, NULL));
        ERR_RET_LN(read_cfg_int(stock, "prec", &settings.markets[i].stock_prec, true, 0));

        json_t *money = json_object_get(row, "money");
        if (!money || !json_is_object(money))
            return -__LINE__;
        ERR_RET_LN(read_cfg_str(money, "name", &settings.markets[i].money, NULL));
        ERR_RET_LN(read_cfg_int(money, "prec", &settings.markets[i].money_prec, true, 0));
    }

    return 0;
}

static int read_config_from_json(json_t *root)
{
    int ret;
    ret = read_cfg_bool(root, "debug", &settings.debug, false, false);
    if (ret < 0) {
        printf("read debug config fail: %d\n", ret);
        return -__LINE__;
    }
    ret = load_cfg_process(root, "process", &settings.process);
    if (ret < 0) {
        printf("load process config fail: %d\n", ret);
        return -__LINE__;
    }
    ret = load_cfg_log(root, "log", &settings.log);
    if (ret < 0) {
        printf("load log config fail: %d\n", ret);
        return -__LINE__;
    }
    ret = load_cfg_alert(root, "alert", &settings.alert);
    if (ret < 0) {
        printf("load alert config fail: %d\n", ret);
        return -__LINE__;
    }
    ret = load_cfg_rpc_svr(root, "svr", &settings.svr);
    if (ret < 0) {
        printf("load svr config fail: %d\n", ret);
        return -__LINE__;
    }
    ret = load_cfg_cli_svr(root, "cli", &settings.cli);
    if (ret < 0) {
        printf("load cli config fail: %d\n", ret);
        return -__LINE__;
    }
    ret = load_cfg_mysql(root, "db_log", &settings.db_log);
    if (ret < 0) {
        printf("load log db config fail: %d\n", ret);
        return -__LINE__;
    }
    ret = load_cfg_mysql(root, "db_history", &settings.db_history);
    if (ret < 0) {
        printf("load history db config fail: %d\n", ret);
        return -__LINE__;
    }
    ret = load_assets(root, "assets");
    if (ret < 0) {
        printf("load assets config fail: %d\n", ret);
        return -__LINE__;
    }
    ret = load_markets(root, "markets");
    if (ret < 0) {
        printf("load markets config fail: %d\n", ret);
        return -__LINE__;
    }
    ret = read_cfg_str(root, "brokers", &settings.brokers, NULL);
    if (ret < 0) {
        printf("load brokers fail: %d\n", ret);
        return -__LINE__;
    }
    ret = read_cfg_int(root, "slice_interval", &settings.slice_interval, false, 86400);
    if (ret < 0) {
        printf("load slice_interval fail: %d", ret);
        return -__LINE__;
    }
    ret = read_cfg_int(root, "slice_keeptime", &settings.slice_keeptime, false, 86400 * 3);
    if (ret < 0) {
        printf("load slice_keeptime fail: %d", ret);
        return -__LINE__;
    }
    ret = read_cfg_int(root, "history_thread", &settings.history_thread, false, 10);
    if (ret < 0) {
        printf("load history_thread fail: %d", ret);
        return -__LINE__;
    }
    
    ret = read_cfg_int(root, "bot_user_id", &settings.bot_user_id, false, 10);
    if (ret < 0) {
        printf("load bot_user_id fail: %d", ret);
        return -__LINE__;
    }

    ERR_RET_LN(read_cfg_real(root, "cache_timeout", &settings.cache_timeout, false, 0.45));

    return 0;
}

static int load_asset_by_name(json_t *root, const char *asset_name)
{
    int last_num = -1;
    for (int i = 0; i < settings.asset_num; ++i) {
        if (strcmp(settings.assets[i].name, asset_name) == 0) {
            return 0;
        }
    }

    json_t *node = json_object_get(root, "assets");
    if (!node || !json_is_array(node)) {
        return -__LINE__;
    }

    size_t node_num = json_array_size(node);
    char *node_name = NULL;
    for (size_t i = 0; i < node_num; ++i) {
        json_t *row = json_array_get(node, i);
        if (!json_is_object(row))
            return -__LINE__;
        
        ERR_RET_LN(read_cfg_str(row, "name", &node_name, NULL));
        if (strcmp(node_name, asset_name) == 0) {
            if (strlen(node_name) > ASSET_NAME_MAX_LEN)
                return -__LINE__;
            settings.asset_num = settings.asset_num + 1;
            last_num = settings.asset_num;
            ERR_RET_LN(read_cfg_str(row, "name", &settings.assets[last_num - 1].name, NULL));
            ERR_RET_LN(read_cfg_int(row, "prec_save", &settings.assets[last_num - 1].prec_save, true, 0));
            ERR_RET_LN(read_cfg_int(row, "prec_show", &settings.assets[last_num - 1].prec_show, false, settings.assets[last_num - 1].prec_save));
        }
    }

    return last_num;
}

static int load_market_by_name(json_t *root, const char *market_name)
{
    int last_num = -1;
    for (int i = 0; i < settings.market_num; ++i) {
        if (strcmp(settings.markets[i].name, market_name) == 0) {
            return 0;
        }
    }

    json_t *node = json_object_get(root, "markets");
    if (!node || !json_is_array(node)) {
        return -__LINE__;
    }

    size_t node_num = json_array_size(node);
    char *node_name = NULL;
    for (size_t i = 0; i < node_num; ++i) {
        json_t *row = json_array_get(node, i);
        if (!json_is_object(row))
            return -__LINE__;
        
        ERR_RET_LN(read_cfg_str(row, "name", &node_name, NULL));
        if (strcmp(node_name, market_name) == 0) {
            settings.market_num = settings.market_num + 1;
            last_num = settings.market_num;

            ERR_RET_LN(read_cfg_str(row, "name", &settings.markets[last_num - 1].name, NULL));
            ERR_RET_LN(read_cfg_int(row, "fee_prec", &settings.markets[last_num - 1].fee_prec, false, 4));
            ERR_RET_LN(read_cfg_mpd(row, "min_amount", &settings.markets[last_num - 1].min_amount, "0.01"));

            json_t *stock = json_object_get(row, "stock");
            if (!stock || !json_is_object(stock))
                return -__LINE__;
            ERR_RET_LN(read_cfg_str(stock, "name", &settings.markets[last_num - 1].stock, NULL));
            ERR_RET_LN(read_cfg_int(stock, "prec", &settings.markets[last_num - 1].stock_prec, true, 0));

            json_t *money = json_object_get(row, "money");
            if (!money || !json_is_object(money))
                return -__LINE__;
            ERR_RET_LN(read_cfg_str(money, "name", &settings.markets[last_num - 1].money, NULL));
            ERR_RET_LN(read_cfg_int(money, "prec", &settings.markets[last_num - 1].money_prec, true, 0));
        }
    }

    return last_num;
}

int init_config(const char *path)
{
    json_error_t error;
    json_t *root = json_load_file(path, 0, &error);
    if (root == NULL) {
        printf("json_load_file from: %s fail: %s in line: %d\n", path, error.text, error.line);
        return -__LINE__;
    }
    if (!json_is_object(root)) {
        json_decref(root);
        return -__LINE__;
    }

    g_path = path;

    int ret = read_config_from_json(root);
    if (ret < 0) {
        json_decref(root);
        return ret;
    }
    json_decref(root);

    return 0;
}

int get_asset_config(const char *asset_name)
{
    json_error_t error;
    json_t *root = json_load_file(g_path, 0, &error);
    if (root == NULL) {
        log_fatal("json_load_file from: %s fail: %s in line: %d", g_path, error.text, error.line);
        return -__LINE__;
    }
    if (!json_is_object(root)) {
        json_decref(root);
        return -__LINE__;
    }

    int ret = load_asset_by_name(root, asset_name);
    if (ret < 0) {
        log_fatal("load_asset_by_name: %s fail", asset_name);
        json_decref(root);
        return ret;
    }

    json_decref(root);

    return ret;
}

int get_market_config(const char *market_name)
{
    json_error_t error;
    json_t *root = json_load_file(g_path, 0, &error);
    if (root == NULL) {
        log_fatal("json_load_file from: %s fail: %s in line: %d", g_path, error.text, error.line);
        return -__LINE__;
    }
    if (!json_is_object(root)) {
        json_decref(root);
        return -__LINE__;
    }

    int ret = load_market_by_name(root, market_name);
    if (ret < 0) {
        log_fatal("load_market_by_name: %s fail", market_name);
        json_decref(root);
        return ret;
    }

    json_decref(root);

    return ret;
}
