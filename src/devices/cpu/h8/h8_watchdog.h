/***************************************************************************

    h8_watchdog.h

    H8 watchdog/timer

****************************************************************************

    Copyright Olivier Galibert
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY OLIVIER GALIBERT ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL OLIVIER GALIBERT BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#ifndef MAME_CPU_H8_H8_WATCHDOG_H
#define MAME_CPU_H8_H8_WATCHDOG_H

#pragma once

#include "h8.h"
#include "h8_intc.h"

class h8_watchdog_device : public device_t {
public:
	enum { B, H, S };

	h8_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_watchdog_device(const machine_config &mconfig, const char *tag, device_t *owner, const char *intc, int irq, int type)
		: h8_watchdog_device(mconfig, tag, owner, 0)
	{
		set_info(intc, irq, type);
	}

	void set_info(const char *intc, int irq, int type);

	uint64_t internal_update(uint64_t current_time);

	uint16_t wd_r();
	void wd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t rst_r();
	void rst_w(uint16_t data);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum {
		TCSR_CKS  = 0x07,
		TCSR_NMI  = 0x08,
		TCSR_TME  = 0x20,
		TCSR_WT   = 0x40,
		TCSR_OVF  = 0x80,

		RST_RSTS  = 0x20,
		RST_RSTE  = 0x40,
		RST_RSTEO = 0x40,
		RST_WRST  = 0x80
	};

	static const int div_bh[8];
	static const int div_s[8];

	required_device<h8_device> m_cpu;
	h8_intc_device *m_intc;
	const char *m_intc_tag;
	int m_irq;
	int m_type;
	uint8_t m_tcnt, m_tcsr, m_rst;
	uint64_t m_tcnt_cycle_base;

	void tcnt_update(uint64_t current_time = 0);
};

DECLARE_DEVICE_TYPE(H8_WATCHDOG, h8_watchdog_device)

#endif // MAME_CPU_H8_H8_WATCHDOG_H
