# log

异步批量日志系统。线程本地缓冲（TLS）+ 后台调度线程 + 多态 Handler。

## 架构

```sh
  日志宏 (CT_* / CT_LOGGER_*)
       │
       ▼
  ct_log_submit_fmt() / ct_log_submit_payload()
       │
       ▼
  TLS Cache (per-thread)
       │
       ▼
  Dispatcher (后台线程)
       │
       ▼
  Handler (vtable): console / file / callback
```

日志运行时采用零初始化模型：第一次获取默认 logger、第一次启动自定义 logger、或第一次写入运行态 logger 时，内部运行时会按需懒加载。调用方不需要，也不能显式启停全局日志系统。

## 快速开始

### 默认 Logger

```c
#include "coter/log/log.h"

int main(void) {
    CT_DEBUG("Hello %s", "world");
    return 0;
}
```

默认 logger 由库维护，写入 `CT_*` 宏时自动创建。进程退出时会执行兜底排空。

### 自定义 Logger

```c
ct_logger_t logger;
ct_logger_init(&logger);

ct_log_console_handler_config_t config;
ct_log_console_handler_config_default(&config);
ct_log_handler_t* handler = ct_log_console_handler_create(&config);

ct_logger_add_handler(&logger, handler);
ct_logger_start(&logger);

CT_LOGGER_DEBUG(&logger, "custom logger");

ct_logger_close(&logger);
```

自定义 logger 的生命周期由调用方负责。`ct_logger_start()` 之后 handler 拓扑不可变，`ct_logger_close()` 会同步排空 pending block 后再销毁 handler。

### 文件日志

```c
ct_logger_t logger;
ct_logger_init(&logger);

ct_log_file_handler_config_t config;
ct_log_file_handler_config_default(&config);
strncpy(config.dir,  "./logs", sizeof(config.dir)  - 1);
strncpy(config.name, "app",    sizeof(config.name) - 1);

ct_logger_add_handler(&logger, ct_log_file_handler_create(&config));
ct_logger_start(&logger);

CT_LOGGER_DEBUG(&logger, "file logger");

ct_logger_close(&logger);
```

文件轮转规则：`{dir}/{name}.log0` -> `{name}.log1` -> ... -> `{name}.logN`，循环覆写。默认上限 3 个文件，每个最大 4MB。

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

### C++ 使用

```cpp
#include "coter/log/log.hpp"

int main() {
    auto logger = coter::log::default_logger();
    COTER_LOGGER_DEBUG(logger, "Hello {}!", "world");
    return 0;
}
```

## 日志级别

| 级别 | 值 | 缩写 |
| --- | --- | --- |
| VERBOSE | 0 | VER |
| DEBUG | 1 | DBG |
| TRACE | 2 | TRC |
| WARNING | 3 | WRN |
| ERROR | 4 | ERR |
| FATAL | 5 | FTL |

通过 `ct_logger_set_level()` 设置最低输出级别，低于该级别的日志被丢弃。

## 宏 API

### 带 Logger 参数

```c
CT_LOGGER_VERBOSE(logger, "message");
CT_LOGGER_DEBUG(logger, "message");
CT_LOGGER_TRACE(logger, "message");
CT_LOGGER_WARNING(logger, "message");
CT_LOGGER_ERROR(logger, "message");
CT_LOGGER_FATAL(logger, "message");
```

### 使用默认 Logger

```c
CT_VERBOSE("message");
CT_DEBUG("message");
CT_TRACE("message");
CT_WARNING("message");
CT_ERROR("message");
CT_FATAL("message");
```

## API 参考

### 运行时配置与统计

```c
void ct_logger_config_default(ct_logger_config_t* config);
int  ct_logger_set_global_config(const ct_logger_config_t* config);
void ct_logger_get_stats(ct_logger_stats_t* stats);
ct_logger_t* ct_logger_default(void);
```

`ct_logger_set_global_config()` 必须在日志运行时首次懒加载前调用。运行时创建后配置被冻结，后续调用返回 `-1`。

### Logger

```c
void ct_logger_init(ct_logger_t* logger);
int  ct_logger_start(ct_logger_t* logger);
void ct_logger_close(ct_logger_t* logger);

void ct_logger_set_level(ct_logger_t* logger, int level);
int  ct_logger_get_level(const ct_logger_t* logger);
int  ct_logger_add_handler(ct_logger_t* logger, ct_log_handler_t* handler);

bool ct_logger_is_enabled(const ct_logger_t* logger, int level);
```

### Handler

```c
void ct_log_console_handler_config_default(ct_log_console_handler_config_t* config);
ct_log_handler_t* ct_log_console_handler_create(const ct_log_console_handler_config_t* config);

void ct_log_file_handler_config_default(ct_log_file_handler_config_t* config);
ct_log_handler_t* ct_log_file_handler_create(const ct_log_file_handler_config_t* config);

void ct_log_callback_handler_config_default(ct_log_callback_handler_config_t* config);
ct_log_handler_t* ct_log_callback_handler_create(const ct_log_callback_handler_config_t* config);
```

## 线程安全

| 操作 | 线程安全性 | 说明 |
| --- | --- | --- |
| `CT_*` / `CT_LOGGER_*` 写入 | 安全 | TLS 缓冲，每线程独立 |
| `ct_logger_set_level` | 安全 | atomic 操作 |
| `ct_logger_add_handler` | 配置期安全 | 只能在 `ct_logger_start()` 前调用 |
| `ct_logger_close` | 安全 | 同步屏障排空 TLS 与 dispatcher 后销毁 handler |
| Handler `write_batch` | 串行调用 | Dispatcher 单线程分发 |
