/*
 * File      : drv_sd.c
 * LPC MCI  Driver
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-11-06     xiaonong      first version.
 */

#include <string.h>
#include "drv_sd.h"

//#define MCI_TRACE

#ifdef MCI_TRACE
#define MCI_DEBUG(...)         rt_kprintf("[MCI] %d ", rt_tick_get()); rt_kprintf(__VA_ARGS__);
#else
#define MCI_DEBUG(...)
#endif /* #ifdef MCI_TRACE */

#define MCI_DMA_WRITE_CHANNEL            (0)
#define MCI_DMA_READ_CHANNEL             (1)
#define DMA_MCI_SIZE                     BLOCK_LENGTH
#define MCI_ACMD41_HCS_POS                     (30)

#define MCI_PWRCTRL_BMASK                      (0xC3)

#define MCI_PWRCTRL_OPENDRAIN_POS              (6)
#define MCI_PWRCTRL_OPENDRAIN_NUMBIT           (1)
#define MCI_PWRCTRL_OPENDRAIN_BMASK            (0x01)

#define SDCARD_DET_VALUE ((LPC_GPIO2->PIN>>19)&0x01)

#define  MCI_RAM_BASE LPC_PERI_RAM_BASE

///* This is the area original data is stored or data to be written to the SD/MMC card. */
//#define MCI_DMA_SRC_ADDR        MCI_RAM_BASE
///* This is the area, after reading from the SD/MMC*/
//#define MCI_DMA_DST_ADDR        (MCI_RAM_BASE + MCI_DMA_SIZE)

rt_uint32_t dmaWrCh_TermianalCnt, dmaWrCh_ErrorCnt;
rt_uint32_t dmaRdCh_TermianalCnt, dmaRdCh_ErrorCnt;

static struct mci_device* _mci_device;
volatile rt_uint32_t CardRCA;

volatile rt_uint8_t *dataSrcBlock = (rt_uint8_t*)MCI_DMA_SRC_ADDR;
volatile rt_uint8_t *dataDestBlock = (rt_uint8_t*)MCI_DMA_DST_ADDR;

volatile rt_uint8_t CCS;
rt_bool_t MCI_SettingDma(rt_uint8_t *memBuf, rt_uint32_t ChannelNum, rt_uint32_t DMAMode);
static rt_err_t mci_get_cardStatus(int32_t *cardStatus);
static void mci_tx_enable(rt_bool_t enable);
static void mci_rx_enable(rt_bool_t enable);
static rt_err_t mci_check_status(uint8_t expect_status);
static void mci_set_clock(rt_uint32_t clk);
static rt_err_t mci_send_buswidth(rt_uint32_t buswidth);
static void mci_set_outputMode(rt_uint32_t mode);

static void mci_tx_enable(rt_bool_t enable)
{
    if (RT_TRUE == enable)
    {
        LPC_MCI->MASK0 |= ((DATA_END_INT_MASK) | (ERR_TX_INT_MASK));  /* Enable TX interrupt*/
    }
    else
    {
        LPC_MCI->MASK0 &= ~((DATA_END_INT_MASK) | (ERR_TX_INT_MASK));  /* disable TX interrupt*/
    }
}
static void mci_rx_enable(rt_bool_t enable)
{
    if (RT_TRUE == enable)
    {
        LPC_MCI->MASK0 |= ((DATA_END_INT_MASK) | (ERR_RX_INT_MASK));   /* Enable RX interrupt*/
    }
    else
    {
        LPC_MCI->MASK0 &= ~((DATA_END_INT_MASK) | (ERR_RX_INT_MASK));  /* disable RX interrupt*/
    }
}
/*********************************************************************//**
 * @brief      Check if the card is in the given state.
 *@param       expect_status    expected status
 * @details    Continuously get the card status until the card is ready. if its status matches
 *             with the given state, return with success. Else, return MCI_FUNC_ERR_STATE.
 *             If the card is still not ready, return MCI_FUNC_NOT_READY.
 *
 * @param      None
 *
 * @return     MCI_FUNC_OK if all success
 *************************************************************************/
static rt_err_t mci_check_status(uint8_t expect_status)
{
    int32_t respValue;
    rt_err_t retval = RT_ERROR;
    rt_uint32_t retryCnt = 0xFFFF;
    while (retryCnt > 0)
    {
        if (mci_get_cardStatus(&respValue) != RT_EOK)
        {
            break;
        }
        else
        {
            /* The only valid state is TRANS per MMC and SD state diagram.
            RCV state may be seen, but, it happens only when TX_ACTIVE or
            RX_ACTIVE occurs before the WRITE_BLOCK and READ_BLOCK cmds are
            being sent, which is not a valid sequence. */
            if (!(respValue & CARD_STATUS_READY_FOR_DATA))
            {
                retval = RT_EIO;
            }
            else if (CARDSTATEOF(respValue) != expect_status)
            {
                // If card is in prg state, wait until it changes to trans state
                // when "operation complete"
                if (CARDSTATEOF(respValue) != CARD_STATE_PRG)
                {
                    return RT_ERROR;
                }
            }
            else
            {
                return RT_EOK;
            }
        }
        retryCnt--;
        // rt_thread_delay(1);

    }

    return retval;
}
#if MCI_DMA_ENABLED


rt_bool_t MCI_SettingDma(rt_uint8_t *memBuf, rt_uint32_t ChannelNum, rt_uint32_t DMAMode)
{
    GPDMA_Channel_CFG_Type GPDMACfg;

    // Transfer size
    GPDMACfg.TransferSize = DMA_MCI_SIZE;
    // Transfer width
    GPDMACfg.TransferWidth = GPDMA_WIDTH_WORD;
    // Transfer type
    GPDMACfg.TransferType = DMAMode;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;

    /* USB RAM is used for test.
    Please note, Ethernet has its own SRAM, but GPDMA can't access
    that. GPDMA can access USB SRAM and IRAM. Ethernet DMA controller can
    access both IRAM and Ethernet SRAM. */
    GPDMACfg.ChannelNum = ChannelNum;

    if (DMAMode == GPDMA_TRANSFERTYPE_M2P_DEST_CTRL)
    {
        /* Ch0 set for M2P transfer from mempry to MCI FIFO. */
        // Source memory
        GPDMACfg.SrcMemAddr = (rt_uint32_t)memBuf;
        // Destination memory
        GPDMACfg.DstMemAddr = (rt_uint32_t)LPC_MCI->FIFO;

        // Source connection
        GPDMACfg.SrcConn = 0;
        // Destination connection
        GPDMACfg.DstConn = GPDMA_CONN_MCI;

    }
    else if (DMAMode == GPDMA_TRANSFERTYPE_P2M_SRC_CTRL)
    {
        /* Ch0 set for P2M transfer from MCI FIFO to memory. */
        // Source memory
        GPDMACfg.SrcMemAddr = (rt_uint32_t)LPC_MCI->FIFO;
        // Destination memory
        GPDMACfg.DstMemAddr = (rt_uint32_t)memBuf;

        // Source connection
        GPDMACfg.SrcConn = GPDMA_CONN_MCI;
        // Destination connection
        GPDMACfg.DstConn = 0;
    }
    else
    {
        return (RT_FALSE);
    }

    // Setup channel with given parameter
    GPDMA_Setup(&GPDMACfg);

    // Enable GPDMA channel
    GPDMA_ChannelCmd(ChannelNum, ENABLE);

    /* Enable GPDMA interrupt */
    NVIC_EnableIRQ(DMA_IRQn);

    return (RT_TRUE);
}

void MCI_DMA_IRQHandler(void)
{
    // check GPDMA interrupt on channel 0
    if (GPDMA_IntGetStatus(GPDMA_STAT_INT, MCI_DMA_WRITE_CHANNEL))
    {
        //check interrupt status on channel 0
        // Check counter terminal status
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, MCI_DMA_WRITE_CHANNEL))
        {
            // Clear terminate counter Interrupt pending
            GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, MCI_DMA_WRITE_CHANNEL);

            dmaWrCh_TermianalCnt++;
        }
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, MCI_DMA_WRITE_CHANNEL))
        {
            // Clear error counter Interrupt pending
            GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, MCI_DMA_WRITE_CHANNEL);

            dmaWrCh_ErrorCnt++;
        }
    }
    else if (GPDMA_IntGetStatus(GPDMA_STAT_INT, MCI_DMA_READ_CHANNEL))
    {
        //check interrupt status on channel 0
        // Check counter terminal status
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, MCI_DMA_READ_CHANNEL))
        {
            // Clear terminate counter Interrupt pending
            GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, MCI_DMA_READ_CHANNEL);

            dmaRdCh_TermianalCnt++;
        }
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, MCI_DMA_READ_CHANNEL))
        {
            // Clear error counter Interrupt pending
            GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, MCI_DMA_READ_CHANNEL);

            dmaRdCh_ErrorCnt++;
        }
    }
}
/**************************************************************************/
/*!
    @brief DMA Handler
*/
/**************************************************************************/
void DMA_IRQHandler(void)
{
    rt_interrupt_enter();
    MCI_DMA_IRQHandler();
    rt_interrupt_leave();
}

