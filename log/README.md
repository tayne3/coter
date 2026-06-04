# log

固定资源的异步日志系统。生产线程使用 TLS scratch buffer 完成格式化，然后把日志快照提交到固定容量队列，由单个 dispatcher 线程串行调用 Handler。

## 架构

```text
日志宏 (CT_* / CT_LOGGER_*)
     |
     v
TLS scratch buffer
     |
     v
Fixed async queue
     |
     v
Dispatcher thread
     |
     v
Handler: console / file / callback / base
```

日志运行时按需懒加载。调用方不需要显式初始化全局日志系统；默认 logger 首次使用时自动创建，进程退出时会做兜底排空。

## 快速开始

### 默认 Logger

```c
#include "coter/log/log.h"

int main(void) {
    CT_DEBUG("Hello %s", "world");
    ct_logger_close(ct_logger_default());
    return 0;
}
```

### 自定义 Logger

```c
ct_logger_t logger;
ct_logger_init(&logger);

ct_log_console_handler_config_t config;
ct_log_console_handler_config_default(&config);

ct_logger_add_handler(&logger, ct_log_console_handler_create(&config));
ct_logger_start(&logger);

CT_LOGGER_DEBUG(&logger, "custom logger");

ct_logger_close(&logger);
```

`ct_logger_start()` 之后 handler 拓扑不可变。`ct_logger_close()` 会阻止新的写入，等待当前 producer 和 dispatcher 中引用该 logger 的 job 完成，然后 flush/destroy handlers。

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

### 文本回调 Handler

```c
static void write_text(const char* data, size_t size, void* userdata) {
    fwrite(data, 1, size, (FILE*)userdata);
}

ct_log_callback_handler_config_t config;
ct_log_callback_handler_config_default(&config);
config.routine = write_text;
config.userdata = stderr;

ct_log_handler_t* handler = ct_log_callback_handler_create(&config);
```

### 结构化回调 Handler

```c
static void collect_record(const ct_log_record_t* record, void* userdata) {
    (void)userdata;
    fwrite(record->data, 1, record->size, stdout);
}

ct_log_base_handler_config_t config;
ct_log_base_handler_config_default(&config);
config.routine = collect_record;

ct_log_handler_t* handler = ct_log_base_handler_create(&config);
```

### C++ 使用

```cpp
#include "coter/log/log.hpp"

int main() {
    auto logger = coter::log::default_logger();
    COTER_LOGGER_DEBUG(logger, "Hello {}!", "world");
    ct_logger_close(ct_logger_default());
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

## API 参考

### Logger

```c
ct_logger_t* ct_logger_default(void);

void ct_logger_init(ct_logger_t* logger);
int  ct_logger_start(ct_logger_t* logger);
void ct_logger_close(ct_logger_t* logger);

void ct_logger_set_level(ct_logger_t* logger, int level);
int  ct_logger_get_level(const ct_logger_t* logger);
int  ct_logger_add_handler(ct_logger_t* logger, ct_log_handler_t* handler);

bool ct_logger_is_enabled(const ct_logger_t* logger, int level);
```

未 `start` 的自定义 logger 不写入日志。运行时初始化失败时 `ct_logger_start()` 返回 `-1`。

### Handler

```c
void ct_log_console_handler_config_default(ct_log_console_handler_config_t* config);
ct_log_handler_t* ct_log_console_handler_create(const ct_log_console_handler_config_t* config);

void ct_log_file_handler_config_default(ct_log_file_handler_config_t* config);
ct_log_handler_t* ct_log_file_handler_create(const ct_log_file_handler_config_t* config);

void ct_log_callback_handler_config_default(ct_log_callback_handler_config_t* config);
ct_log_handler_t* ct_log_callback_handler_create(const ct_log_callback_handler_config_t* config);

void ct_log_base_handler_config_default(ct_log_base_handler_config_t* config);
ct_log_handler_t* ct_log_base_handler_create(const ct_log_base_handler_config_t* config);
```

## 线程安全

| 操作 | 线程安全性 | 说明 |
| --- | --- | --- |
| `CT_*` / `CT_LOGGER_*` 写入 | 安全 | TLS scratch buffer，每线程独立 |
| `ct_logger_set_level` | 安全 | atomic 操作 |
| `ct_logger_add_handler` | 配置期安全 | 只能在 `ct_logger_start()` 前调用 |
| `ct_logger_close` | 安全 | 等待 active writer 和 pending job 后销毁 handler |
| Handler `puts` | 串行调用 | Dispatcher 单线程分发 |

## 资源策略

- 单条日志 payload 固定上限为内部常量，超长日志会截断并以 `...` 结尾。
- 异步队列固定容量，队列满时丢弃当前日志，不阻塞业务线程。
- 不提供同步 fallback，不暴露运行时队列和 block 配置。

## 内部格式化

console/file handler 共用内部 formatter。格式当前固定，不开放自定义模板。console 输出带 ANSI 颜色，file 输出不带颜色。
