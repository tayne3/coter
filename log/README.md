# log

固定资源的异步日志系统。生产线程把完整日志快照提交到固定容量 bounded queue；单个 dispatcher
线程顺序消费并串行调用 handler。正常运行路径下，队列满时 producer 阻塞等待，不因容量压力丢日志。

## 架构

```text
日志宏 (CT_* / CT_LOGGER_*)
     |
     v
job snapshot
     |
     v
bounded queue
     |
     v
dispatcher thread
     |
     v
handler: console / file / text / record
```

日志运行时按需懒加载。调用方不需要显式初始化全局日志系统；默认 logger 首次使用时自动创建，
进程退出时会做兜底 flush。

## 快速开始

### 默认 Logger

```c
#include "coter/log/log.h"

int main(void) {
    CT_DEBUG("Hello %s", "world");
    ct_logger_flush(NULL);
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

`ct_logger_start()` 之后 handler 拓扑不可变。`ct_logger_close()` 会阻止新的写入，等待当前
producer 退出，并等待该 logger 已成功入队的日志全部被 dispatcher 处理完成，然后 flush 并销毁 handlers。
close 是生命周期屏障，不提供丢弃策略。

默认 logger 可以在启动配置期替换。替换必须发生在首次默认 logger 使用之前；任何 `CT_*` 默认日志宏、
`ct_logger_get_default()`、`ct_logger_set_level(NULL, ...)`、`ct_logger_get_level(NULL)` 都会封印默认 logger。

```c
static ct_logger_t logger;

ct_logger_init(&logger);
/* add handlers ... */
ct_logger_start(&logger);

ct_logger_set_default(&logger);
CT_DEBUG("goes to custom default");
```

`ct_logger_set_default()` 不接管 logger 所有权；调用方必须保证自定义默认 logger 具有进程级或静态生命周期，
并在使用期间保持 started 且存活。该接口用于启动配置期，不支持运行期热切换。默认 logger 一旦被封印，
`ct_logger_set_default()` 会返回失败；关闭已封印的自定义默认 logger 也会返回失败。需要显式刷新时使用
`ct_logger_flush(NULL)`。

### 文件日志

```c
ct_logger_t logger;
ct_logger_init(&logger);

ct_log_file_handler_config_t config;
ct_log_file_handler_config_default(&config);
strncpy(config.dir, "./logs", sizeof(config.dir) - 1);
strncpy(config.name, "app", sizeof(config.name) - 1);

ct_logger_add_handler(&logger, ct_log_file_handler_create(&config));
ct_logger_start(&logger);

CT_LOGGER_DEBUG(&logger, "file logger");

ct_logger_close(&logger);
```

文件轮转规则：`{dir}/{name}.log0` -> `{name}.log1` -> ... -> `{name}.logN`，循环覆写。
默认上限 3 个文件，每个最大 4MB。

### 文本回调 Handler

```c
static void write_text(const char* data, size_t size, void* userdata) {
    fwrite(data, 1, size, (FILE*)userdata);
}

ct_log_text_handler_config_t config;
ct_log_text_handler_config_default(&config);
config.routine  = write_text;
config.userdata = stderr;

ct_log_handler_t* handler = ct_log_text_handler_create(&config);
```

### 结构化回调 Handler

```c
static void collect_record(const ct_log_record_t* record, void* userdata) {
    (void)userdata;
    fwrite(record->data, 1, record->size, stdout);
}

ct_log_record_handler_config_t config;
ct_log_record_handler_config_default(&config);
config.routine = collect_record;

ct_log_handler_t* handler = ct_log_record_handler_create(&config);
```

### C++ 使用

```cpp
#include "coter/log/log.h"

int main() {
    auto logger = coter::log::default_logger();
    COTER_LOGGER_DEBUG(logger, "Hello {}!", "world");
    ct_logger_flush(NULL);
    return 0;
}
```

## 日志级别

| 级别    | 值  | 缩写 |
| ------- | --- | ---- |
| TRACE   | 0   | TRC  |
| DEBUG   | 1   | DBG  |
| INFO    | 2   | INF  |
| WARNING | 3   | WRN  |
| ERROR   | 4   | ERR  |
| FATAL   | 5   | FTL  |

通过 `ct_logger_set_level()` 设置最低输出级别，低于该级别的日志不会入队。

## API 参考

### Logger

```c
ct_logger_t* ct_logger_get_default(void);
int          ct_logger_set_default(ct_logger_t* logger);

