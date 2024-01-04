#pragma once

#define BSD_NT_WRAP(x) (x == 0 ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL)

int re_check_mac_version(struct re_softc* sc);
void re_init_software_variable(struct re_softc* sc);
void re_exit_oob(struct re_softc* sc);
void re_hw_init(struct re_softc* sc);
void re_reset(struct re_softc* sc);
void re_get_hw_mac_address(struct re_softc* sc, u_int8_t* eaddr);