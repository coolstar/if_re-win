#pragma once

#define BSD_NT_WRAP(x) (x == 0 ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL)

int re_check_mac_version(struct re_softc* sc);
void re_init_software_variable(struct re_softc* sc);
void re_exit_oob(struct re_softc* sc);
void re_hw_init(struct re_softc* sc);
void re_reset(struct re_softc* sc);
void re_get_hw_mac_address(struct re_softc* sc, u_int8_t* eaddr);
void re_phy_power_up(struct re_softc* sc);
void re_hw_phy_config(struct re_softc* sc);

void re_hw_start_unlock(struct re_softc* sc);
void re_hw_start_unlock_8125(struct re_softc* sc);
u_int8_t re_link_ok(struct re_softc* sc);

void re_link_on_patch(struct re_softc* sc);
void re_stop(struct re_softc* sc);

void re_hw_d3_para(struct re_softc* sc);

int re_ifmedia_upd(struct re_softc* sc);
int re_ifmedia_upd_8125(struct re_softc* sc);