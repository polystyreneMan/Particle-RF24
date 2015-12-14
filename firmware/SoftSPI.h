#include "application.h"

class SoftSPI
{
  public:
    SoftSPI( int8_t _MOSI, int8_t _SCLK, int8_t _MISO );
    void begin();
    void spiwrite(uint8_t);
    uint8_t spiread(void);

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

  private:
    int8_t _mosi;
    int8_t _miso;
    int8_t _sclk;

    inline void writeFast(uint8_t value);
};
