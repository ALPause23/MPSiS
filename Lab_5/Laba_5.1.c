#include <msp430.h>
#include <math.h>

typedef unsigned char uint8_t;

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
#define SET_MIRROR_SEG              0xA1  //Mirror SEG (column) mapping (set bit0 to mirror display)
#define SET_MIRROR_COM              0xC8  //Mirror COM (row) mapping (set bit3 to mirror display)
#define BIAS_RATIO_VCC              0xA2  //Set voltage bias ratio (BR = bit0)
#define ADV_CTL_MSB                 0xFA  //Set temp. compensation curve to -0.11%/C
#define ADV_CTL_LSB                 0x90

#define DATA_Y                      0x1C  //read from address 07h
#define DATA_Z                      0x20  //read from address 08h

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

int mirror_status = 0; // 0 - default, 1 - mirror horizontal
int COLUMN_START_ADDRESS = 0; // 0 - default(0), 1 - mirror horizontal(31)


long int convert_Mili = 79020;  // 3600 * 3600 / 1609 * 9.81 -- convert to miles per hour^2

int physic_equivalents[] = { 4571, 2286, 1141, 571, 286, 143, 71 };
uint8_t BITx[] = {BIT6, BIT5, BIT4, BIT3, BIT2, BIT1, BIT0};

uint8_t currentPage = 0, currentColumn = 0;

uint8_t Dogs102x6_initMacro[] = {
    SCROL_CTL, //нулевая позиция скролла
    SET_MIRROR_SEG, // установка зеркальности по столбцам
    SET_MIRROR_COM, // установка зеркальности по строкам
    ALL_PIXEL_ON, // запрет режима включения всех пискселей (на экран отображается содержимое памяти);
    LCD_INVERSE, // отключ инверсии
    BIAS_RATIO_VCC, //смещение напряжения делителя 1/9;
    POW_CTL, // включение питания усилителя, регулятора и повторителя;
    SET_CONTRAST_RESISTOR, // установка контраста;
    MSB_ELECT_VOLUME,
    LSB_ELECT_VOLUME,
    ADV_CTL_MSB, //установка температурной компенсации -0.11%/°С;
    ADV_CTL_LSB,
    LCD_EN, //включение экрана.
    PAGE_ADR, // установка нулевой страницы
    COLUMN_ADR_MSB, // установка столбца
    COLUMN_ADR_LSB
};



void delay(int j)
{
    volatile i, k = 1;
    for (i = 0; i < j; i++)
    {
        k++;
    }
}

long int miliG(uint8_t projectionByte)
{
    volatile int minus = projectionByte & BIT7;

    int i;
    volatile long int projection = 0;

    for (i = 0; i < 7; i++)
    {
        if (minus)
        {
            projection += (BITx[i] & projectionByte) ? 0 : physic_equivalents[i];
        }
        else
        {
            projection += (BITx[i] & projectionByte) ? physic_equivalents[i] : 0;
        }
    }

    projection *= minus ? -1 : 1;

    return projection;
}


long int grad(long int projection)
{
    double ratio = ((double)projection) / 1000;

    volatile double angle = asin(ratio);

    // convert rad to deg
    angle *= 57.3;

    return (long int)angle;
}

void Dogs102x6_writeCommand(uint8_t *sCmd, uint8_t i)
{
    // CS Low
    P7OUT &= ~BIT4; // LCD_CS - выбор устройства

    // CD Low
    P5OUT &= ~BIT6; // ожидание команды на slave
    while (i)
    {
        // USCI_B1 TX buffer ready?
        while (!(UCB1IFG & UCTXIFG)); // ожидание прерывания UCTXIFG - о том, что буфер свободен
        // буфер освободился, идём дальше
        // Transmit data
        UCB1TXBUF = *sCmd; // загружаем 8-бит данных

        // Increment the pointer on the array
        sCmd++;

        // Decrement the Byte counter
        i--;
    }

    // Wait for all TX/RX to finish
    while (UCB1STAT & UCBUSY); //пока не закончится передача

    // Dummy read to empty RX buffer and clear any overrun conditions
    UCB1RXBUF;

    // CS High
    P7OUT |= BIT4;

}

void set_ADR(uint8_t page, uint8_t column)
{
    uint8_t cmd[1];

    if (page > 7) // заглушка, если вдруг выход за пределы страницы
    {
        page = 7;
    }

    if (column > 101) // заглушка, если вдруг выход за пределы столбцов (т.к. в LCD их 102, а в мк 132)
    {
        column = 101;
    }

    cmd[0] = PAGE_ADR + (7 - page);// выставление реального адресса страницы
    uint8_t H = 0x00;
    uint8_t L = 0x00;
    uint8_t ColumnAddress[] = { COLUMN_ADR_MSB, COLUMN_ADR_LSB }; // т.к. режим MSB, то заносим старшую, потом младшую часть адреса

    currentPage = page;
    currentColumn = column;

    L = (column & 0x0F); // формируем LSB
    H = (column & 0xF0); // формируем МSB
    H = (H >> 4);

    // реальный адрес столбца (команда двухбайтная)
    ColumnAddress[0] = COLUMN_ADR_LSB + L;
    ColumnAddress[1] = COLUMN_ADR_MSB + H;


    Dogs102x6_writeCommand(cmd, 1);
    Dogs102x6_writeCommand(ColumnAddress, 2);
}

