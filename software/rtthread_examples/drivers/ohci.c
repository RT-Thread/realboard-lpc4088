/*
 * File      : ohci.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-01-10     Yi Qiu      first version
 */

#include <rtthread.h>
#include <rthw.h>
#include <drivers/usb_host.h>
#include "ohci.h"

#define OHCI_PORT_STATUS(port)    (ohci_ptr->hc->roothub_portstatus[port])

static struct ohci_data ohci, *ohci_ptr;
static rt_bool_t ignore_disconnect = RT_FALSE;
static struct uhcd __ohci_hcd;

/**
 * This function will unlink an ed structure.
 *
 * @param ed the pointer to ed structure.
 *
 * @return the error code, RT_EOK on successfully.
 */
static rt_err_t ohci_unlink_ed(struct ohci_ed *ed)
{
    struct ohci_td *item, *next;

    /* parameter check */
    RT_ASSERT(ed != RT_NULL);

    switch (ed->ep_attr)
    {
    case USB_EP_ATTR_CONTROL:
        ohci_ptr->hc->cmdstatus &= ~CS_CLF;
        break;
    case USB_EP_ATTR_BULK:
        ohci_ptr->hc->cmdstatus &= ~CS_BLF;
        break;
    case USB_EP_ATTR_INT:
        break;
    default:
        break;
    }

    next = item = (struct ohci_td *)(ed->headp & ~0x3);
    while (item != (struct ohci_td *)ed->tailp)
    {
        next = (struct ohci_td *)item->nexttd;
        rt_mp_free((void *)item);
        item = next;
    }

    rt_mp_free((void *)item);

    return RT_EOK;
}

/**
 * This function will link an ed structure.
 *
 * @param ed the pointer to ed structure.
 *
 * @return the error code, RT_EOK on successfully.
 */
static rt_err_t ohci_link_ed(struct ohci_ed *ed)
{
    int i;

    RT_ASSERT(ed != RT_NULL);

    switch (ed->ep_attr)
    {
    case USB_EP_ATTR_BULK:
        ed->nexted = ohci_ptr->hc->ed_bulkhead;
        ohci_ptr->hc->ed_bulkhead = (rt_uint32_t)ed;
        ohci_ptr->hc->control |= CTRL_BLE;
        break;
    case USB_EP_ATTR_INT:
        for (i = 0; i < 32; i++)
        {
            ed->nexted = 0;
            ohci_ptr->hcca->inttbl[i] = (rt_uint32_t)ed;
        }

        break;
    case USB_EP_ATTR_CONTROL:
        ed->nexted = ohci_ptr->hc->ed_controlhead;
        ohci_ptr->hc->ed_controlhead = (rt_uint32_t)ed;
        ohci_ptr->hc->control |= CTRL_CLE;
        break;
    default:
        break;
    }

    return RT_EOK;
}

/**
 * This function will initialize an ed structure.
 *
 * @param ed the pointer to ed structure.
 *
 * @return the error code, RT_EOK on successfully.
 */
static rt_err_t ohci_init_ed(struct ohci_ed *ed)
{
    struct ohci_td *td;

    RT_ASSERT(ed != RT_NULL);

    /* allocate the first td for ed */
    td = (struct ohci_td *)rt_mp_alloc(&(ohci_ptr->td_mp), RT_WAITING_FOREVER);
    if (td == RT_NULL)
    {
        rt_kprintf("alloc td failed\n");
        return -RT_ERROR;
    }
    rt_memset(td, 0, sizeof(struct ohci_td));
    td->ctl.value = 0;
    td->ctl.bits.condition_code = 0xe;
    td->ctl.bits.buffer_rounding = 1;

    td->ed = ed;
    ed->headp = ed->tailp = (rt_uint32_t)td;
    ed->err_code = 0;

    return RT_EOK;
}

/**
 * This function will reset an ed structure.
 *
 * @param ed the pointer to ed structure.
 *
 * @return the error code, RT_EOK on successfully.
 */
