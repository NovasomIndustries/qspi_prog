/*
    spi.h :  Header file for a user-space program to comunicate using spidev.

*/


#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>



#define CMD_GET_IDENTIFICATION      0x9F
#define CMD_FORMAT_CHIP             0x60
#define CMD_WRITE_ENABLED           0x06
#define CMD_WRITE_DISABLED          0x04
#define CMD_GET_STATUS              0x05
#define CMD_CLEAR_STATUS            0x30
#define CMD_WRITE_512B_PAGE         0x02
#define CMD_READ_512B_PAGE          0x0B
#define CMD_ERASE_512B_PAGE         0xD8


#define CMD_4FAST_READ_PAGE         0x0C
#define CMD_4PROGRAM_PAGE           0x12
#define CMD_4ERASE_SECTOR           0xDC
#define CMD_BULK_ERASE              0xC7


#define CMD_READ_CR                 0x35
#define CMD_READ_SR1                0x05
#define CMD_WRR                     0x01




#define PAGE_SIZE                   0x200       /*512 bytes => totale 131072 pagine*/
#define SECTOR_SIZE                 0x40000     /* 256 settori totale 256KBytes*/
#define FLASH_SIZE                  0X4000000   /* 512MBits => 64MBytes*/


int spi_init(char filename[40]);
char * spi_handler(unsigned char *write_buf, uint32_t rd_wr_len, unsigned char *read_buf, int file);



void ErasePage(unsigned int address, int file);
void ReadPage(unsigned int address, unsigned char *read_buf, int file);
void Flash_Read_File(uint32_t address, uint32_t len_buff, unsigned char *read_buf, int file);
void WritePage(unsigned int address, unsigned char *write_buf, int file);
int SizeOfFile(const char *filename, uint32_t *size_file);

void WIP_FlagStatus(unsigned char *wip, int file);
void BulkErase(int file);
int CopyFileToBuffer(const char *filename, unsigned char *data_file_in, uint32_t num_data_byte);
void Flash_Write_File(uint32_t addr, uint8_t *write_buf, uint32_t len, uint32_t file);

void ReadRegister(uint8_t reg_to_read, uint8_t *reg_read, int file);

void WriteRegister(uint8_t reg_to_write, uint8_t value_to_write, int file);

void Write_CR_SR1(uint8_t CR_to_write, uint8_t SR1_to_write, int file);

void flash_write_enable(uint32_t file);
void flash_write_disable(uint32_t file);


void ReadIdentifications(uint8_t reg_to_read, uint8_t *reg_read, int file);
























