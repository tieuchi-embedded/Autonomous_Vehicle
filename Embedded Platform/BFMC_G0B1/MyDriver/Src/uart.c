/*
 * uart.c — G0B1 port
 *
 * G0B1 USART is a v2 peripheral: status/data via ISR/TDR/RDR (NOT SR/DR like F4).
 * TXE/RXNE/TC flags moved into ISR; UE/RE/TE bit positions in CR1 are unchanged.
 * BRR formula for OVER8=0 is the same (USARTDIV = fCLK / baud).
 *
 * Pin AF: PA2/PA3 = USART2 TX/RX is AF1 on G0B1 (was AF7 on F411).
 *         PA9/PA10 = USART1 TX/RX is AF1 on G0B1 (was AF7 on F411).
 *
 * NOTE: UART2_DMA_RX_Init from the F411 port is NOT ported — G0B1 DMA is
 * channel/DMAMUX-based (completely different programming model from F4
 * stream-based DMA). Not required by current main.c; add later if needed.
 */
#include "mydriver.h"

#define UART2_RX_BUF_SIZE 256
volatile uint8_t uart2_rx_dma_buf[UART2_RX_BUF_SIZE];

void UART1_init(void)
{
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;

    GPIOA->MODER &= ~(0b11 << (9 * 2));            //PA9 - TX - AF1
    GPIOA->MODER |=  (0b10 << (9 * 2));
    GPIOA->AFR[1] &= ~(0xF << ((9 - 8) * 4));
    GPIOA->AFR[1] |=  (0x1 << ((9 - 8) * 4));

    GPIOA->MODER &= ~(0b11 << (10 * 2));           //PA10 - RX - AF1
    GPIOA->MODER |=  (0b10 << (10 * 2));
    GPIOA->AFR[1] &= ~(0xF << ((10 - 8) * 4));
    GPIOA->AFR[1] |=  (0x1 << ((10 - 8) * 4));

    /* Baudrate = 115200, fCLK = 16 MHz, OVER8=0
       USARTDIV = 16MHz / 115200 = 138.9 -> 139 */
    USART1->BRR = 139;

    USART1->CR1 = 0;
    USART1->CR1 |=  (1 << 2);    // RE
    USART1->CR1 |=  (1 << 3);    // TE
    USART1->CR1 |=  (1 << 0);    // UE
}

void UART2_init(void)
{
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;

    /* PA2 TX, PA3 RX — AF1 on G0B1 */
    GPIOA->MODER &= ~((3 << (2*2)) | (3 << (3*2)));
    GPIOA->MODER |=  ((2 << (2*2)) | (2 << (3*2)));

    GPIOA->AFR[0] &= ~((0xF << (2*4)) | (0xF << (3*4)));
    GPIOA->AFR[0] |=  ((1 << (2*4)) | (1 << (3*4)));

    USART2->BRR = 139;   // 115200 @ fCLK=16MHz, OVER8=0

    USART2->CR1 = 0;
    USART2->CR1 |= (1 << 2);   // RE
    USART2->CR1 |= (1 << 3);   // TE
    USART2->CR1 |= (1 << 5);   // RXNEIE
    USART2->CR1 |= (1 << 0);   // UE
    USART2->CR3 &= ~(1U << 0); // disable error interrupt

    NVIC->ISER[0] = (1U << USART2_IRQn);
}

void UART2_DMA_RX_Init(void)
{
    /* Not ported: G0B1 uses channel/DMAMUX-based DMA, incompatible with the
       F4 DMA_Stream programming model used here. Implement separately if
       DMA-based UART RX becomes necessary. */
}

void USART2_LPUART2_IRQHandler(void)
{
    if (USART2->ISR & USART_ISR_RXNE_RXFNE)
    {
        uint8_t c = (uint8_t)(USART2->RDR & 0xFF);   // Reading RDR clears RXNE
        UART_Parse_Byte(c);
    }
}

void UART1_Transmit(uint8_t data)
{
    while ((USART1->ISR & USART_ISR_TXE_TXFNF) == 0);
    USART1->TDR = data;
    while ((USART1->ISR & USART_ISR_TC) == 0);
}

