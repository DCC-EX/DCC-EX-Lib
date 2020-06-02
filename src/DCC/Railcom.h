// Using tables and definitions copied from OpenMRN project.

#ifndef Railcom_h
#define Railcom_h

#include <Arduino.h>

extern const uint8_t railcom_decode[256];

/// invalid value (not conforming to the 4bit weighting requirement)
static const uint8_t INV = 0xFF;
/// Railcom ACK; the decoder received the message ok. NOTE: some early
/// software versions may have ACK and NACK exchanged.
static const uint8_t ACK = 0xFE;
/// The decoder rejected the packet.
static const uint8_t NACK = 0xFD;
/// The decoder is busy; send the packet again. This is typically returned
/// when a POM CV write is still pending; the caller must re-try sending the
/// packet later.
static const uint8_t BUSY = 0xFC;
/// Reserved for future expansion.
static const uint8_t RESVD1 = 0xFB;
/// Reserved for future expansion.
static const uint8_t RESVD2 = 0xFA;
/// Reserved for future expansion.
static const uint8_t RESVD3 = 0xF8;

struct Railcom
{
    enum
    {
        GARBAGE,
        ACK,
        NACK,
        BUSY,
        MOB_POM,
        MOB_ADRHIGH,
        MOB_ADRLOW,
        MOB_EXT,
        MOB_DYN,
        MOB_SUBID
    };

    // static void parseData(const uint8_t data[8]);
};

#endif