#include "App.h"
#include "Xslot/communication.h"
void start_task(void);
TaskHandle_t LED_Handler;

timer_parameter_struct initpara;
struct ibl_api_t *p_ibl_api = (struct ibl_api_t *)FLASH_API_ARRAY_BASE;

HANDLE m_RS4850;
BYTE InBuf[50];
BYTE OutBuf[50];
EKFILE fileComTest = {{InBuf, 0, 0, 50}, {OutBuf, 0, 0, 50}, NULL};

void ComTestInit(void) {
  UART_CONFIG sCfg;
  sCfg.nBaud = 19200;
  sCfg.nControl = HALFDUPLEX | EIGHTBIT | PARITY_NONE;

  m_RS4850 = EKfopen("RS4851", &fileComTest, &sCfg);
}

void ComTestObj(void) {
  s16_t ch;
  BYTE j;

  EKfOnIdle(m_RS4850);
  ch = EKfgetc(m_RS4850);
  if (ch >= 0) {
    j = ch & 0xFF;
    EKfputc(m_RS4850, j);
  }
}

/************************************************************************************************/
/* All Objects Initialization */
/************************************************************************************************/
void ObjectInit(void) {
  DigitalInputInit();
  DigitalOutputInit();
  AnalogueInputInit();
  AnalogueOutputInit();
  UniversalIOInit();

#ifdef MODBUSAPPLICATION
  ModbusServerInit();
#endif

  // ComTestInit();  //COM port Test, unmask for testing
  SerialPortInit();
  PulseAccumInit();
}

