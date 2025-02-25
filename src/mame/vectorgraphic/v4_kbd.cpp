// license:BSD-3-Clause
// copyright-holders:Eric Anderson
/**********************************************************************

Vector 4 91-key keyboard

The circuit board has printed numbers on the back. Those numbers are
used in the schematic on 7200-0001[1] p. 222 (VI A-19). The ROM uses a
lookup table to map them to an extended ASCII, p. 123 (II 6-4). Below
are the key numbers on the circuit board.

Circuit board:
A65-02366-052
PCB 251
Doesn't say Key Tronic, but clearly Key Tronic.

Row 1
1   help
2   F1
3   F2
4   F3
5   F4
6   F5
7   F6
8   F7
9   F8
10  F9
11  F10
12  F11
13  F12
14  F13
15  F14
16  up
17  down
18  left
19  right

Row 2
20  esc
21  1
22  2
23  3
24  4
25  5
26  6
27  7
28  8
29  9
30  0
31  -
32  =
33  `
34  del
35  10-key 7
36  10-key 8
37  10-key 9
38  10-key -

Row 3
39  tab
40  q
41  w
42  e
43  r
44  t
45  y
46  u
47  i
48  o
49  p
50  [
51  ]
52  backspace
53  10-key 4
54  10-key 5
55  10-key 6
56  10-key ,

Row 4
57  ctrl
58  caps lock
59  a
60  s
61  d
62  f
63  g
64  h
65  j
66  k
67  l
68  ;
69  '
70  return
71  \
72  10-key 1
73  10-key 2
74  10-key 3

Row 5
75  left shift
76  z
77  x
78  c
79  v
80  b
81  n
82  m
83  ,
84  .
85  /
86  right shift
87  line feed
88  10-key 0
89  10-key .
90  10-key enter

Row 6
91  space

1. https://archive.org/details/7200-0001-vector-4-technical-information-sep-82

**********************************************************************/

#include "emu.h"
#include "v4_kbd.h"
#include "speaker.h"

vector4_keyboard_device::vector4_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VECTOR4_KEYBOARD, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_beeper(*this, "beeper")
	, m_keys(*this, "Y%d", 0U)
	, m_led(*this, "led")
	, m_txd_cb(*this)
	, m_column(0)
	, m_p24(0)
{
}

void vector4_keyboard_device::device_start()
{
	save_item(NAME(m_column));
	save_item(NAME(m_p24));
}

void vector4_keyboard_device::device_resolve_objects()
{
	m_txd_cb.resolve_safe();
	m_led.resolve();
}

void vector4_keyboard_device::write_rxd(int state)
{
	m_mcu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}

uint8_t vector4_keyboard_device::p1_r()
{
	if (!m_p24 || m_column >= m_keys.size())
		return 0xff;
	return m_keys[m_column]->read();
}

void vector4_keyboard_device::p2_w(uint8_t data)
{
	if (!m_p24 && BIT(data, 4))
		m_column = m_mcu->p1_r() & 0x0f;
	m_p24 = BIT(data, 4);
	m_beeper->set_state(!BIT(data, 5));
	m_led = BIT(data, 6);
	m_txd_cb(BIT(data, 7));
}

void vector4_keyboard_device::prog_map(address_map &map)
{
	map(0x000, 0x7ff).mirror(0x800).rom().region("program", 0);
}

void vector4_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8035(config, m_mcu, 3'580'000); // P8048H in EA mode
	m_mcu->set_addrmap(AS_PROGRAM, &vector4_keyboard_device::prog_map);
	m_mcu->p1_in_cb().set(FUNC(vector4_keyboard_device::p1_r));
	m_mcu->p2_out_cb().set(FUNC(vector4_keyboard_device::p2_w));

	SPEAKER(config, "mono").front_center();
	// Correct frequency, but lacking harmonics
	BEEP(config, m_beeper, 786).add_route(ALL_OUTPUTS, "mono", 0.125);
}

INPUT_PORTS_START(vector4_keyboard)
	PORT_START("Y0") // _ _ _ _ 58 75 57 86
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_SHIFT_1)           PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Left Shift")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_SHIFT_2)           PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_SHIFT_1)           PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Right Shift")

	PORT_START("Y1") // 18 19 37 38 55 56 74 90
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LEFT))      PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))     PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))     PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))     PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("Y2") // 16 17 35 36 53 54 72 73
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(UP))    PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DOWN))  PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)

	PORT_START("Y3") // 14 15 33 34 51 52 70 71
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F13))  PORT_CODE(KEYCODE_F13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F14))  PORT_CODE(KEYCODE_F14)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~')  PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL))  PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}')  PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x08)                PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x0d)                PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_BACKSLASH)

	PORT_START("Y4") // 78 77 80 91 79 _ 76 59
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ')                PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)

	PORT_START("Y5") // 25 26 6 7 62 63 43 44
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F5))  PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F6))  PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)

	PORT_START("Y6") // 23 24 4 5 60 61 41 42
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F3))  PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F4))  PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)

	PORT_START("Y7") // 21 22 2 3 20 1 39 40
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F1))  PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F2))  PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x1b)               PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Help")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x09)               PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)

	PORT_START("Y8") // 27 28 8 9 64 65 45 46
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F7))  PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F8))  PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)

	PORT_START("Y9") // 10 11 29 30 66 67 47 48
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F9))  PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)

	PORT_START("Y10") // 12 13 31 32 49 69 68 50
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F11))  PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F12))  PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_')  PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+')  PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P')  PORT_CODE(KEYCODE_P)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':')  PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{')  PORT_CODE(KEYCODE_OPENBRACE)

	PORT_START("Y11") // 83 84 81 82 88 87 85 89
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<')     PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>')     PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N')     PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M')     PORT_CODE(KEYCODE_M)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))   PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x0a)                   PORT_CODE(KEYCODE_RALT) PORT_NAME("Line Feed")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?')     PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD) // period
INPUT_PORTS_END

ioport_constructor vector4_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(vector4_keyboard);
}

ROM_START(vector4_kbd)
	ROM_REGION(0x800, "program", 0)
	ROM_LOAD("v4_kbd_10.bin", 0x000, 0x800, CRC(3166e57b) SHA1(0023f0639e2f1d9e8860c6c4244e25fd8bf82331))
ROM_END

const tiny_rom_entry *vector4_keyboard_device::device_rom_region() const
{
	return ROM_NAME(vector4_kbd);
}

DEFINE_DEVICE_TYPE(VECTOR4_KEYBOARD, vector4_keyboard_device, "vector4_kbd", "Vector 4 Keyboard")
