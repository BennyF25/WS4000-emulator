// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_sci.h

    H8 Serial Communications Interface


***************************************************************************/

#ifndef MAME_CPU_H8_H8_SCI_H
#define MAME_CPU_H8_H8_SCI_H

#pragma once

#include "h8.h"
#include "h8_intc.h"

class h8_sci_device : public device_t {
public:
	h8_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc, int eri, int rxi, int txi, int tei)
		: h8_sci_device(mconfig, tag, owner, 0)
	{
		set_info(intc, eri, rxi, txi, tei);
	}

	void set_info(const char *intc, int eri, int rxi, int txi, int tei);
	void set_external_clock_period(const attotime &_period);

	void smr_w(uint8_t data);
	uint8_t smr_r();
	void brr_w(uint8_t data);
	uint8_t brr_r();
	void scr_w(uint8_t data);
	uint8_t scr_r();
	void tdr_w(uint8_t data);
	uint8_t tdr_r();
	void ssr_w(uint8_t data);
	uint8_t ssr_r();
	uint8_t rdr_r();
	void scmr_w(uint8_t data);
	uint8_t scmr_r();

	void rx_w(int state);
	void clk_w(int state);

	auto tx_handler() { return m_tx_cb.bind(); }
	auto clk_handler() { return m_clk_cb.bind(); }

	uint64_t internal_update(uint64_t current_time);

protected:
	enum {
		ST_IDLE, ST_START, ST_BIT, ST_PARITY, ST_STOP, ST_LAST_TICK
	};

	static const char *const state_names[];

	enum {
		CLK_TX = 1,
		CLK_RX = 2
	};

	enum class clock_mode_t {
		INTERNAL_ASYNC,
		INTERNAL_ASYNC_OUT,
		EXTERNAL_ASYNC,
		EXTERNAL_RATE_ASYNC,
		INTERNAL_SYNC_OUT,
		EXTERNAL_SYNC,
		EXTERNAL_RATE_SYNC
	};

	enum {
		SMR_CA   = 0x80,
		SMR_CHR  = 0x40,
		SMR_PE   = 0x20,
		SMR_OE   = 0x10,
		SMR_STOP = 0x08,
		SMR_MP   = 0x04,
		SMR_CKS  = 0x03,

		SCR_TIE  = 0x80,
		SCR_RIE  = 0x40,
		SCR_TE   = 0x20,
		SCR_RE   = 0x10,
		SCR_MPIE = 0x08,
		SCR_TEIE = 0x04,
		SCR_CKE  = 0x03,
		SCR_CKE1 = 0x02,
		SCR_CKE0 = 0x01,

		SSR_TDRE = 0x80,
		SSR_RDRF = 0x40,
		SSR_ORER = 0x20,
		SSR_FER  = 0x10,
		SSR_PER  = 0x08,
		SSR_TEND = 0x04,
		SSR_MPB  = 0x02,
		SSR_MPBT = 0x01
	};

	required_device<h8_device> m_cpu;
	devcb_write_line m_tx_cb, m_clk_cb;
	h8_intc_device *m_intc;
	const char *m_intc_tag;
	attotime m_external_clock_period, m_cur_sync_time;
	double m_external_to_internal_ratio, m_internal_to_external_ratio;
	emu_timer *m_sync_timer;

	int m_eri_int, m_rxi_int, m_txi_int, m_tei_int;

	int m_tx_state, m_rx_state, m_tx_bit, m_rx_bit, m_clock_state, m_tx_parity, m_rx_parity, m_ext_clock_counter;
	clock_mode_t m_clock_mode;
	bool m_clock_value, m_ext_clock_value, m_rx_value;

	uint8_t m_rdr, m_tdr, m_smr, m_scr, m_ssr, m_brr, m_rsr, m_tsr;
	uint64_t m_clock_base, m_divider;

	std::string m_last_clock_message;

	void device_start() override;
	void device_reset() override;
	void device_post_load() override;

	TIMER_CALLBACK_MEMBER(sync_tick);

	void clock_start(int mode);
	void clock_stop(int mode);
	void clock_update();
	void tx_start();
	void tx_dropped_edge();
	void rx_start();
	void rx_done();
	void rx_raised_edge();

	bool is_sync_start() const;
	bool has_recv_error() const;
};

DECLARE_DEVICE_TYPE(H8_SCI, h8_sci_device)

#endif // MAME_CPU_H8_H8_SCI_H