#endif

static void mci_set_clock(rt_uint32_t clk)
{
    rt_uint32_t value = 0;
    rt_uint32_t pclk;

    pclk = PeripheralClock;

    value = (pclk + 2 * clk - 1) / (2 * clk);
    if (value > 0)
    {
        value -= 1;
    }
    LPC_MCI->CLOCK = (LPC_MCI->CLOCK & ~(0xFF)) | (1 << 8)  | value;

    // rt_thread_delay(1);    /* delay 3MCLK + 2PCLK before next write */

}


rt_err_t mci_set_buswidth(rt_uint32_t width)
{
    uint8_t bus_width = BUS_WIDTH_1BIT;

    if (width == SD_1_BIT)
    {
        LPC_MCI->CLOCK &=  ~(1 << 11);    /* 1 bit bus */
    }
    else if (width == SD_4_BIT)
    {
        LPC_MCI->CLOCK |= (1 << 11); /* 4 bit bus */
        bus_width = BUS_WIDTH_4BITS;
    }

    // rt_thread_delay(1);    /* delay 3MCLK + 2PCLK  */
    if ((_mci_device->card_type == MCI_SDSC_V1_CARD) ||
            (_mci_device->card_type == MCI_SDSC_V2_CARD) ||
            (_mci_device->card_type == MCI_SDHC_SDXC_CARD))
    {
        if (mci_send_buswidth(bus_width) != RT_EOK)
        {
            return RT_ERROR;
        }
    }
    return RT_EOK;
}
/************************************************************************//**
 * @brief         Set output in open drain mode or pushpull mode
 *
 * @param[in]    mode the mode going to set
 *
 * @return         None
 ***************************************************************************/
static void mci_set_outputMode(rt_uint32_t mode)
{
    if (mode == MCI_OUTPUT_MODE_OPENDRAIN)
    {
        /* Set Open Drain output control for MMC */
        LPC_MCI->POWER |= (1 << MCI_PWRCTRL_OPENDRAIN_POS) & MCI_PWRCTRL_BMASK;
    }
    else
    {
        /* Clear Open Drain output control for SD */
        LPC_MCI->POWER &= (~(1 << MCI_PWRCTRL_OPENDRAIN_POS) & MCI_PWRCTRL_BMASK);
    }
    // rt_thread_delay(1);    /* delay 3MCLK + 2PCLK  */
}
void mci_process_ISR()
{
    rt_uint32_t mci_status;
    mci_status = LPC_MCI->STATUS;

    /* handle MCI_STATUS interrupt */
    if (mci_status & DATA_ERR_INT_MASK)
    {
        if (mci_status &  MCI_DATA_CRC_FAIL)
        {
            LPC_MCI->CLEAR = MCI_DATA_CRC_FAIL;
            MCI_DEBUG("CRC FAIL!\n");
        }

        if (mci_status &  MCI_DATA_TIMEOUT)
        {
            LPC_MCI->CLEAR =  MCI_DATA_TIMEOUT;
            MCI_DEBUG("data time out!\n");
        }

        /* Underrun or overrun */
        if (mci_status &  MCI_TX_UNDERRUN)
        {
            LPC_MCI->CLEAR = MCI_TX_UNDERRUN;
            MCI_DEBUG("mci tx under run!\n");
        }

        if (mci_status &  MCI_RX_OVERRUN)
        {
            LPC_MCI->CLEAR =  MCI_RX_OVERRUN;
            MCI_DEBUG("mci rx over run!\n");
        }

        /* Start bit error on data signal */
        if (mci_status &  MCI_START_BIT_ERR)
        {
            LPC_MCI->CLEAR =  MCI_START_BIT_ERR;
            MCI_DEBUG("mci start bit error!\n");
        }
          _mci_device->data_error = RT_TRUE;
         rt_event_send(_mci_device->finish_event,1);
       

    }
    else if (mci_status & DATA_END_INT_MASK)
    {
        if (mci_status &  MCI_DATA_END)          /* Data end, and Data block end  */
        {
            LPC_MCI->CLEAR = MCI_DATA_END;
             _mci_device->data_error = RT_FALSE;
					  rt_event_send(_mci_device->finish_event,1);
          
            mci_tx_enable(RT_FALSE);

            mci_rx_enable(RT_FALSE);
        }

        if (mci_status &  MCI_DATA_BLK_END)
        {
            LPC_MCI->CLEAR =  MCI_DATA_BLK_END;

            //MCI_TXDisable();

            return;
        }

        /* Tx active  */
        if (mci_status & MCI_TX_ACTIVE)
        {

        }

        /* Rx active  */
        if (mci_status & MCI_RX_ACTIVE)
        {

        }
    }
    else if (mci_status & CMD_INT_MASK)
    {
        if (mci_status &  MCI_CMD_CRC_FAIL)
        {
            LPC_MCI->CLEAR =  MCI_CMD_CRC_FAIL;
        }

        if (mci_status &  MCI_CMD_TIMEOUT)
        {
            LPC_MCI->CLEAR =  MCI_CMD_TIMEOUT;
        }

        /* Cmd Resp End or Cmd Sent */
        if (mci_status &  MCI_CMD_RESP_END)
        {
            LPC_MCI->CLEAR =  MCI_CMD_RESP_END;
        }

        if (mci_status &  MCI_CMD_SENT)
        {
            LPC_MCI->CLEAR =  MCI_CMD_SENT;
        }

        if (mci_status &  MCI_CMD_ACTIVE)
        {
            LPC_MCI->CLEAR =  MCI_CMD_ACTIVE;
        }

    }
    else if (mci_status & FIFO_INT_MASK)
    {
        /* no used in dma*/
        return;
    }

}
void MCI_IRQHandler(void)
{
    rt_interrupt_enter();
    mci_process_ISR();
    rt_interrupt_leave();
}
rt_err_t mci_send_cmd(mci_cmd_t *cmd)
{
    rt_uint32_t cmd_data = 0;
    rt_uint32_t cmd_status;
    volatile uint32_t i;
    rt_uint32_t command = cmd->command;
    rt_uint32_t arg = cmd->argument;
    rt_uint32_t expect_resp = cmd->expect_resp;
    rt_uint32_t allow_timeout = cmd->allow_timeout;

    /* the command engine must be disabled when we modify the argument
    or the peripheral resends */
    while ((cmd_status = LPC_MCI->STATUS) & MCI_CMD_ACTIVE)     /* Command in progress. */
    {
        LPC_MCI->COMMAND = 0;
        for (i = 0; i < 0x10; i++);      /* delay 3MCLK + 2PCLK  */
        LPC_MCI->CLEAR = cmd_status | MCI_CMD_ACTIVE;
    }


    /*set the command details, the CmdIndex should 0 through 0x3F only */
    cmd_data |= (command & 0x3F);    /* bit 0 through 5 only */

    if (expect_resp == EXPECT_NO_RESP)              /* no response */
    {
        cmd_data &= ~((1 << 6) | (1 << 7));        /* Clear long response bit as well */
    }
    else if (expect_resp == EXPECT_SHORT_RESP)      /* expect short response */
    {
        cmd_data |= (1 << 6);
    }
    else if (expect_resp == EXPECT_LONG_RESP)      /* expect long response */
    {
        cmd_data |= (1 << 6) | (1 << 7);
    }

    if (allow_timeout == ALLOW_CMD_TIMER)              /* allow timeout or not */
    {
        cmd_data &= ~ MCI_DISABLE_CMD_TIMER;
    }
    else
    {
        cmd_data |= MCI_DISABLE_CMD_TIMER;
    }

    /*send the command*/
    cmd_data |= (1 << 10);        /* This bit needs to be set last. */

    // clear status register
    LPC_MCI->CLEAR = 0x7FF;

    LPC_MCI->ARGUMENT = arg;    /* Set the argument first, finally command */

    LPC_MCI->COMMAND = cmd_data;

    for (i = 0; i < 0x10; i++);       /* delay 3MCLK + 2PCLK  */

    // Wait until command is processed
    while (!LPC_MCI->STATUS);

    // Wait until command sent
    while (LPC_MCI->STATUS & MCI_CMD_ACTIVE);

    return RT_EOK;

}
rt_err_t mci_get_cmdresp(rt_uint32_t ExpectCmdData, rt_uint32_t ExpectResp, rt_uint32_t *CmdResp)
{
    rt_uint32_t CmdRespStatus = 0;
    rt_uint32_t LastCmdIndex;
    uint32_t i = 0;

    if (ExpectResp == EXPECT_NO_RESP)
    {
        return RT_EOK;
    }

    while (1)
    {
        // Get the status of the component
        CmdRespStatus = LPC_MCI->STATUS;

        if (CmdRespStatus & (MCI_CMD_TIMEOUT))
        {
            LPC_MCI->CLEAR = CmdRespStatus | MCI_CMD_TIMEOUT;

            LPC_MCI->COMMAND = 0;
            LPC_MCI->ARGUMENT = 0xFFFFFFFF;

            for (i = 0; i < 0x10; i++);

            return (CmdRespStatus);
        }

        if (CmdRespStatus & MCI_CMD_CRC_FAIL)
        {
            LPC_MCI->CLEAR = CmdRespStatus | MCI_CMD_CRC_FAIL;
            LastCmdIndex = LPC_MCI->COMMAND & 0x003F;

            if ((LastCmdIndex == CMD1_SEND_OP_COND) || (LastCmdIndex == ACMD41_SEND_APP_OP_COND)
                    || (LastCmdIndex == CMD12_STOP_TRANSMISSION))
            {
                LPC_MCI->COMMAND = 0;
                LPC_MCI->ARGUMENT = 0xFFFFFFFF;
                for (i = 0; i < 0x10; i++);      /* delay 3MCLK + 2PCLK  */
                break;            /* ignore CRC error if it's a resp for SEND_OP_COND
                                or STOP_TRANSMISSION. */
            }
            else
            {
                return (CmdRespStatus);
            }
        }
        else if (CmdRespStatus & MCI_CMD_RESP_END)
        {
            LPC_MCI->CLEAR = CmdRespStatus | MCI_CMD_RESP_END;
            break;    /* cmd response is received, expecting response */
        }

    }

    if ((LPC_MCI->RESP_CMD & 0x3F) != ExpectCmdData)
    {
        /* If the response is not R1, in the response field, the Expected Cmd data
        won't be the same as the CMD data in SendCmd(). Below four cmds have
        R2 or R3 response. We don't need to check if MCI_RESP_CMD is the same
        as the Expected or not. */
        if ((ExpectCmdData != CMD1_SEND_OP_COND) && (ExpectCmdData != ACMD41_SEND_APP_OP_COND)
                && (ExpectCmdData != CMD2_ALL_SEND_CID) && (ExpectCmdData != CMD9_SEND_CSD))
        {
            CmdRespStatus = INVALID_RESPONSE;    /* Reuse error status */
            return (INVALID_RESPONSE);
        }
    }

    /* Read MCI_RESP0 register assuming it's not long response. */
    if (CmdResp != NULL)
    {
        if (ExpectResp == EXPECT_SHORT_RESP)
        {
            *(CmdResp + 0) = LPC_MCI->RESP0;
        }
        else if (ExpectResp == EXPECT_LONG_RESP)
        {
            *(CmdResp + 0) = LPC_MCI->RESP0;
            *(CmdResp + 1) = LPC_MCI->RESP1;
            *(CmdResp + 2) = LPC_MCI->RESP2;
            *(CmdResp + 3) = LPC_MCI->RESP3;
        }
    }

    return RT_EOK;
}


