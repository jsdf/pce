/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8250.h                                     *
 * Created:     2003-08-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


/* UART 8250/16450/16550 */


#ifndef PCE_E8250_H
#define PCE_E8250_H 1


/* specific chip types */
#define E8250_CHIP_8250  0
#define E8250_CHIP_16450 1

/* register offsets */
#define E8250_RXD     0x00
#define E8250_TXD     0x00
#define E8250_DIV_LO  0x00
#define E8250_DIV_HI  0x01
#define E8250_IER     0x01
#define E8250_IIR     0x02
#define E8250_LCR     0x03
#define E8250_MCR     0x04
#define E8250_LSR     0x05
#define E8250_MSR     0x06
#define E8250_SCRATCH 0x07

/* Interrupt Enable Register */
#define E8250_IER_SINP 0x08
#define E8250_IER_ERBK 0x04
#define E8250_IER_TBE  0x02
#define E8250_IER_RRD  0x01

/* Interrupt Indication Register */
#define E8250_IIR_SINP (0x00 << 1)
#define E8250_IIR_TBE  (0x01 << 1)
#define E8250_IIR_RRD  (0x02 << 1)
#define E8250_IIR_ERBK (0x03 << 1)
#define E8250_IIR_PND  0x01

/* Line Control Register */
#define E8250_LCR_DLAB   0x80
#define E8250_LCR_BRK    0x40
#define E8250_LCR_PARITY 0x38
#define E8250_LCR_STOP   0x04
#define E8250_LCR_DATA   0x03

/* parity codes */
#define E8250_PARITY_N 0x00
#define E8250_PARITY_O 0x01
#define E8250_PARITY_E 0x03
#define E8250_PARITY_M 0x05
#define E8250_PARITY_S 0x07

/* Line Status Register */
#define E8250_LSR_TXE 0x40
#define E8250_LSR_TBE 0x20
#define E8250_LSR_RRD 0x01

/* Modem Control Register */
#define E8250_MCR_LOOP 0x10
#define E8250_MCR_OUT2 0x08
#define E8250_MCR_OUT1 0x04
#define E8250_MCR_RTS  0x02
#define E8250_MCR_DTR  0x01

/* Modem Status Register */
#define E8250_MSR_DCD  0x80
#define E8250_MSR_RI   0x40
#define E8250_MSR_DSR  0x20
#define E8250_MSR_CTS  0x10
#define E8250_MSR_DDCD 0x08
#define E8250_MSR_DRI  0x04
#define E8250_MSR_DDSR 0x02
#define E8250_MSR_DCTS 0x01


#define E8250_BUF_MAX 256


/*!***************************************************************************
 * @short The UART 8250 structure
 *****************************************************************************/
typedef struct {
	unsigned       chip;

	/* input buffer. holds bytes that have not yet been received. */
	unsigned       inp_i;
	unsigned       inp_j;
	unsigned       inp_n;
	unsigned char  inp[E8250_BUF_MAX];

	/* output buffer. holds bytes that have been sent. */
	unsigned       out_i;
	unsigned       out_j;
	unsigned       out_n;
	unsigned char  out[E8250_BUF_MAX];

	unsigned char  txd;
	unsigned char  rxd;
	unsigned char  ier;
	unsigned char  iir;
	unsigned char  lcr;
	unsigned char  lsr;
	unsigned char  mcr;
	unsigned char  msr;
	unsigned char  scratch;

	char           tbe_ack;

	/* (input clock) / (bit clock) */
	unsigned       bit_clk_div;

	char           clocking;

	unsigned       read_clk_cnt;
	unsigned       read_clk_div;
	unsigned       read_char_cnt;
	unsigned       read_char_max;

	unsigned       write_clk_cnt;
	unsigned       write_clk_div;
	unsigned       write_char_cnt;
	unsigned       write_char_max;

	/* enables or disables the scratch register */
	unsigned char  have_scratch;

	unsigned short divisor;

	unsigned char  irq_val;
	void           *irq_ext;
	void           (*irq) (void *ext, unsigned char val);

	/* output buffer is not empty */
	void           *send_ext;
	void           (*send) (void *ext, unsigned char val);

	/* input buffer is not full */
	void           *recv_ext;
	void           (*recv) (void *ext, unsigned char val);

	/* setup (DIV, MCR or LCR) has changed */
	void           *setup_ext;
	void           (*setup) (void *ext, unsigned char val);

	/* MSR is about to be read */
	void           *check_ext;
	void           (*check) (void *ext, unsigned char val);
} e8250_t;


/*!***************************************************************************
 * @short Initialize an UART
 * @param uart The UART structure
 *****************************************************************************/
