/* Includes ------------------------------------------------------------------*/
#include "VN_user.h"
#include "VN_lib.h"
#include "gio.h"
#include "spi.h"
#include "sys_common.h"
#include "system.h"
#include "mibspi.h"
#include "sys_dma.h"

/* Define -------------------------------------------------------------------*/
#define D_SIZE 127
/*******************************************************************************
* Function Name  : VN_SPI_SetSS(unsigned char sensorID, bool LineState)
* Description    : This is a generic function that will set the SPI slave select
*                  line for the given sensor. This function needs to be added by
*                  the user with the logic specific to their hardware to perform
*                  the necessary actions to either raise or lower the slave
*                  select line for the given sensor.  If a multiplexer is used
*                  then the logic/communications neccessary to perform the
*                  actions should be placed here.                                        
* Input          : sensorID  -> The sensor to set the slave select line for.
*                : state -   -> The state to set the slave select to.
* Output         : None
* Return         : None
*******************************************************************************/
void VN_SPI_SetSS(unsigned char sensorID, VN_PinState state){

/* User code to set SPI SS lines goes here. */   
  switch(sensorID){
    case 0:
      if(state == VN_PIN_LOW) {
        /* Start SPI Transaction - Pull SPI CS line low */
        gioSetBit(mibspiPORT1, PIN_CS0, 0);
      }
      else {
        /* End SPI transaction - Pull SPI CS line high */
        gioSetBit(mibspiPORT1, PIN_CS0, 1);
      }
      break;
  }
}

/*******************************************************************************
* Function Name  : VN_SPI_SendReceiveWord(unsigned long data)
* Description    : Transmits the given 32-bit word on the SPI bus. The user needs
*                  to place their hardware specific logic here to send 4 bytes
*                  out the SPI bus. The slave select line is controlled by the 
*                  function VN_SPI_SetSS given above, so the user only needs
*                  to deal with sending the data out the SPI bus with this
*                  function. Note that mibspiInit() must be used before using these functions
* Input          : data -> The 32-bit data to send over the SPI bus
* Output         : None
* Return         : The data received on the SPI bus
*******************************************************************************/
unsigned long VN_SPI_SendReceive(unsigned long data){
  /* Send out 4 bytes over SPI */
  unsigned long i;
  unsigned long ret = 0;
  uint16 recieved;

  for(i = 0; i < 4; i++){
    /*Wait until the transfer is complete*/
    while(!mibspiIsTransferComplete(mibspiREG1,0)) {
      /* Send the ith byte */
      mibspiSetData(mibspiREG1, 0, VN_BYTE(data,i));
      mibspiTransfer(mibspiREG1, 0);
    }
    mibspiGetData(mibspiREG1, 0, &recieved);
    ret |= (unsigned long)recieved << (8*i);
  }
  return ret;  
}

/*******************************************************************************
* Function Name  : VN_Delay(unsigned long delay_uS)
* Description    : Delay the processor for deltaT time in microseconds.  The user
*                  needs to place the hardware specific code here necessary to 
*                  delay the processor for the time span given by delay_uS
*                  measured in micro seconds. This function doesn't need to be
*                  ultra precise. The only requirement on this function is that
*                  the processor is delayed a time NO LESS THAN 90% of the time 
*                  given by the variable delay_uS in microseconds. The minimum
*                  timespan that is used by the VectorNav library code is 50uS so
*                  the function call shouldn't affect the timing accuracy much.
*                  If you decide to modify this library or wish to have more
*                  precision on this delay function then you can comment out this
*                  function and replace it with an optimized macro instead. Many
*                  compilers have their own delay routines or macros so make sure
*                  you check your compiler documentation before attempting to
*                  write your own.
* Input          : delay_uS -> Time to delay the processor in microseconds
* Output         : None
* Return         : None
*******************************************************************************/
void VN_Delay(unsigned long delay_uS){

/* User code to delay the processor goes here. Below is example code that
   works for a 32-bit ARM7 Cortex processor clocked at 72 MHz.  For any 
   other processor you will need to replace this with code that works
   for your processor.  Many compilers will have their own delay routines
   so make sure you check your compiler documentation before attempting to
   write your own. */
  unsigned long i;
  for(i=delay_uS*10; i--; );
}

/***************************************** Helper Functions *********************************************/

void mibspiEnableInternalLoopback(mibspiBASE_t *mibspi )
{
    /* enabling internal loopback */
    mibspi->GCR1 |= 1U << 16U;
}
/*@brief mibspiDmaConfig: 
  @param mibspi
  @param channel
  @param txchannel
  @param rxchannel 
*/
void mibspiDmaConfig(mibspiBASE_t *mibspi,uint32 channel, uint32 txchannel, uint32 rxchannel)
{
    uint32 bufid  = 0;
    uint32 icount = 0;

    /* setting transmit and receive channels */
    mibspi->DMACTRL[channel] |= (((rxchannel<<4)|txchannel) << 16);

    /* enabling transmit and receive dma */
    mibspi->DMACTRL[channel] |=  0x8000C000;

    /* setting Initial Count of DMA transfers and the buffer utilized for DMA transfer */
    mibspi->DMACTRL[channel] |=  (icount << 8) |(bufid<<24);

}

void dmaConfigCtrlPacket(uint32 sadd,uint32 dadd,uint32 dsize)
{
  g_dmaCTRLPKT.SADD      = sadd;              /* source address             */
  g_dmaCTRLPKT.DADD      = dadd;              /* destination  address       */
  g_dmaCTRLPKT.CHCTRL    = 0;                 /* channel control            */
  g_dmaCTRLPKT.FRCNT     = 1;                 /* frame count                */
  g_dmaCTRLPKT.ELCNT     = dsize;             /* element count              */
  g_dmaCTRLPKT.ELDOFFSET = 4;                 /* element destination offset */
  g_dmaCTRLPKT.ELSOFFSET = 0;                 /* element destination offset */
  g_dmaCTRLPKT.FRDOFFSET = 0;                 /* frame destination offset   */
  g_dmaCTRLPKT.FRSOFFSET = 0;                 /* frame destination offset   */
  g_dmaCTRLPKT.PORTASGN  = 4;                 /* port b                     */
  g_dmaCTRLPKT.RDSIZE    = ACCESS_16_BIT;     /* read size                  */
  g_dmaCTRLPKT.WRSIZE    = ACCESS_16_BIT;     /* write size                 */
  g_dmaCTRLPKT.TTYPE     = FRAME_TRANSFER ;   /* transfer type              */
  g_dmaCTRLPKT.ADDMODERD = ADDR_INC1;         /* address mode read          */
  g_dmaCTRLPKT.ADDMODEWR = ADDR_OFFSET;       /* address mode write         */
  g_dmaCTRLPKT.AUTOINIT  = AUTOINIT_ON;       /* autoinit                   */
}

void loadDataPattern(uint32 psize, uint16* pptr)
{
    *pptr = 0xD0C0;
    while(psize--)
    {
      *pptr = 0x1111 + *pptr++;
    }
 }
