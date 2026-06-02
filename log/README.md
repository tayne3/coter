# log

异步批量日志系统。线程本地缓冲（TLS）+ 后台调度线程 + 多态 Handler。

## 架构

```sh
用户宏 (CT_LOGGER_BRIEF_* / CT_LOGGER_DETAIL_* / ...)
       │
       ▼
  ct_log_tls_output()         公开 API 入口
       │
       ▼
  TLS Cache (per-thread)      线程本地 block 缓冲，满或超时后提交
       │
       ▼
  Dispatcher (后台线程)        从队列取 block，批量分发给 handler
       │
       ▼
  Handler (vtable)             console / file / callback
```

**数据流**：用户日志先写入线程本地的 block（零拷贝），block 达到容量阈值（7/8）或驻留超过 100ms 后提交给 Dispatcher。Dispatcher 在后台线程中串行读取 block，将记录批量分发给所有注册的 Handler。后台线程每秒执行一次心跳，主动回收各线程积压的 TLS block。

## 快速开始

### 最简用法

```c
#include "coter/log/log.h"

int main(void) {
    ct_log_init();

    CT_LOGGER_BRIEF_DEBUG(CT_DEFAULT_LOGGER, "Hello %s", "world");

    ct_log_close();
    return 0;
}
```

`ct_log_init()` 创建一个内置的 stdout logger，可直接通过 `CT_DEFAULT_LOGGER` 使用。

### 自定义 Logger

```c
ct_logger_t my_logger;
ct_logger_init(&my_logger);

ct_log_console_handler_config_t config;
ct_log_console_handler_config_default(&config);
ct_log_handler_t* handler = ct_log_console_handler_create(&config);

ct_logger_add_handler(&my_logger, handler);
ct_logger_register(&my_logger);

CT_LOGGER_BRIEF_DEBUG(&my_logger, "custom logger");
```

一个 Logger 可挂载多个 Handler，日志会同时写入所有已注册的 Handler。

### 文件日志

```c
ct_log_file_handler_config_t config;
ct_log_file_handler_config_default(&config);
strncpy(config.dir,  "./logs", sizeof(config.dir)  - 1);
strncpy(config.name, "app",    sizeof(config.name) - 1);

ct_log_handler_t* handler = ct_log_file_handler_create(&config);
ct_logger_add_handler(&my_logger, handler);
```

文件轮转规则：`{dir}/{name}.log0` → `{name}.log1` → … → `{name}.logN`，循环覆写。默认上限 3 个文件，每个最大 4MB。

### 回调日志

```c
void my_callback(const ct_log_record_t* record, void* userdata) {
    fwrite(record->data, 1, record->size, (FILE*)userdata);
}

ct_log_callback_handler_config_t config;
ct_log_callback_handler_config_default(&config);
config.routine = my_callback;
config.userdata = stderr;

ct_log_handler_t* handler = ct_log_callback_handler_create(&config);
```

## 日志级别

| 级别 | 值 | 缩写 |
| ------ | --- | --- |
| VERBOSE | 0 | VER |
| DEBUG | 1 | DBG |
| TRACE | 2 | TRC |
| WARNING | 3 | WRN |
| ERROR | 4 | ERR |
| FATAL | 5 | FTL |

通过 `ct_logger_set_level()` 设置最低输出级别，低于该级别的日志被丢弃。

## 日志样式

| 样式 | 宏前缀 | 输出格式 |
| ------ | --- | --- |
| BASIC | `CT_LOGGER_BASIC_*` | 纯消息，无前缀 |
| BRIEF | `CT_LOGGER_BRIEF_*` | `时间 线程ID 级别 消息` |
| DETAIL | `CT_LOGGER_DETAIL_*` | `时间 线程ID 级别 文件:行号 > 消息` |

