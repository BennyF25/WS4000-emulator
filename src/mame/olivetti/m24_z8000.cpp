// license:BSD-3-Clause
// copyright-holders:Carl
#include "emu.h"
#include "m24_z8000.h"

DEFINE_DEVICE_TYPE(M24_Z8000, m24_z8000_device, "m24_z8000", "Olivetti M24 Z8000 Adapter")

m24_z8000_device::m24_z8000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M24_Z8000, tag, owner, clock),
	m_z8000(*this, "z8000"),
	m_maincpu(*this, ":maincpu"),
	m_pic(*this, ":pic"),
	m_halt_out(*this),
	m_z8000_halt(true)
{
}

void m24_z8000_device::device_start()
{
	m_halt_out.resolve_safe();
}

void m24_z8000_device::device_reset()
{
	m_z8000_halt = true;
	m_z8000_mem = false;
	m_timer_irq = false;
	m_irq = 0;
	m_z8000->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

ROM_START( m24_z8000 )
	ROM_REGION(0x4000, "z8000", 0)
	ROM_LOAD("m24apb.bin", 0x0000, 0x4000, CRC(3b3d2895) SHA1(ff048cf61b090b147be7e29a929a0be7b3ac8409))
ROM_END


const tiny_rom_entry *m24_z8000_device::device_rom_region() const
{
	return ROM_NAME( m24_z8000 );
}

void m24_z8000_device::z8000_prog(address_map &map)
{
	map(0x000000, 0x0fffff).rw(FUNC(m24_z8000_device::pmem_r), FUNC(m24_z8000_device::pmem_w));

	map(0x040000, 0x043fff).rom().region("z8000", 0);
	map(0x050000, 0x053fff).rom().region("z8000", 0);
	map(0x070000, 0x073fff).rom().region("z8000", 0);
	// TODO: segments 0x10 and higher are trapped
}

void m24_z8000_device::z8000_data(address_map &map)
{
	map(0x000000, 0x0fffff).rw(FUNC(m24_z8000_device::dmem_r), FUNC(m24_z8000_device::dmem_w));

	map(0x040000, 0x043fff).rom().region("z8000", 0);
	map(0x070000, 0x073fff).rom().region("z8000", 0);
	// TODO: segments 0x10 and higher are trapped
}

void m24_z8000_device::z8000_io(address_map &map)
{
	map(0x0081, 0x0081).w(FUNC(m24_z8000_device::irqctl_w));
	map(0x00a1, 0x00a1).w(FUNC(m24_z8000_device::serctl_w));
	map(0x00c1, 0x00c1).rw("i8251", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x00c3, 0x00c3).rw("i8251", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x0120, 0x0127).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x8000, 0x83ff).rw(FUNC(m24_z8000_device::i86_io_r), FUNC(m24_z8000_device::i86_io_w));
	map(0x80c1, 0x80c1).rw(FUNC(m24_z8000_device::handshake_r), FUNC(m24_z8000_device::handshake_w));
}

void m24_z8000_device::device_add_mconfig(machine_config &config)
{
	Z8001(config, m_z8000, XTAL(8'000'000)/2);
	m_z8000->set_addrmap(AS_PROGRAM, &m24_z8000_device::z8000_prog);
	m_z8000->set_addrmap(AS_DATA, &m24_z8000_device::z8000_data);
	m_z8000->set_addrmap(AS_IO, &m24_z8000_device::z8000_io);
	m_z8000->nviack().set(FUNC(m24_z8000_device::nviack_r));
	m_z8000->viack().set(FUNC(m24_z8000_device::viack_r));
	m_z8000->mo().set(FUNC(m24_z8000_device::mo_w));

	pit8253_device &pit8253(PIT8253(config, "pit8253", 0));
	pit8253.set_clk<0>(19660000/15); //8251
	pit8253.out_handler<0>().set_nop();
	pit8253.set_clk<1>(19660000/15);
	pit8253.out_handler<1>().set_nop();
	pit8253.set_clk<2>(19660000/15);
	pit8253.out_handler<2>().set(FUNC(m24_z8000_device::timer_irq_w));

	I8251(config, "i8251", 0);
}

const uint8_t m24_z8000_device::pmem_table[16][4] =
	{{0, 1, 2, 3}, {1, 2, 3, 255}, {4, 5, 6, 7}, {46, 40, 41, 42},
	{255, 255, 255, 255}, {255, 255, 255, 47}, {1, 2, 3, 255}, {255, 255, 255, 255},
	{1, 2, 8, 9}, {5, 6, 10, 11}, {1, 2, 8, 9}, {12, 13, 14, 15},
	{16, 17, 18, 19}, {20, 21, 22, 23}, {24, 25, 26, 27}, {28, 29, 30, 31}};