static rt_err_t ohci_reset_ed(struct ohci_ed *ed)
{
    struct ohci_td *td;

    RT_ASSERT(ed != RT_NULL);

    /* unlink ed */
    ohci_unlink_ed(ed);

    td = (struct ohci_td *)rt_mp_alloc(&(ohci_ptr->td_mp), RT_WAITING_FOREVER);
    if (td == RT_NULL)
    {
        rt_kprintf("alloc td failed\n");
        return -RT_ERROR;
    }
    rt_memset(td, 0, sizeof(struct ohci_td));
    td->ctl.value = 0;
    td->ctl.bits.condition_code = 0xe;
    td->ctl.bits.buffer_rounding = 1;
    td->ed = ed;
    ed->headp = ed->tailp = (rt_uint32_t)td;
    ed->err_code = 0;

    return RT_EOK;
}

/**
 * This function will enqueue a td to specified ed.
 *
 * @param ed the pointer to ed structure.
 * @param control the control value of ed.
 * @param buffer the data buffer to save requested data
 * @param nbytes the size of buffer
 *
 * @return the error code, RT_EOK on successfully.
 */
static rt_err_t ohci_enqueue_td(struct ohci_ed *ed, rt_uint32_t control,
                                void *buffer, rt_size_t nbytes)
{
    struct ohci_td *tail, *td;

    tail = (struct ohci_td *)ed->tailp;
    tail->data = tail->cbp = (rt_uint32_t)buffer;
    tail->be = nbytes ? ((rt_uint32_t)buffer + nbytes - 1) : 0;
    tail->ctl.value = control;

    /* prepare for next td */
    td = (struct ohci_td *)rt_mp_alloc(&(ohci_ptr->td_mp), RT_WAITING_FOREVER);
    if (td == RT_NULL)
    {
        rt_kprintf("alloc td failed\n");
        return -RT_ERROR;
    }
    rt_memset(td, 0, sizeof(struct ohci_td));
    td->ctl.value = 0;
    td->ctl.bits.condition_code = 0xe;
    td->ctl.bits.buffer_rounding = 1;
    td->ed = ed;
    tail->nexttd = (rt_uint32_t)td;
    ed->tailp = (rt_uint32_t)td;

    return RT_EOK;
}

/**
 * This function will allocate a pipe for specified endpoint, it will be used to do transfer.
 *
 * @param pipe the pointer of pipe handle to be allocated.
 * @param intf the usb interface instance.
 * @param ep the endpoint descriptor.
 * @param func_callback callback function to be registed
 *
 * @return the error code, RT_EOK on successfully.
 */
rt_err_t ohci_alloc_pipe(upipe_t *pipe, struct uintf *intf, uep_desc_t ep,
                         func_callback callback)
{
    upipe_t p;
    struct ohci_ed *ed;
    struct ohci_td *td;

    RT_ASSERT(ep != RT_NULL);

    RT_DEBUG_LOG(RT_DEBUG_USB, ("ohci_alloc_pipe:%d\n", ep->bEndpointAddress));

    rt_sem_take(&ohci_ptr->sem_lock, RT_WAITING_FOREVER);

    ed = (struct ohci_ed *)rt_mp_alloc(&(ohci_ptr->ed_mp), RT_WAITING_FOREVER);
    if (ed == RT_NULL)
    {
        rt_kprintf("alloc ed failed\n");
        *pipe = RT_NULL;

        rt_sem_release(&ohci_ptr->sem_lock);
        return -RT_ERROR;
    }

    td = (struct ohci_td *)rt_mp_alloc(&(ohci_ptr->td_mp), RT_WAITING_FOREVER);
    if (td == RT_NULL)
    {
        rt_kprintf("alloc td failed\n");
        *pipe = RT_NULL;

        rt_sem_release(&ohci_ptr->sem_lock);
        return -RT_ERROR;
    }
    rt_memset(td, 0, sizeof(struct ohci_td));
    td->ctl.value = 0;
    td->ctl.bits.condition_code = 0xe;
    td->ctl.bits.buffer_rounding = 1;

    p = (upipe_t)rt_malloc(sizeof(struct upipe));
    p->intf = intf;
    p->callback = callback;
    p->status = UPIPE_STATUS_OK;
    rt_memcpy(&p->ep, ep, ep->bLength);

    rt_memset(ed, 0, sizeof(struct ohci_ed));
    ed->interval = p->ep.bInterval;
    ed->ep_attr = p->ep.bmAttributes & USB_EP_ATTR_TYPE_MASK;
    ed->pipe = (rt_uint32_t)p;

    ed->ctl.bits.speed = intf->device->speed;
    ed->ctl.bits.func_addr = intf->device->address;
    ed->ctl.bits.ep_number = USB_EP_DESC_NUM(p->ep.bEndpointAddress);
    ed->ctl.bits.skip = 0;
    ed->ctl.bits.max_packet_size = p->ep.wMaxPacketSize;
    ed->ctl.bits.dir =
        (ed->ep_attr == USB_EP_ATTR_CONTROL) ? 0 :
        ((p->ep.bEndpointAddress & USB_DIR_IN) ? 2 : 1);
    ed->ctl.bits.format = (ed->ep_attr == USB_EP_ATTR_ISOC) ? 1 : 0;

    td->ed = ed;
    ed->headp = ed->tailp = (rt_uint32_t)td;

    p->user_data = (void *)ed;

    if (ohci_link_ed(ed) != RT_EOK)
    {
        rt_kprintf("ohci_link_ed failed\n");

        rt_mp_free((void *)td);
        ohci_unlink_ed(ed);
        rt_mp_free((void *)ed);

        *pipe = RT_NULL;

        rt_sem_release(&ohci_ptr->sem_lock);
        return -RT_ERROR;
    }

    *pipe = p;

    rt_sem_release(&ohci_ptr->sem_lock);
    return RT_EOK;
}

