// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    microdrv.cpp

    MAME interface to the Sinclair Microdrive image abstraction code
    (this seems to be QL format specific and hardcoded to a specific
     size, Spectrum might need a different implementation?)

*********************************************************************/

#include "emu.h"
#include "microdrv.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define LOG 0

#define MDV_SECTOR_COUNT            255
#define MDV_SECTOR_LENGTH           686
#define MDV_IMAGE_LENGTH            (MDV_SECTOR_COUNT * MDV_SECTOR_LENGTH)

#define MDV_PREAMBLE_LENGTH         12
#define MDV_GAP_LENGTH              120

#define MDV_OFFSET_HEADER_PREAMBLE  0
#define MDV_OFFSET_HEADER           MDV_OFFSET_HEADER_PREAMBLE + MDV_PREAMBLE_LENGTH
#define MDV_OFFSET_DATA_PREAMBLE    28
#define MDV_OFFSET_DATA             MDV_OFFSET_DATA_PREAMBLE + MDV_PREAMBLE_LENGTH
#define MDV_OFFSET_GAP              566

#define MDV_BITRATE                 120000 // invalid, from ZX microdrive

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(MICRODRIVE, microdrive_image_device, "microdrive_image", "Sinclair Microdrive")

//-------------------------------------------------
//  microdrive_image_device - constructor
//-------------------------------------------------

microdrive_image_device::microdrive_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	microtape_image_device(mconfig, MICRODRIVE, tag, owner, clock),
	m_write_comms_out(*this)
{
}

//-------------------------------------------------
//  microdrive_image_device - destructor
//-------------------------------------------------

microdrive_image_device::~microdrive_image_device()
{
}


void microdrive_image_device::device_start()
{
	// resolve callbacks
	m_write_comms_out.resolve_safe();

	// allocate track buffers
	m_left = std::make_unique<uint8_t[]>(MDV_IMAGE_LENGTH / 2);
	m_right = std::make_unique<uint8_t[]>(MDV_IMAGE_LENGTH / 2);

	// allocate timers
	m_bit_timer = timer_alloc(FUNC(microdrive_image_device::bit_timer), this);
	m_bit_timer->adjust(attotime::zero, 0, attotime::from_hz(MDV_BITRATE));
	m_bit_timer->enable(0);

	m_clk = 0;
	m_comms_in = 0;
	m_comms_out = 0;
}

std::pair<std::error_condition, std::string> microdrive_image_device::call_load()
{
	if (length() != MDV_IMAGE_LENGTH)
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	// huh
	for (int i = 0; i < MDV_IMAGE_LENGTH / 2; i++)
	{
		fread(m_left.get(), 1);
		fread(m_right.get(), 1);
	}

	m_bit_offset = 0;
	m_byte_offset = 0;

	return std::make_pair(std::error_condition(), std::string());
}

void microdrive_image_device::call_unload()
{
	if (m_left.get())
		memset(m_left.get(), 0, MDV_IMAGE_LENGTH / 2);

	if (m_right.get())
		memset(m_right.get(), 0, MDV_IMAGE_LENGTH / 2);
}

TIMER_CALLBACK_MEMBER(microdrive_image_device::bit_timer)
{
	m_bit_offset++;

	if (m_bit_offset == 8)
	{
		m_bit_offset = 0;
		m_byte_offset++;

		if (m_byte_offset == MDV_IMAGE_LENGTH)
		{
			m_byte_offset = 0;
		}
	}
}

void microdrive_image_device::clk_w(int state)
{
	if (LOG) logerror("Microdrive '%s' CLK: %u\n", tag(), state);
	if (!m_clk && state)
	{
		m_comms_out = m_comms_in;
		if (LOG) logerror("Microdrive '%s' COMMS OUT: %u\n", tag(), m_comms_out);
		m_write_comms_out(m_comms_out);
		m_bit_timer->enable(m_comms_out);
	}
	m_clk = state;
}

void microdrive_image_device::comms_in_w(int state)
{
	if (LOG) logerror("Microdrive '%s' COMMS IN: %u\n", tag(), state);
	m_comms_in = state;
}

void microdrive_image_device::erase_w(int state)
{
	if (LOG) logerror("Microdrive '%s' ERASE: %u\n", tag(), state);
	m_erase = state;
}

void microdrive_image_device::read_write_w(int state)
{
	if (LOG) logerror("Microdrive '%s' READ/WRITE: %u\n", tag(), state);
	m_read_write = state;
}

void microdrive_image_device::data1_w(int state)
{
	if (m_comms_out && !m_read_write)
	{
		// TODO
	}
}

void microdrive_image_device::data2_w(int state)
{
	if (m_comms_out && !m_read_write)
	{
		// TODO
	}
}

int microdrive_image_device::data1_r()
{
	int data = 0;
	if (m_comms_out && m_read_write)
	{
		data = BIT(m_left[m_byte_offset], 7 - m_bit_offset);
	}
	return data;
}

int microdrive_image_device::data2_r()
{
	int data = 0;
	if (m_comms_out && m_read_write)
	{
		data = BIT(m_right[m_byte_offset], 7 - m_bit_offset);
	}
	return data;
}
