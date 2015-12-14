
class SoftSPI
{
  public:
    SoftSPI( int8_t _MOSI, int8_t _SCLK, int8_t _MISO );
    void begin();
    void spiwrite(uint8_t);
    uint8_t spiread(void);
    inline __attribute__((always_inline)) uint8_t spitransfer(uint8_t data);

  private:
    inline void writeFast(uint8_t value);
}