rt_err_t mci_cmd_resp(mci_cmd_t *pCmdIf)
{
    int32_t respStatus;
    rt_uint32_t CmdIndex = pCmdIf->command;
    rt_uint32_t ExpectResp = pCmdIf->expect_resp;
    rt_uint32_t *CmdResp = pCmdIf->cmd_resp;


    mci_send_cmd(pCmdIf);

    if ((CmdResp != NULL) || (ExpectResp != EXPECT_NO_RESP))
    {
        respStatus = mci_get_cmdresp(CmdIndex, ExpectResp, CmdResp);
    }
    else
    {
        respStatus = RT_ERROR;
    }

    return respStatus;
}


rt_err_t mci_card_reset(void)
{
    mci_cmd_t cmdIf;
    /* Because CMD0 command to put the device to idle state does not need response
    since, it's only sending commad */
    cmdIf.command = CMD0_GO_IDLE_STATE;
    cmdIf.argument = 0x00000000;
    cmdIf.expect_resp = EXPECT_NO_RESP;
    cmdIf.allow_timeout = 0;
    cmdIf.cmd_resp = 0;
    mci_send_cmd(&cmdIf);

    return RT_EOK;
}


/************************************************************************//**
 * @brief        Send CMD1 (SEND_OP_COND) to card.
 *
 * @param        None
 *
 * @return       MCI_FUNC_OK if all success
 ****************************************************************************/
rt_err_t mci_cmd_sendOpCond(void)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    mci_cmd_t cmdIf;
    rt_err_t retval = RT_ERROR;

    retryCount = 0x200;            /* reset retry counter */

    cmdIf.command = CMD1_SEND_OP_COND;
    cmdIf.argument = OCR_INDEX;
    cmdIf.expect_resp = EXPECT_SHORT_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];
    /* continuously sends until the busy bit is cleared */
    while (retryCount > 0)
    {
        respStatus = mci_cmd_resp(&cmdIf);

        if (respStatus & MCI_CMD_TIMEOUT)
        {
            retval = RT_ETIMEOUT;
        }
        else if ((respValue[0] & 0x80000000) == 0)
        {
            //The card has not finished the power up routine
            retval = RT_EIO;
        }
        else
        {
            retval = RT_EOK;
            break;
        }

        rt_thread_delay(1);

        retryCount--;
    }

    return(retval);
}


/************************************************************************//**
 * @brief        Send CMD8 (SEND_IF_COND) for interface condition to card.
 *
 * @param        None
 *
 * @return       MCI_FUNC_OK if all success
 ****************************************************************************/
rt_err_t mci_cmd_sendIfCond(void)
{
    rt_uint32_t retryCount;
    rt_uint32_t CmdArgument;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];

    rt_err_t retval = RT_ERROR;

    rt_uint8_t voltageSupplied = MCI_CMD8_VOLATAGESUPPLIED_27_36;//in range 2.7-3.6V
    rt_uint8_t checkPattern = 0xAA;
    mci_cmd_t cmdIf;

    CmdArgument = (voltageSupplied << MCI_CMD8_VOLTAGESUPPLIED_POS) | (checkPattern << MCI_CMD8_CHECKPATTERN_POS);

    retryCount = 20;

    cmdIf.command = CMD8_SEND_IF_COND;
    cmdIf.argument = CmdArgument;
    cmdIf.expect_resp = EXPECT_SHORT_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];
    while (retryCount > 0)
    {
        respStatus = mci_cmd_resp(&cmdIf);

        if (respStatus & MCI_CMD_TIMEOUT)
        {
            //Consider as no response
            retval = RT_ETIMEOUT;
        }
        else if (((respValue[0] >> MCI_CMD8_CHECKPATTERN_POS) & MCI_CMD8_CHECKPATTERN_BMASK) != checkPattern)
        {
            return RT_ERROR;
        }
        else if (((respValue[0] >> MCI_CMD8_VOLTAGESUPPLIED_POS) & MCI_CMD8_VOLTAGESUPPLIED_BMASK)
                 != voltageSupplied)
        {
            return RT_ERROR;
        }
        else
        {
            return RT_EOK;
        }

        // rt_thread_delay(1);

        retryCount--;
    }

    return retval;
}


/************************************************************************//**
 * @brief        Send CMD55 (APP_CMD) to indicate to the card that the next
 *               command is an application specific command rather than a
 *               standard command. Before an ACMD, call this routine first
 *
 * @param        None
 *
 * @return       MCI_FUNC_OK if all success
 ****************************************************************************/
rt_err_t mci_cmd_sendACMD(void)
{
    rt_uint32_t retryCount;
    rt_uint32_t CmdArgument;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    mci_cmd_t cmdIf;
    rt_err_t retval = RT_EOK;

    if ((_mci_device->card_type == MCI_SDSC_V1_CARD) ||
            (_mci_device->card_type == MCI_SDSC_V2_CARD) ||
            (_mci_device->card_type == MCI_SDHC_SDXC_CARD))
    {
        CmdArgument = CardRCA;    /* Use the address from SET_RELATIVE_ADDR cmd */
    }
    else            /* if MMC or unknown card type, use 0x0. */
    {
        CmdArgument = 0x00000000;
    }

    retryCount = 20;

    cmdIf.command = CMD55_APP_CMD;
    cmdIf.argument = CmdArgument;
    cmdIf.expect_resp = EXPECT_SHORT_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];
    while (retryCount > 0)
    {
        respStatus = mci_cmd_resp(&cmdIf);

        if (respStatus != 0)
        {
            retval = RT_ERROR;
        }
        else if (respValue[0] & CARD_STATUS_ACMD_ENABLE)
        {
            retval = RT_EOK;
            break;
        }
        else
        {
            retval = RT_EIO;
        }

        //  rt_thread_delay(1);

        retryCount--;
    }

    return retval;
}


/************************************************************************//**
 * @brief        Send ACMD41 (SEND_APP_OP_COND) to Host Capacity Support (HCS)
 *               information and asks the accessed card to send its operating
 *               condition (OCR).
 *
 * @param[in]    hcsVal input the Host Capacity Support
 *
 * @return       MCI_FUNC_OK if all success
 *
 * @note         If SEND_APP_OP_COND is timeout, the card in the slot is not MMC
 *                type, try this combination to see if we can communicate with
 *                a SD type.
 ****************************************************************************/
