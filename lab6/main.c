#include <msp430.h>
#include "CTS_Layer.h"
const float COEF = 1.5 / 4096;

volatile unsigned int int_count = 0; // Global potent. mode
volatile unsigned int result; // RESULT OF ADC CONVERSION

#define COLUMN_ADR_MSB              0x10  //Set SRAM col. addr. before write, last 4 bits = ca4-ca7
#define COLUMN_ADR_LSB              0x00  //Set SRAM col. addr. before write, last 4 bits = ca0-ca3
#define POW_CTL                     0x2F  //Set Power control - booster, regulator, and follower on
#define SCROL_CTL                   0x40  //Scroll image up by SL rows (SL = last 5 bits), range:0-63
#define PAGE_ADR                    0xB0  //Set SRAM page addr (pa = last 4 bits), range:0-8
#define SET_CONTRAST_RESISTOR       0x27  //Set internal resistor ratio Rb/Ra to adjust contrast
#define MSB_ELECT_VOLUME            0x81  //Set Electronic Volume "PM" to adjust contrast
#define LSB_ELECT_VOLUME            0x0F  //Set Electronic Volume "PM" to adjust contrast (PM = last 5 bits)
#define ALL_PIXEL_ON                0xA4  //Disable all pixel on (last bit 1 to turn on all pixels - does not affect memory)
#define LCD_INVERSE                 0xA6  //Inverse display off (last bit 1 to invert display - does not affect memory)
#define LCD_EN                      0xAF  //Enable display (exit sleep mode & restore power)
#define SET_MIRROR_SEG 				0xA1  //Mirror SEG (column) mapping (set bit0 to mirror display)
#define SET_MIRROR_COM 				0xC8  //Mirror COM (row) mapping (set bit3 to mirror display)
#define BIAS_RATIO_VCC              0xA2  //Set voltage bias ratio (BR = bit0)
#define ADV_CTL_MSB                 0xFA  //Set temp. compensation curve to -0.11%/C
#define ADV_CTL_LSB                 0x90

uint8_t plus[] = {0x00, 0x00, 0x1C, 0x1C, 0x1C, 0x7F, 0x7F, 0x7F, 0x1C, 0x1C, 0x1C, 0x00, 0x00};
uint8_t minus[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t num0[] = {0x7F, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x7F};
uint8_t num1[] = {0x7F, 0x7F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x4C, 0x6C, 0x3C, 0x1C, 0x0C};
uint8_t num2[] = {0x7F, 0x7F, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x03, 0x03, 0x03, 0x03, 0x7F, 0x7F};
uint8_t num3[] = {0x7F, 0x7F, 0x03, 0x03, 0x03, 0x03, 0x7F, 0x7F, 0x03, 0x03, 0x03, 0x7F, 0x7F};
uint8_t num4[] = {0x0C, 0x0C, 0x7F, 0x7F, 0x6C, 0x6C, 0x60, 0x70, 0x38, 0x1C, 0x0E, 0x07, 0x03};
uint8_t num5[] = {0x7F, 0x7F, 0x03, 0x03, 0x03, 0x7F, 0x7F, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7F};
uint8_t num6[] = {0x7F, 0x7F, 0x63, 0x63, 0x63, 0x7F, 0x7F, 0x60, 0x60, 0x60, 0x60, 0x7F, 0x7F};
uint8_t num7[] = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x7F, 0x7F};
uint8_t num8[] = {0x7F, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x7F, 0x63, 0x63, 0x63, 0x7F, 0x7F};
uint8_t num9[] = {0x60, 0x30, 0x18, 0x0C, 0x06, 0x7F, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7F, 0x7F};

uint8_t currentPage = 0, currentColumn = 0;
int COLUMN_START_ADDRESS = 0; // 0 - default(0), 1 - mirror horizontal(31)
int number = 0;

