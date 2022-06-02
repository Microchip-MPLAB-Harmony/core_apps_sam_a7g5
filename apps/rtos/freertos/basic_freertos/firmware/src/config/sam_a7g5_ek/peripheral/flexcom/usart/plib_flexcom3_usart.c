/*******************************************************************************
  FLEXCOM3 USART PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_flexcom3_usart.c

  Summary:
    FLEXCOM3 USART PLIB Implementation File

  Description
    This file defines the interface to the FLEXCOM3 USART
    peripheral library. This library provides access to and control of the
    associated peripheral instance.

  Remarks:
    None.
*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "plib_flexcom3_usart.h"
#include "interrupts.h"

#define FLEXCOM3_USART_READ_BUFFER_SIZE             16
#define FLEXCOM3_USART_READ_BUFFER_SIZE_9BIT        (16 >> 1)


/* Disable Read, Overrun, Parity and Framing error interrupts */
#define FLEXCOM3_USART_RX_INT_DISABLE()      FLEXCOM3_REGS->FLEX_US_IDR = (FLEX_US_IDR_RXRDY_Msk | FLEX_US_IDR_FRAME_Msk | FLEX_US_IDR_PARE_Msk | FLEX_US_IDR_OVRE_Msk)

/* Enable Read, Overrun, Parity and Framing error interrupts */
#define FLEXCOM3_USART_RX_INT_ENABLE()       FLEXCOM3_REGS->FLEX_US_IER = (FLEX_US_IER_RXRDY_Msk | FLEX_US_IER_FRAME_Msk | FLEX_US_IER_PARE_Msk | FLEX_US_IER_OVRE_Msk)

#define FLEXCOM3_USART_TX_INT_DISABLE()      FLEXCOM3_REGS->FLEX_US_IDR = FLEX_US_IDR_TXRDY_Msk
#define FLEXCOM3_USART_TX_INT_ENABLE()       FLEXCOM3_REGS->FLEX_US_IER = FLEX_US_IER_TXRDY_Msk

static uint8_t FLEXCOM3_USART_ReadBuffer[FLEXCOM3_USART_READ_BUFFER_SIZE];

#define FLEXCOM3_USART_WRITE_BUFFER_SIZE            512
#define FLEXCOM3_USART_WRITE_BUFFER_SIZE_9BIT       (512 >> 1)

static uint8_t FLEXCOM3_USART_WriteBuffer[FLEXCOM3_USART_WRITE_BUFFER_SIZE];

// *****************************************************************************
// *****************************************************************************
// Section: FLEXCOM3 USART Ring Buffer Implementation
// *****************************************************************************
// *****************************************************************************

FLEXCOM_USART_RING_BUFFER_OBJECT flexcom3UsartObj;