void e8250_init (e8250_t *uart);

/*!***************************************************************************
 * @short  Create and initialize an UART
 * @return A new UART structure or NULL on error
 *****************************************************************************/
e8250_t *e8250_new (void);

/*!***************************************************************************
 * @short Free the resources used by an UART structure
 * @param uart The UART structure
 *****************************************************************************/
void e8250_free (e8250_t *uart);

/*!***************************************************************************
 * @short Delete an UART structure
 * @param uart The UART structure
 *****************************************************************************/
void e8250_del (e8250_t *uart);


/*!***************************************************************************
 * @short Set the emulated chip type to 8250
 * @param uart The UART structure
 *****************************************************************************/
void e8250_set_chip_8250 (e8250_t *uart);

/*!***************************************************************************
 * @short Set the emulated chip type to 16450
 * @param uart The UART structure
 *****************************************************************************/
void e8250_set_chip_16450 (e8250_t *uart);

/*!***************************************************************************
 * @short Set the emulated chip type
 * @param uart The UART structure
 * @param chip The chip type
 *****************************************************************************/
int e8250_set_chip (e8250_t *uart, unsigned chip);

/*!***************************************************************************
 * @short Set the emulated chip type
 * @param uart The UART structure
 * @param str  The chip type as a string
 *****************************************************************************/
int e8250_set_chip_str (e8250_t *uart, const char *str);


/*!***************************************************************************
 * @short Set the IRQ function
 * @param uart The UART structure
 * @param ext  The transparent parameter for fct
 * @param fct  The function to be called on IRQ
 *****************************************************************************/
void e8250_set_irq_fct (e8250_t *uart, void *ext, void *fct);

/*!***************************************************************************
 * @short Set the send function
 * @param uart The UART structure
 * @param ext  The transparent parameter for fct
 * @param fct  The function to be called when the output queue is not empty
 *****************************************************************************/
void e8250_set_send_fct (e8250_t *uart, void *ext, void *fct);

/*!***************************************************************************
 * @short Set the receive function
 * @param uart The UART structure
 * @param ext  The transparent parameter for fct
 * @param fct  The function to be called when the input queue is not full
 *****************************************************************************/
void e8250_set_recv_fct (e8250_t *uart, void *ext, void *fct);

/*!***************************************************************************
 * @short Set the setup function
 * @param uart The UART structure
 * @param ext  The transparent parameter for fct
 * @param fct  The function to be called when the UART setup changes
 *****************************************************************************/
void e8250_set_setup_fct (e8250_t *uart, void *ext, void *fct);

/*!***************************************************************************
 * @short Set the check function
 * @param uart The UART structure
 * @param ext  The transparent parameter for fct
 * @param fct  The function to be called when the MSR is about to be read
 *****************************************************************************/
void e8250_set_check_fct (e8250_t *uart, void *ext, void *fct);


/*!***************************************************************************
 * @short Set the input and output queue sizes
 * @param uart The UART structure
 * @param inp  The input queue size
 * @param out  The output queue size
 *****************************************************************************/
void e8250_set_buf_size (e8250_t *uart, unsigned inp, unsigned out);

/*!***************************************************************************
 * @short Set the transmission rate
 *****************************************************************************/
void e8250_set_multichar (e8250_t *uart, unsigned read_max, unsigned write_max);

/*!***************************************************************************
 * @short Set the bit clock divisor
 *****************************************************************************/
void e8250_set_bit_clk_div (e8250_t *uart, unsigned div);

/*!***************************************************************************
 * @short  Get the rate divisor
 * @param  uart The UART structure
 * @return The 16 bit divisor
 *****************************************************************************/
unsigned short e8250_get_divisor (e8250_t *uart);

/*!***************************************************************************
 * @short  Get the data rate
 * @param  uart The UART structure
 * @return The data rate in bits per second
 *****************************************************************************/
unsigned long e8250_get_bps (e8250_t *uart);

/*!***************************************************************************
 * @short  Get the number of data bits per word
 * @param  uart The UART structure
 * @return The number of data bits per word
 *****************************************************************************/
unsigned e8250_get_databits (e8250_t *uart);

/*!***************************************************************************
 * @short  Get the number of stop bits
 * @param  uart The UART structure
 * @return The number of stop bits
 *****************************************************************************/
unsigned e8250_get_stopbits (e8250_t *uart);

/*!***************************************************************************
 * @short  Get the parity
 * @param  uart The UART structure
 * @return The parity
 *****************************************************************************/
unsigned e8250_get_parity (e8250_t *uart);


/*!***************************************************************************
 * @short  Get the DTR output signal
 * @param  uart The UART structure
 * @return The state of the DTR signal
 *****************************************************************************/
