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

#include "Sdhc.h" 

#include <stddef.h>

namespace AvrPlusPlus
{
namespace Devices
{

#define SDHC_COMMANDS_BASE       (unsigned char) 0x40
#define SDHC_GO_IDLE_STATE       SDHC_COMMANDS_BASE + 0         // CMD0
#define SDHC_SEND_IF_COND        SDHC_COMMANDS_BASE + 8         // CMD8
#define SDHC_SEND_CID            SDHC_COMMANDS_BASE + 10        // CMD10
#define SDHC_READ_SINGLE_BLOCK   SDHC_COMMANDS_BASE + 17        // CMD17
#define SDHC_WRITE_SINGLE_BLOCK  SDHC_COMMANDS_BASE + 24        // CMD24
#define SDHC_CMD_SD_SEND_OP_COND SDHC_COMMANDS_BASE + 41        // ACMD41
#define SDHC_APP_CMD             SDHC_COMMANDS_BASE + 55        // CMD55
#define SDHC_READ_OCR            SDHC_COMMANDS_BASE + 58        // CMD58
#define SDHC_START_TOKEN_SINGLE  (unsigned char) 0xFE

Sdhc_SPI::Sdhc_SPI(Name spiPortName, unsigned char pinMosiNr, unsigned char pinSckNr, Name devicePortName, unsigned char pinCsNr) :
        SpiDevice(spiPortName, pinMosiNr, pinSckNr, devicePortName, pinCsNr), 
        cardStatus(ecsNOCARD), 
        cardType(ectNOTSUPPORT)
#ifdef UART_DEBUG
,usart(NULL)
#endif
{
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA);
    SPSR |= (1 << SPI2X);
}

Sdhc_SPI::InitStatus Sdhc_SPI::init()
{
    unsigned int uiMaxErrorsCMD = 1;
    const ResponceDataR1 * responceCode = (ResponceDataR1*) &responceData;
    onLog("Initializing SD card...\n\r");

    // As in datasheet min 74 clk before init
    for (unsigned int i = 0; i < 10; i++)
    {
        putChar(0xFF);
    }

    startTransfer();

    // Send CMD0. Switch up SPI mode
    uiMaxErrorsCMD = 0xFF;
    do
    {
        onLog("SDHC_GO_IDLE_STATE: ");
        crcValue = 0x95;
        responceData[0] = responceData[1] = responceData[2] = responceData[3] = responceData[4] = 0;
        sendCommand(SDHC_GO_IDLE_STATE, (unsigned long) 0x00, RESPONCE_R1);
        readChar();
    }
    while (responceCode->ucData != 0x01 && (--uiMaxErrorsCMD));
    if (responceCode->ucData != 0x01)
    {
        onLog("No card detected\n\r");
        finishTransfer();
        return NO_CARD;
    }
    else
    {
        onLog("OK\n\r");
    }

    // Send CMD8: Sends SD Memory Card interface condition that includes host supply voltage information and asks the
    // accessed card whether card can operate in supplied voltage range. Supported only for SDC ver.2+ card.
    {
        onLog("SDHC_SEND_IF_COND: ");
        crcValue = 0x87;
        responceData[0] = responceData[1] = responceData[2] = responceData[3] = responceData[4] = 0;
        sendCommand(SDHC_SEND_IF_COND, 0x01AA, RESPONCE_R7);
    }
    if (responceCode->ucData != 0x01)
    {
        onLog("MMC or SDC v1.x not supported\n\r");
        finishTransfer();
        return UNSUPPORTED_CARD_VERSION;
    }
    else if ((0xAA != ((ResponceDataR7*) &responceData)->R7Data.bits.bitCheckPattern)
            || (0x01 != ((ResponceDataR7*) &responceData)->R7Data.bits.bitVoltageAccepted))
    {
        onLog("The card does not support Vdd 2.7-3.6V\n\r");
        finishTransfer();
        return UNSUPPORTED_VOLTAGE;
    }
    else
    {
        onLog("OK\n\r");
    }

    // Send ACMD41: Sends host capacity support information and activates the card's initialization process.
    {
        onLog("SDHC_CMD_SD_SEND_OP_COND: ");
        responceData[0] = responceData[1] = responceData[2] = responceData[3] = responceData[4] = 0;
        uiMaxErrorsCMD = 0xFFFF;
        do
        {
            sendAppCommand(SDHC_CMD_SD_SEND_OP_COND, (unsigned long) 1 << 30, RESPONCE_R1);
        }
        while (responceCode->ucData != 0 && (--uiMaxErrorsCMD));
    }
    if (responceCode->ucData != 0)
    {
        onLog("The card can not finish initialization process\n\r");
        finishTransfer();
        return CARD_BUSY;
    }
    else
    {
        onLog("OK\n\r");
    }

    // CMD58: Reads the OCR register of a card.
    {
        onLog("SDHC_READ_OCR: ");
        responceData[0] = responceData[1] = responceData[2] = responceData[3] = responceData[4] = 0;
        uiMaxErrorsCMD = 0xFF;
        do
        {
            sendCommand(SDHC_READ_OCR, 0x00, RESPONCE_R3);
        }
        while (responceCode->ucData != 0 && (--uiMaxErrorsCMD));
    }
    if (responceCode->ucData != 0)
    {
        onLog("The card does not answer for OCR register reading\n\r");
        finishTransfer();
        return OCR_REGISTED_NOT_READED;
    }
    else
    {
        cardStatus = ecsOK;
        if (((ResponceDataR3*) &responceData)->R3Data.bits.bitCCS)
        {
            cardType = ectSDHC;
            onLog("SDHC");
        }
        else
        {
            cardType = ectSDCv2x;
            onLog("SDCv2x");
        }
        onLog(": Ready\n\r");
    }

    finishOperation();
    return INIT_OK;
}

bool Sdhc_SPI::readBlock(unsigned long ulAddress, unsigned char* ucBuffer, bool checkSum)
{
    bool retValue = false;
    if (cardStatus != ecsOK)
    {
        return retValue;
    }
    startTransfer();
    sendCommand(SDHC_READ_SINGLE_BLOCK, ulAddress, RESPONCE_R1);
    if (((ResponceDataR1*) &responceData)->ucData == 0x00)
    {
        bool tokenValid = waitDataToken();
        readBuffer(ucBuffer, SDHC_BLOCK_SIZE);
        uint16_t crc1 = readChar() << 8;
        crc1 |= readChar();
        uint16_t crc2 = (checkSum) ? CRC16_buf(ucBuffer, SDHC_BLOCK_SIZE) : crc1;
        retValue = crc1 == crc2;
        if (tokenValid && !retValue)
        {
            onLog("SDHC_READ_SINGLE_BLOCK: CRC error\n\r");
        }
    }
    else
    {
        onLog("SDHC_READ_SINGLE_BLOCK: Card not responding\n\r");
    }
    finishOperation();
    return retValue;
}

void Sdhc_SPI::writeBlock(unsigned long ulAddress, unsigned char* ucBuffer)
{
    if (cardStatus != ecsOK)
    {
        return;
    }
    startTransfer();
    sendCommand(SDHC_WRITE_SINGLE_BLOCK, ulAddress, RESPONCE_R1);
    if (((ResponceDataR1*) &responceData)->ucData == 0x00)
    {
        putInt(0xFFFF);
        putChar(SDHC_START_TOKEN_SINGLE);
        writeBuffer(ucBuffer, SDHC_BLOCK_SIZE);
        putInt(0xFFFF);
        while (readChar() == 0x00);
    }
    else
    {
        cardStatus = ecsERROR;
    }
    finishOperation();
}

void Sdhc_SPI::onLog(const char * str)
{
#ifdef UART_DEBUG
    if (usart != NULL)
    {
        usart->putString(str);
    }
#endif
}

void Sdhc_SPI::sendCommand(unsigned char ucCommand, unsigned long ulParam, unsigned char ucRespType)
{
    unsigned char uc_MaxErrors = 0xFF;
    putChar(0xFF);
    putChar(ucCommand);
    putChar((unsigned char) (ulParam >> 24));
    putChar((unsigned char) (ulParam >> 16));
    putChar((unsigned char) (ulParam >> 8));
    putChar((unsigned char) (ulParam));
    putChar(crcValue);

    // Responce is R1 must read allways
    do
    {
        responceData[0] = readChar();
    }
    while ((uc_MaxErrors--) && (responceData[0] & 0x80));
    switch (ucRespType)
    {
    case RESPONCE_R1:
        return;
        // Responce R2 is 2 bytes long
    case RESPONCE_R2:
        responceData[1] = readChar();
        return;
        // Responces R3, R7 are 5 bytes long
    case RESPONCE_R3:
    case RESPONCE_R7:
        responceData[4] = readChar();
        responceData[3] = readChar();
        responceData[2] = readChar();
        responceData[1] = readChar();
        return;
    }
}

void Sdhc_SPI::sendAppCommand(unsigned char ucCommandA, unsigned long ulParam, unsigned char ucRespType)
{
    sendCommand(SDHC_APP_CMD, 0x00, RESPONCE_R1);
    if (((ResponceDataR1*) &responceData)->ucData > 1)
    {
        return;
    }
    sendCommand(ucCommandA, ulParam, ucRespType);
}

void Sdhc_SPI::finishOperation()
{
    putChar(0xFF);
    finishTransfer();
}

bool Sdhc_SPI::waitDataToken()
{
    unsigned char trial = 0;
    do
    {
        if (readChar() == SDHC_START_TOKEN_SINGLE)
        {
            return true;
        }
        ++trial;
    }
    while (trial < 0xFF);
    onLog("SDHC_START_TOKEN_SINGLE: Card not responding.\n\r");
    return false;
}

uint8_t Sdhc_SPI::CRC7_one(uint8_t crcIn, uint8_t data)
{
    const uint8_t g = 0x89;
    uint8_t i;
    crcIn ^= data;
    for (i = 0; i < 8; i++)
    {
        if (crcIn & 0x80)
            crcIn ^= g;
        crcIn <<= 1;
    }
    return crcIn;
}

uint8_t Sdhc_SPI::CRC7_buf(uint8_t *pBuf, uint8_t len)
{
    uint8_t crc = 0;
    while (len--)
        crc = CRC7_one(crc, *pBuf++);
    return crc;
}

uint16_t Sdhc_SPI::CRC16_one(uint16_t crcIn, uint8_t data)
{
    crcIn = (uint8_t) (crcIn >> 8) | (crcIn << 8);
    crcIn ^= data;
    crcIn ^= (uint8_t) (crcIn & 0xff) >> 4;
    crcIn ^= (crcIn << 8) << 4;
    crcIn ^= ((crcIn & 0xff) << 4) << 1;
    return crcIn;
}

uint16_t Sdhc_SPI::CRC16_buf(const uint8_t * pBuf, uint16_t len)
{
    uint16_t crc = 0;
    while (len--)
        crc = CRC16_one(crc, *pBuf++);
    return crc;
}

}
}

