/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include "math.h"
#include "st7789/st7789.h"
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
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
#define GRAVITY 9.81
#define HEIGHT 240
#define WIDTH 240
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
float CalcHorPos(float speed, float time);
float CalcVerPos(float initialPos, float time);
uint16_t MetersToPx(float pos);
float PxToMeters(float px);
void DrawPlane(uint16_t x, uint16_t y, uint16_t color);
uint16_t ConvertHeightToScreenY(float height);
uint8_t AnyButtonPressed();
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
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  ST7789_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  ST7789_Fill_Color(BLACK);
	  ST7789_WriteString(30, 50, "Aperte para iniciar", Font_11x18, WHITE, BLACK);
	  while(!AnyButtonPressed()){}
	  ST7789_Fill_Color(BLACK);
	  float h = 1000, velAviao = 55, tempo; //Valores de teste, SI
	  tempo = h*2/GRAVITY;
	  tempo = sqrt(tempo);
	  float deltaS = velAviao * tempo;
	  float posAlvo = rand()%(WIDTH/5) + 0.7 * WIDTH; // Posição em px
	  ST7789_DrawFilledRectangle((uint16_t)posAlvo, HEIGHT-2, 5, 2, RED); // Alvo
	  posAlvo = PxToMeters(posAlvo);
	  float posLancamento = posAlvo - deltaS;

	  uint8_t simulando = 1, cargaLancada = 0;
	  uint16_t alturaAviao = ConvertHeightToScreenY(h);
	  uint16_t ultimaPosAviao = 0, ultimaPosCarga = posLancamento, ultimaAltCarga = alturaAviao;
	  posLancamento = MetersToPx(posLancamento);
	  uint32_t inicioSimulacao = HAL_GetTick();
	  float tempoLancamento = 0;
	  while(simulando){
		  float t = (HAL_GetTick() - inicioSimulacao)/(float)1000;
		  float posAviao = CalcHorPos(velAviao, t); posAviao = MetersToPx(posAviao);

		  if(ultimaPosAviao != posAviao){
			  DrawPlane(ultimaPosAviao, alturaAviao, BLACK);
			  ultimaPosAviao = posAviao;
		  }

		  DrawPlane(posAviao, alturaAviao, WHITE);
		  if(posAviao >= posLancamento && !cargaLancada){
			  cargaLancada = 1;
			  tempoLancamento = t;
		  }
		  else if(posAviao >= posLancamento){
			  float posCarga = PxToMeters(posLancamento) + CalcHorPos(velAviao, t-tempoLancamento); posCarga = MetersToPx(posCarga);
			  float altCarga = CalcVerPos(h, t-tempoLancamento); altCarga = ConvertHeightToScreenY(altCarga);
			  if(ultimaPosCarga != posCarga || ultimaAltCarga != altCarga){
				  ST7789_DrawFilledCircle(ultimaPosCarga, ultimaAltCarga, 2, BLACK);
				  ultimaPosCarga = posCarga; ultimaAltCarga = altCarga;
			  }
			  ST7789_DrawFilledCircle((uint16_t)posCarga, (uint16_t)altCarga, 2, WHITE);
			  if(posCarga >= MetersToPx(posAlvo)) {
				  simulando = 0;
			  }
		  }
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ST7789_CS_GPIO_Port, ST7789_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ST7789_DC_Pin|ST7789_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : ST7789_CS_Pin */
  GPIO_InitStruct.Pin = ST7789_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ST7789_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ST7789_DC_Pin ST7789_RST_Pin */
  GPIO_InitStruct.Pin = ST7789_DC_Pin|ST7789_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA9 PA10 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
float CalcHorPos(float speed, float time){
	return time*speed;
}
float CalcVerPos(float initialPos, float time){
	return initialPos - GRAVITY/2*time*time;
}
uint16_t MetersToPx(float pos){
	return pos * ((float)15 / 70);
}
float PxToMeters(float px){
	return px*((float)70/15);
}
void DrawPlane(uint16_t x, uint16_t y, uint16_t color){
	ST7789_DrawFilledRectangle(x, y, 13, 3, color);
	ST7789_DrawFilledTriangle(x, y+3, x, y+4, x+1, y+3, color);
	ST7789_DrawFilledTriangle(x+13, y, x+13, y+1, x+15, y, color);
}
uint16_t ConvertHeightToScreenY(float height) {
	return HEIGHT - MetersToPx(height);
}
uint8_t AnyButtonPressed(){
	return !(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12));
}
/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
