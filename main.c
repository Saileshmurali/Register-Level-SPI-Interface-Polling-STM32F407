#include "stm32f407xx.h"
#define add 0x32
int8_t array[6],buf;
int16_t x,y,z;
float xacc,yacc,zacc;
void GPIO_Config(void)
{
	//PA5,6,7-CLK,MISO,MOSI
	RCC->AHB1ENR |=(1UL<<0);//Enable Port A clock
	GPIOA->MODER |=(2UL<<10);//Set PA5,6,7 to AF
	GPIOA->MODER |=(2UL<<12);
	GPIOA->MODER |=(2UL<<14);
	GPIOA->AFR[0] |=(5UL<<20); //enable SPI CLK to PA5
	GPIOA->AFR[0] |=(5UL<<24); //enable MISO to PA6
	GPIOA->AFR[0] |=(5UL<<28); //enable MOSI to PA7
	//PA4-CS
	GPIOA->MODER |=(1UL<<8);//PA4 as output
}
void SPI_Config(void)
{
	RCC->APB2ENR |=(1UL<<12);//Enable SPI clock
	SPI1->CR1 |=(2UL<<3);//Set baud rate as 2Mbit/s
	SPI1->CR1 |=(1UL<<2);//Set as Master mode
	SPI1->CR1 |=(1UL<<1);//Set clock polarity as HIGH
	SPI1->CR1 |=(1UL<<0);//Set clock phase as HIGH
	SPI1->CR1 |=(3UL<<8);//Should be set HIGH
	SPI1->CR1 |=(1UL<<6);//Start SPI
	SPI1->CR2 = 0x0000;//Motorola format
}
int8_t SPI_Send (uint8_t byte)
{
	SPI1->DR = byte;
	while ((SPI1->SR) & (1<<7));	/* Wait for send to finish */
	buf=SPI1->DR;
	return buf;
}
void Accel_Write(uint8_t address,uint8_t val)
{
	GPIOA->BSRR |=(1UL<<21);//Select accelerometer
	SPI_Send(address);
	SPI_Send(val);
	GPIOA->BSRR |=(1UL<<5);//Disconnect accelerometer
}
void ADXL_Config(void)
{
	Accel_Write(0x2D,0x00);// Reset before configuring power register
	Accel_Write(0x2D,0x08);//Configure the power register, and turn on the device
	// Configuring the data format register
	//The 5th bit corresponds to setting interrupt to active low if set
	Accel_Write(0x31,0x01);//Lower nibble:Selecting +/- 4g range and 4 wire SPI mode
	//Configuring sampling rate to 100hz
	Accel_Write(0x2C,0x0A);
}
void Accel_Read(uint8_t address)
{
	//X0
	GPIOA->BSRR |=(1UL<<21);//Select accelerometer
	address |=0x80;//Read operation
	SPI_Send(address);
	array[0]=SPI_Send(0);
	GPIOA->BSRR |=(1UL<<5);//Disconnect accelerometer
	//X1
	GPIOA->BSRR |=(1UL<<21);//Select accelerometer
	++address;
	SPI_Send(address);
	array[1]=SPI_Send(0);
	GPIOA->BSRR |=(1UL<<5);//Disconnect accelerometer
	//Y0
	GPIOA->BSRR |=(1UL<<21);//Select accelerometer
	++address;
	SPI_Send(address);
	array[2]=SPI_Send(0);
	GPIOA->BSRR |=(1UL<<5);//Disconnect accelerometer
	//y1
	GPIOA->BSRR |=(1UL<<21);//Select accelerometer
	++address;
	SPI_Send(address);
	array[3]=SPI_Send(0);
	GPIOA->BSRR |=(1UL<<5);//Disconnect accelerometer
	//Z0
	GPIOA->BSRR |=(1UL<<21);//Select accelerometer
	++address;
	SPI_Send(address);
	array[4]=SPI_Send(0);
	GPIOA->BSRR |=(1UL<<5);//Disconnect accelerometer
	//Z1
	GPIOA->BSRR |=(1UL<<21);//Select accelerometer
	++address;
	SPI_Send(address);
	array[5]=SPI_Send(0);
	GPIOA->BSRR |=(1UL<<5);//Disconnect accelerometer
	x=((array[1]<<8)|array[0]);
	y=((array[3]<<8)|array[2]);
	z=((array[5]<<8)|array[4]);
	xacc=x*0.0078;
	yacc=y*0.0078;
	zacc=z*0.0078;
}
int main()
{
	GPIO_Config();
	SPI_Config();
	ADXL_Config();
	while(1)
	{
		Accel_Read(add);
	}
}
