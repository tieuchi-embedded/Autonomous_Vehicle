/*
 * uart.c
 *
 *  Created on: Dec 19, 2025
 *      Author: Admin
 */
#include "mydriver.h"

#define USART2_IRQN  38

#define UART2_RX_BUF_SIZE 256
volatile uint8_t uart2_rx_dma_buf[UART2_RX_BUF_SIZE];

void UART1_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    GPIOA->MODER &= ~(0b11 << (9 * 2));            //PA9 - TX - AF7
    GPIOA->MODER |=  (0b10 << (9 * 2));
    GPIOA->AFR[1] &= ~(0xF << ((9 - 8) * 4));
    GPIOA->AFR[1] |=  (0x7 << ((9 - 8) * 4));

    GPIOA->MODER &= ~(0b11 << (10 * 2));           //PA10 - RX - AF7
    GPIOA->MODER |=  (0b10 << (10 * 2));
    GPIOA->AFR[1] &= ~(0xF << ((10 - 8) * 4));
    GPIOA->AFR[1] |=  (0x7 << ((10 - 8) * 4));

    /* Baudrate = 115200, PCLK2 = 16 MHz
       USARTDIV = 16MHz / (16 * 115200) = 8.680 */
    USART1->BRR = (8 << 4) | 11;

    USART1->CR1 &= ~(1 << 13);
    USART1->CR1 = 0;
    USART1->CR1 &= ~(1 << 15);   // Oversampling 16
    USART1->CR1 &= ~(1 << 12);   // 8-bit data
    USART1->CR1 &= ~(1 << 10);   // No parity
    USART1->CR1 |=  (1 << 2);    // RE
    USART1->CR1 |=  (1 << 3);    // TE
    USART1->CR1 |=  (1 << 13);   // UE
}

void UART2_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* PA2 TX, PA3 RX */
    GPIOA->MODER &= ~((3 << (2*2)) | (3 << (3*2)));
    GPIOA->MODER |=  ((2 << (2*2)) | (2 << (3*2)));

    GPIOA->AFR[0] &= ~((0xF << (2*4)) | (0xF << (3*4)));
    GPIOA->AFR[0] |=  ((7 << (2*4)) | (7 << (3*4)));

    USART2->BRR = (8 << 4) | 11;   // 115200 @16MHz

    USART2->CR1 = 0;
    USART2->CR1 |= (1 << 2);   // RE
    USART2->CR1 |= (1 << 3);   // TE
    USART2->CR1 |= (1 << 5);   // RXNEIE
    USART2->CR1 |= (1 << 13);  // UE

    NVIC->ISER[1] = (1U << (USART2_IRQN - 32));
}


void UART2_DMA_RX_Init(void)
{
    DMA1_Stream5->CR &= ~DMA_SCR_EN;
    while (DMA1_Stream5->CR & DMA_SCR_EN);

    DMA1_Stream5->PAR  = (uint32_t)&USART2->DR;
    DMA1_Stream5->M0AR = (uint32_t)uart2_rx_dma_buf;
    DMA1_Stream5->NDTR = UART2_RX_BUF_SIZE;

    DMA1_Stream5->CR = 0;
    DMA1_Stream5->CR |= (4 << 25);   // USART2 - CH4
    DMA1_Stream5->CR |= (0 << 6);    // (PERIPHERAL -> MEMORY)
    DMA1_Stream5->CR |= (1 << 10);   // MINC = 1
    DMA1_Stream5->CR |= (0 << 8);    // PSIZE = 8-bit
    DMA1_Stream5->CR |= (0 << 11);   // MSIZE = 8-bit
    DMA1_Stream5->CR |= (1 << 8);    // CIRCULAR MODE
    DMA1_Stream5->CR |= (0 << 16);   // PL = lOW

    DMA1_Stream5->CR |= DMA_SCR_EN;
}

void USART2_IRQHandler(void)
{
    if (USART2->SR & USART_SR_RXNE)
    {
        uint8_t c = (uint8_t)(USART2->DR & 0xFF);   // Clear RXNE
        UART_Parse_Byte(c);
    }
}

void UART1_Transmit(uint8_t data)
{
    while ((USART1->SR & USART_SR_TXE) == 0);   // TXE
    USART1->DR = data;
    while ((USART1->SR & USART_SR_TC) == 0);    // TC
}

void UART2_Transmit(uint8_t data)
{
    while ((USART2->SR & USART_SR_TXE) == 0);   // TXE
    USART2->DR = data;
    while ((USART2->SR & USART_SR_TC) == 0);    // TC
}

uint8_t UART1_Receive(void)
{
    while ((USART1->SR & USART_SR_RXNE) == 0);
    return (uint8_t)(USART1->DR & 0xFF);
}

uint8_t UART2_Receive(void)
{
    while ((USART2->SR & USART_SR_RXNE) == 0);
    return (uint8_t)(USART2->DR & 0xFF);
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
    //#speed:60;;
    if (strncmp(cmd, "#RPM:", 5) == 0) {
        int speed = atoi(cmd + 5);
        Servo_Speed_Set(speed);
        return;
    }

    if (strncmp(cmd, "#STR:", 5) == 0) {
        int steer = atoi(cmd + 5);
        Servo_Steering_Set(steer);
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