/**
 * This function will free a pipe, it will release all resouces of the pipe.
 *
 * @param pipe the pipe handler to be free.
 *
 * @return the error code, RT_EOK on successfully.
 */
rt_err_t ohci_free_pipe(upipe_t pipe)
{
    int i;
    struct ohci_ed *ed, *prev, *curr;

    RT_ASSERT(pipe != RT_NULL);

    RT_DEBUG_LOG(RT_DEBUG_USB, ("ohci_free_pipe:%d\n",
                                pipe->ep.bEndpointAddress));

    rt_sem_take(&ohci_ptr->sem_lock, RT_WAITING_FOREVER);

    ed = (struct ohci_ed *)pipe->user_data;
    switch (ed->ep_attr)
    {
    case USB_EP_ATTR_BULK:
        if (ed == (struct ohci_ed *)ohci_ptr->hc->ed_bulkhead)
        {
            ohci_ptr->hc->ed_bulkhead = ed->nexted;
            ohci_unlink_ed(ed);
            rt_mp_free((void *)ed);
            break;
        }

        prev = curr = (struct ohci_ed *)ohci_ptr->hc->ed_bulkhead;
        while (curr)
        {
            if (curr == ed)
            {
                prev->nexted = curr->nexted;
                ohci_unlink_ed(ed);
                rt_mp_free((void *)ed);
                break;
            }
            prev = curr;
            curr = (struct ohci_ed *)curr->nexted;
        }

        break;
    case USB_EP_ATTR_CONTROL:
        if (ed == (struct ohci_ed *)ohci_ptr->hc->ed_controlhead)
        {
            ohci_ptr->hc->ed_bulkhead = ed->nexted;
            ohci_unlink_ed(ed);
            rt_mp_free((void *)ed);
            break;
        }

        prev = curr = (struct ohci_ed *)ohci_ptr->hc->ed_controlhead;
        while (curr)
        {
            if (curr == ed)
            {
                prev->nexted = curr->nexted;
                break;
            }
            prev = curr;
            curr = (struct ohci_ed *)curr->nexted;
        }
        break;
    case USB_EP_ATTR_INT:
        for (i = 0; i < 32; i += 4)
        {
            if (ed == (struct ohci_ed *)ohci_ptr->hcca->inttbl[i])
            {
                ohci_ptr->hcca->inttbl[i] = ed->nexted;
                continue;
            }

            prev = curr = (struct ohci_ed *)ohci_ptr->hcca->inttbl[i];
            while (curr)
            {
                if (curr == ed)
                {
                    prev->nexted = curr->nexted;
                    break;
                }
                prev = curr;
                curr = (struct ohci_ed *)curr->nexted;
            }
        }
        ohci_unlink_ed(ed);
        rt_mp_free((void *)ed);
        break;
    default:
        break;
    }

    rt_free(pipe);

    rt_sem_release(&ohci_ptr->sem_lock);
    return RT_EOK;
}

