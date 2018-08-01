
/*
    spi.c :  A user-space program to comunicate using spidev.

*/


#include "spi.h"

char buf[10];
char buf2[10];
int com_serial;
int failcount;

struct spi_ioc_transfer xfer[2];


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Init SPIdev
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int spi_init(char filename[40])
{
    int file;
    __u8    mode, lsb, bits;
    __u32   speed = 25000000;

    if ((file = open(filename,O_RDWR)) < 0)
    {
        printf("Failed to open the bus.");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        com_serial = 0;
        //exit(1);
        return -1;
    }

    /*mode = SPI_MODE_0;
    if (ioctl(file, SPI_IOC_WR_MODE, &mode)<0)
    {
        perror("can't set spi mode");
        return;
    }
    */

    if (ioctl(file, SPI_IOC_RD_MODE, &mode) < 0)//
    {
        perror("SPI rd_mode");
        return -1;
    }

    lsb = 0;
    if (ioctl(file, SPI_IOC_WR_LSB_FIRST, &lsb) < 0)
    {
        perror("SPI error wr !");
        return -1;
    }

    if (ioctl(file, SPI_IOC_RD_LSB_FIRST, &lsb) < 0)
    {
        perror("SPI rd_lsb_fist");
        return -1;
    }

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
        return -1;
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
        return -1;
    }

    printf("\n%s: spi mode %d, %d bits %sper word, %d Hz max\n",filename, mode, bits, lsb ? "(lsb first) " : "(msb first) ", speed);

    xfer[0].len             = 0;        /* Length of  command to write*/
    xfer[0].cs_change       = 0;        /* Keep CS activated */
    xfer[0].delay_usecs     = 0;        // delay in us
    xfer[0].speed_hz        = 2500000;  // speed
    xfer[0].bits_per_word   = 8;        // bites per word 8

    return file;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//
//  SPI DEV handler dummy byte
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
char * spi_send_dummy(int file)
{
    int             status;
    unsigned char   *write_buf;
    uint32_t        rd_wr_len;
    unsigned char   *read_buf;

    xfer[0].tx_buf = (unsigned long)write_buf;
    xfer[0].rx_buf = (unsigned long)read_buf;
    xfer[0].len    = (uint32_t)20;

    status = ioctl(file, SPI_IOC_MESSAGE(1), xfer);

    if (status < 0)
    {
        perror("SPI_IOC_MESSAGE ");
        return 0;
    }

    return (1);
}





