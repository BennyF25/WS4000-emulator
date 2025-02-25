// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**************************************************************************

  Yamaha YM-2148 emulation

TODO:
- Finish implemention of midi out

**************************************************************************/

#include "emu.h"
#include "ym2148.h"


DEFINE_DEVICE_TYPE(YM2148, ym2148_device, "ym2148", "Yamaha YM2148 MIDI/Keyboard Interface")


ym2148_device::ym2148_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, YM2148, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_txd_handler(*this)
	, m_irq_handler(*this)
	, m_port_write_handler(*this)
	, m_port_read_handler(*this)
	, m_irq_state(CLEAR_LINE)
	, m_irq_vector(0xff) // guess
	, m_external_irq_vector(0xff) // guess
	, m_data_out(0)
	, m_data_in(0)
	, m_control(0)
	, m_status(0), m_timer(nullptr)
	, m_rxd(1)
	, m_tx_busy(false)
{
}


void ym2148_device::device_start()
{
	m_txd_handler.resolve_safe();
	m_irq_handler.resolve_safe();
	m_port_write_handler.resolve_safe();
	m_port_read_handler.resolve_safe(0xff);

	// Start a timer to trigger at clock / 8 / 16
	const attotime rate = clocks_to_attotime(8 * 16);
	m_timer = timer_alloc(FUNC(ym2148_device::serial_clock_tick), this);
	m_timer->adjust(rate, 0, rate);
}


void ym2148_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	transmit_register_reset();
	receive_register_reset();
	m_status |= STATUS_TRANSMIT_READY;
}


void ym2148_device::receive_clock()
{
	if (m_control & CONTROL_RECEIVE_ENABLE)
	{
		/* get bit received from other side and update receive register */
		receive_register_update_bit(m_rxd);

		if (is_receive_register_full())
		{
			receive_register_extract();

			m_data_in = get_received_char();

			if (is_receive_framing_error())
				m_status |= STATUS_FRAMING_ERROR;

			if (m_status & STATUS_RECEIVE_BUFFER_FULL)
				m_status |= STATUS_OVERRUN_ERROR;
			else
				m_status |= STATUS_RECEIVE_BUFFER_FULL;

			update_irq();
		}
	}
}


void ym2148_device::transmit_clock()
{
	if (m_control & CONTROL_TRANSMIT_ENABLE)
	{
		if (!(m_status & STATUS_TRANSMIT_READY))
		{
			/* is diserial ready for it? */
			if (is_transmit_register_empty())
			{
				/* set it up */
				transmit_register_setup(m_data_out);
				/* ready for next transmit */
				m_status |= STATUS_TRANSMIT_READY;
			}
		}

		/* if diserial has bits to send, make them so */
		if (!is_transmit_register_empty())
		{
			uint8_t data = transmit_register_get_data_bit();
			m_tx_busy = true;
			m_txd_handler(data);
		}

		// is transmitter totally done?
		if ((m_status & STATUS_TRANSMIT_READY) && is_transmit_register_empty())
		{
			m_tx_busy = false;
		}
	}
}


void ym2148_device::update_irq()
{
	m_irq_state = CLEAR_LINE;

	if ((m_status & STATUS_RECEIVE_BUFFER_FULL) && (m_control & CONTROL_RECEIVE_IRQ_ENABLE))
	{
		m_irq_state = ASSERT_LINE;
	}
	if ((m_status & STATUS_TRANSMIT_READY) && (m_control & CONTROL_TRANSMIT_IRQ_ENABLE))
	{
//      m_irq_state = ASSERT_LINE;
	}

	m_irq_handler(m_irq_state);
}


TIMER_CALLBACK_MEMBER(ym2148_device::serial_clock_tick)
{
	receive_clock();
	transmit_clock();
}


uint8_t ym2148_device::read(offs_t offset)
{
	switch (offset & 7)
	{
	case 2:  // External port read
		return m_port_read_handler();

	case 5:  // Midi data read register
		if (!machine().side_effects_disabled())
		{
			m_status &= ~STATUS_RECEIVE_BUFFER_FULL;
			update_irq();
		}
		return m_data_in;

	case 6:  // Midi status register
		return m_status;
	}
	return 0xff;
}


void ym2148_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 7)
	{
	case 2:  // External port write
		m_port_write_handler(data);
		break;

	case 3:  // IRQ vector
		m_irq_vector = data;
		break;

	case 4:  // External IRQ vector
		m_external_irq_vector = data;
		break;

	case 5:  // Midi data write register
		m_data_out = data;
		m_status &= ~STATUS_TRANSMIT_READY;
		break;

	case 6:  // Midi control register
		m_control = data;

		if (BIT(m_control, 4))  // Error reset
			m_status &= ~(STATUS_FRAMING_ERROR | STATUS_OVERRUN_ERROR);

		if (BIT(m_control, 7))
		{
			// Reset
			receive_clock();
			transmit_clock();
			m_irq_state = CLEAR_LINE;
			m_irq_handler(m_irq_state);
		}
		update_irq();
		break;
	}
}


uint8_t ym2148_device::get_irq_vector()
{
	return (m_irq_state == ASSERT_LINE) ? m_irq_vector : m_external_irq_vector;
}


void ym2148_device::write_rxd(int state)
{
	m_rxd = state;
}
