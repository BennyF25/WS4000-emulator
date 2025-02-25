// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2357.h

    H8S-2357 family emulation

    H8S/2000-based mcus.

    Variant           ROM        RAM
    H8S/2357         128K         8K
    H8S/2352         -            8K
    H8S/2398         256K         8K
    H8S/2394         -           32K
    H8S/2392         -            8K
    H8S/2390         -            4K



***************************************************************************/

#ifndef MAME_CPU_H8_H8S2357_H
#define MAME_CPU_H8_H8S2357_H

#pragma once

#include "h8s2000.h"
#include "h8_intc.h"
#include "h8_adc.h"
#include "h8_port.h"
#include "h8_timer8.h"
#include "h8_timer16.h"
#include "h8_sci.h"
#include "h8_watchdog.h"

class h8s2357_device : public h8s2000_device {
public:
	h8s2357_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t syscr_r();
	void syscr_w(uint8_t data);

protected:
	required_device<h8s_intc_device> m_intc;
	required_device<h8_adc_device> m_adc;
	required_device<h8_port_device> m_port1;
	required_device<h8_port_device> m_port2;
	required_device<h8_port_device> m_port3;
	required_device<h8_port_device> m_port4;
	required_device<h8_port_device> m_port5;
	required_device<h8_port_device> m_port6;
	required_device<h8_port_device> m_porta;
	required_device<h8_port_device> m_portb;
	required_device<h8_port_device> m_portc;
	required_device<h8_port_device> m_portd;
	required_device<h8_port_device> m_porte;
	required_device<h8_port_device> m_portf;
	required_device<h8_port_device> m_portg;
	required_device<h8h_timer8_channel_device> m_timer8_0;
	required_device<h8h_timer8_channel_device> m_timer8_1;
	required_device<h8_timer16_device> m_timer16;
	required_device<h8s_timer16_channel_device> m_timer16_0;
	required_device<h8s_timer16_channel_device> m_timer16_1;
	required_device<h8s_timer16_channel_device> m_timer16_2;
	required_device<h8s_timer16_channel_device> m_timer16_3;
	required_device<h8s_timer16_channel_device> m_timer16_4;
	required_device<h8s_timer16_channel_device> m_timer16_5;
	required_device<h8_sci_device> m_sci0;
	required_device<h8_sci_device> m_sci1;
	required_device<h8_sci_device> m_sci2;
	required_device<h8_watchdog_device> m_watchdog;

	uint32_t m_ram_start;
	unsigned char m_syscr;

	h8s2357_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t start);

	virtual bool exr_in_stack() const override;
	virtual void update_irq_filter() override;
	virtual void interrupt_taken() override;
	virtual int trace_setup() override;
	virtual int trapa_setup() override;
	virtual void irq_setup() override;
	virtual void internal_update(uint64_t current_time) override;
	virtual void device_add_mconfig(machine_config &config) override;
	void map(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
};

class h8s2352_device : public h8s2357_device {
public:
	h8s2352_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h8s2398_device : public h8s2357_device {
public:
	h8s2398_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h8s2394_device : public h8s2357_device {
public:
	h8s2394_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h8s2392_device : public h8s2357_device {
public:
	h8s2392_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class h8s2390_device : public h8s2357_device {
public:
	h8s2390_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(H8S2357, h8s2357_device)
DECLARE_DEVICE_TYPE(H8S2352, h8s2352_device)
DECLARE_DEVICE_TYPE(H8S2398, h8s2398_device)
DECLARE_DEVICE_TYPE(H8S2394, h8s2394_device)
DECLARE_DEVICE_TYPE(H8S2392, h8s2392_device)
DECLARE_DEVICE_TYPE(H8S2390, h8s2390_device)

#endif // MAME_CPU_H8_H8S2357_H