uint8_t Dogs102x6_initMacro[] = {
    SCROL_CTL,
    SET_MIRROR_SEG,
    SET_MIRROR_COM,
    ALL_PIXEL_ON,
    LCD_INVERSE,
    BIAS_RATIO_VCC,
    POW_CTL,
    SET_CONTRAST_RESISTOR,
    MSB_ELECT_VOLUME,
    LSB_ELECT_VOLUME,
    ADV_CTL_MSB,
    ADV_CTL_LSB,
    LCD_EN,
    PAGE_ADR,
    COLUMN_ADR_MSB,
    COLUMN_ADR_LSB
};

void Dogs102x6_writeCommand(uint8_t *sCmd, uint8_t i)
{
	// CS Low
    P7OUT &= ~BIT4;

    // CD Low
    P5OUT &= ~BIT6;
    while (i)
    {
        // USCI_B1 TX buffer ready?
        while (!(UCB1IFG & UCTXIFG)) ;

        // Transmit data
        UCB1TXBUF = *sCmd;

        // Increment the pointer on the array
        sCmd++;

        // Decrement the Byte counter
        i--;
    }

    // Wait for all TX/RX to finish
    while (UCB1STAT & UCBUSY) ;

    // Dummy read to empty RX buffer and clear any overrun conditions
    UCB1RXBUF;

    // CS High
    P7OUT |= BIT4;

}

void set_ADR(uint8_t page, uint8_t column)
{
    uint8_t cmd[1];

    if (page > 7)
    {
        page = 7;
    }

    if (column > 101)
    {
        column = 101;
    }

    cmd[0] = PAGE_ADR + (7 - page);
    uint8_t H = 0x00;
    uint8_t L = 0x00;
    uint8_t ColumnAddress[] = { COLUMN_ADR_MSB, COLUMN_ADR_LSB };

    currentPage = page;
    currentColumn = column;

    L = (column & 0x0F);
    H = (column & 0xF0);
    H = (H >> 4);

    ColumnAddress[0] = COLUMN_ADR_LSB + L;
    ColumnAddress[1] = COLUMN_ADR_MSB + H;


    Dogs102x6_writeCommand(cmd, 1);
    Dogs102x6_writeCommand(ColumnAddress, 2);
}

void Dogs102x6_writeData(uint8_t *sData, uint8_t i)
{

    // CS Low
    P7OUT &= ~BIT4;
    //CD High
    P5OUT |= BIT6;

    while (i)
    {
        currentColumn++;

        if (currentColumn > 101)
        {
            currentColumn = 101;
        }

        // USCI_B1 TX buffer ready?
        while (!(UCB1IFG & UCTXIFG)) ;

        // Transmit data and increment pointer
        UCB1TXBUF = *sData++;

        // Decrement the Byte counter
        i--;
    }

    // Wait for all TX/RX to finish
    while (UCB1STAT & UCBUSY) ;

    // Dummy read to empty RX buffer and clear any overrun conditions
    UCB1RXBUF;

    // CS High
    P7OUT |= BIT4;

    // Restore original GIE state
}

void displayNum(void)
{
    volatile int lenght = 1;
    volatile int hren = 0;
    volatile int j = 0;
    volatile int i = 10;
    while(1)
    {
        if(number / i != 0)
        {
            i *= 10;
            lenght++;
        }
        else break;
    }
    set_ADR(0, COLUMN_START_ADDRESS);
    if(number <= 0)
        Dogs102x6_writeData(minus, 13);
    else Dogs102x6_writeData(plus, 13);
    int num = number;
    for(j = 0; j < lenght; j++)
    {
        i = i/10;
        hren = num / i;
        set_ADR(j + 1, COLUMN_START_ADDRESS);
        switch (abs(hren))
        {
        case 0:
            Dogs102x6_writeData(num0, 13);
            break;
        case 1:
            Dogs102x6_writeData(num1, 13);
            break;
        case 2:
            Dogs102x6_writeData(num2, 13);
            break;
        case 3:
            Dogs102x6_writeData(num3, 13);
            break;
        case 4:
            Dogs102x6_writeData(num4, 13);
            break;
        case 5:
            Dogs102x6_writeData(num5, 13);
            break;
        case 6:
            Dogs102x6_writeData(num6, 13);
            break;
        case 7:
            Dogs102x6_writeData(num7, 13);
            break;
        case 8:
            Dogs102x6_writeData(num8, 13);
            break;
        case 9:
            Dogs102x6_writeData(num9, 13);
            break;
        default:
            break;
        }
        num %= i;

    }
}

