#include "lwip/sockets.h"

#define TCP_PORT 8888
#define TEMP_BUF_SIZE 256

void tcp_server_task(void *arg) {
  int listen_fd = -1, client_fd = -1;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len = sizeof(client_addr);
  uint8_t temp_buf[TEMP_BUF_SIZE];
  int len;

  /* 1. 创建并绑定 Socket (标准流程) */
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    printf("Socket create failed\n");
    vTaskDelete(NULL);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(TCP_PORT);
  server_addr.sin_addr.s_addr = IPADDR_ANY;

  if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    printf("Bind failed\n");
    close(listen_fd);
    vTaskDelete(NULL);
  }

  listen(listen_fd, 1);
  printf("DDC Transparent Server Listening on %d...\n", TCP_PORT);

  while (1) {
    /* 2. 等待连接 (阻塞) */
    printf("Waiting for PC Client...\n");
    client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_len);

    if (client_fd >= 0) {
      printf("PC Connected: %s\n", inet_ntoa(client_addr.sin_addr));

      /* 关键：设置 Socket 为非阻塞模式 */
      /* 这样 recv 不会卡死，我们才能在同一个循环里处理串口数据 */
      int flags = fcntl(client_fd, F_GETFL, 0);
      fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

      /* 可选：禁用 Nagle 算法，提高 AT 指令响应速度 */
      int flag = 1;
      setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&flag,
                 sizeof(flag));

      /* 3. 进入透传循环 */
      while (1) {
        /* -------------------------------------------
           方向 A: TCP -> UART (PC 发命令给模块)
           ------------------------------------------- */
        len = recv(client_fd, temp_buf, TEMP_BUF_SIZE, 0);

        if (len > 0) {
          /* 收到数据，直接发给串口 */
          HAL_UART_Transmit(&huart2, temp_buf, len, 100);
          // 最好用 DMA 发送防止阻塞: HAL_UART_Transmit_DMA(&huart1, temp_buf,
          // len); 如果用 DMA 发送，需要注意忙状态判断
        } else if (len == 0) {
          /* PC 断开了连接 */
          printf("PC Disconnected\n");
          close(client_fd);
          break; /* 跳出循环，回到 accept */
        } else {
          /* len < 0: 只要错误码是 EWOULDBLOCK，说明没数据，正常 */
          if (errno != EWOULDBLOCK && errno != EAGAIN) {
            printf("Recv Error: %d\n", errno);
            close(client_fd);
            break;
          }
        }

        /* -------------------------------------------
           方向 B: UART -> TCP (模块回复给 PC)
           ------------------------------------------- */
        /* 检查串口缓冲区有没有数据 */
        uint16_t uart_len = UART_Get_Data_Count();
        if (uart_len > 0) {
          /* 从 RingBuffer 取出数据 */
          // 注意：这里复用了 temp_buf，确保单次读取不越界
          int read_len = UART_Read_Data(temp_buf, TEMP_BUF_SIZE);

          /* 发送给 PC */
          int sent = send(client_fd, temp_buf, read_len, 0);
          if (sent < 0) {
            printf("Send Error\n");
            close(client_fd);
            break;
          }
        }

        /* -------------------------------------------
           CPU 释放 (关键)
           ------------------------------------------- */
        /* 因为是非阻塞轮询，必须加延时，否则 IDLE 任务无法运行，看门狗会叫 */
        /* 2ms - 5ms 是比较合适的 AT 指令交互延时 */
        vTaskDelay(pdMS_TO_TICKS(5));
      }
    } else {
      /* accept 失败 (可能是内存不够等)，稍微延时重试 */
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
}