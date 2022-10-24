#ifndef CONF_H
#define CONF_H

char* conf__server_host();
int conf__server_port();

char* conf__mqtt_broker_host();

char* conf__lte_modem_fake_number();
char* conf__lte_modem_serial_port();
int conf__lte_modem_serial_baudrate();

#endif /* CONF_H */
