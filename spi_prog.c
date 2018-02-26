/**********************************************************************************************



    App tester for SPI dev & FLASH Handler




**********************************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <math.h>
#include "spi.h"
#include "gpio.h"

/**********************************************************************************************
    Global define
**********************************************************************************************/

#define IN_PIN_POWER_PRES
#define OUT_PIN_FAIL_LED
#define IN_PIN_POWER_ENA
#define IN_PIN_SS0

#define IN_PIN_POWER_PRES
#define IN_PIN_POWER_PRES

/**********************************************************************************************
    Global declarations
**********************************************************************************************/
static const char SPIDEV_PATH[] = "/dev/spidev3.0";

unsigned char data_file_out_buffer[67108864 + 20];// 64 Mbytes + 20
unsigned char data_file_in_buffer[67108864 + 20]; // 64 Mbytes + 20

void case_b_function(void);


//4_5
#define POW_PRE_PRT     4
#define POW_PRE_BIT     5
//4_0
#define QSPI_WP_PRT     4
#define QSPI_WP_BIT     0
//4_3
#define FAIL_LD_PRT     4
#define FAIL_LD_BIT     3
//4_2
#define LDO_ENA_PRT     4
#define LDO_ENA_BIT     2
//4_1
#define QSPI_RST_PRT    4
#define QSPI_RST_BIT    1
//4_22
#define ENA_BUF_PRT     4
#define ENA_BUF_BIT     22







