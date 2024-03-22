#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <msp430.h>
#include "system.h"
#include "CommDataDefine.h"

#include "Oled.h"
#include "I2c.h"
#include "Eeprom.h"

BOOL Eeprom_ReadByte(WORD Addr, BYTE *Data)
{
    I2C_Start();
    if (!I2C_Write_Byte(EEPROM_ADDR))
    {
        return FALSE;
    }
    
    if (!I2C_Write_Byte((BYTE)(Addr>>8)))
    {
        return FALSE;
    }
    if (!I2C_Write_Byte((BYTE)Addr))
    {
        return FALSE;
    }

    I2C_Start();
    if (!I2C_Write_Byte(EEPROM_ADDR+1))
    {
        return FALSE;
    }
    
    *Data = I2C_Read_Byte(I2C_NACK);

    I2C_Stop();

    return TRUE;
}

BOOL Eeprom_ReadBuff(WORD Addr, BYTE *Buff, WORD Len)
{
    WORD i;
    
    I2C_Start();
    if (!I2C_Write_Byte(EEPROM_ADDR))
    {
        return FALSE;
    }
    
    if (!I2C_Write_Byte((BYTE)(Addr>>8)))
    {
        return FALSE;
    }
    if (!I2C_Write_Byte((BYTE)Addr))
    {
        return FALSE;
    }

    I2C_Start();
    if (!I2C_Write_Byte(EEPROM_ADDR+1))
    {
        return FALSE;
    }

    for (i=0;i<Len-1;i++)
    {
       Buff[i] = I2C_Read_Byte(I2C_ACK);
    }
    Buff[i] = I2C_Read_Byte(I2C_NACK);

    I2C_Stop();

    return TRUE;

}


BOOL Eeprom_WriteByte(WORD Addr, BYTE Dat)
{
    I2C_Start();
    if (!I2C_Write_Byte(EEPROM_ADDR))
    {
        return FALSE;
    }
    
    if (!I2C_Write_Byte((BYTE)(Addr>>8)))
    {
        return FALSE;
    }
    if (!I2C_Write_Byte((BYTE)Addr))
    {
        return FALSE;
    }

    if (!I2C_Write_Byte(Dat))
    {
        return FALSE;
    }
    
    I2C_Stop();
    delay_ms(6);  // ÖÁÉÙµÈ5ms
    return TRUE;
}

#if 0
BOOL Eeprom_WritePage(WORD Addr, BYTE *Buff, BYTE Len)
{
    I2C_Start();
    if (!I2C_Write_Byte(EEPROM_ADDR))
    {
        return FALSE;
    }
    
    if (!I2C_Write_Byte((BYTE)(Addr>>8)))
    {
        return FALSE;
    }
    if (!I2C_Write_Byte((BYTE)Addr))
    {
        return FALSE;
    }

    if (!I2C_Write_Byte(Dat))
    {
        return FALSE;
    }
    
    I2C_Stop();

    return TRUE;
}
#endif

BOOL Eeprom_Test()
{
    BYTE i;
    const BYTE dt[8] = {0x33,0x77,0x55,0xAA,0x0F,0xF0,0xCC,0x11};

    for (i=0;i<8;i++)
    {
        if (!Eeprom_WriteByte(i,dt[i]))
        {
            return FALSE;
        }
    }
    
    
    BYTE Buff[8] = {0};
    Eeprom_ReadBuff(0,Buff,8);

    #if 0
    for (i=0;i<8;i++)
    {
        Show5X7Int(100,10+i*8,Buff[i],0);
    }
    #endif

    if (0 == memcmp(dt,Buff,8))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