void clear(void)
{
    uint8_t LcdData[] = {0x00};
    uint8_t p, c;

    for (p = 0; p < 8; p++)
    {
        set_ADR(p, 0);
        for (c = 0; c < 132; c++)
        {
            Dogs102x6_writeData(LcdData, 1);
        }
    }
}

void initLCD()
{
    P5DIR |= BIT7;
    P5OUT &= BIT7;
    P5OUT |= BIT7;
    P7DIR |= BIT4;
    P7OUT &= ~BIT4;
    P5DIR |= BIT6;
    P5OUT &= ~BIT6;

    P4SEL |= BIT1 | BIT3;
    P4DIR |= BIT1 | BIT3;

    P7DIR |= BIT6;
    P7OUT |= BIT6;
    P7SEL &= ~BIT6;
}

void initSPI()
{
    UCB1CTL1 |= UCSWRST;
    UCB1CTL0 = (UCCKPH | UCMSB | UCMST | UCSYNC | UCMODE_0);
    UCB1CTL1 = UCSSEL_2 | UCSWRST;
    UCB1BR0 = 0x02;
    UCB1BR1 = 0;

    UCB1CTL1 &= ~UCSWRST;
    UCB1IFG &= ~UCRXIFG;
    Dogs102x6_writeCommand(Dogs102x6_initMacro, 13);
    P7OUT |= BIT4;
}

int pow(int x, int y) {
    int result = 1;
    while(y) {
        result *= x;
        y--;
    }

    return result;
}
// ENDOF LED part

void setupLed() {
    P1DIR |= BIT4;
    P1SEL &= ~BIT4;
    P1OUT &= ~BIT4;
}

void initADC() {

      ADC12CTL0 = ADC12ON + ADC12MSC + ADC12SHT0_15;// Turn on ADC12, set sampling time
      ADC12CTL1 =  ADC12CONSEQ_0 | ADC12SSEL_3 | ADC12SHS_1;       // Одноканальный режим + SMCLK + измерение по таймеру А0 // ADC12SHP |
      ADC12MCTL0 = ADC12INCH_5;

      ADC12IE |= 0x01;                           // Enable ADC12IFG.1, end channel
      ADC12CTL0 |= ADC12ENC;                    // Enable conversions
}

void initPoten() {

    P6DIR &= ~(BIT5);

    P6SEL |= (BIT5);

    P8DIR |= BIT0;
    P8SEL &= ~BIT0;
    P8OUT |= BIT0;
}

void initClock() {
//    TA0CCTL0 = CCIE;
//    // SMCLK, divide by *, updown mode, clear TAR
//    TA0CTL = TASSEL_2 | ID_0 | MC_1 | TACLR;
//    TA0CCTL1 = OUTMOD_4;// Set output mode 'toggle'
//    TA0CCR0 = 50000;
		TA0CTL = TASSEL__SMCLK | MC__UP | ID__1 | TACLR;     // SMCLK, UP-mode
		long int second = 32768;
		long int period = second / 2;
		TA0CCR0 = second;
		TA0CCR1 = period;
		TA0CCTL1 = OUTMOD_3;
}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR (void)
{
    if(__even_in_range(ADC12IV,34) == 6) {
        number = ADC12MEM0 * COEF * 1000;
        clear();
        displayNum();
    }

    P1OUT ^= BIT3;
}
// PAD PART

