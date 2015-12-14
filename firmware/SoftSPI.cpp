/***************************************************
  This is our library for the Adafruit HX8357D Breakout
  ----> http://www.adafruit.com/products/2050

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

SoftSPI::SoftSPI( int8_t mosi, int8_t sclk, int8_t miso )
{
  _mosi  = mosi;
  _miso = miso;
  _sclk = sclk;
}

void SoftSPI::spiwrite( uint8_t c )
{
  //Serial.print("0x"); Serial.print(c, HEX); Serial.print(", ");
  writeFast(c);
}


uint8_t SoftSPI::spiread(void)
{
  uint8_t r = 0;

  for (uint8_t i=0; i<8; i++)
  {
    digitalWrite(_sclk, LOW);
    digitalWrite(_sclk, HIGH);
    r <<= 1;
    if (digitalRead(_miso))
      r |= 0x1;
  }

  //Serial.print("read: 0x"); Serial.print(r, HEX);

  return r;
}

void SoftSPI::begin()
{
    pinMode(_sclk, OUTPUT);
    pinMode(_mosi, OUTPUT);
    pinMode(_miso, INPUT);

    digitalWrite(_sclk, LOW);
    digitalWrite(_mosi, LOW);
}

inline __attribute__((always_inline)) uint8_t spitransfer(uint8_t data) {
	uint8_t b=0;

	for (uint8_t bit = 0; bit < 8; bit++)  {	// walks down mask from bit 7 to bit 0
		(data & (1 << (7-bit))) ? pinSetFast(_mosi) : pinResetFast(_mosi);	// write the outgoing bit
		pinSetFast(_sclk);
		b <<= 1;
		if (pinReadFast(_miso)) b |= 1;	// Read the incoming bit
		pinResetFast(_sclk);
	}
	return b;
}

// based on
/*
* HARDWARE AND SOFTWARE LIQUIDCRYSTAL SPI
* 74HC595 LIBRARY FOR SPARK CORE
* =======================================================
*  Author: BDub
* Website: http://technobly.com
*    Date: Feb 24th 2014
* =======================================================
* https://github.com/technobly/SparkCore-LiquidCrystalSPI
*/
inline void Adafruit_HX8357::writeFast(uint8_t value)
{
  for (int8_t i = 7; i >= 0; i--)
  {
	  digitalWriteFast(_mosi, (value & (1 << i)));
    asm volatile("mov r0, r0" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" ::: "r0", "cc", "memory");
		pinSetFast(_sclk);
    asm volatile("mov r0, r0" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" ::: "r0", "cc", "memory");
    pinResetFast(_sclk); // Clock Low
  }

  asm volatile("mov r0, r0" "\n\t" "nop" "\n\t" "nop" "\n\t" "nop" "\n\t" ::: "r0", "cc", "memory");
}