/**
 * This function will handle work done interrupt generated from ohci host controller.
 *
 * @return the error code, RT_EOK on successfully.
 *
 */
static rt_err_t ohci_wd_handler(void)
{
    struct ohci_td *item, *next;
    struct ohci_ed *ed;
    upipe_t pipe;

    item = (struct ohci_td *)(ohci_ptr->hcca->donehead & ~0x1);
    ohci_ptr->hcca->donehead = 0;
    ed = item->ed;
    ed->xfer_len = 0;
    pipe = (upipe_t)ed->pipe;

    while (item != RT_NULL)
    {
        if (item->ctl.bits.condition_code && (ed->headp & 0x1))
        {
            ed->err_code = item->ctl.bits.condition_code;

            RT_DEBUG_LOG(RT_DEBUG_USB, ("response, 0x%x\n", ed->err_code));

            item->ctl.bits.condition_code = 0;
            ed->headp &= ~0x1;

            if (ed->ep_attr != USB_EP_ATTR_CONTROL)
            {
                upipe_t p = (upipe_t)ed->pipe;

                if (item->ctl.bits.condition_code == TD_CC_STALL)
                    p->status = UPIPE_STATUS_STALL;
                else p->status = UPIPE_STATUS_ERROR;
            }
        }
        else
        {
            /* get transfer length */
            ed->xfer_len += (item->cbp == 0) ?
                            (item->be - item->data + 1) : (item->cbp - item->data);
        }

        next = (struct ohci_td *)item->nexttd;
        rt_mp_free((void *)item);
        item = next;
    }

    if (ed->err_code != TD_CC_NOERROR && ed->err_code != TD_CC_STALL)
    {
        ohci_reset_ed(ed);
        return -RT_ERROR;
    }

    switch (ed->ep_attr)
    {
    case USB_EP_ATTR_CONTROL:
        rt_completion_done(&ohci_ptr->ctl_comp);
        break;
    case USB_EP_ATTR_BULK:
        rt_completion_done(&ohci_ptr->bulk_comp);
        break;
    case USB_EP_ATTR_INT:
        if (--ohci_ptr->int_req == 0)
        {
            ohci_ptr->hc->control &= ~(CTRL_PLE | CTRL_IE);
        }
        rt_completion_done(&ohci_ptr->int_comp);
        break;
    default:
        break;
    }

    if (pipe != RT_NULL && pipe->callback != RT_NULL &&
            ed->ep_attr == USB_EP_ATTR_INT)
    {
        struct uhost_msg msg;

        msg.content.cb.function = pipe->callback;
        msg.content.cb.context = (void *)pipe;
        msg.type = USB_MSG_CALLBACK;

        rt_usbh_event_signal(&msg);
    }

    return RT_EOK;
}

/**
 * This function will handle all irq interrupts generated from ohci host controller.
 *
 * @param irqno the irq number generated.
 *
 */
void ohci_irq_handler(void)
{
    rt_uint32_t status;
    struct uhost_msg msg;

    ohci_ptr->hc->intrdisable = INT_MIE;
    status = ohci_ptr->hc->intrstatus;

    /* writeback done status */
    if (status & INT_WD)
    {
        ohci_wd_handler();
        ohci_ptr->hc->intrstatus = INT_WD;
    }

    if (status & INT_UE)
    {
        rt_kprintf("Uncovered Error\n");
        ohci_ptr->hc->intrstatus = INT_UE;
    }

    if (status & INT_RD)
    {
        rt_kprintf("Resume Detect\n");
        ohci_ptr->hc->intrstatus = INT_RD;
    }

    if (status & INT_SO)
    {
        rt_kprintf("Scheduling Overrun\n");
        ohci_ptr->hc->intrstatus = INT_SO;
    }

    if (status & INT_FNO)
    {
        //rt_kprintf("Frame Number Overflow 0x%x\n", ohci_ptr->hc->fmnumber);
        ohci_ptr->hc->intrstatus = INT_FNO;
    }

    if (status & INT_OC)
    {
        rt_kprintf("Ownership Change\n");
        ohci_ptr->hc->intrstatus = INT_OC;
    }

    /* roothub status change */
    if (status & INT_RHSC)
    {
        if (ignore_disconnect == RT_FALSE)
        {
            msg.type = USB_MSG_CONNECT_CHANGE;
            msg.content.hub = &ohci_ptr->roothub;
            rt_usbh_event_signal(&msg);
        }
        ohci_ptr->hc->intrstatus = (INT_RHSC | INT_RD);
    }

    if (status & INT_SOF)
        ohci_ptr->hc->intrstatus = INT_SOF;

    ohci_ptr->hc->intrenable = INT_MIE;
}