rt_err_t mci_acmd_sendOpCond(uint8_t hcsVal)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus, argument;
    rt_uint32_t respValue[4];
    mci_cmd_t cmdIf;

    rt_err_t retval = RT_ERROR;

    argument = OCR_INDEX | (hcsVal << MCI_ACMD41_HCS_POS);

    /* timeout on SEND_OP_COND command on MMC, now, try SEND_APP_OP_COND
    command to SD */
    retryCount = 0x2000;            /* reset retry counter */

    cmdIf.command = ACMD41_SEND_APP_OP_COND;
    cmdIf.argument = argument;
    cmdIf.expect_resp = EXPECT_SHORT_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];

    /* Clear Open Drain output control for SD */
    mci_set_outputMode(MCI_OUTPUT_MODE_PUSHPULL);

    /* The host repeatedly issues ACMD41 for at least 1 second or */
    /* until the busy bit are set to 1 */
    while (retryCount > 0)
    {
        if ((retval = mci_cmd_sendACMD()) == RT_EOK)
        {
            respStatus = mci_cmd_resp(&cmdIf);

            if (respStatus & MCI_CMD_TIMEOUT)
            {
                retval = RT_ETIMEOUT;
            }
            else if (!(respValue[0] & 0x80000000))
            {
                retval = RT_EIO;
            }
            else
            {
                CCS = (respValue[0] & (1 << 30)) ? 1 : 0;
                retval = RT_EOK;
                break;
            }
        }
        else    /* The command isn't accepted by the card.*/
        {
            return retval;
        }

        // rt_thread_delay(1);

        retryCount--;
    }

    return retval;
}


/************************************************************************//**
 * @brief         Get the all the Identifier (CID) of the card by sending the
 *                CMD2 (ALL_SEND_CID) command. Then parse 4-byte data obtained
 *                from the card by the CID meaning.
 *
 * @param[out]    cidValue the CID Result after parsing the data from the card
 *
 * @return        MCI_FUNC_OK if all success
 ****************************************************************************/
rt_err_t mci_get_CID(mci_cid_t *cidValue)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    mci_cmd_t cmdIf;

    /* This command is normally after CMD1(MMC) or ACMD41(SD). */
    retryCount = 0x200;// 0x20;            /* reset retry counter */

    cmdIf.command = CMD2_ALL_SEND_CID;
    cmdIf.argument = 0;
    cmdIf.expect_resp = EXPECT_LONG_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];
    while (retryCount > 0)
    {
        respStatus = mci_cmd_resp(&cmdIf);

        /* bit 0 and bit 2 must be zero, or it's timeout or CRC error */
        //if ((!(respStatus & MCI_CMD_TIMEOUT)) && (!(respStatus & MCI_CMD_CRC_FAIL)))
        if (!(respStatus & MCI_CMD_TIMEOUT))
        {
            // Parsing the data retrieved
            if (cidValue != NULL)
            {
                cidValue->MID = (respValue[0] >> MCI_CID_MANUFACTURER_ID_WPOS) & MCI_CID_MANUFACTURER_ID_WBMASK;

                cidValue->OID = (respValue[0] >> MCI_CID_OEMAPPLICATION_ID_WPOS) & MCI_CID_OEMAPPLICATION_ID_WBMASK;

                cidValue->PNM_H = (respValue[0] >> MCI_CID_PRODUCTNAME_ID_H_WPOS) & MCI_CID_PRODUCTNAME_ID_H_WBMASK;

                cidValue->PNM_L = (respValue[1] >> MCI_CID_PRODUCTNAME_ID_L_WPOS) & MCI_CID_PRODUCTNAME_ID_L_WBMASK;

                cidValue->PRV = (respValue[2] >> MCI_CID_PRODUCTREVISION_ID_WPOS) & MCI_CID_PRODUCTREVISION_ID_WBMASK;

                cidValue->PSN = (((respValue[2] >> MCI_CID_PRODUCTSERIALNUM_ID_H_WPOS) & MCI_CID_PRODUCTSERIALNUM_ID_H_WBMASK) << 8)
                                | ((respValue[3] >> MCI_CID_PRODUCTSERIALNUM_ID_L_WPOS) & MCI_CID_PRODUCTSERIALNUM_ID_L_WBMASK);

                cidValue->reserved = (respValue[3] >> MCI_CID_RESERVED_ID_WPOS) & MCI_CID_RESERVED_ID_WBMASK;

                cidValue->MDT = (respValue[3] >> MCI_CID_MANUFACTURINGDATE_ID_WPOS) & MCI_CID_MANUFACTURINGDATE_ID_WBMASK;

                cidValue->CRC = (respValue[3] >> MCI_CID_CHECKSUM_ID_WPOS) & MCI_CID_CHECKSUM_ID_WBMASK;

                cidValue->unused = (respValue[3] >> MCI_CID_UNUSED_ID_WPOS) & MCI_CID_UNUSED_ID_WBMASK;

            }

            return  RT_EOK;    /* response is back and correct. */
        }


        rt_thread_delay(1);

        retryCount--;
    }

    return RT_ETIMEOUT;
}


/************************************************************************//**
 * @brief        Set the address for the card in the slot by sending CMD3
 *                (SET_RELATIVE_ADDR) command. To get the address of the card
 *                currently in used, needs to call MCI_GetCardAddress()
 *
 * @param        None
 *
 * @return       MCI_FUNC_OK if all success
 ****************************************************************************/
rt_err_t mci_set_cardAddress(void)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue;
    rt_uint32_t CmdArgument;
    mci_cmd_t cmdIf;
    rt_err_t retval = RT_ERROR;

    /* If it's a SD card, SET_RELATIVE_ADDR is to get the address
    from the card and use this value in RCA, if it's a MMC, set default
    RCA addr. 0x00010000. */
    if ((_mci_device->card_type == MCI_SDSC_V1_CARD) ||
            (_mci_device->card_type == MCI_SDSC_V2_CARD) ||
            (_mci_device->card_type == MCI_SDHC_SDXC_CARD))
    {
        CmdArgument = 0;
    }
    else            /* If it's unknown or MMC_CARD, fix the RCA address */
    {
        CmdArgument = 0x00010000;
    }

    retryCount = 0x20;            /* reset retry counter */
    cmdIf.command = CMD3_SET_RELATIVE_ADDR;
    cmdIf.argument = CmdArgument;
    cmdIf.expect_resp = EXPECT_SHORT_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = &respValue;
    while (retryCount > 0)
    {
        /* Send CMD3 command repeatedly until the response is back correctly */
        respStatus = mci_cmd_resp(&cmdIf);

        if (respStatus & MCI_CMD_TIMEOUT)
        {
            retval = RT_ETIMEOUT;
        }
        else if (!((respValue >> RCA_RES_CARD_STATUS_POS)& CARD_STATUS_READY_FOR_DATA))
        {
            retval = RT_EIO;
        }
        else if ((CARDSTATEOF(respValue) != MCI_CARDSTATE_IDENDTIFIED))
        {
            retval = RT_EIO;
        }
        else
        {
            CardRCA = (respValue >> RCA_RES_NEW_PUBLISHED_RCA_POS) & RCA_RES_NEW_PUBLISHED_RCA_MASK;    /* Save the RCA value from SD card */

            CardRCA <<= RCA_ARGUMENT_POS;

            mci_set_outputMode(MCI_OUTPUT_MODE_PUSHPULL);

            return RT_EOK;    /* response is back and correct. */
        }

        rt_thread_delay(1);

        retryCount--;
    }

    return retval;
}


/************************************************************************//**
 * @brief        Get the address for the card in the slot
 *
 * @param        None
 *
 * @return       MCI_FUNC_OK if all success
 *
 * @note        This function must be called after MCI_SetCardAddress() executing
 ****************************************************************************/
rt_uint32_t mci_get_cardAddress(void)
{
    return CardRCA;
}


/************************************************************************//**
 * @brief       Get the Card-Specific Data of in-slot card by sending CMD9
 *                (SEND_CSD) command
 *
 * @param[out]  csdVal a buffer stored the value of CSD obtained from the card
 *
 * @return      MCI_FUNC_OK if all success
 *
 * @note        CMD9 (SEND_CSD) command should be sent only at standby state
 *                (STBY) after CMD3
 ****************************************************************************/
rt_err_t mci_get_CSD(rt_uint32_t *csdVal)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    rt_uint32_t CmdArgument;
    mci_cmd_t cmdIf;

    if ((_mci_device->card_type == MCI_SDSC_V1_CARD) ||
            (_mci_device->card_type == MCI_SDSC_V2_CARD) ||
            (_mci_device->card_type == MCI_SDHC_SDXC_CARD))
    {
        CmdArgument = CardRCA;
    }
    else            /* if MMC or unknown card type, use default RCA addr. */
    {
        CmdArgument = 0x00010000;
    }

    retryCount = 0x20;
    cmdIf.command = CMD9_SEND_CSD;
    cmdIf.argument = CmdArgument;
    cmdIf.expect_resp = EXPECT_LONG_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];
    while (retryCount > 0)
    {
        /* Check current status */
        if (((mci_check_status(CARD_STATE_STBY) != RT_EOK)))
            return RT_EIO;

        respStatus = mci_cmd_resp(&cmdIf);

        if (!respStatus)
        {
            if (csdVal != NULL)
            {
                csdVal[0] = respValue[0];
                csdVal[1] = respValue[1];
                csdVal[2] = respValue[2];
                csdVal[3] = respValue[3];
            }

            return RT_EOK;
        }

        rt_thread_delay(1);

        retryCount--;
    }

    return RT_ERROR;
}