uint16_t m24_z8000_device::pmem_r(offs_t offset, uint16_t mem_mask)
{
	offset <<= 1;
	if(!m_z8000_mem)
		return memregion(subtag("z8000").c_str())->as_u16(offset >> 1);

	uint8_t hostseg = pmem_table[(offset >> 16) & 0xf][(offset >> 14) & 3];
	if(hostseg == 255)
		return 0;
	offset = (offset & 0x3fff) | (hostseg << 14);
	if((hostseg >= 40) && (hostseg <= 47))
		offset = (offset & 0xf0000) | bitswap<16>(offset,15,7,6,14,13,12,11,10,9,8,5,4,3,2,1,0); // move A6/A7 so CGA framebuffer appears linear
	uint16_t ret = m_maincpu->space(AS_PROGRAM).read_word(offset, swapendian_int16(mem_mask));
	return swapendian_int16(ret);
}

void m24_z8000_device::pmem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset <<= 1;
	uint8_t hostseg = pmem_table[(offset >> 16) & 0xf][(offset >> 14) & 3];
	if(hostseg == 255)
		return;
	offset = (offset & 0x3fff) | (hostseg << 14);
	if((hostseg >= 40) && (hostseg <= 47))
		offset = (offset & 0xf0000) | bitswap<16>(offset,15,7,6,14,13,12,11,10,9,8,5,4,3,2,1,0);
	m_maincpu->space(AS_PROGRAM).write_word(offset, swapendian_int16(data), swapendian_int16(mem_mask));
}

const uint8_t m24_z8000_device::dmem_table[16][4] =
	{{0, 1, 2, 3}, {4, 5, 6, 7}, {4, 5, 6, 7}, {46, 40, 41, 42},
	{255, 255, 255, 255}, {1, 2, 3, 47}, {1, 2, 3, 255}, {255, 255, 255, 255},
	{5, 6, 10, 11}, {5, 6, 10, 11}, {1, 2, 8, 9}, {12, 13, 14, 15},
	{16, 17, 18, 19}, {20, 21, 22, 23}, {24, 25, 26, 27}, {28, 29, 30, 31}};

uint16_t m24_z8000_device::dmem_r(offs_t offset, uint16_t mem_mask)
{
	offset <<= 1;
	uint8_t hostseg = dmem_table[(offset >> 16) & 0xf][(offset >> 14) & 3];
	if(hostseg == 255)
		return 0;
	offset = (offset & 0x3fff) | (hostseg << 14);
	if((hostseg >= 40) && (hostseg <= 47))
		offset = (offset & 0xf0000) | bitswap<16>(offset,15,7,6,14,13,12,11,10,9,8,5,4,3,2,1,0);
	uint16_t ret = m_maincpu->space(AS_PROGRAM).read_word(offset, swapendian_int16(mem_mask));
	return swapendian_int16(ret);
}

void m24_z8000_device::dmem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset <<= 1;
	uint8_t hostseg = dmem_table[(offset >> 16) & 0xf][(offset >> 14) & 3];
	if(hostseg == 255)
		return;
	offset = (offset & 0x3fff) | (hostseg << 14);
	if((hostseg >= 40) && (hostseg <= 47))
		offset = (offset & 0xf0000) | bitswap<16>(offset,15,7,6,14,13,12,11,10,9,8,5,4,3,2,1,0);
	m_maincpu->space(AS_PROGRAM).write_word(offset, swapendian_int16(data), swapendian_int16(mem_mask));
}

uint16_t m24_z8000_device::i86_io_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t ret = m_maincpu->space(AS_IO).read_word(offset << 1, mem_mask == 0xffff ? 0xff00 : swapendian_int16(mem_mask));
	return swapendian_int16(ret);
}

void m24_z8000_device::i86_io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_maincpu->space(AS_IO).write_word(offset << 1, swapendian_int16(data), mem_mask == 0xffff ? 0xff00 : swapendian_int16(mem_mask));
}

void m24_z8000_device::irqctl_w(uint8_t data)
{
	m_irq = data;
}

void m24_z8000_device::serctl_w(uint8_t data)
{
	m_z8000_mem = (data & 0x20) ? true : false;
}

uint16_t m24_z8000_device::nviack_r()
{
	m_z8000->set_input_line(z8001_device::NVI_LINE, CLEAR_LINE);
	return 0xffff;
}

uint16_t m24_z8000_device::viack_r()
{
	return m_pic->acknowledge() << 1;
}

uint8_t m24_z8000_device::handshake_r()
{
	return 0;
}

void m24_z8000_device::handshake_w(uint8_t data)
{
	m_handshake = data;
	if(data & 1)
	{
		m_z8000->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_z8000->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		m_z8000->mi_w(CLEAR_LINE);
		m_z8000_halt = false;
	}
	else
	{
		m_z8000->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_z8000_halt = true;
		m_z8000_mem = false;
		m_halt_out(CLEAR_LINE);
	}
}

void m24_z8000_device::mo_w(int state)
{
	m_z8000->mi_w(state ? ASSERT_LINE : CLEAR_LINE);
	m_halt_out(state);
}

void m24_z8000_device::timer_irq_w(int state)
{
	m_timer_irq = state ? true : false;
	m_z8000->set_input_line(z8001_device::NVI_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}
