/*
 *  Railcom.h
 * 
 *  This file is part of CommandStation.
 *
 *  CommandStation is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CommandStation is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COMMANDSTATION_DCC_RAILCOM_H_
#define COMMANDSTATION_DCC_RAILCOM_H_

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

#endif  // COMMANDSTATION_DCC_RAILCOM_H_