/************************************************************************//**
 * @brief       Select the card by the specified address. This is done by sending
 *              out the CMD7 with the address argument to needed card
 *
 * @param       None
 *
 * @return      MCI_FUNC_OK if all success
 *
 * @note        CMD7 (SELECT_CARD) command should be sent after CMD9 ((SEND_CSD).
 *                The state will be inter-changed between STBY to TRANS by this
 *                CMD7 command
 ****************************************************************************/
rt_err_t mci_cmd_selectCard(void)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    rt_uint32_t CmdArgument;
    mci_cmd_t cmdIf;
    rt_err_t retval = RT_ERROR;

    if ((_mci_device->card_type == MCI_SDSC_V1_CARD) ||
            (_mci_device->card_type == MCI_SDSC_V2_CARD) ||
            (_mci_device->card_type == MCI_SDHC_SDXC_CARD))
    {
        CmdArgument = CardRCA;
    }
    else            /* if MMC or unknown card type, use default RCA addr. */
    {
        CmdArgument = 0x00010000;
    }

    retryCount = 0x20;
    cmdIf.command = CMD7_SELECT_CARD;
    cmdIf.argument = CmdArgument;
    cmdIf.expect_resp = EXPECT_SHORT_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];
    while (retryCount > 0)
    {
        /* Check current status */
        if (((mci_check_status(CARD_STATE_STBY) != RT_EOK)) &&
                (mci_check_status(CARD_STATE_TRAN) != RT_EOK) &&
                (mci_check_status(CARD_STATE_DATA) != RT_EOK) &&
                (mci_check_status(CARD_STATE_PRG) != RT_EOK) &&
                (mci_check_status(CARD_STATE_DIS) != RT_EOK))
            return RT_ERROR;

        respStatus = mci_cmd_resp(&cmdIf);

        if (respStatus)
        {
            retval = RT_ERROR;
        }
        else if (respValue[0] & CARD_STATUS_ERR_MASK)
        {
            return RT_EIO;
        }
        else
        {
            if (((mci_check_status(CARD_STATE_STBY) != RT_EOK)) &&
                    (mci_check_status(CARD_STATE_TRAN) != RT_EOK) &&
                    (mci_check_status(CARD_STATE_PRG) != RT_EOK) &&
                    (mci_check_status(CARD_STATE_DIS) != RT_EOK))
                return RT_ERROR;
            return RT_EOK;
        }

        rt_thread_delay(1);

        retryCount--;
    }

    return retval;
}


/************************************************************************//**
 * @brief         Get the status of the card. The return is from the card.
 *                By sending CMD13 (SEND_STATUS), the status of the card
 *                will be responded from card addressed
 *
 * @param[out]    cardStatus the status returned from the card
 *
 * @return        MCI_FUNC_OK if all success
 ****************************************************************************/
static rt_err_t mci_get_cardStatus(int32_t *cardStatus)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    rt_uint32_t CmdArgument;
    mci_cmd_t cmdIf;
    rt_err_t retval = RT_ERROR;
    ;

    if (cardStatus == NULL)
        return RT_EOK;

    if ((_mci_device->card_type == MCI_SDSC_V1_CARD) ||
            (_mci_device->card_type == MCI_SDSC_V2_CARD) ||
            (_mci_device->card_type == MCI_SDHC_SDXC_CARD))
    {
        CmdArgument = CardRCA;
    }
    else            /* if MMC or unknown card type, use default RCA addr. */
    {
        CmdArgument = 0x00010000;
    }

    retryCount = 0x20;
    cmdIf.command = CMD13_SEND_STATUS;
    cmdIf.argument = CmdArgument;
    cmdIf.expect_resp = EXPECT_SHORT_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];
    while (retryCount > 0)
    {
        respStatus = mci_cmd_resp(&cmdIf);

        if (respStatus) /* only retry if sending command failed */
        {
            retval = RT_ERROR;
        }
        else
        {
            *cardStatus = respValue[0];

            return RT_EOK;
        }
        // rt_thread_delay(1);
        retryCount--;
    }

    return retval;
}

/************************************************************************//**
 * @brief        Set the length for the blocks in the next action on data
 *                manipulation (as read, write, erase). This function is to
 *                send CMD16 (SET_BLOCK_LEN) to cards.
 *
 * @param[in]    blockLength the value for the length of block will be handled
 *
 * @return         MCI_FUNC_OK if all success
 *
 * @note         CMD16 command should be sent after the card is selected by CMD7
 *                (SELECT_CARD).
 *  In the case of SDHC and SDXC Cards, block length set by CMD16 command doen't
 *  affect memory read and write commands. Always 512 Bytes fixed block length is
 *  used. This command is effective for LOCK_UNLOCK command..
 ****************************************************************************/
rt_err_t mci_set_blockLen(rt_uint32_t blockLength)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    mci_cmd_t cmdIf;
    rt_err_t retval = RT_ERROR;

    retryCount = 0x20;
    cmdIf.command = CMD16_SET_BLOCK_LEN;
    cmdIf.argument = blockLength;
    cmdIf.expect_resp = EXPECT_SHORT_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];
    while (retryCount > 0)
    {
        /* Check current status */
        if ((mci_check_status(CARD_STATE_TRAN) != RT_EOK))
            return RT_ERROR;

        respStatus = mci_cmd_resp(&cmdIf);

        if (respStatus)
        {
            retval = RT_ERROR;
        }
        else if (respValue[0] & CARD_STATUS_ERR_MASK)
        {
            return RT_ERROR;
        }
        else
        {
            return mci_check_status(CARD_STATE_TRAN);
        }


        rt_thread_delay(1);

        retryCount--;
    }

    return retval;
}


/************************************************************************//**
 * @brief         Set bus-width (1 bit or 4 bit) to work with the card by command
 *                CMD6 (SET_ACMD_BUS_WIDTH).
 *
 * @param[in]    buswidth The value represented for bus-width
 *                - 0b00: 1-bit bus-width
 *                - 0b10: 4-bit bus-width
 *
 * @return         MCI_FUNC_OK if all success
 *
 * @note
 *                - If SD card is currently in used, it's possible to enable 4-bit
 *                bus-width instead of 1-bit to speed up.
 *                - This command can only be transferred during TRANS state.
 *                - Since, it's a ACMD, CMD55 (APP_CMD) needs to be sent out first
 ****************************************************************************/
static rt_err_t mci_send_buswidth(rt_uint32_t buswidth)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    mci_cmd_t cmdIf;
    rt_err_t retval = RT_ERROR;

    retryCount = 0x20;            /* reset retry counter */
    cmdIf.command = ACMD6_SET_BUS_WIDTH;
    cmdIf.argument = buswidth;
    cmdIf.expect_resp = EXPECT_SHORT_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];
    while (retryCount > 0)
    {
        /* The card must be in tran state in order to change the bus width */
        retval = mci_check_status(CARD_STATE_TRAN);
        if (retval != RT_EOK)
            return retval;

        if (mci_cmd_sendACMD() == RT_EOK)
        {
            respStatus = mci_cmd_resp(&cmdIf);

            if (respStatus)
            {
                retval = RT_ERROR;
            }
            else if (respValue[0] & CARD_STATUS_ERR_MASK)
            {
                return RT_EIO;
            }
            else
            {
                return mci_check_status(CARD_STATE_TRAN);
            }
        }

        rt_thread_delay(1);

        retryCount--;
    }

    return retval;
}

/************************************************************************//**
 * @brief        Stop the current transmission on the bus by sending command CMD12
 *                (STOP_TRANSMISSION). In this case, the card may be in a unknown
 *                state. So that it need a warm reset for normal operation.
 *
 * @param[in]    None
 *
 * @return       MCI_FUNC_OK if all success
 ****************************************************************************/
rt_err_t mci_cmd_stopTransmission(void)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    mci_cmd_t cmdIf;
    rt_err_t retval = RT_ERROR;

    /* do nothing when the card is in tran state */
    if (mci_check_status(CARD_STATE_TRAN) == RT_EOK)
    {
        return RT_EOK;
    }

    retryCount = 0x20;
    cmdIf.command = CMD12_STOP_TRANSMISSION;
    cmdIf.argument = 0x00000000;
    cmdIf.expect_resp = EXPECT_SHORT_RESP;
    cmdIf.allow_timeout = ALLOW_CMD_TIMER;
    cmdIf.cmd_resp = (rt_uint32_t *)&respValue[0];
    while (retryCount > 0)
    {
        /* Check current status */
        if ((mci_check_status(CARD_STATE_DATA) != RT_EOK) &&
                (mci_check_status(CARD_STATE_RCV) != RT_EOK))
            return RT_ERROR;

        respStatus = mci_cmd_resp(&cmdIf);

        if (respStatus)
        {
            retval = RT_ERROR;
        }
        else if (respValue[0] & CARD_STATUS_ERR_MASK)
        {
            return RT_EIO;
        }
        else
        {
            if ((mci_check_status(CARD_STATE_PRG) != RT_EOK) &&
                    (mci_check_status(CARD_STATE_TRAN) != RT_EOK))
                return RT_ERROR;
            return RT_EOK;
        }

        rt_thread_delay(1);

        retryCount--;
    }

    return retval;
}