#define NUM_KEYS    5
#define LED4        BIT1
#define LED5        BIT2
#define LED6        BIT3
#define LED7        BIT4
#define LED8        BIT5

//uint16_t dCnt[NUM_KEYS];
struct Element* keypressed;

const struct Element* address_list[NUM_KEYS] =
{
    &element0,
    &element1,
    &element2,
    &element3,
    &element4
};
/*
 * Invert order of LEDs so that when pad1 is touched the LED with pad5 is
 * illuminated.
 */
const uint8_t ledMask[NUM_KEYS] =
{
    LED4,
    LED5,
    LED6,
    LED7,
    LED8
};
void sleep(uint16_t);
void SetVcoreUp (uint16_t level);


#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    TA0CTL = MC_0 | TACLR;
    if(int_count == 0) {
        ADC12CTL0 &= ~ADC12ENC;
        int_count = 0;
    }
//    else {
//        int_count++;
//    }
}


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    P1DIR &= ~BIT7;
    P1OUT |= BIT7;
    P1REN |= BIT7;

    // For LCD + ADC
    initLCD();
    initSPI();
    clear();
    initPoten();
    initADC();

    P1DIR |= BIT1 | BIT2 | BIT3 | BIT4 | BIT5;
    P1OUT &= ~(BIT1 | BIT2 | BIT3 | BIT4 | BIT5);

    /*
     *  Set DCO to 25Mhz and SMCLK to DCO. Taken from MSP430F55xx_UCS_10.c code
     *  example.
     */
    // Increase Vcore setting to level3 to support fsystem=25MHz
    // NOTE: Change core voltage one level at a time..
    SetVcoreUp (0x01);
    SetVcoreUp (0x02);
    SetVcoreUp (0x03);

    UCSCTL3 = SELREF_2;                       // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_2;                        // Set ACLK = REFO

    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_7;                      // Select DCO range 50MHz operation
    UCSCTL2 = FLLD_1 + 762;                   // Set DCO Multiplier for 25MHz
                                            // (N + 1) * FLLRef = Fdco
                                            // (762 + 1) * 32768 = 25MHz
                                            // Set FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);                  // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 25 MHz / 32,768 Hz ~ 780k MCLK cycles for DCO to settle
    __delay_cycles(782000);
    // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
    do
    {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                            // Clear XT2,XT1,DCO fault flags
    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag


    /*  establish baseline */
    TI_CAPT_Init_Baseline(&slider);
    TI_CAPT_Update_Baseline(&slider,5);

    __bis_SR_register(GIE);

    while (1)
    {
        P1OUT &= ~(ledMask[3]);
        keypressed = (struct Element *)TI_CAPT_Buttons(&slider);

        __no_operation();
        if (keypressed == address_list[0]) // PAD1
        {
            P1OUT |= ledMask[3];
            ADC12CTL0 |= ADC12ENC;
            initClock();
        }
        /* 32ms delay. This delay can be replaced with other application tasks. */
        __delay_cycles(900000);
    }
}


/*
 *  ======== SetVcorUp(uint16_t) ========
 *  Taken from MSP430F55xx_UCS_10.c code example.
 */
void SetVcoreUp (uint16_t level)
{
  // Open PMM registers for write
  PMMCTL0_H = PMMPW_H;
  // Set SVS/SVM high side new level
  SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
  // Set SVM low side to new level
  SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
  // Wait till SVM is settled
  while ((PMMIFG & SVSMLDLYIFG) == 0);
  // Clear already set flags
  PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
  // Set VCore to new level
  PMMCTL0_L = PMMCOREV0 * level;
  // Wait till new level reached
  if ((PMMIFG & SVMLIFG))
    while ((PMMIFG & SVMLVLRIFG) == 0);
  // Set SVS/SVM low side to new level
  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
  // Lock PMM registers for write access
  PMMCTL0_H = 0x00;
}