/**
 * This function will reset ohci host controller.
 *
 * @return the error code, RT_EOK on successfully.
 */
static rt_err_t ohci_reset(void)
{
    int i = 0;

    /* do host controller reset */
    if (ohci_ptr->hc->control & CTRL_IR)
    {
        ohci_ptr->hc->cmdstatus = CS_OCR;
        while (ohci_ptr->hc->control & CTRL_IR)
        {
            rt_thread_delay(10);
            if (i++ < 100) continue;

            rt_kprintf("ohci host controller ocr timeout\n");
            return -RT_ERROR;
        }
    }
    ohci_ptr->hc->control = 0;
    ohci_ptr->hc->cmdstatus = CS_HCR;

    i = 0;

    /* wait controller ready */
    while (ohci_ptr->hc->cmdstatus != 0)
    {
        rt_thread_delay(10);
        if (i++ < 1000) continue;

        rt_kprintf("ohci host controller reset timeout\n");
        return -RT_ERROR;
    }

    ohci_ptr->hc->control = 0;
    rt_thread_delay(10);

    return RT_EOK;
}

/**
 * This function will do control transfer in lowlevel, it will send request to the host controller
 *
 * @param device the usb device instance.
 * @param setup the buffer to save sending request packet.
 * @param buffer the data buffer to save requested data
 * @param nbytes the size of buffer
 *
 * @return the error code, RT_EOK on successfully.
 */
int ohci_ctrl_xfer(struct uinstance *device, ureq_t setup, void *buffer,
                   int nbytes, int timeout)
{
    struct ohci_ed *ed;
    rt_uint32_t ctl_value;
    int xfer_len;
    rt_err_t ret;
    ed = ohci_ptr->ctl_ed;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(setup != RT_NULL);
    RT_ASSERT(ed != RT_NULL);

    RT_DEBUG_LOG(RT_DEBUG_USB, ("ohci_control_xfer\n"));

    if (device->status == DEV_STATUS_IDLE)
    {
        rt_kprintf("ohci_control_xfer:usb device is not ready\n");
        return -1;
    }

    rt_sem_take(&ohci_ptr->sem_lock, RT_WAITING_FOREVER);

    rt_completion_init(&ohci_ptr->ctl_comp);

    ed->ctl.bits.func_addr = device->address;
    ed->ctl.bits.max_packet_size = device->max_packet_size;
    ed->ctl.bits.speed = device->speed;
    ed->xfer_len = 0;

    rt_memcpy((void *)(ohci_ptr->tx_buffer), (void *)setup,
              sizeof(struct urequest));

    /* prepare for token stage */
    ctl_value = TD_CC | TD_DP_SETUP | TD_T_DATA0 | TD_DI;
    ohci_enqueue_td(ed, ctl_value,
                    (void *)(ohci_ptr->tx_buffer), SIZEOF_USB_REQUEST);

    /* prepare for data stage */
    ctl_value = (setup->request_type & USB_REQ_TYPE_DIR_IN) ?
                TD_CC | TD_DP_IN | TD_T_DATA1 | TD_DI :
                TD_CC | TD_DP_OUT | TD_T_DATA1 | TD_DI;
    if (nbytes > 0)
    {
        ohci_enqueue_td(ed, ctl_value,
                        (void *)(ohci_ptr->rx_buffer), nbytes);
    }

    /* prepare for status stage */
    ctl_value = (setup->request_type & USB_REQ_TYPE_DIR_IN) ?
                TD_CC | TD_DP_OUT | TD_T_DATA1 :
                TD_CC | TD_DP_IN | TD_T_DATA1;
    ohci_enqueue_td(ed, ctl_value, RT_NULL, 0);

    ohci_ptr->hc->cmdstatus |= CS_CLF;
    ohci_ptr->hc->control |= CTRL_CLE;

    ret = rt_completion_wait(&ohci_ptr->ctl_comp, timeout);
    if (ret != RT_EOK)
    {
        device->status = DEV_STATUS_ERROR;

        rt_kprintf("ohci_control_xfer timeout\n");
        rt_sem_release(&ohci_ptr->sem_lock);
        return -1;
    }

    xfer_len = ed->xfer_len - SIZEOF_USB_REQUEST - 1;
    if (buffer != RT_NULL && xfer_len > 0)
        rt_memcpy(buffer, (void *)(ohci_ptr->rx_buffer), xfer_len);

    rt_sem_release(&ohci_ptr->sem_lock);

    if (xfer_len < 0) return -1;
    else return xfer_len;
}