int e8250_get_dtr (e8250_t *uart);

/*!***************************************************************************
 * @short  Get the RTS output signal
 * @param  uart The UART structure
 * @return The state of the RTS signal
 *****************************************************************************/
int e8250_get_rts (e8250_t *uart);


/*!***************************************************************************
 * @short  Set the DSR input signal
 * @param  uart The UART structure
 * @return The state of the DSR signal
 *****************************************************************************/
void e8250_set_dsr (e8250_t *uart, unsigned char val);

/*!***************************************************************************
 * @short  Set the CTS input signal
 * @param  uart The UART structure
 * @return The state of the CTS signal
 *****************************************************************************/
void e8250_set_cts (e8250_t *uart, unsigned char val);

/*!***************************************************************************
 * @short  Set the DCD input signal
 * @param  uart The UART structure
 * @return The state of the DCD signal
 *****************************************************************************/
void e8250_set_dcd (e8250_t *uart, unsigned char val);

/*!***************************************************************************
 * @short  Set the RI input signal
 * @param  uart The UART structure
 * @return The state of the RI signal
 *****************************************************************************/
void e8250_set_ri (e8250_t *uart, unsigned char val);


/*!***************************************************************************
 * @short  Add a byte to the input queue
 * @param  uart The UART structure
 * @param  val  The byte
 * @return Nonzero on error (queue full)
 *****************************************************************************/
int e8250_set_inp (e8250_t *uart, unsigned char val);

/*!***************************************************************************
 * @short  Clear the input queue
 * @param  uart The UART structure
 *****************************************************************************/
void e8250_get_inp_all (e8250_t *uart);

/*!***************************************************************************
 * @short  Check if the input queue is full
 * @param  uart The UART structure
 * @return Nonzero if the queue is full
 *****************************************************************************/
int e8250_inp_full (e8250_t *uart);

/*!***************************************************************************
 * @short  Check if the input queue is empty
 * @param  uart The UART structure
 * @return Nonzero if the queue is empty
 *****************************************************************************/
int e8250_inp_empty (e8250_t *uart);


/*!***************************************************************************
 * @short  Get a byte from the output queue
 * @param  uart   The UART structure
 * @retval val    The byte
 * @param  remove If true, remove the byte from the output queue
 * @return Nonzero on error (queue empty)
 *****************************************************************************/
int e8250_get_out (e8250_t *uart, unsigned char *val, int remove);

/*!***************************************************************************
 * @short  Clear the output queue
 * @param  uart The UART structure
 *****************************************************************************/
void e8250_get_out_all (e8250_t *uart);

/*!***************************************************************************
 * @short  Check if the output queue is empty
 * @param  uart The UART structure
 * @return Nonzero if the queue is empty
 *****************************************************************************/
int e8250_out_empty (e8250_t *uart);


/*!***************************************************************************
 * @short  Send a byte
 * @param  uart The UART structure
 * @param  val  The byte
 * @return Nonzero on error (output queue empty)
 *****************************************************************************/
int e8250_send (e8250_t *uart, unsigned char *val);

/*!***************************************************************************
 * @short  Receive a byte
 * @param  uart The UART structure
 * @param  val  The byte
 * @return Nonzero on error (input queue full)
 *****************************************************************************/
int e8250_receive (e8250_t *uart, unsigned char val);


unsigned char e8250_get_div_lo (e8250_t *uart);
unsigned char e8250_get_div_hi (e8250_t *uart);
unsigned char e8250_get_rxd (e8250_t *uart);
unsigned char e8250_get_ier (e8250_t *uart);
unsigned char e8250_get_iir (e8250_t *uart);
unsigned char e8250_get_lcr (e8250_t *uart);
unsigned char e8250_get_mcr (e8250_t *uart);
unsigned char e8250_get_lsr (e8250_t *uart);
unsigned char e8250_get_msr (e8250_t *uart);
unsigned char e8250_get_scratch (e8250_t *uart);

void e8250_set_scratch (e8250_t *uart, unsigned char val);


void e8250_set_uint8 (e8250_t *uart, unsigned long addr, unsigned char val);
void e8250_set_uint16 (e8250_t *uart, unsigned long addr, unsigned short val);
void e8250_set_uint32 (e8250_t *uart, unsigned long addr, unsigned long val);

unsigned char e8250_get_uint8 (e8250_t *uart, unsigned long addr);
unsigned short e8250_get_uint16 (e8250_t *uart, unsigned long addr);
unsigned long e8250_get_uint32 (e8250_t *uart, unsigned long addr);

void e8250_clock (e8250_t *uart, unsigned clk);


#endif
