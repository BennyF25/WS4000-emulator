// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    5/7404 Hex Inverters

*****************************************************************************/

#include "emu.h"
#include "7404.h"

DEFINE_DEVICE_TYPE(TTL7404, ttl7404_device, "7404", "5/7404 Hex Inverters")

ttl7404_device::ttl7404_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TTL7404, tag, owner, clock)
	, m_y_func(*this)
	, m_a(0)
	, m_y(0x3f)
{
}

void ttl7404_device::device_start()
{
	m_y_func.resolve_all_safe();

	save_item(NAME(m_a));
	save_item(NAME(m_y));
}

void ttl7404_device::device_reset()
{
	m_a = 0;
	m_y = 0x3f;
}

void ttl7404_device::update()
{
	uint8_t last_y = m_y;

	m_y = (~m_a) & 0x3f;

	if (m_y != last_y)
	{
		for (int bit = 0; bit < 6; bit++)
		{
			if (BIT(m_y, bit) == BIT(last_y, bit))
				continue;

			m_y_func[bit](BIT(m_y, bit));
		}
	}
}

void ttl7404_device::a_w(uint8_t line, uint8_t state)
{
	uint8_t old_a = m_a;
	m_a &= ~(1 << line);
	m_a |= (state << line);
	if (old_a != m_a)
		update();
}

uint8_t ttl7404_device::y_r(uint8_t line)
{
	return (m_y >> line) & 1;
}


void ttl7404_device::a1_w(int state) { a_w(0, state); }
void ttl7404_device::a2_w(int state) { a_w(1, state); }
void ttl7404_device::a3_w(int state) { a_w(2, state); }
void ttl7404_device::a4_w(int state) { a_w(3, state); }
void ttl7404_device::a5_w(int state) { a_w(4, state); }
void ttl7404_device::a6_w(int state) { a_w(5, state); }

int ttl7404_device::y1_r() { return y_r(0); }
int ttl7404_device::y2_r() { return y_r(1); }
int ttl7404_device::y3_r() { return y_r(2); }
int ttl7404_device::y4_r() { return y_r(3); }
int ttl7404_device::y5_r() { return y_r(4); }
int ttl7404_device::y6_r() { return y_r(5); }