void Dogs102x6_writeData(uint8_t *sData, uint8_t i)
{

    // CS Low
    P7OUT &= ~BIT4; // LCD_CS - выбор устройства
    //CD High
    P5OUT |= BIT6; // ожидание данных на slave

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

uint8_t cma3000_Rx_Tx(char first_byte, char second_byte)
{
    char data;

    P3OUT &= ~BIT5;

    data = UCA0RXBUF; //reset overrun eror flag UCOE

    while(!(UCA0IFG & UCTXIFG)); // USCI_A0 TX buffer ready?
    UCA0TXBUF = first_byte;

    // USCI_A0 RX buffer ready?
    while(!(UCA0IFG & UCRXIFG));
    data = UCA0RXBUF; // read first frame (empty)

    // USCI_A0 TX buffer ready?
    while(!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = second_byte;

    while(!(UCA0IFG & UCRXIFG));
    data = UCA0RXBUF; // read second frame (data)

    while(UCA0STAT & UCBUSY);
    P3OUT |= BIT5;  // disable cma3000 SPI data transfer
    return data;
}

void displayNum(long int number)
{
    volatile int lenght = 1;
    volatile int hren = 0;
    volatile long int j = 0;
    volatile long int i = 10;
    while(1) // считаем длинну числа
    {
        if(number / i != 0)
        {
            i *= 10;
            lenght++;
        }
        else break;
    }
    set_ADR(0, COLUMN_START_ADDRESS); //начальный адресс
    if(number <= 0) // печатаем знак +/-
        Dogs102x6_writeData(minus, 13);
    else Dogs102x6_writeData(plus, 13);
    long int num = number;
    for(j = 0; j < lenght; j++) //выбираем и печатаем символ (от старшего к младшему)
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

void initLED(void)
{
	P1DIR |= BIT5;
	P1OUT &= ~BIT5;
}

void initLCD()
{
    //устанавливаем сброс LCD_RST
    P5DIR |= BIT7;
    P5OUT &= BIT7; //сброс
    P5OUT |= BIT7;

    //устанавливаем выбор устройства LCD_CS
    P7DIR |= BIT4;
    P7OUT &= ~BIT4;

    //устьанавливаем пин для выбора команд и данных LCD_D/C
    P5DIR |= BIT6;
    P5OUT &= ~BIT6;

    //установка SIMO и SCLK
    P4SEL |= BIT1 | BIT3;
    P4DIR |= BIT1 | BIT3;

    //подсветка
    P7DIR |= BIT6;
    P7OUT |= BIT6;
    P7SEL &= ~BIT6;
}

void initSPI()
{
    // interupt
    P2DIR  &= ~BIT5;
    P2OUT  |=  BIT5;
    P2REN  |=  BIT5;
    P2IE   |=  BIT5;
    P2IES  &= ~BIT5;
    P2IFG  &= ~BIT5;

    // chip select (~CSB) 
    P3DIR  |=  BIT5;
    P3OUT  |=  BIT5;

    // set SCK
    P2DIR  |=  BIT7;
    P2SEL  |=  BIT7;

    P3DIR  |= (BIT3 | BIT6);    // Set MOSI and power
    P3DIR  &= ~BIT4;            // Set SOMI
    P3SEL  |= (BIT3 | BIT4);
    P3OUT  |= BIT6;

    // set configuration for device USCI_B (LCD)
    UCB1CTL1 |= UCSWRST; //Разрешение программного сброса:
    UCB1CTL0 = (UCCKPH | UCMSB | UCMST | UCSYNC | UCMODE_0); //(|порядок передачи MSB|режим Master|синхронный|режим 3-pin)
    UCB1CTL1 = UCSSEL_2 | UCSWRST; // выбор источника
    UCB1BR0 = 0x02; // Младший байт делителя частоты
    UCB1BR1 = 0; //Старший байт делителя частоты

    UCB1CTL1 &= ~UCSWRST;
    UCB1IFG &= ~UCRXIFG;
    Dogs102x6_writeCommand(Dogs102x6_initMacro, 13);
    P7OUT |= BIT4;

    // set configuration for device USCI_A (cma3000)
    UCA0CTL1 |= UCSWRST;        // set UCSWRST bit to disable USCI and change its control registers
    UCA0CTL0 = (UCCKPH & ~UCCKPL) | UCMSB | UCMST | UCSYNC | UCMODE_0;

    // set SMCLK as source and keep RESET
    UCA0CTL1 = UCSSEL_2 | UCSWRST;
    UCA0BR0 = 0x30;
    UCA0BR1 = 0x0;
    UCA0CTL1 &= ~UCSWRST;

    // dummy read from REVID
    cma3000_Rx_Tx(0x04, 0);
    __delay_cycles(1250);

    // write to CTRL register
    cma3000_Rx_Tx(0x0A, BIT4 | BIT2); // I2C disable, mode 010 =
    __delay_cycles(25000);
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    initLED();
    initLCD();
    initSPI();

    __bis_SR_register(LPM0_bits + GIE);
    __no_operation();
    return 0;
}

#pragma vector = PORT2_VECTOR
__interrupt void accelerometer(void)
{
	clear();

    volatile long int Y_pr = miliG(cma3000_Rx_Tx(DATA_Y, 0));
    volatile long int Y_convert = Y_pr * convert_Mili;
    displayNum(Y_convert);

    uint8_t z_byte = cma3000_Rx_Tx(DATA_Z, 0);
    long int Z_pr = miliG(z_byte);

    long int angle = grad(Y_pr);

    if(Y_pr < 0)
    {
        if(Z_pr < 0)
        {
            angle = -180 - angle;
        }
    }
    else
    {
        if(Z_pr < 0)
        {
            angle = 180 - angle;
        }
    }

   if ((angle >= -120) && (angle <= -60))
   {
       P1OUT |= BIT5;
   }
   else
   {
       P1OUT &= ~BIT5;
   }

}
