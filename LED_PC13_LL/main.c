/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : CẤU HÌNH STM32F103 LÊN 72MHZ CHUẨN LL TRÊN KEILC
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"        
#include "stm32f1xx_ll_bus.h"   
#include "stm32f1xx_ll_gpio.h"  
#include "stm32f1xx_ll_rcc.h"   
#include "stm32f1xx_ll_system.h"

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
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
// Đặt tiền tố Reg_ hoặc LL_ (như ý đồ Naming Convention) để định danh các hàm tự viết
void Reg_SystemClock_Config_72MHz(void);
void Reg_Delay_Spin(uint32_t count);
void Reg_GPIO_Init_Custom(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* 
 * Chú thích: Bạn có thể bỏ comment hàm này nếu muốn tước quyền can thiệp 
 * ngầm của file system_stm32f1xx.c, nhưng hãy cẩn thận khi sử dụng các 
 * trình biên dịch khác nhau. Tạm thời mình ẩn đi như code gốc của bạn.
 */
/*
void SystemInit(void) {
    // Để trống hoàn toàn để tước quyền can thiệp ngầm 
}
*/
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
  
  /* Configure the system clock */
  /* USER CODE BEGIN SysInit */
  // =========================================================================
  // BƯỚC 1: OVERRIDE SYSTEM CLOCK (Ghi đè xung nhịp hệ thống)
  // =========================================================================
  // Gọi cấu hình đưa chip lên tốc độ tối đa 72MHz bằng thạch anh thực tế
  Reg_SystemClock_Config_72MHz(); 
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  // =========================================================================
  // BƯỚC 2: KHỞI TẠO NGOẠI VI
  // =========================================================================
  Reg_GPIO_Init_Custom();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);
    
    // Vì chip đã lên 72MHz (nhanh gấp 9 lần mức 8MHz cũ), 
    // ta tăng số vòng lặp lên 4500000 để LED nháy chuẩn ~500ms
    Reg_Delay_Spin(900000); 
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */
// HÀM CẤU HÌNH THẠCH ANH NGOÀI 72MHZ CHUẨN LL (KHÔNG BỊ AI NGÁNG ĐƯỜNG)
void Reg_SystemClock_Config_72MHz(void)
{
    // 1. Bật thạch anh ngoài (HSE) và đợi nó ổn định
    LL_RCC_HSE_Enable();
    while(LL_RCC_HSE_IsReady() != 1);
    
    // 2. Cấu hình bộ nhân tần số PLL (Lấy HSE làm nguồn, nhân lên thành 72MHz)
    // Chip F103 dùng thạch anh 8MHz -> Chia 1 nhân 9 = 72MHz
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
    LL_RCC_PLL_Enable();
    while(LL_RCC_PLL_IsReady() != 1);
    
    // 3. Cấu hình bộ chia cho các đường Bus ngoại vi (Flash Latency, AHB, APB1, APB2)
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2); // Ép bộ nhớ Flash mở độ trễ tương thích 72MHz
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2); // Bus APB1 tối đa chỉ chịu được 36MHz
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1); // Bus APB2 chạy tối đa 72MHz
    
    // 4. Chuyển đổi xung nhịp tổng của hệ thống sang nguồn PLL vừa cấu hình
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);
}

void Reg_GPIO_Init_Custom(void)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_OUTPUT);             
    LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_13, LL_GPIO_OUTPUT_PUSHPULL);  
    LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_13, LL_GPIO_SPEED_FREQ_MEDIUM); 
}

void Reg_Delay_Spin(uint32_t count) 
{
    for (volatile uint32_t i = 0; i < count; i++) { __asm("NOP"); }
}
/* USER CODE END 4 */