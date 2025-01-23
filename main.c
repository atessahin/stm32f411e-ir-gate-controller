#include "stm32f4xx_hal.h"

// Timer and ADC handles
TIM_HandleTypeDef htim2;
ADC_HandleTypeDef hadc1;
uint16_t adcValue;
uint16_t logicValue;

// Infinite loop for error handling
void Error_Handler(void)
{
    while(1)
    {
    }
}
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);


    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }


    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
    {
        Error_Handler();
    }
}

// GPIO Configuration
// PA1: Timer2 Channel2 (PWM output)
// PA2: ADC1 Channel2 (Analog input)
void GPIO_CONFIG() {
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStructA;
    GPIO_InitStructA.Pin = GPIO_PIN_1;
    GPIO_InitStructA.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStructA.Alternate = GPIO_AF1_TIM2;
    GPIO_InitStructA.Speed = GPIO_SPEED_MEDIUM;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructA);
	
		GPIO_InitTypeDef gpioStrucutA={0};
		gpioStrucutA.Mode=GPIO_MODE_ANALOG;
		gpioStrucutA.Pull=GPIO_NOPULL;
		gpioStrucutA.Pin=GPIO_PIN_2;
		HAL_GPIO_Init(GPIOA,&gpioStrucutA);
}

// ADC1 Configuration
// Single conversion mode
// 12-bit resolution
// Channel 2 on PA2
void hal_adc_config()
{
		__HAL_RCC_ADC1_CLK_ENABLE();
		hadc1.Instance=ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
		hadc1.Init.Resolution=ADC_RESOLUTION_12B;
		hadc1.Init.ScanConvMode=DISABLE;
		hadc1.Init.ContinuousConvMode=DISABLE;
	  hadc1.Init.DiscontinuousConvMode = DISABLE; 
		hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
		hadc1.Init.DataAlign=ADC_DATAALIGN_RIGHT;
		hadc1.Init.NbrOfConversion = 1;
		hadc1.Init.DMAContinuousRequests = DISABLE;
	
		HAL_ADC_Init(&hadc1);
	  ADC_ChannelConfTypeDef chanlefConfig = {0};
		chanlefConfig.Channel=ADC_CHANNEL_2;		
		chanlefConfig.Rank=1;
		chanlefConfig.SamplingTime=ADC_SAMPLETIME_56CYCLES;
		
		HAL_ADC_ConfigChannel(&hadc1,&chanlefConfig);

}

// Function to read ADC value
// Returns: 12-bit ADC conversion result
uint16_t readADC()
{
	HAL_ADC_Start(&hadc1);
	
	HAL_ADC_PollForConversion(&hadc1,HAL_MAX_DELAY);

	uint16_t getAdcVal = HAL_ADC_GetValue(&hadc1);
	
	HAL_ADC_Stop(&hadc1);
	
	return getAdcVal;

}

// Timer2 Configuration
// PWM mode on Channel 2
// Period: 20ms (50Hz)
// Initial duty cycle: 1.5ms
void MX_TIM2_Init(void)
{
	  __HAL_RCC_TIM2_CLK_ENABLE();
    TIM_OC_InitTypeDef sConfigOC = {0};

    /* Timer base configuration */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 47;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 19999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
		HAL_TIM_PWM_Init(&htim2); 
		
		//TIM PWM CONFIG
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 1500;  // Initially off
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);
}

// Convert ADC value to logic level
// Returns: 1 if ADC value < 570, 0 otherwise
uint16_t toLogicSegment(uint16_t val)
{
		if(val<570)
			return 1;
		else 
			return 0;

}

// Main function
// Continuously reads ADC and controls PWM based on input
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();
		
    /* Initialize all configured peripherals */
    GPIO_CONFIG();
		hal_adc_config();
    MX_TIM2_Init();


    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

    // Main loop
    // If ADC reads low (logic 1): PWM = 0.5ms
    // If ADC reads high (logic 0): PWM = 1.5ms
    while (1)
    {
			
			adcValue = readADC();
			logicValue=toLogicSegment(adcValue);
			
			if(logicValue==1)
			{
				__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
				HAL_Delay(500);
			}
			else
			{
				 __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1500);
					HAL_Delay(500);
			}
    }
}

// SysTick interrupt handler
// Required for HAL delay functions
void SysTick_Handler(void) {
    HAL_IncTick();
}