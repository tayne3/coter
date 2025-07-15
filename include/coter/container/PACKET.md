# Packet

## 简介

Packet 是一个高效、灵活的报文缓冲处理库，专为嵌入式系统和网络通信应用设计。它提供了一套简洁的 API，用于管理和操作二进制数据缓冲区，支持各种数据类型和字节序。

## 特性

- 支持多种数据类型：uint8_t, uint16_t, uint32_t, float
- 灵活的字节序处理：支持大端和小端
- 高效的内存管理：使用三段式结构，有效利用内存空间
- 丰富的操作接口：包括读取、写入、设置和获取等功能
- 安全性：内置边界检查，防止缓冲区溢出
- 跨平台：纯 C 实现，易于移植

## 使用说明

1. 包含头文件：

```c
#include "ct_packet.h"
```

2. 创建并初始化 Packet：

```c
uint8_t buffer[256];
ct_packet_t packet;
ct_packet_init(&packet, buffer, sizeof(buffer));
```

3. 写入数据：

```c
ct_packet_put_u8(&packet, 0x7E);
ct_packet_put_u16(&packet, 1234, CTEndian_Big);
ct_packet_put_float(&packet, 3.14f, CTEndian_Little);
```

4. 读取数据：

```c
uint8_t value8 = ct_packet_get_u8(&packet, 0);
uint16_t value16 = ct_packet_get_u16(&packet, 1, CTEndian_Big);
float valuef = ct_packet_get_float(&packet, 3, CTEndian_Little);
```

5. 重置或清空缓冲区：

```c
// 只重置计数
ct_packet_reset(&packet);
// 重置计数并将缓冲区清零
ct_packet_clean(&packet);
```

## 示例代码

以下是一个简单的示例，展示了如何使用 Packet Box 来封装和解析数据：

```c
#include "ct_packet.h"

void encode_packet(ct_packet_t packet)
{
	ct_packet_put_u8(&packet, 0x7E);                       // 起始标志
	ct_packet_put_u16(&packet, 0x0001, CTEndian_Big);      // 包类型
	ct_packet_put_u32(&packet, 12345678, CTEndian_Big);    // 数据
	ct_packet_put_float(&packet, 3.14159f, CTEndian_Big);  // 浮点数据
	ct_packet_put_u8(&packet, 0x7E);                       // 结束标志
}

void decode_packet(ct_packet_t packet)
{
	uint8_t  start = ct_packet_take_u8(&packet);
	uint16_t type  = ct_packet_take_u16(&packet, CTEndian_Big);
	uint32_t data  = ct_packet_take_u32(&packet, CTEndian_Big);
	float    fdata = ct_packet_take_float(&packet, CTEndian_Big);
	uint8_t  end   = ct_packet_take_u8(&packet);

	printf("Start: 0x%02X\n", start);
	printf("Type: 0x%04X\n", type);
	printf("Data: %u\n", data);
	printf("Float Data: %f\n", fdata);
	printf("End: 0x%02X\n", end);
}

int main()
{
	uint8_t     buffer[256];
	ct_packet_t packet;
	ct_packet_init(&packet, buffer, sizeof(buffer));

	encode_packet(&packet);
	ct_packet_reset(&packet);
	decode_packet(&packet);

	return 0;
}
```
