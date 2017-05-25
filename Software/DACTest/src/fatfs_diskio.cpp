/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"        /* FatFs lower layer API */
#include "sdcard.h"

SD_Error sdcard_init(void);

DSTATUS disk_initialize (
    BYTE pdrv              /* Physical drive nmuber (0..) */
)
{
    switch (pdrv)
    {
        case 0:
        {
            return sdcard_init() == SD_OK ? RES_OK : RES_NOTRDY;
        }
    }

    return STA_NOINIT;
}

DSTATUS disk_status (
    BYTE pdrv        /* Physical drive nmuber (0..) */
)
{
    switch (pdrv)
    {
        case 0:
        {
            uint32_t status;
            return SD_SendStatus(&status) == SD_OK ? RES_OK : RES_NOTRDY;
        }
    }

    return STA_NOINIT;
}

enum { SECTOR_SIZE = 512 };

DRESULT disk_read (
    BYTE pdrv,        /* Physical drive nmuber (0..) */
    BYTE *buff,       /* Data buffer to store read data */
    DWORD sector,     /* Sector address (LBA) */
    UINT count        /* Number of sectors to read (1..128) */
)
{
    switch (pdrv)
    {
        case 0:
        {
            for (BYTE i = 0; i < count; ++i)
            {
                SD_Error status = SD_ReadBlock(buff + SECTOR_SIZE * i, SECTOR_SIZE * (sector + i), SECTOR_SIZE);

                if (status != SD_OK)
                    return RES_ERROR;
            }

            break;
        }
    }

    return RES_OK;
}


DRESULT disk_write (
    BYTE pdrv,            /* Physical drive nmuber (0..) */
    const BYTE *buff,     /* Data to be written */
    DWORD sector,         /* Sector address (LBA) */
    UINT count            /* Number of sectors to write (1..128) */
)
{
    switch (pdrv)
    {
        case 0:
        {
            for (BYTE i = 0; i < count; ++i)
            {
                SD_Error status = SD_WriteBlock((uint8_t*)buff + SECTOR_SIZE * i, SECTOR_SIZE * (sector + i), SECTOR_SIZE);

                if (status != SD_OK)
                    return RES_ERROR;
            }

            break;
        }
    }

    return RES_OK;
}

DRESULT disk_ioctl (
    BYTE pdrv,        /* Physical drive nmuber (0..) */
    BYTE cmd,         /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
    switch (pdrv)
    {
        case 0:
            break; // TODO: disk_ioctl() if needed
    }

    return RES_PARERR;
}

DWORD get_fattime (void)
{
    return 0;
}
