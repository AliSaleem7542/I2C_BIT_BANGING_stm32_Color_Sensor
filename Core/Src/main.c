/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "BitBangI2C.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t sensor_id = 0;
uint8_t ack1, ack2, ack3;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  setvbuf(stdout, NULL, _IONBF, 0);
  HAL_TIM_Base_Start(&htim2);
  BitBangI2C_Init();

  HAL_Delay(100);
  printf("Starting I2C bit bang... \n");

  while (1)
  {
    uint16_t c, r, g, b;

    /* ---- LED1 ON -> Sensor 0 & Sensor 3 -> LED1 OFF ---- */
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    {
      uint8_t group1[] = {0, 3};
      for (uint8_t i = 0; i < 2; i++)
      {
        uint8_t channel = group1[i];
        TCA9548A_SelectChannel(channel);
        TCS34727_Enable();
        TCS34727_ReadRGBC(&c, &r, &g, &b);
        printf("Sensor %d: C=%u R=%u G=%u B=%u\r\n", channel, c, r, g, b);
      }
    }
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);

    /* ---- LED2 ON -> Sensor 2 & Sensor 4 -> LED2 OFF ---- */
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    {
      uint8_t group2[] = {2, 4};
      for (uint8_t i = 0; i < 2; i++)
      {
        uint8_t channel = group2[i];
        TCA9548A_SelectChannel(channel);
        TCS34727_Enable();
        TCS34727_ReadRGBC(&c, &r, &g, &b);
        printf("Sensor %d: C=%u R=%u G=%u B=%u\r\n", channel, c, r, g, b);
      }
    }
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);

    /* ---- LED3 ON -> Sensor 1 -> LED3 OFF ---- */
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
    {
      uint8_t channel = 1;
      TCA9548A_SelectChannel(channel);
      TCS34727_Enable();
      TCS34727_ReadRGBC(&c, &r, &g, &b);
      printf("Sensor %d: C=%u R=%u G=%u B=%u\r\n", channel, c, r, g, b);
    }
    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);

    printf("-----------------------------\r\n");

    //...........................................
    // uint16_t c, r, g, b;

    // for (uint8_t channel = 0; channel < 5; channel++)
    // {
    //   printf("\r\nSelecting sensor channel %d...\r\n", channel);

    //   /* Select one TCS34727 through the MUX */
    //   TCA9548A_SelectChannel(channel);

    //   /* Enable this sensor */
    //   TCS34727_Enable();

    //   /* Read RGBC */
    //   TCS34727_ReadRGBC(&c, &r, &g, &b);

    //   printf("Sensor %d: C=%u R=%u G=%u B=%u\r\n",
    //          channel,
    //          c,
    //          r,
    //          g,
    //          b);

    //   HAL_Delay(50);
    // }

    // printf("-----------------------------\r\n");

    // HAL_Delay(500);
    //.........................
    // // ================= MUX =================

    // printf("Selecting MUX channel 1...\r\n");

    // I2C_Start();

    // uint8_t mux_ack1 = I2C_WriteByte(0x70 << 1);
    // uint8_t mux_ack2 = I2C_WriteByte(0x02);

    // I2C_Stop();

    // printf("MUX ACK1=%d ACK2=%d\r\n",
    //        mux_ack1, mux_ack2);

    // HAL_Delay(10);

    // // ================= SENSOR ENABLE =================

    // printf("Enabling TCS34727...\r\n");

    // I2C_Start();

    // uint8_t en_ack1 = I2C_WriteByte(0x29 << 1);
    // uint8_t en_ack2 = I2C_WriteByte(0x80 | 0x00);
    // uint8_t en_ack3 = I2C_WriteByte(0x03);

    // I2C_Stop();

    // printf("ENABLE ACK: %d %d %d\r\n",
    //        en_ack1, en_ack2, en_ack3);

    // HAL_Delay(100);

    // // ================= READ RGBC =================

    // uint16_t clear = TCS34727_Read16(0x14);
    // uint16_t red = TCS34727_Read16(0x16);
    // uint16_t green = TCS34727_Read16(0x18);
    // uint16_t blue = TCS34727_Read16(0x1A);

    // printf("C=%u  R=%u  G=%u  B=%u\r\n",
    //        clear, red, green, blue);

    // HAL_Delay(1000);

    //..............................
    // HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    // HAL_Delay(200);
    // HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

    // HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    // HAL_Delay(200);
    // HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

    // HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
    // HAL_Delay(200);
    // HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
    // printf("Selecting MUX channel 1...\r\n");

    // I2C_Start();

    // uint8_t ack1 = I2C_WriteByte(0x70 << 1);
    // uint8_t ack2 = I2C_WriteByte(0x02);

    // I2C_Stop();
    // printf("MUX ACK1=%d ACK2=%d\r\n", ack1, ack2);

    // HAL_Delay(10);

    // I2C_Scan();

    // HAL_Delay(100);

    // /* ================= TCS34727 ID READ ================= */

    // printf("Reading TCS34727 ID...\r\n");

    // I2C_Start();

    // uint8_t sensor_ack1 = I2C_WriteByte(0x29 << 1);   // 0x52, WRITE
    // uint8_t sensor_ack2 = I2C_WriteByte(0x80 | 0x12); // 0x92, command + ID register

    // I2C_Start();

    // uint8_t sensor_ack3 = I2C_WriteByte((0x29 << 1) | 1); // 0x53, READ

    // uint8_t sensor_id = I2C_ReadByte(0); // NACK after final byte

    // I2C_Stop();

    // printf("ACK1=%d ACK2=%d ACK3=%d ID=0x%02X\r\n",
    //        sensor_ack1,
    //        sensor_ack2,
    //        sensor_ack3,
    //        sensor_id);

    // /* ==================================================== */

    // HAL_Delay(1000);
    //...............
    // HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    // HAL_Delay(200);
    // HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

    // HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    // HAL_Delay(200);
    // HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

    // HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
    // HAL_Delay(200);
    // HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);

    // select_mux_channel(1); // schematic ke mutabik jis channel pe sensor hai
    // delay_us(50);          // switch ho jaane do

    //     printf("SDA idle = %d\r\n", I2C_ReadSDA());

    // I2C_Start();

    // uint8_t ack1 = I2C_WriteByte(0x29 << 1);
    // uint8_t ack2 = I2C_WriteByte(0x80 | 0x12);

    // I2C_Start();

    // uint8_t ack3 = I2C_WriteByte((0x29 << 1) | 1);

    // uint8_t sensor_id = I2C_ReadByte(0);

    // I2C_Stop();

    // printf("ACK1:%d ACK2:%d ACK3:%d ID:0x%02X\r\n",
    //        ack1, ack2, ack3, sensor_id);

    // HAL_Delay(500);
    //.....................................
    // I2C_Start();
    // uint8_t mux_ack = I2C_WriteByte(0x70 << 1); // mux address, write mode
    // I2C_Stop();
    // printf("MUX ACK: %d\r\n", mux_ack); // 0 = mux zinda hai, 1 = mux khud NACK kar raha
    // HAL_Delay(500);
    // I2C_Start();

    // SDA_HIGH();
    // delay_us(10);

    // printf("SDA idle = %d\r\n", SDA_READ());
    // uint8_t ack1 = I2C_WriteByte(0x29 << 1);   // WRITE
    // uint8_t ack2 = I2C_WriteByte(0x80 | 0x12); // ID register

    // I2C_Start();

    // uint8_t ack3 = I2C_WriteByte((0x29 << 1) | 1); // READ

    // uint8_t sensor_id = I2C_ReadByte(0);
    // I2C_Stop();

    // printf("ACK1:%d ACK2:%d ACK3:%d ID:0x%02X\r\n",
    //        ack1, ack2, ack3, sensor_id);

    // I2C_Start();
    // ack1 = I2C_WriteByte((0x39 << 1) | 0);
    // ack2 = I2C_WriteByte(0x92);
    // I2C_Start();
    // ack3 = I2C_WriteByte((0x39 << 1) | 1); // repeated start ke baad SIRF ek baar
    // sensor_id = I2C_ReadByte(0);
    // I2C_Stop();
    // printf("ACK1: %d, ACK2: %d, ACK3: %d, Sensor ID: 0x%02X\r\n", ack1, ack2, ack3, sensor_id);
    // HAL_Delay(500);

    //  select_mux_channel(1);
    //  HAL_Delay(5);

    // Write 0xF6 to ATIME register
    // I2C_Start();
    // uint8_t w1 = I2C_WriteByte((0x39 << 1) | 0);
    // uint8_t w2 = I2C_WriteByte(0x81);      // ATIME register (0x01), CMD bit set
    // uint8_t w3 = I2C_WriteByte(0xF6);
    // I2C_Stop();

    // HAL_Delay(10);

    // I2C_Start();
    // uint8_t r1 = I2C_WriteByte((0x39 << 1) | 0);
    // uint8_t r2 = I2C_WriteByte(0x81);
    // I2C_Start();
    // uint8_t r3 = I2C_WriteByte((0x39 << 1) | 1);
    // uint8_t readback = I2C_ReadByte(0);
    // I2C_Stop();

    // printf("Write ACKs: %d,%d,%d | Read ACKs: %d,%d,%d | ATIME Readback: 0x%02X\r\n",
    //        w1, w2, w3, r1, r2, r3, readback);
    // HAL_Delay(1000);

    // I2C mux scan test
    // for (uint8_t ch = 0; ch < 5; ch++) {
    //     select_mux_channel(ch);
    //     HAL_Delay(5);

    //     I2C_Start();
    //     uint8_t ack1 = I2C_WriteByte((0x39 << 1) | 0);
    //     uint8_t ack2 = I2C_WriteByte(0x92);
    //     I2C_Start();
    //     I2C_WriteByte((0x39 << 1) | 1);
    //     uint8_t id = I2C_ReadByte(0);
    //     I2C_Stop();

    //     printf("Channel %d: ACK1=%d ACK2=%d ID=0x%02X\r\n", ch, ack1, ack2, id);
    //     HAL_Delay(200);
    // }
    // HAL_Delay(1000);
    // // Pehle SDA ko actively Low drive karo
    // HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_RESET);
    // delay_us(20);

    // // Ab release karo (open-drain "float" — pull-up ko High le jana chahiye)
    // HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_SET);
    // delay_us(20);
    // uint8_t released_state = HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port, I2C_SDA_Pin);

    // printf("SDA release ke baad state: %d (1 hona chahiye agar pull-up kaam kar raha hai)\r\n", released_state);
    // HAL_Delay(500);

    // for (uint8_t ch = 0; ch < 5; ch++) {
    //     select_mux_channel(ch);
    //     HAL_Delay(5);

    //     I2C_Start();
    //     uint8_t ack1 = I2C_WriteByte((0x39 << 1) | 0);
    //     uint8_t ack2 = I2C_WriteByte(0x92);
    //     I2C_Start();
    //     I2C_WriteByte((0x39 << 1) | 1);
    //     uint8_t id = I2C_ReadByte(0);
    //     I2C_Stop();

    //     printf("Channel %d: ACK1=%d ACK2=%d ID=0x%02X\r\n", ch, ack1, ack2, id);
    //     HAL_Delay(200);
    // }
    // HAL_Delay(1000);

    //  I2C_Start();
    //     uint8_t w_ack1 = I2C_WriteByte((0x39 << 1) | 0);
    //     uint8_t w_ack2 = I2C_WriteByte(0x80);
    //     uint8_t w_ack3 = I2C_WriteByte(0x01);
    //     I2C_Stop();

    //     HAL_Delay(10);

    //     I2C_Start();
    //     uint8_t r_ack1 = I2C_WriteByte((0x39 << 1) | 0);
    //     uint8_t r_ack2 = I2C_WriteByte(0x80);
    //     I2C_Start();
    //     uint8_t r_ack3 = I2C_WriteByte((0x39 << 1) | 1);
    //     uint8_t enable_readback = I2C_ReadByte(0);
    //     I2C_Stop();

    //     printf("Write ACKs: %d,%d,%d | Read ACKs: %d,%d,%d | ENABLE readback: 0x%02X\r\n",
    //            w_ack1, w_ack2, w_ack3, r_ack1, r_ack2, r_ack3, enable_readback);

    //     // ---- LED Diagnostic Logic ----
    //     uint8_t all_ack_ok = (w_ack1 == 0 && w_ack2 == 0 && w_ack3 == 0 &&
    //                           r_ack1 == 0 && r_ack2 == 0 && r_ack3 == 0);

    //     if (all_ack_ok) {
    //         HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);   // Steady ON
    //         HAL_Delay(1000);
    //     } else {
    //         // Fast blink = problem
    //         for (int i = 0; i < 5; i++) {
    //             HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    //             HAL_Delay(100);
    //         }

    // Write 0x01 (PON bit) to ENABLE register
    // I2C_Start();
    // uint8_t w_ack1 = I2C_WriteByte((0x39 << 1) | 0);
    // uint8_t w_ack2 = I2C_WriteByte(0x80);   // Command: ENABLE register (0x00), CMD bit set
    // uint8_t w_ack3 = I2C_WriteByte(0x01);   // Value: PON = 1
    // I2C_Stop();

    // HAL_Delay(10);

    // // Read back ENABLE register
    // I2C_Start();
    // uint8_t r_ack1 = I2C_WriteByte((0x39 << 1) | 0);
    // uint8_t r_ack2 = I2C_WriteByte(0x80);
    // I2C_Start();
    // uint8_t r_ack3 = I2C_WriteByte((0x39 << 1) | 1);
    // uint8_t enable_readback = I2C_ReadByte(0);
    // I2C_Stop();

    // printf("Write ACKs: %d,%d,%d | Read ACKs: %d,%d,%d | ENABLE readback: 0x%02X\r\n",
    //        w_ack1, w_ack2, w_ack3, r_ack1, r_ack2, r_ack3, enable_readback);

    // HAL_Delay(1000);

    // I2C_Start();
    // ack1 = I2C_WriteByte((0x39 << 1) | 0);
    // ack2 = I2C_WriteByte(0x92);
    // I2C_Start();
    // I2C_WriteByte((0x39 << 1) | 1);
    // uint8_t ack3 = I2C_WriteByte((0x39 << 1) | 1);
    // sensor_id = I2C_ReadByte(0);
    // I2C_Stop();

    // // printf("ACK1: %d, ACK2: %d, Sensor ID: 0x%02X\r\n", ack1, ack2, sensor_id);
    // printf("ACK1: %d, ACK2: %d, ACK3: %d, Sensor ID: 0x%02X\r\n", ack1, ack2, ack3, sensor_id);
    // // HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    // HAL_Delay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* USER CODE END 3 */
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
   */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}
/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
   */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
