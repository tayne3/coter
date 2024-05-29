/**
 * @file ct_syscmd.h
 * @brief 系统命令
 * @author tayne3@dingtalk.com
 * @date 2023.12.05
 */
#ifndef _CT_SYSCMD_H
#define _CT_SYSCMD_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

#include "common/ct_time.h"

/**
 * @brief 执行系统命令
 * @param cmd 要执行的命令字符串
 * @param is_print_error 是否打印错误信息
 * @return 执行成功返回true，否则返回false
 */
bool ct_syscmd_execute(const char *cmd, bool is_print_error);

/**
 * @brief 执行系统命令并获取执行结果
 * @param command 要执行的命令字符串
 * @param buffer 存储命令执行结果的缓冲区
 * @param max 缓冲区的最大长度
 * @return 执行成功返回true，否则返回false
 */
bool ct_syscmd_execute_r(const char *command, char *buffer, size_t max);

/**
 * @brief 判断文件夹是否存在
 * @param path 文件夹路径
 * @return 文件夹存在返回true，否则返回false
 */
bool ct_syscmd_folder_isexist(const char *path);

/**
 * @brief 删除文件夹
 * @param path 要删除的文件夹路径
 * @return 删除成功返回true，否则返回false
 */
bool ct_syscmd_folder_delete(const char *path);

/**
 * @brief 创建文件夹
 * @param path 要创建的文件夹路径
 * @return 创建成功返回true，否则返回false
 */
bool ct_syscmd_folder_create(const char *path);

/**
 * @brief 递归创建文件夹
 * @param path 要创建的文件夹路径
 * @return 创建成功返回true，否则返回false
 */
bool ct_syscmd_folder_recursive_create(const char *path);

/**
 * @brief 判断文件是否存在
 * @param path 文件路径
 * @return 文件存在返回true，否则返回false
 */
bool ct_syscmd_file_isexist(const char *path);

/**
 * @brief 获取文件大小
 * @param path 文件路径
 * @param size 指向存储文件大小的变量的指针
 * @return 获取成功返回true，否则返回false
 */
bool ct_syscmd_file_size(const char *path, size_t *size);

/**
 * @brief 读取文件内容
 * @param path 文件路径
 * @param buffer 存储文件内容的缓冲区
 * @param offset 从文件的哪个位置开始读取
 * @param max 缓冲区的最大长度
 * @param size 实际读取的字节数
 * @return 读取成功返回true，否则返回false
 */
bool ct_syscmd_file_read(const char *path, char *buffer, size_t offset, size_t max, size_t *size);

/**
 * @brief 设置系统时间
 * @param cdt 时间结构体指针
 * @return 设置成功返回true; 设置失败返回false
 */
bool ct_syscmd_set_time(const ct_datetime_buf_t cdt);

#ifdef __cplusplus
}
#endif
#endif  // _CT_SYSCMD_H