extern void EnetInit(u8_t bUseInterrupt);
void main_task(void) {
  while (1) {
    TimerObj();
    // LEDObj();
    WatchDogToggle();
    SerialPortObj();
    DigitalInputObj();
#ifndef DEBUG_PULSEACC
    DigitalOutputObj();
    AnalogueOutputObj();
#endif
    AnalogueInputObj();
    WatchDogToggle();
    SerialPortObj();
    UniversalIOObj();
    PulseAccumObj();
#ifdef DEBUG_PULSEACC // used DO to test PulseAccum, undefine after testing
    if (dev_svc_gpio_get()) {
      if (dev_svc_gpio_get()) {
        if (dev_svc_gpio_get()) {
          dev_uo_type_set(2, 2);
          timer_interrupt_enable(TIMER4, TIMER_INT_UP);
        }
      }
    }
#else
    dev_svc_Obj();
    SerialPortObj();
    if (dev_svc_get_pressDuration() >= 5) {
      ConfigState |= CONFIGSTATE_RESETREGSALL;
    }
#endif
    RTCObj();
    ConfigObj();
    ModbusUSBSerialObj();

    while (enet_rxframe_size_get() > 1) {
      lwip_frame_recv();
      WatchDogToggle();
    }

    WatchDogToggle();
    CommMonObj();
    SerialPortObj();

    // ComTestObj();  //COM port Test, unmask for testing
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

extern void test_enet_task(void *pvParameters);

HANDLE m_UART6;
s32_t test_num = 0;
u8_t test_tx_buffer[50] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
u8_t test_rx_buffer[50];
EKFILE fileUartTest = {
    {test_rx_buffer, 0, 0, 50}, {test_tx_buffer, 0, 0, 50}, NULL};
extern const tUARTInfo uartInfo[DEV_UART_MAX];

void TestOnRx(u8_t bStatus, u8_t ch) {

  if (bStatus == UARTCBSTATUS_RECEIVE) {
    test_rx_buffer[test_num] = ch;
    test_num++;
    usart_interrupt_enable(UART6, USART_INT_IDLE);
  }
  if (bStatus == 0xf) {

    EKfwrite(m_UART6, (LPSTR)test_rx_buffer, test_num, &test_num, 1000);
    usart_interrupt_disable(UART6, USART_INT_IDLE);
    test_num = 0;
  }
}

void uartTestTask(void *pvParameters) {
  (void)(pvParameters);
  UART_CONFIG sCfg;
  // s32_t index = 9;
  sCfg.nBaud = 19200;
  sCfg.nControl = HALFDUPLEX | EIGHTBIT | PARITY_NONE;

  m_UART6 = EKfopen("UART2", &fileUartTest, &sCfg);
  SetOnUARTSendCompleteCallBack(m_UART6, NULL);
  SetCommCallBack(m_UART6, (UARTCALLBACK *)TestOnRx);

  while (1) {
    //   EKfwrite(m_UART6, (LPSTR) test_tx_buffer, index, &index, 1000);
    //   vTaskDelay(10000);
  }
}


void test_spi_task(void *pvParameters) {
  (void)(pvParameters);
  uint8_t spi_send_array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  dev_ssi_init(1);

  while (1) {
    dev_ssi_ChipSelect(1, ENABLE, 0);

    for (int i = 0; i < 9; i++) {
      spi_i2s_data_transmit(SPI3, spi_send_array[i]);
      while (spi_i2s_flag_get(SPI3, SPI_FLAG_TBE) == RESET)
        ;
      while (spi_i2s_flag_get(SPI3, SPI_FLAG_TRANS) != RESET)
        ;

      while (RESET == spi_i2s_flag_get(SPI3, SPI_FLAG_RBNE))
        ;
      spi_i2s_data_receive(SPI3);
    }

    dev_ssi_ChipSelect(1, DISABLE, 0);

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void FuncBlock_Task(void *pvParameters) {
  while (1) {
    FunctionBlockExec();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/**
 * @brief MQTT 客户端任务
 * 在 FreeRTOS 调度器启动后运行，确保 tcpip 线程已就绪
 * 定时发布设备状态消息
 */
void mqtt_client_task(void *pvParameters) {
  (void)pvParameters;
  uint32_t msg_count = 0;
  char msg_buf[128];

  /* 等待一段时间确保网络初始化完成 */
  vTaskDelay(pdMS_TO_TICKS(3000));

  /* 初始化 MQTT 客户端 */
  mqtt_init();

  /* 等待连接建立 */
  vTaskDelay(pdMS_TO_TICKS(2000));

  for (;;) {
    /* 构造动态消息内容 */
    msg_count++;
    snprintf(msg_buf, sizeof(msg_buf),
             "{\"device\":\"XC8064\",\"count\":%lu,\"status\":\"online\"}",
             (unsigned long)msg_count);

    /* 发布到指定 topic */
    mqtt_publish_wrapper("DDC_TEST/status", msg_buf);

    /* 每 5 秒发布一次 */
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

#include "lwip/sys.h"
#include "lwiperf.h"

static void *lwiperf_session = NULL;
void iperfClientTask(void *pvParameters) // Todo Only Demo1 Testing purpose
{

  ip_addr_t server_ip;
  IP4_ADDR(&server_ip, 192, 168, 10, 100); // Replace with your PC IP
  EnetInit(true);
  lwiperf_session = lwiperf_start_tcp_client_default(&server_ip, NULL, NULL);
  while (1) {
  }

  // int sock = socket(AF_INET, SOCK_DGRAM, 0);
  // u16_t srlen;
  // struct sockaddr_in addr = {0};
  // addr.sin_family = AF_INET;
  // addr.sin_port   = PP_HTONS(5001);
  // addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
  // bind(sock, (struct sockaddr*)&addr, sizeof(addr));
  // srlen = sizeof(addr);
  // while(1) {
  //     char buf[1500];
  //     int len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&addr,
  //     (socklen_t*)&srlen);

  //     if (len > 0) {
  //         // optionally echo back
  //         sendto(sock, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
  //     }
  // }
}

void vApplicationMallocFailedHook(void) { LEDSet(LED_STATUS_UNINIT_PROG_LOST); }

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  LEDSet(LED_STATUS_UNINIT_PROG_LOST);
}

void start_app(void) {
  const struct flash_area *fap;
  // struct image_header *hdr = (struct image_header *)(RE_FLASH_BASE +
  // RE_IMG_0_APP_OFFSET);
  SCB->VTOR = APP_CODE_START;

  /* write image ok flag, then this image will always run */
  flash_area_open(FLASH_AREA_0_ID, &fap);
  boot_write_image_ok(fap);
}

int main(void) {
  WatchDogInit();
  WatchDogToggle();
  start_app();
  DISABLE_INTERRUPT();
  TimerInit();
  WatchDogToggle();
  LEDInit();
  LEDSet(LED_POWER_NORMAL);
  LEDSet(LED_STATUS_UNINIT_PROG_LOST);
  ENABLE_INTERRUPT();
  ConfigInit();
  CalibrationInit();
  UsbCDC_Connect();
  ModbusUSBSerialInit();
  DISABLE_INTERRUPT();
  ObjectInit();
  dev_svc_init();
  RTCInit();
  CommMonInit();
  WatchDogToggle();
  EnetInit(false);
  ModbusTCPInit();
  BacnetAppInit();
  LEDSet(LED_STATUS_NORMAL);

  FunctionBlockInit(true);
  if (isConfigEmpty()) {
    ConfigWriteAllSetting(TRUE);
  }
  ENABLE_INTERRUPT();

  xTaskCreate((TaskFunction_t)main_task, (const char *)"main_task", 0x400,
              (void *)NULL, (UBaseType_t)tskIDLE_PRIORITY + 3, &LED_Handler);
  // xTaskCreate(test_enet_task, "test_enet_task", 2048, NULL, tskIDLE_PRIORITY,
  // NULL);
  xTaskCreate(test_sdram_task, "test_sdram_task", 0x400, NULL, tskIDLE_PRIORITY,
              NULL);
  // xTaskCreate(test_w25q128_task, "test_w25q128_task", 0x400, NULL,
  // tskIDLE_PRIORITY, NULL); xTaskCreate(test_eepromemul_task,
  // "test_w25q128_task", 0x400, NULL, tskIDLE_PRIORITY, NULL);
  // xTaskCreate(iperfClientTask, "iperfClientTask", 0x1000, NULL,
  // tskIDLE_PRIORITY, NULL);
  xTaskCreate(ntp_task, "ntp_task", 0x100, NULL, tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(bacnet_ip_server_task, "bacnet_ip_server_task", 0x400, NULL,
              tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(bacnet_app_task, "bacnet_app_task", 0x800, NULL,
              tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(FuncBlock_Task, "FuncBlockApp", 0x400, NULL, tskIDLE_PRIORITY + 1,
              NULL);

  /* HTTP server (需要取消注释以启用) */
  http_server_netconn_init();

  /* MQTT 客户端任务 - 在调度器启动后执行初始化 */
  // xTaskCreate(mqtt_client_task, "mqtt_client", 0x800, NULL,
  //             tskIDLE_PRIORITY + 1, NULL);

  vTaskStartScheduler();
}