/**
 * This function will do bulk transfer in lowlevel, it will send request to the host controller
 *
 * @param pipe the bulk transfer pipe.
 * @param buffer the data buffer to save requested data
 * @param nbytes the size of buffer
 *
 * @return the error code, RT_EOK on successfully.
 */
int ohci_bulk_xfer(upipe_t pipe, void *buffer, int nbytes, int timeout)
{
    rt_uint32_t ctl_value, remain;
    struct ohci_ed *ed;
    rt_uint8_t *ptr;
    rt_err_t ret;

    RT_ASSERT(buffer != RT_NULL);
    RT_ASSERT(nbytes != 0);
    RT_ASSERT(pipe != RT_NULL);

    RT_DEBUG_LOG(RT_DEBUG_USB, ("ohci_bulk_xfer %d bytes, pipe 0x%x\n", nbytes, pipe->ep.bEndpointAddress));
    remain = nbytes;

    if (pipe->intf == RT_NULL)
    {
        rt_kprintf("ohci_bulk_xfer:usb device is not ready\n");
        return -1;
    }

    rt_sem_take(&ohci_ptr->sem_lock, RT_WAITING_FOREVER);

    rt_completion_init(&ohci_ptr->bulk_comp);

    ed = (struct ohci_ed *)pipe->user_data;
    ed->xfer_len = 0;

    if (pipe->ep.bEndpointAddress & USB_DIR_IN)
    {
        ptr = (rt_uint8_t *)ohci_ptr->rx_buffer;

        while (remain > 4096)
        {
            ctl_value = TD_CC | TD_DP_IN | TD_DI;
            ohci_enqueue_td(ed, ctl_value, (void *)ptr, 4096);
            remain -= 4096;
            ptr += 4096;
        }

        ctl_value = TD_CC | TD_DP_IN;

        if (remain != 0)
            ohci_enqueue_td(ed, ctl_value, (void *)ptr, remain);
    }
    else
    {
        ptr = (rt_uint8_t *)ohci_ptr->tx_buffer;
        rt_memcpy((void *)ptr, (void *)buffer, nbytes);

        while (remain > 4096)
        {
            ctl_value = TD_CC | TD_DP_OUT | TD_DI;
            ohci_enqueue_td(ed, ctl_value, (void *)ptr, 4096);
            remain -= 4096;
            ptr += 4096;
        }

        ctl_value = TD_CC | TD_DP_OUT;

        if (remain != 0)
            ohci_enqueue_td(ed, ctl_value, (void *)ptr, remain);
    }

    ohci_ptr->hc->cmdstatus |= CS_BLF;
    ohci_ptr->hc->control |= CTRL_BLE;

    /* wait bulk transfer finished */
    ret = rt_completion_wait(&ohci_ptr->bulk_comp, timeout);
    if (ret != RT_EOK)
    {
        pipe->status = UPIPE_STATUS_ERROR;
        rt_kprintf("ohci bulk xfer timeout 0x%x\n", pipe);

        rt_sem_release(&ohci_ptr->sem_lock);
        return -1;
    }

    if ((pipe->ep.bEndpointAddress & USB_DIR_IN) && ed->xfer_len != 0)
    {
        rt_memcpy(buffer, (void *)ohci_ptr->rx_buffer,
                  ed->xfer_len);
    }

    if (pipe->status != UPIPE_STATUS_OK)
    {
        rt_sem_release(&ohci_ptr->sem_lock);
        return -1;
    }

    rt_sem_release(&ohci_ptr->sem_lock);
    return ed->xfer_len;
}