每种样式对应六个级别的便捷宏：`CT_LOGGER_BRIEF_VERBOSE`、`CT_LOGGER_BRIEF_DEBUG`、……、`CT_LOGGER_BRIEF_FATAL`。

另有 `CT_LOGGER_HEX_*` 系列宏用于十六进制 dump。

## API 参考

### 系统级

```c
int  ct_log_init(void);                        // 初始化日志系统
void ct_log_close(void);                       // 关闭并排空所有缓冲
void ct_log_flush(void);                       // 强制刷新所有 pending 的日志
ct_logger_t* ct_log_get_default(void);         // 获取默认 logger
void ct_log_set_default(ct_logger_t* logger);  // 设置默认 logger
```

### Logger

```c
void ct_logger_init(ct_logger_t* logger);
int  ct_logger_register(ct_logger_t* logger);    // 注册到全局管理
void ct_logger_unregister(ct_logger_t* logger);  // 从全局管理注销
void ct_logger_close(ct_logger_t* logger);       // 注销 + 销毁 handler

void ct_logger_set_level(ct_logger_t* logger, int level);
int  ct_logger_get_level(const ct_logger_t* logger);
int  ct_logger_add_handler(ct_logger_t* logger, ct_log_handler_t* handler);

bool ct_logger_is_enabled(const ct_logger_t* logger, int level);
void ct_logger_handle(ct_logger_t* logger, int level, const char* buf, size_t size);
```

### Handler

```c
// 控制台
void ct_log_console_handler_config_default(ct_log_console_handler_config_t* config);
ct_log_handler_t* ct_log_console_handler_create(const ct_log_console_handler_config_t* config);

// 文件
void ct_log_file_handler_config_default(ct_log_file_handler_config_t* config);
ct_log_handler_t* ct_log_file_handler_create(const ct_log_file_handler_config_t* config);

// 回调
void ct_log_callback_handler_config_default(ct_log_callback_handler_config_t* config);
ct_log_handler_t* ct_log_callback_handler_create(const ct_log_callback_handler_config_t* config);
```

## 线程安全

| 操作 | 线程安全性 | 说明 |
| --- | --- | --- |
| `CT_LOG_*` 宏（写入） | 无锁 | TLS 缓冲，每线程独立 |
| `ct_logger_set_level` | 安全 | atomic 操作 |
| `ct_logger_register` / `unregister` | 安全 | mutex 保护 |
| `ct_log_close` | 安全 | 同步屏障排空所有 TLS 缓冲后才销毁 |
| Handler `write_batch` | 串行调用 | Dispatcher 单线程分发，handler 内部按需自行加锁保护 I/O |

## 内部架构（面向贡献者）

### 文件职责

```sh
log/
├── include/coter/log/
│   ├── log.h          入口头文件，宏 API + 系统级函数
│   ├── logger.h       Logger 结构与操作
│   ├── handler.h      Handler 接口（vtable）+ 各类型 config/create
│   └── tls.h          TLS 输出函数（公开，仅两个）
└── src/
    ├── logger.c            全局管理器 + Logger 生命周期
    ├── dispatcher.c        后台线程 + 消息队列 + block pool
    ├── tls.c               线程本地缓存 + 时间/格式化
    ├── handler_console.c   控制台 handler
    ├── handler_file.c      文件 handler（依赖 rotator）
    ├── handler_callback.c  回调 handler
    ├── rotator.c           文件轮转器（内部组件）
    ├── rotator.h           轮转器接口
    └── log_internal.h      内部共享定义（block、dispatcher、TLS 内部接口）
```

### block 生命周期

1. TLS 层通过 block__pool_acquire() 从 pool 获取 block
2. 用户日志以 record header + payload 的格式追加到 block.data[]
3. block 达到阈值后提交给 Dispatcher 的消息队列
4. Dispatcher 从 block 中解析出记录数组，批量调用 handler->write_batch()
5. 处理完毕后 block 通过 block__pool_release() 归还 pool（或释放）
