#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <iostream>


// I2C definitions

#define I2C_SLAVE	0x0703
#define I2C_SMBUS	0x0720	/* SMBus-level access */

#define I2C_SMBUS_READ	1
#define I2C_SMBUS_WRITE	0

// SMBus transaction types

#define I2C_SMBUS_QUICK		    0
#define I2C_SMBUS_BYTE		    1
#define I2C_SMBUS_BYTE_DATA	    2
#define I2C_SMBUS_WORD_DATA	    3
#define I2C_SMBUS_PROC_CALL	    4
#define I2C_SMBUS_BLOCK_DATA	    5
#define I2C_SMBUS_I2C_BLOCK_BROKEN  6
#define I2C_SMBUS_BLOCK_PROC_CALL   7		/* SMBus 2.0 */
#define I2C_SMBUS_I2C_BLOCK_DATA    8

// SMBus messages

#define I2C_SMBUS_BLOCK_MAX	32	/* taille maxi d'un bloc de données */
#define I2C_SMBUS_I2C_BLOCK_MAX	32	/* Not specified but we use same structure */

// Structures utilisées par les appels ioctl()

using namespace std;

// La donnée peut être soit un Octet, soit un Mot ou un tableau d'octet
union i2c_smbus_data
{
  uint8_t  byte ;
  uint16_t word ;
  uint8_t  block [I2C_SMBUS_BLOCK_MAX + 2] ;	// block [0] is used for length + one more for PEC
};

struct i2c_smbus_ioctl_data
{
  char read_write ;
  uint8_t command ;
  int size ;
  union i2c_smbus_data *data ;
};



class i2c
{

    public:


            // le constructeur
            i2c(int adresseI2C);

            unsigned char Read ();
            unsigned char ReadReg8 (int reg);
            unsigned short ReadReg16 (int reg);

            unsigned char Write (int data);
            unsigned char WriteReg8 (int reg, int value);
            unsigned short WriteReg16 (int reg, int value);



    private:

            int fd;
            inline int i2c_smbus_access (char rw, uint8_t command, int size, union i2c_smbus_data *data);

};

#endif // I2C_H_INCLUDED
