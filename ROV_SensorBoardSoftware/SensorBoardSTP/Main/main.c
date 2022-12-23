#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "delay.h"
#include "MultiTimer.h"
#include "led.h"
#include "usart.h"
#include "myiic.h"
#include "MS5837.h"
#include "sht20_iic.h"
#include "SHT20.h"

uint64_t TIM2_SYSTICK = 0;

u8 ENABLE_SEND = 0;


void TIM3_Int_Init(u16 arr, u16 psc);
void TIM2_Int_Init(u16 arr, u16 psc);
uint64_t TIM2_SoftTimerTick(void);

static volatile MultiTimer led_timer;

static inline void UART_Trans(u16 data)
{
    USART_SendData(USART1, data);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

static void UART_Report(u8 is_negative, u16 u_temp, u32 u_pres, u16 u_depth, u16 u_carbin_temp, u16 u_carbin_hum)
{
    UART_Trans(0x21);
    UART_Trans((u8)((u_temp & 0xFF00) >> 8));
    UART_Trans((u8)(u_temp & 0x00FF));
    UART_Trans((u8)((u_pres & 0xFF000000) >> 24));
    UART_Trans((u8)((u_pres & 0x00FF0000) >> 16));
    UART_Trans((u8)((u_pres & 0x0000FF00) >> 8));
    UART_Trans((u8)(u_pres & 0x000000FF));
    UART_Trans(is_negative);
    UART_Trans((u8)((u_depth & 0xFF00) >> 8));
    UART_Trans((u8)(u_depth & 0x00FF));
    UART_Trans((u8)((u_carbin_temp & 0xFF00) >> 8));
    UART_Trans((u8)(u_carbin_temp & 0x00FF));
    UART_Trans((u8)((u_carbin_hum & 0xFF00) >> 8));
    UART_Trans((u8)(u_carbin_hum & 0x00FF));
    UART_Trans(0xFF);
    UART_Trans(0xFF);
}

static void led_timer_cb(MultiTimer* timer, void* userData)
{
    LED = !LED;
    MultiTimerRestart(timer, (uint32_t)(userData));
}

int main(void)
{
    float temp = 0.0;
    float pres = 0.0;
    float depth = 0.0;
    float carbin_temp = 0.0;
    float carbin_hum = 0.0;

    u16 u_temp = 0;
    u32 u_pres = 0;
    u16 u_depth = 0;
    u16 u_carbin_temp = 0;
    u16 u_carbin_hum = 0;

    delay_init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    LED_Init();
    uart_init(115200);
    uart_sensor_init(115200);
    TIM3_Int_Init(500, 4000);
    // TIM3_Int_Init(500, 500);
    TIM2_Int_Init(72 - 1, 1000 - 1); //T=1ms
    MultiTimerInstall(TIM2_SoftTimerTick);
    IIC_Init();
    SHT20_IIC_Init();
    MS5837_Init(SEA_WATER_DENSITY, MS5837_30BA, mBar, temperature_c);
    MS5837_SetRefPressure(TEST_UNDERWATER_PRESSURE);
    SHT20_Init();

    MultiTimerStart(&led_timer, 1000, led_timer_cb, (void*)1000);
    MultiTimerStart(&MS5837_recv_timer, 20, MS5837_GetDataTask_cb, (void*)20);
    MultiTimerStart(&SHT20_recv_timer, 20, SHT20_GetDataTask_cb, (void*)20);

    while (1)
    {
        MultiTimerYield();

        MS5837_getData(&temp, &pres);
        MS5837_ReadDepth_filtered(&depth);
        SHT20_getData(&carbin_temp, &carbin_hum);

        u_temp = (u16)(temp * 100);
        u_pres = (u32)(pres * 100);
        u_carbin_temp = (u16)(carbin_temp * 100);
        u_carbin_hum = (u16)(carbin_hum * 100);

        if ((depth >= -0.05f) && (depth <= 0.05f))
        {
            depth = 0.0f;
        }
        else if (depth < -0.05f)
        {
            depth *= -1000.0f;
        }
        else
        {
            depth *= 1000.0f;
        }
        u_depth = (u16)depth;
        if (depth - u_depth > 0.5) //ceil
        {
            u_depth++;
        }

        if (ENABLE_SEND == 1) //driven by TIM3
        {
            ENABLE_SEND = 0;
            if (GPIO_PA0 == 0) //debug mode
            {
                if (depth < 0)
                {
                    printf("%d, %d, -%d, %d, %d\r\n", u_temp, u_pres, u_depth, u_carbin_temp, u_carbin_hum);
                }
                else
                {
                    printf("%d, %d, %d, %d, %d\r\n", u_temp, u_pres, u_depth, u_carbin_temp, u_carbin_hum);
                }
            }
            else
            {
                if (depth < 0)
                {
                    UART_Report(0x00, u_temp, u_pres, u_depth, u_carbin_temp, u_carbin_hum);
                }
                else
                {
                    UART_Report(0x01, u_temp, u_pres, u_depth, u_carbin_temp, u_carbin_hum);
                }
            }
        }
    }
}

uint64_t TIM2_SoftTimerTick(void)
{
    return TIM2_SYSTICK;
}

void TIM3_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

void TIM2_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM2, ENABLE);
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        ENABLE_SEND = 1;
    }
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        TIM2_SYSTICK++;
    }
}