/**
 * This function will do int transfer in lowlevel, it will send request to the host controller
 *
 * @param pipe the int transfer pipe.
 * @param buffer the data buffer to save requested data
 * @param nbytes the size of buffer
 *
 * @return the error code, RT_EOK on successfully.
 *
 */
int ohci_int_xfer(upipe_t pipe, void *buffer, int nbytes, int timeout)
{
    rt_uint32_t ctl_value;
    rt_uint16_t req_len;
    struct ohci_ed *ed;
    rt_err_t ret;

    RT_ASSERT(pipe != RT_NULL);

    rt_sem_take(&ohci_ptr->sem_lock, RT_WAITING_FOREVER);

    rt_completion_init(&ohci_ptr->int_comp);

    ed = (struct ohci_ed *)pipe->user_data;
    ctl_value = TD_CC | TD_R | TD_DP_IN;
    req_len = nbytes;
    ohci_enqueue_td(ed, ctl_value,
                    (void *)ohci_ptr->rx_buffer, req_len);

    if (ohci_ptr->int_req++ == 0)
    {
        ohci_ptr->hc->control |= (CTRL_PLE | CTRL_IE);
    }

    ret = rt_completion_wait(&ohci_ptr->int_comp, timeout);
    if (ret != RT_EOK)
    {
        rt_kprintf("no reached data\n");
        rt_sem_release(&ohci_ptr->sem_lock);
        return -1;
    }

    if ((pipe->ep.bEndpointAddress & USB_DIR_IN) && ed->xfer_len != 0)
    {
        rt_memcpy(buffer, (void *)ohci_ptr->rx_buffer,
                  ed->xfer_len);
    }

    rt_sem_release(&ohci_ptr->sem_lock);

    return ed->xfer_len;
}

/**
 * This function will do isochronous transfer in lowlevel, it will send request to the host controller
 *
 * @param pipe the isochronous transfer pipe.
 * @param buffer the data buffer to save requested data
 * @param nbytes the size of buffer
 *
 * @return the error code, RT_EOK on successfully.
 *
 * @note unimplement yet
 */
int ohci_iso_xfer(upipe_t pipe, void *buffer, int nbytes, int timeout)
{
    /* no implement */
    RT_ASSERT(0);

    return 0;
}

rt_err_t ohci_hub_ctrl(rt_uint16_t port, rt_uint8_t cmd, void *args)
{
    RT_ASSERT(port <= 2);

    switch (cmd)
    {
    case RH_GET_PORT_STATUS:
        *(rt_uint32_t *)args = OHCI_PORT_STATUS(port - 1);
        break;
    case RH_SET_PORT_STATUS:
        break;
    case RH_CLEAR_PORT_FEATURE:
        switch ((rt_uint32_t)args & 0xFF)
        {
        case PORT_FEAT_C_RESET:
            OHCI_PORT_STATUS(port - 1) = PORT_PRSC;
            ignore_disconnect = RT_FALSE;
            break;
        case PORT_FEAT_C_CONNECTION:
            OHCI_PORT_STATUS(port - 1) = PORT_CCSC;
            break;
        case PORT_FEAT_C_ENABLE:
            OHCI_PORT_STATUS(port - 1) = PORT_PESC;
            break;
        default:
            break;
        }
        break;
    case RH_SET_PORT_FEATURE:
        switch ((rt_uint32_t)args & 0xFF)
        {
        case PORT_FEAT_POWER:
            OHCI_PORT_STATUS(port - 1) = PORT_PPS;
            break;
        case PORT_FEAT_RESET:
            OHCI_PORT_STATUS(port - 1) = PORT_PRS;
            ignore_disconnect = RT_TRUE;
            break;
        case PORT_FEAT_ENABLE:
            OHCI_PORT_STATUS(port - 1) = PORT_PES;
            break;
        }
        break;
    default:
        break;
    }

    return RT_EOK;
}

