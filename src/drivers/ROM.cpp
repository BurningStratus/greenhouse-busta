
#include "ROM.hpp"
#include "I2c.hpp"

size_t ROM::store
(uint16_t rom_addr, const uint8_t* payload, size_t payload_size)
{
    uint8_t     packet[2+payload_size];
    int         stat=0;

    packet[0] = 0xFF; // placeholder for address (hi)
    packet[1] = 0xFF; // placeholder for address (lo)

    //printf("ADDR(b4r cat): >0x%X%X<\n", packet[0],packet[1] );
    // strcat(packet, payload); 
    memcpy (packet + 2, payload, payload_size);

    packet[0] = ((rom_addr & 0xFF00) >> 8);     // extract hi bits
    packet[1] = (uint8_t)(rom_addr & 0x00FF);   // extract lo bits
    
    //printf("ADDR(aft cat): >0x%X%X<\n", packet[0],packet[1] );

    // stat=i2c_write_blocking( i2c0, I2C_ADDR, packet, ARRSIZE(packet), 0 ); 
    stat = i2c_bus.write (i2c_addr, packet, sizeof (packet));

    sleep_ms(10);
    return stat - 2;
}

size_t ROM::load
(uint16_t rom_addr, uint8_t *dst, size_t length)
{
    uint8_t     address[2];

    address[0] = ((rom_addr & 0xFF00) >> 8);     // extract hi bits
    address[1] = (uint8_t)(rom_addr & 0x00FF);   // extract lo bits
    
    // i2c_write_blocking( i2c0, I2C_ADDR, address, 2, 1 );
    i2c_bus.write (i2c_addr, address, sizeof address);

    sleep_ms(5);
    // i2c_read_blocking ( i2c0, I2C_ADDR, dst, wcnt, 0 );
    size_t bytes_read = i2c_bus.read (i2c_addr, dst, length);
    return bytes_read;
}