/**********************************************************************************************
    Main function
**********************************************************************************************/
int main (int argc, char **argv)
{

    FILE *file;

    unsigned char temp_data[5];
    unsigned char CR_register, SR1_register;
    unsigned char wip = 0xff;


    char     *path_file     = NULL;
    char     *c_temp        = NULL;
    uint32_t  address_to_w_r= 0;


    int flag_path_file      = 0;
    int flag_ini_addr      = 0;
    int flag_check_file     = 0;

    int index;
    int c;

    uint32_t num_byte       = 0;
    uint32_t address_init   = 0;
    uint32_t flag_numb_byte = 0;

    uint32_t dim_mem        = 50000;//test eliminare default!


    opterr = 0;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ export QSPI WP pin.
    if (GPIOExport(QSPI_WP_PRT, QSPI_WP_BIT) == -1)
        printf("error to export a pin QSPI_WP\r\n");
    else
        if (GPIODirection(QSPI_WP_PRT, QSPI_WP_BIT, OUT) == -1)
            printf("error to set direction of pin QSPI_WP\r\n");


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ export POWER PRESENCE pin.
    if (GPIOExport(POW_PRE_PRT, POW_PRE_BIT) == -1)
        printf("error to export a pin POW_PRE\r\n");
    else
        if (GPIODirection(POW_PRE_PRT, POW_PRE_BIT, IN) == -1)
            printf("error to set direction of pin POW_PRE\r\n");


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ export FAIL LED pin.
    if (GPIOExport(FAIL_LD_PRT, FAIL_LD_BIT) == -1)
        printf("error to export a pin FAIL_LD\r\n");
    else
        if (GPIODirection(FAIL_LD_PRT, FAIL_LD_BIT, OUT) == -1)
            printf("error to set direction of pin FAIL_LD\r\n");


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ export LDO ENABLE pin.
    if (GPIOExport(LDO_ENA_PRT, LDO_ENA_BIT) == -1)
        printf("error to export a pin LDO_ENA\r\n");
    else
        if (GPIODirection(LDO_ENA_PRT, LDO_ENA_BIT, OUT) == -1)
            printf("error to set direction of pin LDO_ENA\r\n");


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ export QSPI RST pin.
    if (GPIOExport(QSPI_RST_PRT, QSPI_RST_BIT) == -1)
        printf("error to export a pin QSPI_RST_\r\n");
    else
        if (GPIODirection(QSPI_RST_PRT, QSPI_RST_BIT, OUT) == -1)
            printf("error to set direction of pin QSPI_RST_\r\n");


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ export ENA BUF pin.
    if (GPIOExport(ENA_BUF_PRT, ENA_BUF_BIT) == -1)
        printf("error to export a pin ENA_BUF\r\n");
    else
        if (GPIODirection(ENA_BUF_PRT, ENA_BUF_BIT, OUT) == -1)
            printf("error to set direction of pin ENA_BUF\r\n");


    usleep(1000);//1ms
    GPIOWrite(ENA_BUF_PRT, ENA_BUF_BIT, 0);//Pull OE high to place all outputs in 3-state mode
    usleep(1000);//1ms


    if (!GPIORead(POW_PRE_PRT, POW_PRE_BIT))
    {
        printf("\nTarget selfpowered\n");
        GPIOWrite(LDO_ENA_PRT, LDO_ENA_BIT, 0);//power off LDO on board

    }
    else
    {
        printf("\nTarget powered from programmer\n");
        GPIOWrite(LDO_ENA_PRT, LDO_ENA_BIT, 1);//power off LDO on board

    }

    GPIOWrite(QSPI_RST_PRT, QSPI_RST_BIT, 0);
    usleep(1000);//1ms
    GPIOWrite(QSPI_RST_PRT, QSPI_RST_BIT, 1);

    GPIOWrite(QSPI_WP_PRT, QSPI_WP_BIT, 1);
    GPIOWrite(FAIL_LD_PRT, FAIL_LD_BIT, 0);


    sleep(1);
    printf("Init...\r\n");

    while ((c = getopt (argc, argv, "f:a:i:n:cpwrbsh")) != -1)

    switch (c)
    {
      case 'f':
        path_file = optarg;
        flag_path_file = 1;

        //test eliminare
        //printf("\nPath File: %s\n", path_file);
        break;

      case 'a':
        c_temp = optarg;
        if(!xtoi(c_temp, &address_to_w_r))
            abort ();

        flag_ini_addr = 1;

        //test eliminare
        //printf("\nIndirizzo: %u\n", address_to_w_r);
        break;

      case 'w':
        if (flag_ini_addr & flag_path_file)
        {
            if (!flag_numb_byte)
                num_byte = 0;

            flag_ini_addr = flag_path_file = flag_numb_byte = 0;
            case_w_function(path_file, address_to_w_r, flag_check_file, num_byte);
        }
        else
            printf("\n use -f and -a first\n");
        break;

      case 'r':
        if (flag_ini_addr & flag_path_file)
        {
            if (!flag_numb_byte)
                 num_byte = 0;

            flag_ini_addr = flag_path_file = flag_numb_byte = 0;
            case_r_function(path_file, address_to_w_r, num_byte);
        }
        else
            printf("\n use -f and -a first\n");

        break;

      case 'b':
        case_b_function();
        break;

     case 'p':/* print buffer read */
        printf ("\naddress_init %u - num_byte %u\n", address_init, num_byte);
        case_p_function(address_init, num_byte);
        break;


   case 'i':/* ini add buffer, to print */
        c_temp = optarg;
        address_init = atoi(c_temp);
        break;


   case 'n':/* number of data to print */
        flag_numb_byte = 1;

        c_temp   = optarg;
        num_byte = atoi(c_temp);

        //test eliminare
        //printf("\numero byte: %u\n", num_byte);
        break;

    case 'c':/* check scrittura */
        flag_check_file = 1;
        break;


     case 's':/* size of memory */
        case_s_function();
        break;






   case 'h':/* helph */

        printf ("---------------------------------------------------------------\n");
        printf ("HELP MENU\n");
        printf ("\n");
        printf (" -s QSPI Memory size\n");
        printf (" -r Read file\n");
        printf (" -w Write file\n");
        printf (" -f [option for -r -w]Path of file\n");
        printf (" -a [option for -r -w]Initial address to read/write in flash\n");
        printf (" -n [option for -r -w]Number of byte to read/write in flash\n");


        /*
        printf (" -p [option for -r]Print buffer of file read\n");
        printf (" -i [option for -p]Initial address of buffer to read\n");
        printf (" -c [option for -w]Check write operation\n");
        */
        printf ("---------------------------------------------------------------\n");
        break;


      default:
        printf ("Non-option argument \n");
        abort ();
      }

    //test eliminare
    //printf ("flag_path_file = %d, flag_ini_addr = %d\n",  flag_path_file, flag_ini_addr);


    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);

    return 0;


}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+           CASE -s size of memory
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void case_s_function(void)
{
    FILE *file;
    int8_t data_read[100];
    uint32_t loc_memory_size_byte;
    uint32_t loc_memory_size_bit;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ open dev
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    file = spi_init(SPIDEV_PATH); //dev
    if(file == -1)
    {
        GPIOWrite(FAIL_LD_PRT, FAIL_LD_BIT, 1);//led error
        printf("\nERROR TO OPEN SPIDEV\n");
    }

    GetMemorySize(&loc_memory_size_byte, &loc_memory_size_bit, file);
    printf("\nDevice Size : %d MB - %d Mb\n", loc_memory_size_byte, loc_memory_size_bit);

    close(file);
    printf("\nFinish\n");


}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+           CASE -b bulk erase
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void case_b_function(void)
{
    FILE *file;
    uint8_t wip_flag = 0xff;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ BULK ERASE
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    file = spi_init(SPIDEV_PATH); //dev
    if(file == -1)
    {
        GPIOWrite(FAIL_LD_PRT, FAIL_LD_BIT, 1);//led error
        printf("\nERROR TO OPEN SPIDEV\n");
    }

    BulkErase(file);
    printf("Bulk Erase Init wait:\n");
    do {

        printf("|\r");fflush(stdout);
        usleep(500000);
        printf("/\r");fflush(stdout);
        usleep(500000);
        printf("-\r");fflush(stdout);
        usleep(500000);
        printf("\\\r");fflush(stdout);
        usleep(500000);

        WIP_FlagStatus(&wip_flag, file);//write in progres flag


    } while (wip_flag);

    close(file);
    printf("\nBulk Erase Finish\n");


}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+           CASE -w Write a File
//+
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void case_w_function(const char *path_file,  uint32_t address, uint8_t flag_check_file, uint32_t num_byte_wr)
{
    FILE *file;
    uint32_t sizeoffile, i;
    uint8_t  c_error = 0;

    uint32_t loc_memory_size_byte;
    uint32_t loc_memory_size_bit;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ Ini write a file
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    file = spi_init(SPIDEV_PATH); //open dev
    if(file == -1)
    {
        GPIOWrite(FAIL_LD_PRT, FAIL_LD_BIT, 1);//led error
        printf("\nERROR TO OPEN SPIDEV\n");
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ Size of memory
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    GetMemorySize(&loc_memory_size_byte, &loc_memory_size_bit, file);
    printf("\nDevice Size : %d MB - %d Mb\n", loc_memory_size_byte, loc_memory_size_bit);
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ Prelevo file
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    SizeOfFile(path_file, &sizeoffile);

    if (num_byte_wr && (num_byte_wr <= sizeoffile))
        sizeoffile = num_byte_wr;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ Controllo sforo memoria
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (sizeoffile > (loc_memory_size_byte * 1048576))
        sizeoffile = loc_memory_size_byte * 1048576;//max size

    CopyFileToBuffer(path_file, data_file_in_buffer, sizeoffile);

    printf("\nWrite File:\n - Path file: %s\n - Write: %u bytes\n - Write @ Address: %u\n", path_file, sizeoffile, address);
    printf("\nWait...\n");


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ Scrittura del file
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Flash_Write_File(address, data_file_in_buffer, sizeoffile, file);

    printf("\nWtite File Finish.\n");

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ ESEGUO CHECK FILE SCRITTO
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ LEGGO FILE SCRITTO
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    printf("\nIni Check File, Wait..\n");
    Flash_Read_File(address, sizeoffile, data_file_out_buffer, file);

    c_error = 1;
    for(i = 0; i < sizeoffile; i++)
    {
        if (data_file_in_buffer[i] != data_file_out_buffer[i])
        {
            c_error = 0;
            break;
        }
    }

    if (c_error)
        printf("\nEnd Check File, PASS!\n");
    else
    {
        GPIOWrite(FAIL_LD_PRT, FAIL_LD_BIT, 1);
        printf("\nError Check File\n");
    }
    close(file);//spi dev andler file

}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+           CASE -r Read a File
//+
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void case_r_function(const char *path_file,  uint32_t address, uint32_t number_byte)
{
    FILE *file;
    FILE *write_ptr;
    uint8_t CR_register, SR1_register;
    uint32_t sizeoffile, i;

    uint32_t loc_memory_size_byte;
    uint32_t loc_memory_size_bit;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+     ini write a file
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    file = spi_init(SPIDEV_PATH); //open dev
    if(file == -1)
    {
        GPIOWrite(FAIL_LD_PRT, FAIL_LD_BIT, 1);//led error
        printf("\nERROR TO OPEN SPIDEV\n");
    }

    GetMemorySize(&loc_memory_size_byte, &loc_memory_size_bit, file);
    if (!number_byte || (number_byte > (loc_memory_size_byte * 1048576)))
        number_byte = loc_memory_size_byte * 1048576;//leggere tutta

    printf("\nDevice Size : %d MB - %d Mb\n", loc_memory_size_byte, loc_memory_size_bit);
    printf("\nRead File:\n - Path file: %s\n - Read: %u bytes\n - Read @ Address: %u\n", path_file, number_byte, address);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //+ LEGGO REGISTRO CR & SR1 ed elimino dummy byte in lettura (no latency)
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ReadRegister(CMD_READ_SR1, &SR1_register, file);
    ReadRegister(CMD_READ_CR,  &CR_register, file);
    Write_CR_SR1((CR_register | 0xc0), SR1_register, file);

    printf("\nWait...\n");

    Flash_Read_File(address, number_byte, data_file_out_buffer, file);

    printf("\nWrite file.\n");
    //scrivo i dati letti nel file passato tra gli argomenti in ingresso..
    write_ptr = fopen(path_file, "wb");
    //fwrite(data_file_out_buffer, sizeof(data_file_out_buffer), 1, write_ptr);
    fwrite(data_file_out_buffer, number_byte, 1, write_ptr);
    close(write_ptr);//file close

    close(file);//file for spi handler
    printf("\nRead QSPI Finish.\n");

}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+           CASE -p Print buffer
//+
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void case_p_function(uint32_t address, uint32_t data_lenght)
{
    uint32_t i;

    printf("\nPrint data read from add 0x%08x : to 0x%08x\n", address, data_lenght-1);
    for(i = address; i < data_lenght; i++)
    {
        printf("add[0x%08x]:%u ", i, data_file_out_buffer[i]); // prints a series of bytes
    }

    printf("\r\n");

}

int xtoi(const char *hptr, uint32_t *in_val)
{
    unsigned int val = 0;

    if (hptr[0] == '0' && hptr[1] == 'x') {
        hptr += 2;
    }
    else
    {
        printf("\nInsert a valid address, in HEX mode.\n");
        return 0;

    }

    while (1)
    {
        int nibble = *hptr++;
        if (nibble >= '0' && nibble <= '9')
        {
            nibble -= '0';
        }
        else
        {
            nibble |= 0x20;
            if (nibble >= 'a' && nibble <= 'f')
            {
                nibble -= 'a' - 10;
            } else
            {
                break;
            }
        }
        val = (val << 4) | nibble;

    }
    *in_val = val;

    return 1;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+           CASE -p Print buffer
//+
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void GetMemorySize(uint32_t *memory_size_byte, uint32_t *memory_size_bit, int file)
{
    int8_t data_read[100];

    ReadIdentifications(CMD_GET_IDENTIFICATION, data_read, file);
    *memory_size_byte = (pow(2, data_read[40])/1024)/1024;
    *memory_size_bit  = *memory_size_byte * 8;

}




