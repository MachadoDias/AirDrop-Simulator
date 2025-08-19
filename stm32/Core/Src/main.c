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
#include "stdio.h"
#include "math.h"
#include "st7789/st7789.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define GRAVITY 9.81
#define PI 3.14
#define HEIGHT 240
#define WIDTH 240
#define AIR_DENSITY 1225
#define CUBE_DC 1.05
#define SPHERE_DC 0.47
#define SQUARE 0
#define CIRCLE 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
typedef struct
{
  uint16_t xLancamento_px;
  uint16_t yLancamento_px;
  uint16_t xAlvo_px;
  uint16_t raioAlvo_px;
  float forcaVento;
  int8_t direcaoVento; // -1 para esquerda e 1 para direita
  float tempoEntrega_s;
  uint8_t teveSucesso;
} Tentativa;
Tentativa *tentativas;
typedef struct
{
  float velocidade;
  uint16_t xAlvo_px;
  uint16_t raioAlvo_px;
  float forcaVento;
  int8_t direcaoVento;
  float massa;
  uint8_t forma;
  float altura;
  float dimensaoCarga_m;
  uint16_t tamanhoAlvo_px;
} Configuracoes;
Configuracoes *configs;
typedef struct
{
  uint16_t x_px;
  uint16_t y_px;
  float x_m;
} Aviao;
typedef struct
{
  uint16_t x_px;
  uint16_t y_px;
  float x_m;
  float y_m;
  uint8_t forma;
  uint16_t dimensao_px;
} Carga;
typedef struct
{
  float posLancamento_m;
  uint16_t posLancamento_px;
  uint16_t alturaAviao_px;
  float dragConstant;
  uint16_t dimensaoCarga_px;
  float posAlvo_m;
  uint16_t posAlvo_px;
  float tempoQueda_s;
  float deslocamentoHorizontal_m;
  float tempoEntrega_s;
} InfoLancamento;
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
void DrawPlane(const Aviao *aviao, uint16_t color);
uint16_t ConvertHeightToScreenY(float height);
uint8_t AnyButtonPressed();
uint8_t PositionIsInDisplay(uint16_t x, uint16_t y);
float CalcSupplyHorPos(float mass, float dragConstant, float velocity, float windW, float time);
float CalcDragConstant(uint8_t shape, float length);
void ExecutarLancamento(const Configuracoes *config, int tentativaAtual);
void RegistrarTentativa(uint8_t tentativaAtual, const InfoLancamento *info, const Configuracoes *config, const Carga *carga);
uint16_t AumentarTamanhoCarga(float dimensaoCarga_m);
uint8_t LancamentoValido(const Configuracoes *config);
void DesenharFrame(const Aviao *aviao, const Carga *carga, uint16_t cor, uint8_t cargaFoiLancada);
void AtualizarPosicaoAviao(Aviao *aviao, float tempo_s, const Configuracoes *config);
uint8_t VerificarFimDeSimulacao(uint16_t posCarga_px, uint16_t posAlvo_px);
void AtualizarPosicaoCarga(Carga *carga, const Configuracoes *config, const InfoLancamento *info, float tempo_s, float instanteLancamento_s);
void DesenharCarga(const Carga *carga, uint16_t color);
void InicializarSimulacao(Configuracoes *config);
void ExibirResumoFinal(Tentativa *tentativas, uint8_t numeroTentativas);
void PrepararLancamento(const Configuracoes *config, InfoLancamento *info, Carga *carga, Aviao *aviao);
void ConfigurarParametros(Configuracoes *config, int indice);
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
    uint8_t numeroTentativas = 5;
    tentativas = malloc(numeroTentativas * sizeof(Tentativa));
    configs = malloc(numeroTentativas * sizeof(Configuracoes));
    ST7789_Fill_Color(BLACK);
    ST7789_WriteString(20, 50, "Aperte para iniciar", Font_11x18, WHITE, BLACK);
    while (!AnyButtonPressed())
    {
    }
    ST7789_Fill_Color(BLACK);
    for (int i = 0; i < numeroTentativas; i++)
    {
      InicializarSimulacao(&configs[i]);
      ConfigurarParametros(&configs[i], i);
    }
    for (int tentativaAtual = 0; tentativaAtual < numeroTentativas; tentativaAtual++)
    {
      ExecutarLancamento(&configs[tentativaAtual], tentativaAtual);
    }
    ExibirResumoFinal(tentativas, numeroTentativas);
    free(tentativas);
    free(configs);
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
  HAL_GPIO_WritePin(GPIOB, ST7789_DC_Pin | ST7789_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : ST7789_CS_Pin */
  GPIO_InitStruct.Pin = ST7789_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ST7789_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ST7789_DC_Pin ST7789_RST_Pin */
  GPIO_InitStruct.Pin = ST7789_DC_Pin | ST7789_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA9 PA10 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
float CalcHorPos(float speed, float time)
{
  return time * speed;
}
float CalcVerPos(float initialPos, float time)
{
  return initialPos - GRAVITY / 2 * time * time;
}
float CalcDragConstant(uint8_t shape, float length)
{
  float dragCoefficient = (shape == SQUARE ? CUBE_DC : SPHERE_DC);
  float crossSectionArea = (shape == SQUARE ? length * length : PI * length * length);
  return 0.5 * AIR_DENSITY * dragCoefficient * crossSectionArea;
}
float CalcSupplyHorPos(float mass, float dragConstant, float velocity, float windW, float time)
{
  float u0 = velocity - windW;
  if (u0 == 0.0f) return windW * time;
  float a  = (dragConstant * fabsf(u0)) / mass;
  float sgn = (u0 > 0.0f) ? 1.0f : -1.0f;
  return windW * time + (mass / dragConstant) * sgn * logf(1.0f + a * time);
}
// Conversões feitas considerando um avião de 70m exibido com 15px
uint16_t MetersToPx(float pos)
{
  return pos * 15 / 70;
}
float PxToMeters(float px)
{
  return px * 70 / 15;
}
void DrawPlane(const Aviao *aviao, uint16_t color)
{
  ST7789_DrawFilledRectangle(aviao->x_px, aviao->y_px, 13, 3, color);
  ST7789_DrawFilledTriangle(aviao->x_px, aviao->y_px + 3, aviao->x_px, aviao->y_px + 4, aviao->x_px + 1, aviao->y_px + 3, color);
  ST7789_DrawFilledTriangle(aviao->x_px + 13, aviao->y_px, aviao->x_px + 13, aviao->y_px + 1, aviao->x_px + 15, aviao->y_px, color);
}
uint16_t ConvertHeightToScreenY(float height)
{
  return HEIGHT - MetersToPx(height);
}
uint8_t AnyButtonPressed()
{
  return !(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12));
}
uint8_t PositionIsInDisplay(uint16_t x, uint16_t y)
{
  return x >= 0 && x <= WIDTH && y >= 0 && y <= HEIGHT;
}
uint8_t AvaliarTentativa(uint16_t finalSupplyX, uint16_t finalSupplyY, uint16_t goalX, uint16_t goalY, float goalLength)
{
  return finalSupplyX >= goalX && finalSupplyX <= goalX + goalLength;
}
void ExecutarLancamento(const Configuracoes *config, int tentativaAtual)
{
  ST7789_Fill_Color(BLACK);

  Aviao aviao;
  Carga carga;
  carga.forma = config->forma;
  InfoLancamento info;

  // Calcule e define todas as condições iniciais
  PrepararLancamento(config, &info, &carga, &aviao);

  uint32_t inicioSimulacao = HAL_GetTick();
  float instanteLancamento_s;

  uint8_t cargaFoiLancada = 0;

  // Desenha o alvo
  ST7789_DrawFilledRectangle(info.posAlvo_px, HEIGHT - 2, config->tamanhoAlvo_px, 2, RED);

  float tempo_s;
  while (!VerificarFimDeSimulacao(carga.x_px, info.posAlvo_px))
  {
    tempo_s = (HAL_GetTick() - inicioSimulacao) / (float)1000;
    DesenharFrame(&aviao, &carga, BLACK, cargaFoiLancada);
    AtualizarPosicaoAviao(&aviao, tempo_s, config);

    if (aviao.x_px >= info.posLancamento_px && !cargaFoiLancada)
    {
      cargaFoiLancada = 1;
      instanteLancamento_s = tempo_s;
      AtualizarPosicaoCarga(&carga, config, &info, tempo_s, instanteLancamento_s);
    }
    else if (aviao.x_px >= info.posLancamento_px)
    {
      AtualizarPosicaoCarga(&carga, config, &info, tempo_s, instanteLancamento_s);
    }

    DesenharFrame(&aviao, &carga, WHITE, cargaFoiLancada);
  }
  info.tempoEntrega_s = tempo_s;

  RegistrarTentativa(tentativaAtual, &info, config, &carga);
}
void PrepararLancamento(const Configuracoes *config, InfoLancamento *info, Carga *carga, Aviao *aviao)
{
  info->tempoQueda_s = config->altura * 2 / GRAVITY;
  info->tempoQueda_s = sqrt(info->tempoQueda_s);
  info->deslocamentoHorizontal_m = CalcSupplyHorPos(config->massa, CalcDragConstant(config->forma, config->dimensaoCarga_m), config->velocidade, config->forcaVento * config->direcaoVento, info->tempoQueda_s);
  info->posAlvo_px = rand() % (WIDTH / 5) + 0.7 * WIDTH;
  info->posAlvo_m = PxToMeters(info->posAlvo_px);
  info->posLancamento_m = info->posAlvo_m - info->deslocamentoHorizontal_m;
  info->alturaAviao_px = ConvertHeightToScreenY(config->altura);
  info->posLancamento_px = MetersToPx(info->posLancamento_m);
  info->dimensaoCarga_px = AumentarTamanhoCarga(config->dimensaoCarga_m);
  info->dragConstant = CalcDragConstant(config->forma, config->dimensaoCarga_m);
  carga->y_m = config->altura;
  carga->dimensao_px = info->dimensaoCarga_px;
  carga->x_px = 0;
  aviao->x_m = 0;
  aviao->y_px = info->alturaAviao_px;
}
void DesenharFrame(const Aviao *aviao, const Carga *carga, uint16_t cor, uint8_t cargaFoiLancada)
{
  DrawPlane(aviao, cor);
  if (cargaFoiLancada)
    DesenharCarga(carga, cor);
}
void AtualizarPosicaoAviao(Aviao *aviao, float tempo_s, const Configuracoes *config)
{
  aviao->x_m = CalcHorPos(config->velocidade, tempo_s);
  aviao->x_px = MetersToPx(aviao->x_m);
}
uint8_t VerificarFimDeSimulacao(uint16_t posCarga_px, uint16_t posAlvo_px)
{
  return posCarga_px >= posAlvo_px;
}
void AtualizarPosicaoCarga(Carga *carga, const Configuracoes *config, const InfoLancamento *info, float tempo_s, float instanteLancamento_s)
{
  float posCarga_m = info->posLancamento_m + CalcSupplyHorPos(config->massa, info->dragConstant, config->velocidade, config->forcaVento * config->direcaoVento, tempo_s - instanteLancamento_s);
  carga->x_px = MetersToPx(posCarga_m);
  float altCarga_m = CalcVerPos(config->altura, tempo_s - instanteLancamento_s);
  carga->y_px = ConvertHeightToScreenY(altCarga_m);
}
void DesenharCarga(const Carga *carga, uint16_t color)
{
  switch (carga->forma)
  {
  case CIRCLE:
    ST7789_DrawFilledCircle(carga->x_px, carga->y_px, carga->dimensao_px/2, color);
    break;
  case SQUARE:
    ST7789_DrawFilledRectangle(carga->x_px, carga->y_px, carga->dimensao_px, carga->dimensao_px, color);
    break;
  }
}
void RegistrarTentativa(uint8_t tentativaAtual, const InfoLancamento *info, const Configuracoes *config, const Carga *carga)
{
  Tentativa tentativa;
  tentativa.xLancamento_px = info->posLancamento_px;
  tentativa.yLancamento_px = info->alturaAviao_px;
  tentativa.xAlvo_px = info->posAlvo_px;
  tentativa.raioAlvo_px = config->tamanhoAlvo_px;
  tentativa.forcaVento = config->forcaVento;
  tentativa.direcaoVento = config->direcaoVento;
  tentativa.tempoEntrega_s = info->tempoEntrega_s;
  tentativa.teveSucesso = AvaliarTentativa(carga->x_px, carga->y_px, info->posAlvo_px, 0, config->tamanhoAlvo_px);
  tentativas[tentativaAtual] = tentativa;
}
void InicializarSimulacao(Configuracoes *config)
{
  do
  {
    config->altura = rand() % (1000 + 1 - 500) + 500;
    config->direcaoVento = (rand() % 2) ? -1 : 1;
    config->forcaVento = rand() % (10 + 1 + 10) - 10;
    config->velocidade = rand() % (90 + 1 - 30) + 30;
    config->massa = rand() % (5 + 1 - 1) + 1;
    config->forma = rand() % 2;
    config->dimensaoCarga_m = (float)(rand() % (5 + 1 - 2) + 2) / 10;
    config->tamanhoAlvo_px = rand() % (7 + 1 - 4) + 4;
    config->xAlvo_px = rand() % (WIDTH / 5) + 0.7 * WIDTH;
  } while (!LancamentoValido(config));
}
void ConfigurarParametros(Configuracoes *config, int indice)
{
  uint8_t parametrosMudaram = 1;

  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11))
  {
    if (parametrosMudaram)
    {
      ST7789_Fill_Color(BLACK);
      char buffer[25];
      sprintf(buffer, "Parametros %d", indice + 1);
      ST7789_WriteString(0, 0, buffer, Font_16x26, WHITE, BLACK);
      sprintf(buffer, "Velocidade %d", (int)config->velocidade);
      ST7789_WriteString(0, 32, buffer, Font_11x18, WHITE, BLACK);
      sprintf(buffer, "Forca Vento %d", (int)config->forcaVento);
      ST7789_WriteString(0, 50, buffer, Font_11x18, WHITE, BLACK);
      sprintf(buffer, "Direcao Vento %d", config->direcaoVento);
      ST7789_WriteString(0, 68, buffer, Font_11x18, WHITE, BLACK);
      sprintf(buffer, (config->forma == CIRCLE) ? "Forma Esfera" : "Forma Cubo");
      ST7789_WriteString(0, 86, buffer, Font_11x18, WHITE, BLACK);
      sprintf(buffer, "Massa %d", (int)config->massa);
      ST7789_WriteString(0, 122, buffer, Font_11x18, WHITE, BLACK);
      sprintf(buffer, "Altura %d", (int)config->altura);
      ST7789_WriteString(0, 140, buffer, Font_11x18, WHITE, BLACK);
      parametrosMudaram = 0;
    }
    if (!(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12))){
      InicializarSimulacao(config);
      parametrosMudaram = 1;
    }
      
  }
  ST7789_Fill_Color(BLACK);
}
void ExibirResumoFinal(Tentativa *tentativas, uint8_t numeroTentativas)
{
  ST7789_Fill_Color(BLACK);
  uint8_t numeroAcertos = 0;
  int tempoMedio = 0;
  for (int i = 0; i < numeroTentativas; i++)
  {
    if (tentativas[i].teveSucesso)
      numeroAcertos++;
    tempoMedio += tentativas[i].tempoEntrega_s;
  }
  tempoMedio /= numeroTentativas;
  char buffer[25];
  ST7789_WriteString(0, 0, "Resumo", Font_16x26, WHITE, BLACK);
  sprintf(buffer, "Acertos %d de %d", numeroAcertos, numeroTentativas);
  ST7789_WriteString(0, 27, buffer, Font_11x18, WHITE, BLACK);
  sprintf(buffer, "Tempo medio %d", tempoMedio);
  ST7789_WriteString(0, 53, buffer, Font_11x18, WHITE, BLACK);
  HAL_Delay(2000);
  ST7789_Fill_Color(BLACK);
}
uint8_t LancamentoValido(const Configuracoes *config)
{
  float tempoQueda_s = sqrt(2 * config->altura / GRAVITY);
  float deltaS = CalcSupplyHorPos(config->massa, CalcDragConstant(config->forma, config->dimensaoCarga_m), config->velocidade, config->forcaVento * config->direcaoVento, tempoQueda_s);
  return deltaS >= 0 && deltaS <= PxToMeters(WIDTH);
}
uint16_t AumentarTamanhoCarga(float dimensaoCarga_m)
{
  if (dimensaoCarga_m >= 0.3)
    return 3;
  return 2;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
