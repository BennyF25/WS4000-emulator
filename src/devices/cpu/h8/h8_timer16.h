// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_timer16.h

    H8 16 bits timer


***************************************************************************/

#ifndef MAME_CPU_H8_H8_TIMER16_H
#define MAME_CPU_H8_H8_TIMER16_H

#pragma once

#include "h8.h"
#include "h8_intc.h"

class h8_timer16_channel_device : public device_t {
public:
	enum {
		CHAIN,
		INPUT_A,
		INPUT_B,
		INPUT_C,
		INPUT_D,
		DIV_1,
		DIV_2,
		DIV_4,
		DIV_8,
		DIV_16,
		DIV_32,
		DIV_64,
		DIV_128,
		DIV_256,
		DIV_512,
		DIV_1024,
		DIV_2048,
		DIV_4096
	};

	enum {
		TGR_CLEAR_NONE = -1,
		TGR_CLEAR_EXT  = -2
	};

	enum {
		IRQ_A = 0x01,
		IRQ_B = 0x02,
		IRQ_C = 0x04,
		IRQ_D = 0x08,
		IRQ_V = 0x10,
		IRQ_U = 0x20,
		IRQ_TRIG = 0x40
	};


	h8_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, int tgr_count, int tbr_count, const char *intc, int irq_base)
		: h8_timer16_channel_device(mconfig, tag, owner, 0)
	{
		set_info(tgr_count, tbr_count, intc, irq_base);
	}

	void set_info(int tgr_count, int tbr_count, const char *intc, int irq_base);

	uint8_t tcr_r();
	void tcr_w(uint8_t data);
	uint8_t tmdr_r();
	void tmdr_w(uint8_t data);
	uint8_t tior_r();
	void tior_w(offs_t offset, uint8_t data);
	uint8_t tier_r();
	void tier_w(uint8_t data);
	uint8_t tsr_r();
	void tsr_w(uint8_t data);
	uint16_t tcnt_r();
	void tcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tgr_r(offs_t offset);
	void tgr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tbr_r(offs_t offset);
	void tbr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint64_t internal_update(uint64_t current_time);
	void set_ier(uint8_t value);
	void set_enable(bool enable);
	void tisr_w(int offset, uint8_t data);
	uint8_t tisr_r(int offset) const;

protected:
	required_device<h8_device> m_cpu;
	h8_timer16_channel_device *m_chained_timer;
	h8_intc_device *m_intc;
	const char *m_chain_tag, *m_intc_tag;
	int m_interrupt[6];
	uint8_t m_tier_mask;

	int m_tgr_count, m_tbr_count;
	int m_tgr_clearing;
	uint8_t m_tcr, m_tier, m_ier, m_isr;
	int m_clock_type, m_clock_divider;
	uint16_t m_tcnt, m_tgr[6];
	uint64_t m_last_clock_update, m_event_time;
	uint32_t m_phase, m_counter_cycle;
	bool m_counter_incrementing;
	bool m_channel_active;

	h8_timer16_channel_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	void update_counter(uint64_t cur_time = 0);
	void recalc_event(uint64_t cur_time = 0);
	virtual void tcr_update();
	virtual void tier_update();
	virtual void isr_update(uint8_t value);
	virtual uint8_t isr_to_sr() const;
};

class h8h_timer16_channel_device : public h8_timer16_channel_device {
public:
	h8h_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8h_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, int tgr_count, int tbr_count, const char *intc, int irq_base)
		: h8h_timer16_channel_device(mconfig, tag, owner, 0)
	{
		set_info(tgr_count, tbr_count, intc, irq_base);
	}
	virtual ~h8h_timer16_channel_device();

	void set_info(int tgr_count, int tbr_count, const char *intc, int irq_base);

protected:
	virtual void tcr_update() override;
	virtual void tier_update() override;
	virtual void isr_update(uint8_t value) override;
	virtual uint8_t isr_to_sr() const override;
};

class h8s_timer16_channel_device : public h8_timer16_channel_device {
public:
	h8s_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8s_timer16_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, int tgr_count, int _tier_mask, const char *intc, int irq_base,
					int t0, int t1, int t2, int t3, int t4, int t5, int t6, int t7)
		: h8s_timer16_channel_device(mconfig, tag, owner, 0)
	{
		set_info(tgr_count, _tier_mask, intc, irq_base, t0, t1, t2, t3, t4, t5, t6, t7);
	}
	virtual ~h8s_timer16_channel_device();

	void set_info(int tgr_count, uint8_t _tier_mask, const char *intc, int irq_base,
					int t0, int t1, int t2, int t3, int t4, int t5, int t6, int t7);
	void set_chain(const char *chain_tag);

protected:
	int m_count_types[8];

	virtual void tcr_update() override;
	virtual void tier_update() override;
	virtual void isr_update(uint8_t value) override;
	virtual uint8_t isr_to_sr() const override;
};

class h8_timer16_device : public device_t {
public:
	h8_timer16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_timer16_device(const machine_config &mconfig, const char *tag, device_t *owner, int timer_count, uint8_t default_tstr)
		: h8_timer16_device(mconfig, tag, owner, 0)
	{
		set_info(timer_count, default_tstr);
	}
	void set_info(int timer_count, uint8_t default_tstr);

	uint8_t tstr_r();
	void tstr_w(uint8_t data);
	uint8_t tsyr_r();
	void tsyr_w(uint8_t data);
	uint8_t tmdr_r();
	void tmdr_w(uint8_t data);
	uint8_t tfcr_r();
	void tfcr_w(uint8_t data);
	uint8_t toer_r();
	void toer_w(uint8_t data);
	uint8_t tocr_r();
	void tocr_w(uint8_t data);
	uint8_t tisr_r(offs_t offset);
	void tisr_w(offs_t offset, uint8_t data);
	uint8_t tisrc_r();
	void tisrc_w(uint8_t data);
	void tolr_w(uint8_t data);

protected:
	required_device<h8_device> m_cpu;
	h8_timer16_channel_device *m_timer_channel[6];
	int m_timer_count;
	uint8_t m_default_tstr;
	uint8_t m_tstr;

	virtual void device_start() override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(H8_TIMER16,          h8_timer16_device)
DECLARE_DEVICE_TYPE(H8_TIMER16_CHANNEL,  h8_timer16_channel_device)
DECLARE_DEVICE_TYPE(H8H_TIMER16_CHANNEL, h8h_timer16_channel_device)
DECLARE_DEVICE_TYPE(H8S_TIMER16_CHANNEL, h8s_timer16_channel_device)

#endif // MAME_CPU_H8_H8_TIMER16_H