void ohci_config(rt_uint32_t regbase, rt_uint32_t rambase, rt_uint32_t port_num)
{
    ohci_ptr = (struct ohci_data *)&ohci;
    rt_memset(ohci_ptr, 0, sizeof(struct ohci_data));

    rt_sem_init(&ohci_ptr->sem_lock, "o_lock", 1, RT_IPC_FLAG_FIFO);

    ohci_ptr->hcca = (volatile struct hcca *)(rambase + 0x000);
    rt_memset((void *)ohci_ptr->hcca, 0, sizeof(struct hcca));

    /* init ohci ed memory pool */
    rt_mp_init(&(ohci_ptr->ed_mp), "ed", (void *)(rambase + 0x100 - 4),
               0x100, sizeof(struct ohci_ed));

    /* init ohci td memory pool */
    rt_mp_init(&(ohci_ptr->td_mp), "td", (void *)(rambase + 0x200 - 4),
               0x100, sizeof(struct ohci_td));

    /* set ohci base register */
    ohci_ptr->hc = (struct ohci_regs *)regbase;

    ohci_ptr->tx_buffer = (rt_uint8_t *)(rambase + 0x1000);
    ohci_ptr->rx_buffer = (rt_uint8_t *)(rambase + 0x2000);
    ohci_ptr->int_req = 0;

    /* roothub initilizition */
    ohci_ptr->roothub.num_ports = port_num;
}

/**
 * This function will initialize ohci host controller device.
 *
 * @param dev the host controller device to be initalize.
 *
 * @return the error code, RT_EOK on successfully.
 */
rt_err_t ohci_init(rt_device_t device)
{
    if (ohci_reset() != RT_EOK)
    {
        return -RT_ERROR;
    }

    /* host controller configuration */
    ohci_ptr->hc->periodicstart = PERIODIC_START;
    ohci_ptr->hc->fminterval = FMINTERVAL_DEFAULT;
    ohci_ptr->hc->lsthresh = LSTHRESH;
    ohci_ptr->hc->control = CTRL_HCFS_OPER | CTRL_CBSR;
    ohci_ptr->hc->roothub_status = RHS_LPSC;
    ohci_ptr->hc->intrdisable = INT_ALL;
    ohci_ptr->hc->roothub_a |= RHD_NPS | RHD_OCPM | (2 << 24);
    ohci_ptr->hc->hcca = (rt_uint32_t)ohci_ptr->hcca;
    ohci_ptr->hc->intrstatus |= ohci_ptr->hc->intrstatus;
    ohci_ptr->hc->intrenable = INT_MIE | INT_WD | INT_RHSC
                               | INT_SO | INT_RD | INT_UE | INT_FNO | INT_OC;

    /* allocated ed for control transfer  */
    ohci_ptr->ctl_ed = (struct ohci_ed *)rt_mp_alloc(&(ohci_ptr->ed_mp), RT_WAITING_FOREVER);
    if (ohci_ptr->ctl_ed != RT_NULL)
    {
        rt_memset(ohci_ptr->ctl_ed, 0, sizeof(struct ohci_ed));

        /* the first ed is reserved for control transfer */
        ohci_ptr->ctl_ed->ep_attr = USB_EP_ATTR_CONTROL;
        ohci_ptr->ctl_ed->pipe = 0;
        ohci_ptr->ctl_ed->xfer_len = 0;
        ohci_ptr->ctl_ed->err_code = 0;
        ohci_ptr->ctl_ed->xfer_len = 0;

        if (ohci_link_ed(ohci_ptr->ctl_ed) != RT_EOK)
        {
            rt_kprintf("ohci_link_ed error\n");
            return -RT_ERROR;
        }

        ohci_init_ed(ohci_ptr->ctl_ed);
    }
    else
    {
        rt_kprintf("ed allocate failed\n");
    }

    return RT_EOK;
}

static struct uhcd_ops __ohci_usbh_ops =
{
    ohci_ctrl_xfer,
    ohci_bulk_xfer,
    ohci_int_xfer,
    ohci_iso_xfer,
    ohci_alloc_pipe,
    ohci_free_pipe,
    ohci_hub_ctrl,
};

void ohci_usbh_register(const char *name)
{
    __ohci_hcd.parent.type = RT_Device_Class_USBHost;
    __ohci_hcd.parent.init = ohci_init;
    __ohci_hcd.ops = &__ohci_usbh_ops;

    ohci_ptr->roothub.is_roothub = RT_TRUE;
    ohci_ptr->roothub.self = RT_NULL;
    ohci_ptr->roothub.hcd = &__ohci_hcd;

    rt_device_register(&__ohci_hcd.parent, name, 0);
}

