// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    7400 Quad 2-Input NAND Gate

*****************************************************************************/

#include "emu.h"
#include "7400.h"

DEFINE_DEVICE_TYPE(TTL7400, ttl7400_device, "7400", "7400 Quad 2-Input NAND Gate")

ttl7400_device::ttl7400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TTL7400, tag, owner, clock)
	, m_y_func(*this)
	, m_a(0)
	, m_b(0)
	, m_y(0)
{
}

void ttl7400_device::device_start()
{
	m_y_func.resolve_all_safe();

	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_y));
}

void ttl7400_device::device_reset()
{
	m_a = 0;
	m_b = 0;
	m_y = 0;
}

void ttl7400_device::update()
{
	uint8_t last_y = m_y;

	m_y = (m_a & m_b) & 0xf;

	if (m_y != last_y)
	{
		for (std::size_t bit = 0; bit < 4; bit++)
		{
			if (BIT(m_y, bit) == BIT(last_y, bit))
				continue;

			m_y_func[bit](BIT(m_y, bit));
		}
	}
}

void ttl7400_device::a_w(uint8_t line, uint8_t state)
{
	uint8_t old_a = m_a;
	m_a &= ~(1 << line);
	m_a |= (state << line);
	if (old_a != m_a)
		update();
}

void ttl7400_device::b_w(uint8_t line, uint8_t state)
{
	uint8_t old_b = m_b;
	m_b &= ~(1 << line);
	m_b |= (state << line);
	if (old_b != m_b)
		update();
}

uint8_t ttl7400_device::y_r(uint8_t line)
{
	return (m_y >> line) & 1;
}


void ttl7400_device::a1_w(int state) { a_w(0, state); }
void ttl7400_device::a2_w(int state) { a_w(1, state); }
void ttl7400_device::a3_w(int state) { a_w(2, state); }
void ttl7400_device::a4_w(int state) { a_w(3, state); }

void ttl7400_device::b1_w(int state) { b_w(0, state); }
void ttl7400_device::b2_w(int state) { b_w(1, state); }
void ttl7400_device::b3_w(int state) { b_w(2, state); }
void ttl7400_device::b4_w(int state) { b_w(3, state); }

int ttl7400_device::y1_r() { return y_r(0); }
int ttl7400_device::y2_r() { return y_r(1); }
int ttl7400_device::y3_r() { return y_r(2); }
int ttl7400_device::y4_r() { return y_r(3); }
