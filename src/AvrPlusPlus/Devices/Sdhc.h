/*******************************************************************************
 * avrDigitalClock - a digital clock based on ATmega644 MCU
 * *****************************************************************************
 * Copyright (C) 2014-2017 Mikhail Kulesh
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef SDHC_H_
#define SDHC_H_

#include "../AvrPlusPlus.h"

namespace AvrPlusPlus
{
namespace Devices
{

#define SDHC_BLOCK_SIZE  (unsigned int) 512
//#define UART_DEBUG

class Sdhc_SPI: public SpiDevice
{
public:

    /** 
     * @brief Enumeration describing card status.
     */
    enum CardStatus
    {
        ecsOK, ecsERROR, ecsNOCARD
    };

    /** 
     * @brief Enumeration describing card type.
     */
    enum CardType
    {
        ectNOTSUPPORT, ectMMC, ectSDCv1x, ectSDCv2x, ectSDHC
    };

    enum InitStatus
    {
        INIT_OK = 0,
        NO_CARD = 1,
        UNSUPPORTED_CARD_VERSION = 2,
        UNSUPPORTED_VOLTAGE = 3,
        CARD_BUSY = 4,
        OCR_REGISTED_NOT_READED = 5
    };

    /** 
     * @brief Default constructor.
     */
    Sdhc_SPI(Name spiPortName, unsigned char pinMosiNr, unsigned char pinSckNr, Name devicePortName,
            unsigned char pinCsNr);

#ifdef UART_DEBUG
    /** 
     * @brief Setter for debug usart object.
     */
    inline void setUsartDebug (Usart * usartDebug)
    {
        usart = usartDebug;
    };
#endif

    /** 
     * @brief Initialization method.
     */
    InitStatus init();

    /** 
     * @brief Procedure return card status after initialization.
     */
    inline CardStatus getCardStatus() const
    {
        return cardStatus;
    };

    /** 
     * @brief Procedure reads single block.
     */
    bool readBlock(unsigned long ulAddress, unsigned char* ucBuffer, bool checkSum);

    /** 
     * @brief Procedure writes single block.
     */
    void writeBlock(unsigned long ulAddress, unsigned char* ucBuffer);

    /** 
     * @brief Procedure reads CID register.
     */
    bool getCID(unsigned char* ucBuffer);

private:

    // class members
    CardStatus cardStatus;
    CardType cardType;

    // temporary members
    unsigned char responceType, crcValue;
    unsigned char responceData[5];

    // UART used for logging
#ifdef UART_DEBUG
    Usart * usart;
    char outStr[128];
#endif

    // Card answers (responce)
    enum ResponceType
    {
        RESPONCE_R1, RESPONCE_R2, RESPONCE_R3, RESPONCE_R7
    };

    typedef union
    {
        unsigned char ucData;
        struct
        {
            unsigned char 
            bitIdleState :1, 
            bitEraseReset :1, 
            bitIllegalCmd :1, 
            bitComCRCError :1, 
            bitEraseSeqError :1,
            bitAddressError :1, 
            bitParameterError :1, 
            bitAlwaysZero :1;
        } bits;
    } ResponceDataR1;

    typedef struct
    {
        ResponceDataR1 R1;
        union
        {
            unsigned char ucData[4];
            struct
            {
                unsigned long 
                bitReserved :15, 
                bit2V7_2V8 :1, 
                bit2V8_2V9 :1, 
                bit2V9_3V0 :1, 
                bit3V0_3V1 :1,
                bit3V1_3V2 :1, 
                bit3V2_3V3 :1, 
                bit3V3_3V4 :1, 
                bit3V4_3V5 :1, 
                bit3V5_3V6 :1, 
                bitS18A :1,
                bitReserved1 :5, 
                bitCCS :1, 
                bitBusy :1;
            } bits;
        } R3Data;
    } ResponceDataR3;

    typedef struct
    {
        ResponceDataR1 R1;
        union
        {
            unsigned char ucData[4];
            struct
            {
                unsigned long 
                bitCheckPattern :8, 
                bitVoltageAccepted :4, 
                bitReserved :16, 
                bitCmdVer :4;
            } bits;
        } R7Data;
    } ResponceDataR7;

protected:

    void onLog(const char * str);
    void sendCommand(unsigned char ucCommand, unsigned long ulParam, unsigned char ucRespType);
    void sendAppCommand(unsigned char ucCommandA, unsigned long ulParam, unsigned char ucRespType);
    void finishOperation();
    bool waitDataToken();

    /** 
     * @brief Calculate 7 bit CRC with polynomial x^7 + x^3 + 1 for single byte.
     */
    uint8_t CRC7_one(uint8_t crcIn, uint8_t data);

    /** 
     * @brief Calculate 7 bit CRC with polynomial x^7 + x^3 + 1 for a buffer.
     */
    uint8_t CRC7_buf(uint8_t *pBuf, uint8_t len);

    /** 
     * @brief Calculate 16 bit CRC with polynomial x^16 + x^12 + x^5 + 1 for single byte.
     */
    uint16_t CRC16_one(uint16_t crcIn, uint8_t data);

    /** 
     * @brief Calculate 16 bit CRC with polynomial x^16 + x^12 + x^5 + 1 for a buffer.
     */
    uint16_t CRC16_buf(const uint8_t * pBuf, uint16_t len);
};

}
}

#endif