void FLEXCOM3_USART_Initialize( void )
{
    /* Set FLEXCOM USART operating mode */
    FLEXCOM3_REGS->FLEX_MR = FLEX_MR_OPMODE_USART;

    /* Reset FLEXCOM3 USART */
    FLEXCOM3_REGS->FLEX_US_CR = (FLEX_US_CR_RSTRX_Msk | FLEX_US_CR_RSTTX_Msk | FLEX_US_CR_RSTSTA_Msk);


    FLEXCOM3_REGS->FLEX_US_TTGR = 0;

    /* Enable FLEXCOM3 USART */
    FLEXCOM3_REGS->FLEX_US_CR = (FLEX_US_CR_TXEN_Msk | FLEX_US_CR_RXEN_Msk);

    /* Configure FLEXCOM3 USART mode */
    FLEXCOM3_REGS->FLEX_US_MR = ( FLEX_US_MR_USART_MODE_NORMAL | FLEX_US_MR_USCLKS_MCK | FLEX_US_MR_CHRL_8_BIT | FLEX_US_MR_PAR_NO | FLEX_US_MR_NBSTOP_1_BIT | (0 << FLEX_US_MR_OVER_Pos));

    /* Configure FLEXCOM3 USART Baud Rate */
    FLEXCOM3_REGS->FLEX_US_BRGR = FLEX_US_BRGR_CD(108) | FLEX_US_BRGR_FP(4);

    flexcom3UsartObj.rdCallback = NULL;
    flexcom3UsartObj.rdInIndex = 0;
    flexcom3UsartObj.rdOutIndex = 0;
    flexcom3UsartObj.isRdNotificationEnabled = false;
    flexcom3UsartObj.isRdNotifyPersistently = false;
    flexcom3UsartObj.rdThreshold = 0;
    flexcom3UsartObj.rdBufferSize = FLEXCOM3_USART_READ_BUFFER_SIZE;
    flexcom3UsartObj.wrCallback = NULL;
    flexcom3UsartObj.wrInIndex = 0;
    flexcom3UsartObj.wrOutIndex = 0;
    flexcom3UsartObj.isWrNotificationEnabled = false;
    flexcom3UsartObj.isWrNotifyPersistently = false;
    flexcom3UsartObj.wrThreshold = 0;
    flexcom3UsartObj.wrBufferSize = FLEXCOM3_USART_WRITE_BUFFER_SIZE;
    flexcom3UsartObj.errorStatus = FLEXCOM_USART_ERROR_NONE;

    if (FLEXCOM3_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
    {
        flexcom3UsartObj.rdBufferSize = FLEXCOM3_USART_READ_BUFFER_SIZE_9BIT;
        flexcom3UsartObj.wrBufferSize = FLEXCOM3_USART_WRITE_BUFFER_SIZE_9BIT;
    }
    else
    {
        flexcom3UsartObj.rdBufferSize = FLEXCOM3_USART_READ_BUFFER_SIZE;
        flexcom3UsartObj.wrBufferSize = FLEXCOM3_USART_WRITE_BUFFER_SIZE;
    }

    FLEXCOM3_USART_RX_INT_ENABLE();
}

void static FLEXCOM3_USART_ErrorClear( void )
{
    uint16_t dummyData = 0u;

    /* Clear the error flags */
    FLEXCOM3_REGS->FLEX_US_CR = FLEX_US_CR_RSTSTA_Msk;

    /* Flush existing error bytes from the RX FIFO */
    while( FLEX_US_CSR_RXRDY_Msk == (FLEXCOM3_REGS->FLEX_US_CSR& FLEX_US_CSR_RXRDY_Msk) )
    {
        if (FLEXCOM3_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
        {
            dummyData = *((uint16_t*)&FLEXCOM3_REGS->FLEX_US_RHR) & FLEX_US_RHR_RXCHR_Msk;
        }
        else
        {
            dummyData = *((uint8_t*)&FLEXCOM3_REGS->FLEX_US_RHR);
        }
    }

    /* Ignore the warning */
    (void)dummyData;

    return;
}

FLEXCOM_USART_ERROR FLEXCOM3_USART_ErrorGet( void )
{
    FLEXCOM_USART_ERROR errors = flexcom3UsartObj.errorStatus;

    flexcom3UsartObj.errorStatus = FLEXCOM_USART_ERROR_NONE;

    /* All errors are cleared, but send the previous error state */
    return errors;
}

static void FLEXCOM3_USART_BaudCalculate(uint32_t srcClkFreq, uint32_t reqBaud, uint8_t overSamp, uint32_t* cd, uint32_t* fp, uint32_t* baudError)
{
    uint32_t actualBaud = 0;

    *cd = srcClkFreq / (reqBaud * 8 * (2 - overSamp));

    if (*cd > 0)
    {
        *fp = ((srcClkFreq / (reqBaud * (2 - overSamp))) - ((*cd) * 8));
        actualBaud = (srcClkFreq / (((*cd) * 8) + (*fp))) / (2 - overSamp);
        *baudError = ((100 * actualBaud)/reqBaud) - 100;
    }
}

bool FLEXCOM3_USART_SerialSetup( FLEXCOM_USART_SERIAL_SETUP *setup, uint32_t srcClkFreq )
{
    uint32_t baud = 0;
    uint32_t overSampVal = 0;
    uint32_t usartMode;
    uint32_t cd0, fp0, cd1, fp1, baudError0, baudError1;
    bool status = false;

    cd0 = fp0 = cd1 = fp1 = baudError0 = baudError1 = 0;

    if (setup != NULL)
    {
        baud = setup->baudRate;

        if(srcClkFreq == 0)
        {
            srcClkFreq = FLEXCOM3_USART_FrequencyGet();
        }

        /* Calculate baud register values for 8x/16x oversampling values */

        FLEXCOM3_USART_BaudCalculate(srcClkFreq, baud, 0, &cd0, &fp0, &baudError0);
        FLEXCOM3_USART_BaudCalculate(srcClkFreq, baud, 1, &cd1, &fp1, &baudError1);

        if ( !(cd0 > 0 && cd0 <= 65535) && !(cd1 > 0 && cd1 <= 65535) )
        {
            /* Requested baud cannot be generated with current clock settings */
            return status;
        }

        if ( (cd0 > 0 && cd0 <= 65535) && (cd1 > 0 && cd1 <= 65535) )
        {
            /* Requested baud can be generated with both 8x and 16x oversampling. Select the one with less % error. */
            if (baudError1 < baudError0)
            {
                cd0 = cd1;
                fp0 = fp1;
                overSampVal = (1 << FLEX_US_MR_OVER_Pos) & FLEX_US_MR_OVER_Msk;
            }
        }
        else
        {
            /* Requested baud can be generated with either with 8x oversampling or with 16x oversampling. Select valid one. */
            if (cd1 > 0 && cd1 <= 65535)
            {
                cd0 = cd1;
                fp0 = fp1;
                overSampVal = (1 << FLEX_US_MR_OVER_Pos) & FLEX_US_MR_OVER_Msk;
            }
        }

        /* Configure FLEXCOM3 USART mode */
        usartMode = FLEXCOM3_REGS->FLEX_US_MR;
        usartMode &= ~(FLEX_US_MR_CHRL_Msk | FLEX_US_MR_MODE9_Msk | FLEX_US_MR_PAR_Msk | FLEX_US_MR_NBSTOP_Msk | FLEX_US_MR_OVER_Msk);
        FLEXCOM3_REGS->FLEX_US_MR = usartMode | ((uint32_t)setup->dataWidth | (uint32_t)setup->parity | (uint32_t)setup->stopBits | overSampVal);

        /* Configure FLEXCOM3 USART Baud Rate */
        FLEXCOM3_REGS->FLEX_US_BRGR = FLEX_US_BRGR_CD(cd0) | FLEX_US_BRGR_FP(fp0);

        if (FLEXCOM3_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
        {
            flexcom3UsartObj.rdBufferSize = FLEXCOM3_USART_READ_BUFFER_SIZE_9BIT;
            flexcom3UsartObj.wrBufferSize = FLEXCOM3_USART_WRITE_BUFFER_SIZE_9BIT;
        }
        else
        {
            flexcom3UsartObj.rdBufferSize = FLEXCOM3_USART_READ_BUFFER_SIZE;
            flexcom3UsartObj.wrBufferSize = FLEXCOM3_USART_WRITE_BUFFER_SIZE;
        }

        status = true;
    }

    return status;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static bool FLEXCOM3_USART_TxPullByte(uint16_t* pWrByte)
{
    bool isSuccess = false;
    uint32_t wrOutIndex = flexcom3UsartObj.wrOutIndex;
    uint32_t wrInIndex = flexcom3UsartObj.wrInIndex;

    if (wrOutIndex != wrInIndex)
    {
        if (FLEXCOM3_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
        {
            *pWrByte = ((uint16_t*)&FLEXCOM3_USART_WriteBuffer)[wrOutIndex++];
        }
        else
        {
            *pWrByte = FLEXCOM3_USART_WriteBuffer[wrOutIndex++];
        }

        if (wrOutIndex >= flexcom3UsartObj.wrBufferSize)
        {
            wrOutIndex = 0;
        }

        flexcom3UsartObj.wrOutIndex = wrOutIndex;

        isSuccess = true;
    }

    return isSuccess;
}

static inline bool FLEXCOM3_USART_TxPushByte(uint16_t wrByte)
{
    uint32_t tempInIndex;
    uint32_t wrOutIndex;
    uint32_t wrInIndex;
    bool isSuccess = false;

    /* Take a snapshot of indices to avoid creation of critical section */
    wrOutIndex = flexcom3UsartObj.wrOutIndex;
    wrInIndex = flexcom3UsartObj.wrInIndex;

    tempInIndex = wrInIndex + 1;

    if (tempInIndex >= flexcom3UsartObj.wrBufferSize)
    {
        tempInIndex = 0;
    }
    if (tempInIndex != wrOutIndex)
    {
        if (FLEXCOM3_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
        {
            ((uint16_t*)&FLEXCOM3_USART_WriteBuffer)[wrInIndex] = wrByte;
        }
        else
        {
            FLEXCOM3_USART_WriteBuffer[wrInIndex] = (uint8_t)wrByte;
        }

        flexcom3UsartObj.wrInIndex = tempInIndex;
        isSuccess = true;
    }
    else
    {
        /* Queue is full. Report Error. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void FLEXCOM3_USART_WriteNotificationSend(void)
{
    uint32_t nFreeWrBufferCount;

    if (flexcom3UsartObj.isWrNotificationEnabled == true)
    {
        nFreeWrBufferCount = FLEXCOM3_USART_WriteFreeBufferCountGet();

        if(flexcom3UsartObj.wrCallback != NULL)
        {
            if (flexcom3UsartObj.isWrNotifyPersistently == true)
            {
                if (nFreeWrBufferCount >= flexcom3UsartObj.wrThreshold)
                {
                    flexcom3UsartObj.wrCallback(FLEXCOM_USART_EVENT_WRITE_THRESHOLD_REACHED, flexcom3UsartObj.wrContext);
                }
            }
            else
            {
                if (nFreeWrBufferCount == flexcom3UsartObj.wrThreshold)
                {
                    flexcom3UsartObj.wrCallback(FLEXCOM_USART_EVENT_WRITE_THRESHOLD_REACHED, flexcom3UsartObj.wrContext);
                }
            }
        }
    }
}

static size_t FLEXCOM3_USART_WritePendingBytesGet(void)
{
    size_t nPendingTxBytes;

    /* Take a snapshot of indices to avoid creation of critical section */

    uint32_t wrOutIndex = flexcom3UsartObj.wrOutIndex;
    uint32_t wrInIndex = flexcom3UsartObj.wrInIndex;

    if ( wrInIndex >=  wrOutIndex)
    {
        nPendingTxBytes =  wrInIndex -  wrOutIndex;
    }
    else
    {
        nPendingTxBytes =  (flexcom3UsartObj.wrBufferSize -  wrOutIndex) + wrInIndex;
    }

    return nPendingTxBytes;
}

size_t FLEXCOM3_USART_WriteCountGet(void)
{
    size_t nPendingTxBytes;

    nPendingTxBytes = FLEXCOM3_USART_WritePendingBytesGet();

    return nPendingTxBytes;
}

size_t FLEXCOM3_USART_Write(uint8_t* pWrBuffer, const size_t size )
{
    size_t nBytesWritten  = 0;

    while (nBytesWritten < size)
    {
        if (FLEXCOM3_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
        {
            if (FLEXCOM3_USART_TxPushByte(((uint16_t*)pWrBuffer)[nBytesWritten]) == true)
            {
                nBytesWritten++;
            }
            else
            {
                /* Queue is full, exit the loop */
                break;
            }
        }
        else
        {
            if (FLEXCOM3_USART_TxPushByte(pWrBuffer[nBytesWritten]) == true)
            {
                nBytesWritten++;
            }
            else
            {
                /* Queue is full, exit the loop */
                break;
            }
        }
    }

    /* Check if any data is pending for transmission */
    if (FLEXCOM3_USART_WritePendingBytesGet() > 0)
    {
        /* Enable TX interrupt as data is pending for transmission */
        FLEXCOM3_USART_TX_INT_ENABLE();
    }

    return nBytesWritten;
}

size_t FLEXCOM3_USART_WriteFreeBufferCountGet(void)
{
    return (flexcom3UsartObj.wrBufferSize - 1) - FLEXCOM3_USART_WriteCountGet();
}

size_t FLEXCOM3_USART_WriteBufferSizeGet(void)
{
    return (flexcom3UsartObj.wrBufferSize - 1);
}

bool FLEXCOM3_USART_TransmitComplete(void)
{
    bool status = false;

    if (FLEXCOM3_REGS->FLEX_US_CSR & FLEX_US_CSR_TXEMPTY_Msk)
    {
        status = true;
    }

    return status;
}

bool FLEXCOM3_USART_WriteNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = flexcom3UsartObj.isWrNotificationEnabled;

    flexcom3UsartObj.isWrNotificationEnabled = isEnabled;

    flexcom3UsartObj.isWrNotifyPersistently = isPersistent;

    return previousStatus;
}

void FLEXCOM3_USART_WriteThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0)
    {
        flexcom3UsartObj.wrThreshold = nBytesThreshold;
    }
}

void FLEXCOM3_USART_WriteCallbackRegister( FLEXCOM_USART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    flexcom3UsartObj.wrCallback = callback;

    flexcom3UsartObj.wrContext = context;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static inline bool FLEXCOM3_USART_RxPushByte(uint16_t rdByte)
{
    uint32_t tempInIndex;
    bool isSuccess = false;

    tempInIndex = flexcom3UsartObj.rdInIndex + 1;

    if (tempInIndex >= flexcom3UsartObj.rdBufferSize)
    {
        tempInIndex = 0;
    }

    if (tempInIndex == flexcom3UsartObj.rdOutIndex)
    {
        /* Queue is full - Report it to the application. Application gets a chance to free up space by reading data out from the RX ring buffer */
        if(flexcom3UsartObj.rdCallback != NULL)
        {
            flexcom3UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_BUFFER_FULL, flexcom3UsartObj.rdContext);

            /* Read the indices again in case application has freed up space in RX ring buffer */
            tempInIndex = flexcom3UsartObj.rdInIndex + 1;

            if (tempInIndex >= flexcom3UsartObj.rdBufferSize)
            {
                tempInIndex = 0;
            }
        }
    }

    /* Attempt to push the data into the ring buffer */
    if (tempInIndex != flexcom3UsartObj.rdOutIndex)
    {
        if (FLEXCOM3_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
        {
            ((uint16_t*)&FLEXCOM3_USART_ReadBuffer)[flexcom3UsartObj.rdInIndex] = rdByte;
        }
        else
        {
            FLEXCOM3_USART_ReadBuffer[flexcom3UsartObj.rdInIndex] = (uint8_t)rdByte;
        }

        flexcom3UsartObj.rdInIndex = tempInIndex;
        isSuccess = true;
    }
    else
    {
        /* Queue is full. Data will be lost. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void FLEXCOM3_USART_ReadNotificationSend(void)
{
    uint32_t nUnreadBytesAvailable;

    if (flexcom3UsartObj.isRdNotificationEnabled == true)
    {
        nUnreadBytesAvailable = FLEXCOM3_USART_ReadCountGet();

        if(flexcom3UsartObj.rdCallback != NULL)
        {
            if (flexcom3UsartObj.isRdNotifyPersistently == true)
            {
                if (nUnreadBytesAvailable >= flexcom3UsartObj.rdThreshold)
                {
                    flexcom3UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_THRESHOLD_REACHED, flexcom3UsartObj.rdContext);
                }
            }
            else
            {
                if (nUnreadBytesAvailable == flexcom3UsartObj.rdThreshold)
                {
                    flexcom3UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_THRESHOLD_REACHED, flexcom3UsartObj.rdContext);
                }
            }
        }
    }
}

size_t FLEXCOM3_USART_Read(uint8_t* pRdBuffer, const size_t size)
{
    size_t nBytesRead = 0;
    uint32_t rdOutIndex = 0;
    uint32_t rdInIndex = 0;


    /* Take a snapshot of indices to avoid creation of critical section */
    rdOutIndex = flexcom3UsartObj.rdOutIndex;
    rdInIndex = flexcom3UsartObj.rdInIndex;

    while (nBytesRead < size)
    {
        if (rdOutIndex != rdInIndex)
        {
            if (FLEXCOM3_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
            {
                ((uint16_t*)pRdBuffer)[nBytesRead++] = ((uint16_t*)&FLEXCOM3_USART_ReadBuffer)[rdOutIndex++];
            }
            else
            {
                pRdBuffer[nBytesRead++] = FLEXCOM3_USART_ReadBuffer[rdOutIndex++];
            }

            if (rdOutIndex >= flexcom3UsartObj.rdBufferSize)
            {
                rdOutIndex = 0;
            }
        }
        else
        {
            /* No more data available in the RX buffer */
            break;
        }
    }

    flexcom3UsartObj.rdOutIndex = rdOutIndex;

    return nBytesRead;
}

size_t FLEXCOM3_USART_ReadCountGet(void)
{
    size_t nUnreadBytesAvailable;
    uint32_t rdOutIndex;
    uint32_t rdInIndex;


    /* Take a snapshot of indices to avoid creation of critical section */
    rdOutIndex = flexcom3UsartObj.rdOutIndex;
    rdInIndex = flexcom3UsartObj.rdInIndex;

    if ( rdInIndex >=  rdOutIndex)
    {
        nUnreadBytesAvailable =  rdInIndex -  rdOutIndex;
    }
    else
    {
        nUnreadBytesAvailable =  (flexcom3UsartObj.rdBufferSize -  rdOutIndex) + rdInIndex;
    }

    return nUnreadBytesAvailable;
}

size_t FLEXCOM3_USART_ReadFreeBufferCountGet(void)
{
    return (flexcom3UsartObj.rdBufferSize - 1) - FLEXCOM3_USART_ReadCountGet();
}

size_t FLEXCOM3_USART_ReadBufferSizeGet(void)
{
    return (flexcom3UsartObj.rdBufferSize - 1);
}


bool FLEXCOM3_USART_ReadNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = flexcom3UsartObj.isRdNotificationEnabled;

    flexcom3UsartObj.isRdNotificationEnabled = isEnabled;

    flexcom3UsartObj.isRdNotifyPersistently = isPersistent;


    return previousStatus;
}

void FLEXCOM3_USART_ReadThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0)
    {
        flexcom3UsartObj.rdThreshold = nBytesThreshold;

    }
}

void FLEXCOM3_USART_ReadCallbackRegister( FLEXCOM_USART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    flexcom3UsartObj.rdCallback = callback;

    flexcom3UsartObj.rdContext = context;
}

void static FLEXCOM3_USART_ISR_RX_Handler( void )
{
    uint16_t rdData = 0;

    /* Keep reading until there is a character availabe in the RX FIFO */
    while(FLEX_US_CSR_RXRDY_Msk == (FLEXCOM3_REGS->FLEX_US_CSR & FLEX_US_CSR_RXRDY_Msk))
    {
        if (FLEXCOM3_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
        {
            rdData = *((uint16_t*)&FLEXCOM3_REGS->FLEX_US_RHR) & FLEX_US_RHR_RXCHR_Msk;
        }
        else
        {
            rdData = *((uint8_t*)&FLEXCOM3_REGS->FLEX_US_RHR);
        }

        if (FLEXCOM3_USART_RxPushByte( rdData ) == true)
        {
            FLEXCOM3_USART_ReadNotificationSend();
        }
        else
        {
            /* UART RX buffer is full */
        }
    }


}

void static FLEXCOM3_USART_ISR_TX_Handler( void )
{
    uint16_t wrByte;

    /* Keep writing to the TX FIFO as long as there is space */
    while(FLEX_US_CSR_TXRDY_Msk == (FLEXCOM3_REGS->FLEX_US_CSR & FLEX_US_CSR_TXRDY_Msk))
    {
        if (FLEXCOM3_USART_TxPullByte(&wrByte) == true)
        {
            if (FLEXCOM3_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk)
            {
                *((uint16_t*)&FLEXCOM3_REGS->FLEX_US_THR) = wrByte & FLEX_US_THR_TXCHR_Msk;
            }
            else
            {
                *((uint8_t*)&FLEXCOM3_REGS->FLEX_US_THR) = (uint8_t)wrByte;
            }

            /* Send notification */
            FLEXCOM3_USART_WriteNotificationSend();
        }
        else
        {
            /* Nothing to transmit. Disable the data register empty/fifo Threshold interrupt. */
            FLEXCOM3_USART_TX_INT_DISABLE();
            break;
        }
    }

}

void FLEXCOM3_InterruptHandler( void )
{
    /* Channel status */
    uint32_t channelStatus = FLEXCOM3_REGS->FLEX_US_CSR;

    /* Error status */
    uint32_t errorStatus = (channelStatus & (FLEX_US_CSR_OVRE_Msk | FLEX_US_CSR_FRAME_Msk | FLEX_US_CSR_PARE_Msk));


    if(errorStatus != 0)
    {
        /* Save the error so that it can be reported when application calls the FLEXCOM3_USART_ErrorGet() API */
        flexcom3UsartObj.errorStatus = (FLEXCOM_USART_ERROR)errorStatus;

        /* Clear the error flags and flush out the error bytes */
        FLEXCOM3_USART_ErrorClear();

        /* USART errors are normally associated with the receiver, hence calling receiver context */
        if( flexcom3UsartObj.rdCallback != NULL )
        {
            flexcom3UsartObj.rdCallback(FLEXCOM_USART_EVENT_READ_ERROR, flexcom3UsartObj.rdContext);
        }
    }

    /* Receiver status. RX interrupt is never disabled. */
    if (channelStatus & FLEX_US_CSR_RXRDY_Msk)
    {
        FLEXCOM3_USART_ISR_RX_Handler();
    }

    /* Transmitter status */
    if( (channelStatus & FLEX_US_CSR_TXRDY_Msk) && (FLEXCOM3_REGS->FLEX_US_IMR & FLEX_US_IMR_TXRDY_Msk) )
    {
        FLEXCOM3_USART_ISR_TX_Handler();
    }
}