/************************************************************************//**
 * @brief        Write blocks to card by sending command CMD24 (WRITE_BLOCK) or
 *                command CMD25 (WRITE_MULTIPLE_BLOCK) followed by the blocks of
 *                data to be written.
 *
 * @param[in]    blockNum The block number to start writting
 *
 * @param[in]    numOfBlock Determine how many blocks will be written (from the
 *                starting block)
 *
 * @return       MCI_FUNC_OK if all success
 *
 * @note        These commands should be sent in TRANS state.
 ****************************************************************************/
rt_err_t mci_cmd_write(rt_uint32_t blockNum, rt_uint32_t numOfBlock)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    mci_cmd_t cmd;
    rt_err_t retval = RT_ERROR;

    if (numOfBlock > 1)
    {
        cmd.command = CMD25_WRITE_MULTIPLE_BLOCK;
    }
    else
    {
        cmd.command = CMD24_WRITE_BLOCK;
    }

    retryCount = 0x20;
    if (_mci_device->card_type == MCI_SDHC_SDXC_CARD)
    {
        cmd.argument = blockNum;                      /* Block Address */
    }
    else
    {
        cmd.argument = blockNum * BLOCK_LENGTH;       /* Byte Address */
    }
    cmd.expect_resp = EXPECT_SHORT_RESP;
    cmd.allow_timeout = ALLOW_CMD_TIMER;
    cmd.cmd_resp = (rt_uint32_t *)&respValue[0];

    while (retryCount > 0)
    {
        /* Check current status */
        if ((mci_check_status(CARD_STATE_TRAN) != RT_EOK))
            return RT_ERROR;

        respStatus = mci_cmd_resp(&cmd);

        if (respStatus)
        {
            retval = RT_ERROR;
        }
        else if (respValue[0] & CARD_STATUS_ERR_MASK)
        {
            return RT_ERROR;
        }
        else
        {
            if ((mci_check_status(CARD_STATE_RCV) != RT_EOK) &&
                    (mci_check_status(CARD_STATE_TRAN) != RT_EOK))
                return RT_ERROR;
            return RT_EOK;
        }


        //rt_thread_delay(1);

        retryCount--;

    }

    return retval;                /* Fatal error */
}




/************************************************************************//**
 * @brief        Read blocks to card by sending CMD17 (READ_SINGLE_BLOCK) or
 *                CMD18 (READ_MULTIPLE_BLOCK) commands followed by the blocks of
 *                data to be read.
 *
 * @param[in]    blockNum The block number to start reading
 *
 * @param[in]    numOfBlock Determine how many blocks will be read (from the
 *                starting block)
 *
 * @return       MCI_FUNC_OK if all success
 *
 * @note        These commands should be sent in TRANS state.
 ****************************************************************************/
rt_err_t mci_cmd_read(rt_uint32_t blockNum, rt_uint32_t numOfBlock)
{
    rt_uint32_t retryCount;
    rt_uint32_t respStatus;
    rt_uint32_t respValue[4];
    mci_cmd_t cmd;
    rt_err_t retval = RT_ERROR;

    // To Do: Read Multi-Block
    if (numOfBlock > 1)
        cmd.command = CMD18_READ_MULTIPLE_BLOCK;
    else
        cmd.command = CMD17_READ_SINGLE_BLOCK;

    retryCount = 0x20;
    if (_mci_device->card_type == MCI_SDHC_SDXC_CARD)
    {
        cmd.argument = blockNum;                      /* Block Address */
    }
    else
    {
        cmd.argument = blockNum * BLOCK_LENGTH;       /* Byte Address */
    }
    cmd.expect_resp = EXPECT_SHORT_RESP;
    cmd.allow_timeout = ALLOW_CMD_TIMER;
    cmd.cmd_resp = (rt_uint32_t *)&respValue[0];
    while (retryCount > 0)
    {
        /* Check current status */
        if ((mci_check_status(CARD_STATE_TRAN) != RT_EOK))
            return RT_ERROR;

        respStatus = mci_cmd_resp(&cmd);

        if (respStatus)
        {
            retval = RT_ERROR;
        }
        else if (respValue[0] & CARD_STATUS_ERR_MASK)
        {
            return RT_ERROR;
        }
        else
        {
            if ((mci_check_status(CARD_STATE_DATA) != RT_EOK) &&
                    (mci_check_status(CARD_STATE_TRAN) != RT_EOK))
                return RT_ERROR;
            return RT_EOK;
        }

        //rt_thread_delay(1);
        retryCount--;

    }

    return retval;
}
/*-----------------------------------------------------------------------*/
/* Get bits from an array
/----------------------------------------------------------------------*/
static rt_uint32_t unstuff_bits(uint8_t *resp, rt_uint32_t start, rt_uint32_t size)
{
    rt_uint32_t byte_idx_stx;
    rt_uint8_t bit_idx_stx, bit_idx;
    rt_uint32_t ret, byte_idx;

    byte_idx_stx = start / 8;
    bit_idx_stx = start - byte_idx_stx * 8;

    if (size < (8 - bit_idx_stx))      // in 1 byte
    {
        return ((resp[byte_idx_stx] >> bit_idx_stx) & ((1 << size) - 1));
    }

    ret = 0;

    ret = (resp[byte_idx_stx] >> bit_idx_stx) & ((1 << (8 - bit_idx_stx)) - 1);
    bit_idx = 8 - bit_idx_stx;
    size -= bit_idx;

    byte_idx = 1;
    while (size > 8)
    {
        ret |= resp[byte_idx_stx + byte_idx] << (bit_idx);
        size -= 8;
        bit_idx += 8;
        byte_idx ++;
    }


    if (size > 0)
    {
        ret |= (resp[byte_idx_stx + byte_idx] & ((1 << size) - 1)) << bit_idx;
    }

    return ret;
}
/*-----------------------------------------------------------------------*/
/* Swap buffer
/----------------------------------------------------------------------*/
static void swap_buff(rt_uint8_t *buff, rt_uint32_t count)
{
    rt_uint8_t tmp;
    rt_uint32_t i;

    for (i = 0; i < count / 2; i++)
    {
        tmp = buff[i];
        buff[i] = buff[count - i - 1];
        buff[count - i - 1] = tmp;
    }
}
static rt_err_t mci_read_config(void)
{
    rt_uint32_t c_size, c_size_mult, read_bl_len;
    mci_cid_t card_cid;
    rt_uint8_t csd_buf[16];
    uint8_t csd_struct = 0;

    /* Read CID */
    if (mci_get_CID(&card_cid) != RT_EOK)
    {
        MCI_DEBUG("get card cid error!\n");
    }
    else
    {
        MCI_DEBUG("get card cid success!\n");
    }

    /* Set Address */
    if (mci_set_cardAddress() != RT_EOK)
    {
        MCI_DEBUG("set card address error!\n");
    }
    else
    {
        MCI_DEBUG("set card address success!\n");
    }
    // CardConfig.CardAddress = mci_get_cardAddress();

    /* Read CSD */
    if (mci_get_CSD((rt_uint32_t *)csd_buf) != RT_EOK)
    {
        MCI_DEBUG("get card CSD error!\n");
    }
    else
    {
        MCI_DEBUG("get card CSD success!\n");
    }
    swap_buff(&csd_buf[0], sizeof(uint32_t));
    swap_buff(&csd_buf[4], sizeof(uint32_t));
    swap_buff(&csd_buf[8], sizeof(uint32_t));
    swap_buff(&csd_buf[12], sizeof(uint32_t));
    swap_buff(csd_buf, 16);

    /* sector size */
    _mci_device->geometry.bytes_per_sector = 512;

    csd_struct =  csd_buf[15] >> 6;
    /* Block count */
    if (csd_struct == 1)    /* CSD V2.0 */
    {
        /* Read C_SIZE */
        c_size =  unstuff_bits(csd_buf, 48, 22);
        /* Calculate block count */
        _mci_device->geometry.sector_count = (c_size + 1) * 1024;
        MCI_DEBUG("card sector_count:%d!\n", _mci_device->geometry.sector_count);
    }
    else     /* CSD V1.0 (for Standard Capacity) */
    {
        /* C_SIZE */
        c_size = unstuff_bits(csd_buf, 62, 12);
        /* C_SIZE_MUTE */
        c_size_mult = unstuff_bits(csd_buf, 47, 3);
        /* READ_BL_LEN */
        read_bl_len = unstuff_bits(csd_buf, 80, 4);
        /* sector count = BLOCKNR*BLOCK_LEN/512, we manually set SECTOR_SIZE to 512*/
        _mci_device->geometry.sector_count = (((c_size + 1) * (0x01 << (c_size_mult + 2))) * (0x01 << read_bl_len)) / 512;
        MCI_DEBUG("card sector_count:%d!\n", _mci_device->geometry.sector_count);
    }

    /* Get erase block size in unit of sector */
    switch (_mci_device->card_type)
    {
    case MCI_MMC_CARD:
        _mci_device->geometry.block_size = unstuff_bits(csd_buf, 42, 5) + 1;
        _mci_device->geometry.block_size <<=  unstuff_bits(csd_buf, 22, 4);
        _mci_device->geometry.block_size /= 512;
        break;
    case MCI_SDHC_SDXC_CARD:
    case MCI_SDSC_V2_CARD:
        _mci_device->geometry.block_size = 1;
        break;
    case MCI_SDSC_V1_CARD:
        if (unstuff_bits(csd_buf, 46, 1))
        {
            _mci_device->geometry.block_size = 1;
        }
        else
        {
            _mci_device->geometry.block_size = unstuff_bits(csd_buf, 39, 7) + 1;
            _mci_device->geometry.block_size <<=  unstuff_bits(csd_buf, 22, 4);
            _mci_device->geometry.block_size /= 512;
        }
        break;
    default:
        break;
    }
    MCI_DEBUG("card block_size:%d!\n", _mci_device->geometry.block_size);
    MCI_DEBUG("[info] card capacity : %d Mbyte\r\n", (_mci_device->geometry.sector_count * _mci_device->geometry.bytes_per_sector) / (1024 * 1024));
    /* Select Card */
    if (mci_cmd_selectCard() != RT_EOK)
    {
        MCI_DEBUG("select card  error!\n");
    }
    else
    {
        MCI_DEBUG("select card  success!\n");
    }

    if ((_mci_device->card_type == MCI_SDSC_V1_CARD) ||
            (_mci_device->card_type == MCI_SDSC_V2_CARD) ||
            (_mci_device->card_type == MCI_SDHC_SDXC_CARD))
    {
        mci_set_clock(MCI_NORMAL_RATE);
        if (mci_set_buswidth(SD_4_BIT) != RT_EOK)
        {
            MCI_DEBUG("set card buswidth to 4bit error!\n");
        }
        else
        {
            MCI_DEBUG("set card buswidth to 4bit success!\n");
        }
    }
    else
    {
        if (mci_set_buswidth(SD_1_BIT) != RT_EOK)
        {
            MCI_DEBUG("set card buswidth to 1bit error!\n");
        }
        else
        {
            MCI_DEBUG("set card buswidth to 1bit success!\n");
        }
    }

    /* For SDHC or SDXC, block length is fixed to 512 bytes, for others,
     the block length is set to 512 manually. */
    if (_mci_device->card_type == MCI_MMC_CARD ||
            _mci_device->card_type == MCI_SDSC_V1_CARD ||
            _mci_device->card_type == MCI_SDSC_V2_CARD)
    {
        if (mci_set_blockLen(BLOCK_LENGTH) != RT_EOK)
        {
            MCI_DEBUG("set card block len error!\n");
        }
        else
        {
            MCI_DEBUG("set card block len success!\n");
        }
    }

    return RT_EOK;
}
static rt_err_t rt_mci_init(rt_device_t dev)
{
    rt_err_t result = RT_EOK;
    _mci_device->card_type = MCI_CARD_UNKNOWN;
    rt_mutex_take(&_mci_device->lock, RT_WAITING_FOREVER);
	  if(SDCARD_DET_VALUE!=RT_EOK)
		{
        MCI_DEBUG("can not found any sdcard!\n");
        goto _exit;
    }
    if (mci_card_reset() != RT_EOK)
    {
        MCI_DEBUG("card reset error!\n");
        goto _exit;
    }

    /* Clear Open Drain output control for SD */
    mci_set_outputMode(MCI_OUTPUT_MODE_PUSHPULL);

    rt_thread_delay(20);

    result = mci_cmd_sendIfCond();

    if (result == RT_EIO)
    {
        //Unknow card is unusable
        MCI_DEBUG("card error for send CMD8\n");
        goto _exit;
    }

    if (result == RT_EOK) /* Ver2.00 or later*/
    {
        //Check in case of High Capacity Supporting Host
        if ((result = mci_acmd_sendOpCond(1)) == RT_EOK)
        {
            _mci_device->card_type = MCI_SDSC_V2_CARD;//SDSC

            if (CCS)
            {
                _mci_device->card_type = MCI_SDHC_SDXC_CARD;//SDHC or SDXC
                MCI_DEBUG("Card is SDHC or SDXC\n");
            }
            else
            {
                MCI_DEBUG("Card is SD V2\n");
            }
        }
    }
    else   /* voltage mismatch (ver2.00)or ver1.X SD Card or not SD Card*/
    {

        //Check in case of Standard Capacity Supporting Host
        if ((result = mci_acmd_sendOpCond(0)) == RT_EOK)
        {
            _mci_device->card_type = MCI_SDSC_V1_CARD;//Support Standard Capacity only
            MCI_DEBUG("Card is SD V1.x\n");
        }
        else
        {
            /* Set Open Drain output control for MMC */
            mci_set_outputMode(MCI_OUTPUT_MODE_OPENDRAIN);

            rt_thread_delay(20);

            /* Try CMD1 first for MMC, if it's timeout, try CMD55 and CMD41 for SD,
            if both failed, initialization faIlure, bailout. */
            if (mci_cmd_sendOpCond() == RT_EOK)
            {
                _mci_device->card_type = MCI_MMC_CARD;
                MCI_DEBUG("Card is MMC\n");
            }
        }
    }
    mci_read_config();
_exit:
    rt_mutex_release(&_mci_device->lock);
    return result;
}
static rt_err_t rt_mci_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_mci_close(rt_device_t dev)
{
    return RT_EOK;
}


