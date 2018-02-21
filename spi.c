
/*
    spi.c :  A user-space program to comunicate using spidev.

*/


#include "spi.h"

char buf[10];
char buf2[10];
int com_serial;
int failcount;

struct spi_ioc_transfer xfer[2];


//////////
// Init SPIdev
//////////
int spi_init(char filename[40])
{
    int file;
    __u8    mode, lsb, bits;
    __u32  speed=25000000;

        if ((file = open(filename,O_RDWR)) < 0)
        {
            printf("Failed to open the bus.");
            /* ERROR HANDLING; you can check errno to see what went wrong */
            com_serial=0;
            exit(1);
        }

        ///////////////
        // Verifications
        ///////////////
        //possible modes: mode |= SPI_LOOP; mode |= SPI_CPHA; mode |= SPI_CPOL; mode |= SPI_LSB_FIRST; mode |= SPI_CS_HIGH; mode |= SPI_3WIRE; mode |= SPI_NO_CS; mode |= SPI_READY;
        //multiple possibilities using |


            /*mode = SPI_MODE_0;


            if (ioctl(file, SPI_IOC_WR_MODE, &mode)<0)   {

                perror("can't set spi mode");
                return;
            }
            */


            if (ioctl(file, SPI_IOC_RD_MODE, &mode) < 0)//
            {
                perror("SPI rd_mode");
                return;
            }


            lsb = 0;

            if (ioctl(file, SPI_IOC_WR_LSB_FIRST, &lsb) < 0)
            {
                perror("SPI error wr !");
                return;
            }



            if (ioctl(file, SPI_IOC_RD_LSB_FIRST, &lsb) < 0)
            {
                perror("SPI rd_lsb_fist");
                return;
            }




        //sunxi supports only 8 bits
        /*
            if (ioctl(file, SPI_IOC_WR_BITS_PER_WORD, (__u8[1]){8})<0)
                {
                perror("can't set bits per word");
                return;
                }
        */
            if (ioctl(file, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
            {
                perror("SPI bits_per_word");
                return;
            }
        /*
            if (ioctl(file, SPI_IOC_WR_MAX_SPEED_HZ, &speed)<0)
                {
                perror("can't set max speed hz");
                return;
                }
        */
            if (ioctl(file, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0)
            {
                perror("SPI max_speed_hz");
                return;
            }


    printf("%s: spi mode %d, %d bits %sper word, %d Hz max\n",filename, mode, bits, lsb ? "(lsb first) " : "(msb first) ", speed);


    xfer[0].len             = 0;        /* Length of  command to write*/
    xfer[0].cs_change       = 0;        /* Keep CS activated */
    xfer[0].delay_usecs     = 0;        // delay in us
    xfer[0].speed_hz        = 2500000;  // speed
    xfer[0].bits_per_word   = 8;        // bites per word 8



    return file;
}





//////////
//
//
//  SPI DEV handler
//
//
//////////
char * spi_handler(unsigned char *write_buf, uint32_t rd_wr_len, unsigned char *read_buf, int file)
{
    int status;
    uint32_t i;


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+                     Check if device is BUSY
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned char loc_tx_buff    = 0x05;
    unsigned char loc_rx_buff[2] = {0x00, 0x00};


    xfer[0].tx_buf = (unsigned long)&loc_tx_buff;
    xfer[0].rx_buf = (unsigned long)loc_rx_buff;
    xfer[0].len    = 3;             /* Length of  command to write+read*/

    for (i=0; i < 1000; i++){

        if ( ioctl(file, SPI_IOC_MESSAGE(1), xfer) )
        {
            usleep(1000);
            //printf("loc_rx_buff: 0x%02x\r\n", loc_rx_buff[1]);//eliminare test

            if (loc_rx_buff[1] & 0x01){

                 if (i > 998){  /* Device Busy, retry*/
                    printf("SPI Device Timeout\n");
                    return;
                }
                //printf("device is busy, wait!\r\n"); //eliminare test
            }
            else{
                /*SPI Device free!*/
                //printf("device is free!\r\n");//eliminare test
                break;
            }

        }
        else
        {
            perror("SPI_IOC_MESSAGE -1");
            return;
        }

    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+                     END Check device  BUSY
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


    xfer[0].tx_buf = (unsigned long)write_buf;
    xfer[0].rx_buf = (unsigned long)read_buf;
    xfer[0].len    = (uint32_t)rd_wr_len; /* Length of  command to write*/




    status = ioctl(file, SPI_IOC_MESSAGE(1), xfer);

    if (status < 0)
    {
        perror("SPI_IOC_MESSAGE ");
        return;
    }

    return (1);
}




//void ErasePage(unsigned int address, int file)
//{
//    unsigned char loc_tx_buff[4];
//    unsigned char loc_rx_buff[1024];
//    unsigned int rd_wr_len  = 0;
//
//
//    /* WRITE_ENABLED */
//    loc_tx_buff[0] = CMD_WRITE_ENABLED;
//    rd_wr_len      = 1;
//
//    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
//
//
//    /* ERASE */
//    loc_tx_buff[0] = CMD_ERASE_512B_PAGE;
//    loc_tx_buff[1] = (unsigned char)(address >> 16);
//    loc_tx_buff[2] = (unsigned char)(address >> 8);
//    loc_tx_buff[3] = (unsigned char)address;
//    rd_wr_len      = 4;
//
//    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
//
//
//
//    /* WRITE_DISABLED */
//    loc_tx_buff[0] = CMD_WRITE_DISABLED;
//    rd_wr_len      = 1;
//
//    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
//
//
//
//}


//void ReadPage(unsigned int address, unsigned char *read_buf, int file)
//{
//
//    unsigned char loc_tx_buff[4];
//    unsigned char loc_rx_buff[4];
//    unsigned int rd_wr_len  = 0;
//
//
//    /* READ OAGE COMMAND */
//    loc_tx_buff[0] = CMD_READ_512B_PAGE;
//    loc_tx_buff[1] = (unsigned char)(address >> 16);
//    loc_tx_buff[2] = (unsigned char)(address >> 8);
//    loc_tx_buff[3] = (unsigned char)address;
//    rd_wr_len      = PAGE_SIZE + 4;
//
//    spi_handler(loc_tx_buff, rd_wr_len, read_buf, file);
//
//}


//void WritePage(unsigned int address, unsigned char *write_buf, int file)
//{
//    unsigned int i;
//    unsigned char loc_tx_buff[1024];
//    unsigned char loc_rx_buff[1024];
//    unsigned int rd_wr_len  = 0;
//
//
//    /* WRITE_ENABLED */
//    loc_tx_buff[0] = CMD_WRITE_ENABLED;
//    rd_wr_len      = 1;
//
//    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
//
//
//    /* WRITE PAGE COMMAND */
//    loc_tx_buff[0] = CMD_WRITE_512B_PAGE;
//    loc_tx_buff[1] = (unsigned char)(address >> 16);
//    loc_tx_buff[2] = (unsigned char)(address >> 8);
//    loc_tx_buff[3] = (unsigned char)address;
//
//    for (i = 0; i < PAGE_SIZE; i++)
//    {
//
//        loc_tx_buff[i + 4] = *write_buf++;
//    }
//
//
//    rd_wr_len      = PAGE_SIZE + 4;
//
//    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
//
//
//    /* WRITE_DISABLED */
//    loc_tx_buff[0] = CMD_WRITE_DISABLED;
//    rd_wr_len      = 1;
//
//    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
//
//}


//void write(uint32_t addr, const void *buf, uint32_t len)
//{
//	const uint8_t *p = (const uint8_t *)buf;
//	uint32_t max, pagelen;
//
//	 //Serial.printf("WR: addr %08X, len %d\n", addr, len);
//	do
//	{
//		if (busy) wait();
//
//		SPIPORT.beginTransaction(SPICONFIG);
//		CSASSERT();
//		// write enable command
//		SPIPORT.transfer(0x06);
//		CSRELEASE();
//
//		max = 256 - (addr & 0xFF);
//		pagelen = (len <= max) ? len : max;
//		 //Serial.printf("WR: addr %08X, pagelen %d\n", addr, pagelen);
//		delayMicroseconds(1); // TODO: reduce this, but prefer safety first
//
//		CSASSERT();
//		if (flags & FLAG_32BIT_ADDR)
//		{
//			SPIPORT.transfer(0x02); // program page command
//			SPIPORT.transfer16(addr >> 16);
//			SPIPORT.transfer16(addr);
//		}
//		else
//		{
//			SPIPORT.transfer16(0x0200 | ((addr >> 16) & 255));
//			SPIPORT.transfer16(addr);
//		}
//		addr += pagelen;
//		len -= pagelen;
//
//
//
//		do
//		{
//			SPIPORT.transfer(*p++);
//		}
//		while (--pagelen > 0);
//
//
//		CSRELEASE();
//		busy = 4;
//		SPIPORT.endTransaction();
//
//	}
//	while (len > 0);
//
//
//}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ReadPage(unsigned int address, unsigned char *read_buf, int file)
{

    unsigned char loc_tx_buff[5];
    unsigned char loc_rx_buff[5];
    unsigned int rd_wr_len  = 0;


    /* READ OAGE COMMAND */
    loc_tx_buff[0] = CMD_4FAST_READ_PAGE;

    loc_tx_buff[1] = (unsigned char)(address >> 24);
    loc_tx_buff[2] = (unsigned char)(address >> 16);
    loc_tx_buff[3] = (unsigned char)(address >> 8);
    loc_tx_buff[4] = (unsigned char)address;
    rd_wr_len      = PAGE_SIZE + 5;

    spi_handler(loc_tx_buff, rd_wr_len, read_buf, file);

}



void Flash_Read_File(uint32_t address, uint32_t len_buff, unsigned char *read_buf, int file)
{
    unsigned char loc_tx_buff[2050];
    unsigned char loc_rx_buff[2050];
    unsigned char *loc_read_buf;

    uint32_t      rd_wr_len    = 0;
    uint32_t      loc_len_buff = 0;
    uint32_t      loc_address  = 0;
    uint32_t      i  = 0;


    loc_len_buff = len_buff;
    loc_address  = address;
    loc_read_buf = read_buf;


    do
    {
        if (loc_len_buff > 2048)/* devo paginare */
        {
            loc_len_buff -= 2048;
            rd_wr_len     = 2048;
        }
        else
        {
            rd_wr_len     = loc_len_buff;
            loc_len_buff  = 0;
        }

        //rd_wr_len    = (loc_len_buff > 2048) ? 2048 : loc_len_buff;
        //loc_len_buff = (loc_len_buff > 2048) ? (loc_len_buff - 2048) : 0;

        //printf("rd_wr_len %u\n", rd_wr_len);

        //printf("loc_len_buff %u\n", loc_len_buff);

        //printf("loc_address 0x%08x\n", loc_address);

         /* READ OAGE COMMAND */
        loc_tx_buff[0] = CMD_4FAST_READ_PAGE;

        loc_tx_buff[1] = (unsigned char)(loc_address >> 24);
        loc_tx_buff[2] = (unsigned char)(loc_address >> 16);
        loc_tx_buff[3] = (unsigned char)(loc_address >> 8);
        loc_tx_buff[4] = (unsigned char)loc_address;
        //rd_wr_len = 5;

        spi_handler(loc_tx_buff, rd_wr_len + 5, loc_rx_buff, file);

        for (i = 0; i < rd_wr_len; i++)
        {
            *loc_read_buf++ = loc_rx_buff[i + 5];
        }

        loc_address += rd_wr_len;

        //printf("Indirizzo alla quale ha finito di scrivere loc_address 0x%08x\n", loc_address);
        //sleep(1);

    }
    while (loc_len_buff > 0);

}



void WritePage(unsigned int address, unsigned char *write_buf, int file)
{
    unsigned int i;
    unsigned char loc_tx_buff[1024];
    unsigned char loc_rx_buff[1024];
    unsigned int rd_wr_len  = 0;


    /* WRITE_ENABLED */
    loc_tx_buff[0] = CMD_WRITE_ENABLED;
    rd_wr_len      = 1;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);


    /* WRITE PAGE COMMAND */
    loc_tx_buff[0] = CMD_4PROGRAM_PAGE;

    loc_tx_buff[1] = (unsigned char)(address >> 24);
    loc_tx_buff[2] = (unsigned char)(address >> 16);
    loc_tx_buff[3] = (unsigned char)(address >> 8);
    loc_tx_buff[4] = (unsigned char)address;

    for (i = 0; i < PAGE_SIZE; i++)
    {

        loc_tx_buff[i + 5] = *write_buf++;
    }

    rd_wr_len      = PAGE_SIZE + 5;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);

    /* WRITE_DISABLED */
    loc_tx_buff[0] = CMD_WRITE_DISABLED;
    rd_wr_len      = 1;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
}

void ErasePage(unsigned int address, int file)
{
    unsigned char loc_tx_buff[10];
    unsigned char loc_rx_buff[10];
    unsigned int rd_wr_len  = 0;


    /* WRITE_ENABLED */
    loc_tx_buff[0] = CMD_WRITE_ENABLED;
    rd_wr_len      = 1;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);


    /* ERASE */
    loc_tx_buff[0] = CMD_4ERASE_SECTOR;

    loc_tx_buff[1] = (unsigned char)(address >> 24);
    loc_tx_buff[2] = (unsigned char)(address >> 16);
    loc_tx_buff[3] = (unsigned char)(address >> 8);
    loc_tx_buff[4] = (unsigned char)address;
    rd_wr_len      = 5;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);



    /* WRITE_DISABLED */
    loc_tx_buff[0] = CMD_WRITE_DISABLED;
    rd_wr_len      = 1;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+
//+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void BulkErase(int file)
{
    unsigned char loc_tx_buff[5];
    unsigned char loc_rx_buff[5];
    unsigned int rd_wr_len  = 0;

    /* write enable */
    flash_write_enable(file);


    /* ERASE */
    loc_tx_buff[0] = CMD_BULK_ERASE;
    rd_wr_len      = 1;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
}



void Flash_Write_File(uint32_t addr, uint8_t *write_buf, uint32_t len, uint32_t file)
{
    uint32_t i;
    unsigned char loc_tx_buff[1024];
    unsigned char loc_rx_buff[1024];
    uint32_t rd_wr_len  = 0;

	uint32_t max;
	uint32_t pagelen, len_data;

	len_data = len;

	do
	{
	    flash_write_enable(file);

        /* controllo indirizzo POSIZIONAMENTO, per non sforare pagina */
		max     = 0x00000200 - (addr & 0x000001FF);
		pagelen = (len_data <= max) ? len_data : max;


        /* WRITE PAGE COMMAND */
        loc_tx_buff[0] = CMD_4PROGRAM_PAGE;

        loc_tx_buff[1] = (unsigned char)(addr >> 24);
        loc_tx_buff[2] = (unsigned char)(addr >> 16);
        loc_tx_buff[3] = (unsigned char)(addr >> 8);
        loc_tx_buff[4] = (unsigned char)addr;

        /* prelevo solo i dati necessari a non sforare la pagina, da add fino a fine pagina*/

        for (i = 0; i < pagelen; i++)
        {

            loc_tx_buff[i + 5] = *write_buf++;
        }

        rd_wr_len = pagelen + 5;

        /* inoltro richiesta scrittura */
        spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);


		addr      += pagelen;
		len_data  -= pagelen;

		flash_write_disable(file);

	}
	while (len_data > 0);

}