void UART2_Transmit(uint8_t data)
{
	NVIC->ICER[0] = (1U << USART2_IRQn);
    while ((USART2->ISR & USART_ISR_TXE_TXFNF) == 0);
    USART2->TDR = data;
    while ((USART2->ISR & USART_ISR_TC) == 0);
    NVIC->ISER[0] = (1U << USART2_IRQn);
}

uint8_t UART1_Receive(void)
{
    while ((USART1->ISR & USART_ISR_RXNE_RXFNE) == 0);
    return (uint8_t)(USART1->RDR & 0xFF);
}

uint8_t UART2_Receive(void)
{
    while ((USART2->ISR & USART_ISR_RXNE_RXFNE) == 0);
    return (uint8_t)(USART2->RDR & 0xFF);
}

void UART1_send_number(int num)
{
    char buf[12];
    int i = 0;
    if (num == 0) {
        UART1_Transmit('0');
        return;
    }
    if (num < 0) {
        UART1_Transmit('-');
        num = -num;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i--)
        UART1_Transmit(buf[i]);
}

void UART2_send_number(int num)
{
    char buf[12];
    int i = 0;
    if (num == 0) {
        UART2_Transmit('0');
        return;
    }
    if (num < 0) {
        UART2_Transmit('-');
        num = -num;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i--)
        UART2_Transmit(buf[i]);
}

void UART1_print_log(char *msg)
{
    for (int i = 0; msg[i]; i++) {
        UART1_Transmit((uint8_t)msg[i]);
    }
}

void UART2_print_log(char *msg)
{
    for (int i = 0; msg[i]; i++) {
        UART2_Transmit((uint8_t)msg[i]);
    }
}

void UART1_send_float(float v)
{
    if (v < 0) {
        UART1_Transmit('-');
        v = -v;
    }

    int ip = (int)v;
    int fp = (int)((v - ip) * 100);

    UART1_send_number(ip);
    UART1_Transmit('.');
    if (fp < 10) UART1_Transmit('0');
    UART1_send_number(fp);
}

void UART2_send_float(float v)
{
    if (v < 0) {
        UART2_Transmit('-');
        v = -v;
    }

    int ip = (int)v;
    int fp = (int)((v - ip) * 100);

    UART2_send_number(ip);
    UART2_Transmit('.');
    if (fp < 10) UART2_Transmit('0');
    UART2_send_number(fp);
}

uint8_t UART_Parse_Speed(uint8_t *buf, uint16_t len, int *speed_out)
{
    if (len < 5) return 0;

    if (len >= UART2_RX_BUF_SIZE)
        len = UART2_RX_BUF_SIZE - 1;

    ((uint8_t*)buf)[len] = '\0';

    if (buf[0] != '#') return 0;

    char *colon = strchr((char*)buf, ':');
    if (!colon) return 0;

    char *end = strstr((char*)buf, ";;");
    if (!end) return 0;

    int speed = atoi(colon + 1);

    *speed_out = speed;
    return 1;
}

void UART_Parse_Command(char *cmd)
{
    if (strncmp(cmd, "#RPM:", 5) == 0) {
        float rpm = atof(cmd + 5);
        Servo_Speed_Set_RPM(rpm);
        return;
    }

    if (strncmp(cmd, "#STR:", 5) == 0) {
        float deg = atof(cmd + 5);
        Servo_Steering_Set_Deg(deg);
        return;
    }
}

static char cmd_buf[64];
static uint8_t cmd_idx = 0;

void UART_Parse_Byte(uint8_t c)
{
    if (c == '#') {
        cmd_idx = 0;
        cmd_buf[cmd_idx++] = c;
        return;
    }

    if (cmd_idx == 0)
        return;

    if (cmd_idx < sizeof(cmd_buf) - 1) {
        cmd_buf[cmd_idx++] = c;

        if (cmd_idx >= 2 && cmd_buf[cmd_idx - 1] == ';' && cmd_buf[cmd_idx - 2] == ';') {
            cmd_buf[cmd_idx] = '\0';
            UART_Parse_Command(cmd_buf);
            cmd_idx = 0;
        }
    }
    else {
        cmd_idx = 0;
    }
}