static rt_size_t rt_mci_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    // struct mci_device * msd = (struct mci_device *)dev;
    rt_uint32_t DataCtrl = 0;
     rt_uint32_t event_value;
    if (BLOCK_LENGTH * size > DATA_RW_MAX_LEN)
    {
        MCI_DEBUG("too many block to read:%d\n", size);
        return RT_ERROR;
    }
    rt_mutex_take(&_mci_device->lock, RT_WAITING_FOREVER);
    dataDestBlock = (uint8_t *)buffer;
    LPC_MCI->CLEAR = 0x7FF;

    LPC_MCI->DATACTRL = 0;

    //rt_thread_delay(1);

    /* Wait the SD Card enters to TRANS state. */
    if (mci_check_status(CARD_STATE_TRAN) != RT_EOK)
    {
        MCI_DEBUG("Wait the SD Card enters to TRANS state error!\n");
        goto _exit;
    }
    mci_rx_enable(RT_TRUE);

    LPC_MCI->DATATMR = DATA_TIMER_VALUE_R;

    LPC_MCI->DATALEN = BLOCK_LENGTH * size;

    _mci_device->data_error = RT_TRUE;



    // Start data engine on READ before command to avoid overflow of the FIFO.
    {
#if MCI_DMA_ENABLED
        MCI_SettingDma((uint8_t *) dataDestBlock, MCI_DMA_READ_CHANNEL, GPDMA_TRANSFERTYPE_P2M_SRC_CTRL);

        /* Write, block transfer, DMA, and data length */
        DataCtrl |= MCI_DATACTRL_ENABLE | MCI_DATACTRL_DIR_FROM_CARD
                    | MCI_DATACTRL_DMA_ENABLE | MCI_DTATCTRL_BLOCKSIZE(DATA_BLOCK_LEN);
#else
        //Retrieving the result after reading the card is done by the FIFO handling for interrupt

        /* Read, enable, block transfer, and data length */
        DataCtrl |= MCI_DATACTRL_ENABLE | MCI_DATACTRL_DIR_FROM_CARD | MCI_DTATCTRL_BLOCKSIZE(DATA_BLOCK_LEN);

#endif
    }

    LPC_MCI->DATACTRL = DataCtrl;

    //rt_thread_delay(1);

    if (mci_cmd_read(pos, size) != RT_EOK)
    {
        MCI_DEBUG("send read cmd error!pos:%d,size:%d\n", pos, size);
        goto _exit;
    }
		rt_event_recv(_mci_device->finish_event,
                          1,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          50,
                          &event_value);
    if ((size > 1) || (_mci_device->data_error == RT_TRUE))
    {
        mci_cmd_stopTransmission();
    }
    if (_mci_device->data_error == RT_TRUE)
    {
        MCI_DEBUG("read data error!pos:%d,size:%d\n", pos, size);
    }
    else
    {
        MCI_DEBUG("read data success!pos:%d,size:%d\n", pos, size);
    }

_exit:

    rt_mutex_release(&_mci_device->lock);

    return size;
}