void ct_logger_init(ct_logger_t* logger);
int  ct_logger_start(ct_logger_t* logger);
int  ct_logger_close(ct_logger_t* logger);
int  ct_logger_flush(ct_logger_t* logger);

void ct_logger_set_level(ct_logger_t* logger, int level);
int  ct_logger_get_level(const ct_logger_t* logger);
int  ct_logger_add_handler(ct_logger_t* logger, ct_log_handler_t* handler);

bool ct_logger_is_enabled(const ct_logger_t* logger, int level);
```

未 `start` 的自定义 logger 不写入日志。运行时初始化失败时 `ct_logger_start()` 返回 `-1`。

### Handler

`ct_log_handler_t` 是不透明句柄。调用方不访问 handler 内部字段，不直接调用 vtable；未加入 logger 的 handler
通过 `ct_log_handler_destroy()` 销毁。加入 logger 后，handler 生命周期由 logger 接管。

```c
void              ct_log_console_handler_config_default(ct_log_console_handler_config_t* config);
ct_log_handler_t* ct_log_console_handler_create(const ct_log_console_handler_config_t* config);

void              ct_log_file_handler_config_default(ct_log_file_handler_config_t* config);
ct_log_handler_t* ct_log_file_handler_create(const ct_log_file_handler_config_t* config);

void              ct_log_text_handler_config_default(ct_log_text_handler_config_t* config);
ct_log_handler_t* ct_log_text_handler_create(const ct_log_text_handler_config_t* config);

void              ct_log_record_handler_config_default(ct_log_record_handler_config_t* config);
ct_log_handler_t* ct_log_record_handler_create(const ct_log_record_handler_config_t* config);

void ct_log_handler_destroy(ct_log_handler_t* handler);
```

## 线程安全

| 操作                        | 线程安全性 | 说明                                                              |
| --------------------------- | ---------- | ----------------------------------------------------------------- |
| `CT_*` / `CT_LOGGER_*` 写入 | 安全       | 构建 job snapshot 后提交到 bounded queue                          |
| `ct_logger_set_level`       | 安全       | atomic 操作                                                       |
| `ct_logger_add_handler`     | 配置期安全 | 只能在 `ct_logger_start()` 前调用                                 |
| `ct_logger_close`           | 安全       | 拒绝新写入，等待 active writer 和 per-logger drain 后销毁 handler |
| `ct_logger_flush`           | 安全       | 等待已提交日志处理完成并刷新 handler                              |
| `ct_logger_set_default`     | 配置期安全 | 只能在默认 logger 首次使用前调用                                  |
| Handler `write`             | 串行调用   | dispatcher 单线程分发                                             |

## 资源策略

- 单条日志 payload 固定上限为内部常量，超长日志会截断并以 `...` 结尾。
- 异步队列固定容量，正常运行路径下队列满时 producer 阻塞等待，不因容量压力丢日志。
- 不提供同步 fallback，不暴露运行时队列、线程池或 block 策略配置。

## 调用限制

- handler 内部再次写日志会被 dispatcher 保护逻辑拒绝，避免递归日志造成死锁。
- handler 内部调用 `ct_logger_close()` 会返回失败，避免递归 close 造成死锁。
- 默认 logger 是进程级常驻对象，使用 `ct_logger_flush(NULL)` 做显式 drain + flush。
- 自定义默认 logger 一旦封印，不能运行期替换或关闭；业务侧应让它保持进程级存活。
- dispatcher 是进程级常驻运行时，进程退出时只做 drain + flush，不关闭队列、不 join worker。

## 内部格式化

console/file handler 共用内部 formatter。格式当前固定，不开放自定义模板。console 输出带 ANSI 颜色，
file 输出不带颜色。