void WIP_FlagStatus(unsigned char *wip, int file)
{
    int i;

    unsigned char loc_tx_buff[5] = {5, 0, 0, 0, 0};
    unsigned char loc_rx_buff[5] = {0, 0, 0, 0, 0};


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+                     Check if device is BUSY
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    xfer[0].tx_buf = (unsigned long)loc_tx_buff;
    xfer[0].rx_buf = (unsigned long)loc_rx_buff;
    xfer[0].len    = 3;             /* Length of  command to write+read*/

    for (i=0; i < 1000; i++)
    {
        if ( ioctl(file, SPI_IOC_MESSAGE(1), xfer) )
        {
            usleep(100);

            if (loc_rx_buff[1] & 0x01)
            {

                 if (i > 998) /* Device Busy, retry*/
                 {
                    *wip = 0x02; /* timeout error */
                    //printf("SPI Device Timeout\n");
                    return;
                }
                *wip = 0x01; /* busy*/
            }
            else
            {
                /*SPI Device free!*/
                *wip = 0x00; /* free*/
                break;
            }
        }
        else
        {
            perror("SPI_IOC_MESSAGE");
            return;
        }

    }

 }




int SizeOfFile(const char *filename, uint32_t *size_file)
{
    FILE *fd;
    int size, dim;

    /* apre il file */
    fd = fopen(filename, "r+");
    if(fd == NULL) {
        perror("Errore in apertura del file");
        exit(1);
    }

    /* determina la dimensione del file */
    fseek(fd, 0, SEEK_END);
    *size_file = ftell(fd);

    /* chiude il file */
    fclose(fd);

    return 0;
}