static rt_size_t rt_mci_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct mci_device *mci = (struct mci_device *)dev;
    rt_uint32_t DataCtrl = 0;
     rt_uint32_t event_value;
	
    if (BLOCK_LENGTH * size > DATA_RW_MAX_LEN)
    {
        MCI_DEBUG("too many block to write:%d\n", size);
        return RT_ERROR;
    }
    rt_mutex_take(&mci->lock, RT_WAITING_FOREVER);
    dataSrcBlock = (uint8_t *)buffer;

    LPC_MCI->CLEAR = 0x7FF;

    LPC_MCI->DATACTRL = 0;

    //rt_thread_delay(1);

    /* Wait the SD Card enters to TRANS state. */
    if (mci_check_status(CARD_STATE_TRAN) != RT_EOK)
    {
        MCI_DEBUG("Wait the SD Card enters to TRANS state error!\n");
        goto _exit;
    }
    LPC_MCI->DATATMR = DATA_TIMER_VALUE_W;

    LPC_MCI->DATALEN = BLOCK_LENGTH * size;

    _mci_device->data_error = TRUE;
    mci_tx_enable(RT_TRUE);

    if (mci_cmd_write(pos, size) != RT_EOK)
    {
        MCI_DEBUG("send write cmd error!pos:%d,size:%d", pos, size);
        goto _exit;
    }

    //for(blockCnt = 0; blockCnt < numOfBlock; blockCnt++)
    {
#if MCI_DMA_ENABLED
        MCI_SettingDma((uint8_t *) dataSrcBlock, MCI_DMA_WRITE_CHANNEL, GPDMA_TRANSFERTYPE_M2P_DEST_CTRL);

        /* Write, block transfer, DMA, and data length */
        DataCtrl |= MCI_DATACTRL_ENABLE | MCI_DATACTRL_DIR_TO_CARD
                    | MCI_DATACTRL_DMA_ENABLE | MCI_DTATCTRL_BLOCKSIZE(DATA_BLOCK_LEN);
#else
        /* Write, block transfer, and data length */
        DataCtrl |= MCI_DATACTRL_ENABLE  | MCI_DATACTRL_DIR_TO_CARD  | MCI_DTATCTRL_BLOCKSIZE(DATA_BLOCK_LEN);
#endif
    }

    LPC_MCI->DATACTRL = DataCtrl;

    rt_event_recv(_mci_device->finish_event,
                          1,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          50,
                          &event_value);
    if ((size > 1) || (_mci_device->data_error == RT_TRUE))
    {
        mci_cmd_stopTransmission();
    }
    if (_mci_device->data_error == RT_TRUE)
    {
        MCI_DEBUG("write data error!pos:%d,size:%d\n", pos, size);
    }
    else
    {
        MCI_DEBUG("write data success!pos:%d,size:%d\n", pos, size);
    }

_exit:
    /* release and exit */
    rt_mutex_release(&_mci_device->lock);

    return size;
}

static rt_err_t rt_mci_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    struct mci_device *mci = (struct mci_device *)dev;

    RT_ASSERT(dev != RT_NULL);

    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry;

        geometry = (struct rt_device_blk_geometry *)args;
        if (geometry == RT_NULL) return -RT_ERROR;

        geometry->bytes_per_sector = mci->geometry.bytes_per_sector;
        geometry->block_size = mci->geometry.block_size;
        geometry->sector_count = mci->geometry.sector_count;
    }

    return RT_EOK;
}
rt_err_t mci_hw_init(const char *device_name)
{
    volatile rt_uint32_t i;

    /* on DMA channel 0, source is memory, destination is MCI FIFO. */
    /* On DMA channel 1, source is MCI FIFO, destination is memory. */
    GPDMA_Init();

    LPC_IOCON->P1_2  &= ~0x1F; /* SD_CLK @ P1.2 */
    LPC_IOCON->P1_3  &= ~0x1F; /* SD_CMD @ P1.3 */
    LPC_IOCON->P1_5  &= ~0x1F; /* SD_PWR @ P1.5 */
    LPC_IOCON->P1_6  &= ~0x1F; /* SD_DAT_0 @ P1.6 */
    LPC_IOCON->P1_7  &= ~0x1F; /* SD_DAT_1 @ P1.7 */
    LPC_IOCON->P1_11 &= ~0x1F; /* SD_DAT_2 @ P1.11 */
    LPC_IOCON->P1_12 &= 0x1F; /* SD_DAT_3 @ P1.12 */

    // Set all MCI pins to outputs
    LPC_GPIO1->DIR |= 0x18EC;
	
    // Force all pins low (except power control pin)
    LPC_GPIO1->CLR = 0x18cc;
	
	  // Set power control pin high
    LPC_GPIO1->SET = 0x0020;
		
		//config DET pin to input mode
		LPC_IOCON->P2_19 &= ~0x07;
		LPC_GPIO2->DIR &= ~(0x01<<19);

    rt_thread_delay(20);

    LPC_SC->PCONP |= (1 << 28);              /* Enable clock to the MCI block */

    if (LPC_MCI->CLOCK & (1 << 8))
    {
        LPC_MCI->CLOCK &= ~(1 << 8);
        for (i = 0; i < 0x10; i++);      /* delay 3MCLK + 2PCLK  */
    }

    if (LPC_MCI->POWER & 0x02)
    {
        LPC_MCI->POWER = 0x00;
        for (i = 0; i < 0x10; i++);      /* delay 3MCLK + 2PCLK  */
    }

    /* Disable all interrupts for now */
    LPC_MCI->MASK0 = 0;
    //SD_CLK
    LPC_IOCON->P1_2 = 0x02;
    //SD_CMD
    LPC_IOCON->P1_3 = 2;
    //SD_PWR
    LPC_IOCON->P1_5 = 2;
    //SD_DAT_0
    LPC_IOCON->P1_6 = 0xa2;
    //SD_DAT_1
    LPC_IOCON->P1_7 = 0xa2;
    //SD_DAT_2
    LPC_IOCON->P1_11 = 2;
    //SD_DAT_3
    LPC_IOCON->P1_12 = 2;

    // SD_PWR is active high (follows the output of the SD Card interface block).
    LPC_SC->SCS &= ~ 0x08;//Becase on EA board SD_PWR is active low
    //Setting for timeout problem
    LPC_MCI->DATATMR = 0x1FFFFFFF;
    /*set up clocking default mode, clear any registers as needed */
    LPC_MCI->COMMAND = 0;
    for (i = 0; i < 0x10; i++);      /* delay 3MCLK + 2PCLK  */
    LPC_MCI->DATACTRL = 0;
    for (i = 0; i < 0x10; i++);      /* delay 3MCLK + 2PCLK  */
    LPC_MCI->CLEAR = 0x7FF;        /* clear all pending interrupts */

    LPC_MCI->POWER = 0x02;        /* power up */
    for (i = 0; i < 0x10; i++);      /* delay 3MCLK + 2PCLK  */
    /* delays for the supply output is stable*/
    for (i = 0; i < 0x80000; i++);
    /* During identification phase, the clock should be less than
    400Khz. Once we pass this phase, the normal clock can be set up
    to 25Mhz on SD card and 20Mhz on MMC card. */
    mci_set_clock(MCI_SLOW_RATE);
    LPC_MCI->POWER |= 0x01;        /* bit 1 is set already, from power up to power on */
    for (i = 0; i < 0x10; i++);      /* delay 3MCLK + 2PCLK  */
    NVIC_EnableIRQ(MCI_IRQn);
		_mci_device=(struct mci_device*)rt_malloc(sizeof(struct mci_device));
    rt_memset(_mci_device, 0, sizeof(struct mci_device));
    /* initialize mutex lock */
    rt_mutex_init(&_mci_device->lock, device_name, RT_IPC_FLAG_FIFO);
		 /* create finish event */
		_mci_device->finish_event=rt_event_create(device_name, RT_IPC_FLAG_FIFO);
		
    _mci_device->card_type = MCI_CARD_UNKNOWN;
    /* register sdcard device */
    _mci_device->parent.type    = RT_Device_Class_Block;

    _mci_device->geometry.bytes_per_sector = 0;
    _mci_device->geometry.sector_count = 0;
    _mci_device->geometry.block_size = 0;

    _mci_device->parent.init    = rt_mci_init;
    _mci_device->parent.open    = rt_mci_open;
    _mci_device->parent.close   = rt_mci_close;
    _mci_device->parent.read    = rt_mci_read;
    _mci_device->parent.write   = rt_mci_write;
    _mci_device->parent.control = rt_mci_control;

    /* no private, no callback */
    _mci_device->parent.user_data = RT_NULL;
    _mci_device->parent.rx_indicate = RT_NULL;
    _mci_device->parent.tx_complete = RT_NULL;

    rt_device_register(&_mci_device->parent, device_name,
                       RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE);
    return RT_EOK;
}
#ifdef RT_USING_FINSH
#include "finsh.h"
void mci_test(void)
{
    rt_uint8_t *tx_buf, *rx_buf;
    rt_uint16_t i = 0;
    rt_device_t dev;
    dev = rt_device_find("sd0");
    tx_buf = (rt_uint8_t *)rt_malloc(2048);
    rx_buf = (rt_uint8_t *)rt_malloc(2048);
    for (i = 0; i < 2048; i++)
    {
        tx_buf[i] = i % 0xff;
    }
    dev->write(dev, 0, tx_buf, 4);
    rt_memset(rx_buf, 0, 2048);
    dev->read(dev, 0, rx_buf, 4);
    for (i = 0; i < 2048; i++)
    {
        rt_kprintf("%02x ", rx_buf[i]);
        if ((i % 32) == 0)
        {
            rt_kprintf("\n");
        }
    }

}
FINSH_FUNCTION_EXPORT(mci_test, mci read write test.)
#endif