//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//
//  SPI DEV handler
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
char * spi_handler(unsigned char *write_buf, uint32_t rd_wr_len, unsigned char *read_buf, int file)
{
    int status;
    uint32_t i;


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+                     Check if device is BUSY
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned char loc_tx_buff    = CMD_READ_SR1;
    unsigned char loc_rx_buff[2] = {0x00, 0x00};//dummy


    xfer[0].tx_buf = (unsigned long)&loc_tx_buff;
    xfer[0].rx_buf = (unsigned long)loc_rx_buff;
    xfer[0].len    = 2;             /* Length of  command to write+read*/



    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+                                 Check device  BUSY
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    for (i=0; i < 1000; i++)
    {
        if ( ioctl(file, SPI_IOC_MESSAGE(1), xfer) )
        {
            usleep(100);
            //printf("loc_rx_buff: 0x%02x\r\n", loc_rx_buff[1]);//eliminare test

            if (loc_rx_buff[1] & 0x01){

                 if (i > 998){  /* Device Busy, retry*/
                    printf("SPI Device Timeout\n");
                    return 0;
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
            return 0;
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
        perror("SPI_IOC_MESSAGE");
        return 0;
    }

    return (1);
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ReadPage(unsigned int address, unsigned char *read_buf, int file)
{

    unsigned char loc_tx_buff[5];
    unsigned char loc_rx_buff[5];
    unsigned int rd_wr_len  = 0;


    /* READ PAGE COMMAND */
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

         /* READ PAGE COMMAND */
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

    rd_wr_len = PAGE_SIZE + 5;
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
    unsigned char loc_tx_buff[6];
    unsigned char loc_rx_buff[6];
    unsigned int rd_wr_len  = 0;

    /* write enable */
    flash_write_enable(file);


    /* ERASE */
    loc_tx_buff[0] = CMD_BULK_ERASE;
    //loc_tx_buff[0] = CMD_WRITE_ENABLED;
    //loc_tx_buff[0] = CMD_4ERASE_SECTOR;
    //loc_tx_buff[1] = loc_tx_buff[2] = loc_tx_buff[3] = loc_tx_buff[4] = 0;
    rd_wr_len      = 1;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+
//+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void SectorErase(int file, int addr)
{
    unsigned char loc_tx_buff[6];
    unsigned char loc_rx_buff[6];
    unsigned int rd_wr_len  = 0;

    addr *= 256*1024;

    /* write enable */
    flash_write_enable(file);


    /* ERASE */

    loc_tx_buff[0] = CMD_4ERASE_SECTOR;//command


    loc_tx_buff[4] = addr & 0xff;           //address lsb
    loc_tx_buff[3] = (addr >> 8) & 0xff;
    loc_tx_buff[2] = (addr >> 16) & 0xff;
    loc_tx_buff[1] = (addr >> 24) & 0xff;      //address msb


    rd_wr_len      = 5;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
}



unsigned char Flash_Write_File(uint32_t addr, uint8_t *write_buf, uint32_t len, uint32_t file)
{
    uint32_t i;
    unsigned char loc_tx_buff[1024];
    unsigned char loc_rx_buff[1024];
    uint32_t rd_wr_len  = 0;

	uint32_t max;
	uint32_t addr_loc;
	uint32_t pagelen, len_data;

	unsigned char uc_error = 0;

	len_data = len;

	do
	{
	    flash_write_enable(file);

        /* controllo indirizzo POSIZIONAMENTO, per non sforare pagina */
		max     = 0x00000200 - (addr & 0x000001FF);     //pagine 512 B
        //max     = 0x00000100 - (addr & 0x000000FF);   //pagine 256 B
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
        /* ++++++++++++++++++++++++++++++++++++++++++++++++++ */
        /* inoltro richiesta scrittura blocco*/
        /* ++++++++++++++++++++++++++++++++++++++++++++++++++ */
        spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
		flash_write_disable(file);

        //#if 0
        /* ++++++++++++++++++++++++++++++++++++++++++++++++++ */
        /* check blocco dati scritti */
        /* ++++++++++++++++++++++++++++++++++++++++++++++++++ */
        Flash_Read_File(addr, pagelen, loc_rx_buff, file);

        uc_error = 1;
        for(i = 0; i < pagelen; i++)
        {
            if (loc_tx_buff[i + 5] != loc_rx_buff[i])
            {
                uc_error = 0;
                // esco ho avuto errore -->break;
                return (uc_error);
            }
        }
        //#endif

        /* ++++++++++++++++++++++++++++++++++++++++++++++++++ */
        /* aggiorno per next step */
        /* ++++++++++++++++++++++++++++++++++++++++++++++++++ */
		addr      += pagelen;
		len_data  -= pagelen;

		printf("Len Data %d\n", len_data);

	}
	while (len_data > 0);

    printf("\n");

    return (uc_error);
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



int CopyFileToBuffer(const char *filename, unsigned char *data_file_in, uint32_t num_data_byte)
{
    FILE *fdata;
    uint32_t sizeoffile = 0;

    /* apre il file */
    fdata = fopen(filename, "rb");
    if(fdata == NULL)
    {
        perror("Errore in apertura del file");
        exit(1);
    }

    //SizeOfFile(filename, &sizeoffile);
    //fread(data_file_in, sizeoffile, 1, fdata); //leggo file

    fread(data_file_in, num_data_byte, 1, fdata); //leggo file
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ReadRegister_RDAR
//
// The Read Any Register (RDAR) command provides a way to read all device registers - non-volatile and volatile.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void ReadRegister_RDAR(uint8_t reg_to_read, uint32_t addr_reg_to_read, uint8_t *reg_read, int file)
{
    unsigned char loc_tx_buff[10];
    unsigned char loc_rx_buff[20];
    unsigned int  rd_wr_len  = 0;

    /* WRITE_ENABLED */

    loc_tx_buff[0] = reg_to_read;                       //command

    loc_tx_buff[3] = addr_reg_to_read & 0xff;           //address
    loc_tx_buff[2] = (addr_reg_to_read >> 8) & 0xff;
    loc_tx_buff[1] = (addr_reg_to_read >> 16) & 0xff;

    loc_tx_buff[4] = 0x00;                              //dummy
    loc_tx_buff[5] = 0x00;


    rd_wr_len      = 6;//test sistemare il numero dei byte

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);
    *reg_read  = loc_rx_buff[5];
}





//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// WriteRegister_WRAR
//
// The Write Any Register (WRAR) command provides a way to write any device register - non-volatile or volatile.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void WriteRegister_WRAR(uint8_t reg_to_write, uint32_t addr_reg_to_read, uint8_t data_to_write, int file)
{
    unsigned char loc_tx_buff[10];
    unsigned char loc_rx_buff[20];
    unsigned int  rd_wr_len  = 0;


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    flash_write_enable(file);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+     Write "Read Latency" dummy cycle
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /* 3byte add */
    loc_tx_buff[0] = reg_to_write;                       //command

    loc_tx_buff[3] = addr_reg_to_read & 0xff;           //address
    loc_tx_buff[2] = (addr_reg_to_read >> 8) & 0xff;
    loc_tx_buff[1] = (addr_reg_to_read >> 16) & 0xff;

    loc_tx_buff[4] = data_to_write;                     //data to write!!!

    rd_wr_len      = 5;

    spi_handler(loc_tx_buff, rd_wr_len, loc_rx_buff, file);


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    flash_write_disable( file);
}


void ReadIdentifications(uint8_t reg_to_read, uint8_t *reg_read, int file)
{
    unsigned char loc_tx_buff[50];
    unsigned char loc_rx_buff[50];
    unsigned int  rd_wr_len  = 0;
    int32_t i;

    for(i=1; i<49; i++)
    {
        loc_tx_buff[i] = i;
    }

    /* WRITE_ENABLED */
    loc_tx_buff[0] = reg_to_read;
    rd_wr_len      = 41; //40 dati + 1 comando

    spi_handler(loc_tx_buff, rd_wr_len, reg_read, file);

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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+ Scrive i registri:
//+ Input Status Register-1
//+ Configuration Register
//
//
// Codice che vale per la memoria: S25FS512S
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void SetPageSize(uint8_t page_size, int file)
{
    uint8_t loc_cr3v;

    ReadRegister_RDAR(0x65, 0x00800004, &loc_cr3v, file);

    if (page_size)
        loc_cr3v |= 0x10;//abilito 512
    else
        loc_cr3v &= 0xEF;//abilito 256

    WriteRegister_WRAR(0x71, 0x00800004, loc_cr3v, file);


}