int CopyFileToBuffer(const char *filename, unsigned char *data_file_in)
{
    FILE *fdata;
    uint32_t sizeoffile = 0;

    /* apre il file */
    fdata = fopen(filename, "r");
    if(fdata == NULL)
    {
        perror("Errore in apertura del file");
        exit(1);
    }

    SizeOfFile(filename, &sizeoffile);
    fread(data_file_in, sizeoffile, 1, fdata); //leggo file
    /* chiude il file */
    fclose(fdata);

    return 0;
}


void flash_write_enable(uint32_t file)
{
    unsigned char loc_tx_buff[5];
    unsigned char loc_rx_buff[5];
    unsigned int  rd_wr_len  = 0;

    /* WRITE_ENABLED */
    loc_tx_buff[0] = CMD_WRITE_ENABLED;
    rd_wr_len      = 1;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
}


void flash_write_disable(uint32_t file)
{
    unsigned char loc_tx_buff[5];
    unsigned char loc_rx_buff[5];
    unsigned int  rd_wr_len  = 0;

    /* WRITE_ENABLED */
    loc_tx_buff[0] = CMD_WRITE_DISABLED;
    rd_wr_len      = 1;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
}



void ReadRegister(uint8_t reg_to_read, uint8_t *reg_read, int file)
{
    unsigned char loc_tx_buff[5];
    unsigned char loc_rx_buff[5];
    unsigned int  rd_wr_len  = 0;

    /* WRITE_ENABLED */
    loc_tx_buff[0] = reg_to_read;
    rd_wr_len      = 2;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);

    *reg_read  = loc_rx_buff[1];
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+ Scrive i registri:
//+ Input Status Register-1
//+ Configuration Register
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Write_CR_SR1(uint8_t CR_to_write, uint8_t SR1_to_write, int file)
{
    unsigned char loc_tx_buff[5];
    unsigned char loc_rx_buff[5];
    unsigned int  rd_wr_len  = 0;

    flash_write_enable( file);

    /* WRITE_ENABLED */
    loc_tx_buff[0] = CMD_WRR;
    loc_tx_buff[2] = CR_to_write;
    loc_tx_buff[1] = SR1_to_write;
    rd_wr_len      = 3;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);

    flash_write_disable( file);
}








