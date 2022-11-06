#include "conf.h"
#include "toml.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define LOG_MODULE_NAME "ujin"
#define MAX_LOG_LEVEL DEBUG_LOG_LEVEL
#include "logger.h"

#define DEFAULT_CONF_FILE "/etc/ujind.conf"

static toml_table_t* conf_tbl;

static toml_table_t* _parse(const char *conf_file)
{
    FILE *fp;
    char errbuf[200] = {0};

    if (conf_tbl) {
	toml_free(conf_tbl);
	conf_tbl = NULL;
    }

    fp = fopen(conf_file, "r");
    if (!fp) {
	fprintf(stderr, "cannot open conf file: %s\n", strerror(errno));
    }

    conf_tbl = toml_parse_file(fp, errbuf, sizeof(errbuf));

    fclose(fp);

    if (!conf_tbl) {
	fprintf(stderr, "cannot parse toml: %s\n", errbuf);
    }

    return conf_tbl;
}

#define CHECK_TBL_INIT()				\
    do {						\
	if (!conf_tbl) {				\
	    conf_tbl = _parse(DEFAULT_CONF_FILE);	\
	}						\
    } while (0);


char *conf__server_host()
{
    CHECK_TBL_INIT();

    toml_datum_t host = toml_string_in(toml_table_in(conf_tbl, "server"), "host");
    if (!host.ok) {
	LOGW("cannot read server.host\n");
    } else {
	LOGD("server.host: %s\n", host.u.s);
    }

    return host.u.s;
}


int conf__server_port()
{
    CHECK_TBL_INIT();

    toml_datum_t port = toml_int_in(toml_table_in(conf_tbl, "server"), "port");
    if (!port.ok) {
	LOGW("cannot read server.port\n");
    } else {
	LOGD("server.port: %d\n", port.u.b);
    }

    return port.u.b;
}


char *conf__mqtt_broker_host()
{
    CHECK_TBL_INIT();

    toml_datum_t host = toml_string_in(toml_table_in(conf_tbl, "mqtt_broker"), "host");
    if (!host.ok) {
	LOGW("cannot read mqtt_broker.host\n");
    } else {
	LOGD("mqtt_broker.host: %s\n", host.u.s);
    }

    return host.u.s;
}


char *conf__lte_modem_fake_number()
{
    CHECK_TBL_INIT();

    toml_datum_t number = toml_string_in(toml_table_in(conf_tbl, "lte_modem"), "fake_number");
    if (!number.ok) {
	LOGW("cannot read lte_modem.fake_number\n");
    } else {
	LOGD("lte_modem.fake_number: %s\n", number.u.s);
    }

    return number.u.s;
}

char *conf__lte_modem_serial_port()
{
    CHECK_TBL_INIT();

    toml_datum_t port = toml_string_in(toml_table_in(conf_tbl, "lte_modem"), "serial_port");
    if (!port.ok) {
	LOGW("cannot read lte_modem.serial_port\n");
    } else {
	LOGD("lte_modem.serial_port: %s\n", port.u.s);
    }

    return port.u.s;
}

int conf__lte_modem_serial_baudrate()
{
    CHECK_TBL_INIT();

    const toml_table_t *arr = toml_table_in(conf_tbl, "lte_modem");
    toml_datum_t baudrate = toml_int_in(arr, "serial_baudrate");
    if (!baudrate.ok) {
	LOGW("cannot read lte_modem.serial_baudrate\n");
    } else {
	LOGD("lte_modem.serial_baudrate: %d\n", baudrate.u.b);
    }

    return baudrate.u.b;
}

char *conf__audiosel_serial_port()
{
    CHECK_TBL_INIT();

    toml_datum_t port = toml_string_in(toml_table_in(conf_tbl, "audiosel"), "serial_port");
    if (!port.ok) {
	LOGW("cannot read audiosel.serial_port\n");
    } else {
	LOGD("audiosel.serial_port: %s\n", port.u.s);
    }

    return port.u.s;
}

int conf__audiosel_serial_baudrate()
{
    CHECK_TBL_INIT();

    const toml_table_t *arr = toml_table_in(conf_tbl, "audiosel");
    toml_datum_t baudrate = toml_int_in(arr, "serial_baudrate");
    if (!baudrate.ok) {
	LOGW("cannot read audiosel.serial_baudrate\n");
    } else {
	LOGD("audiosel.serial_baudrate: %d\n", baudrate.u.b);
    }

    return baudrate.u.b;
}
