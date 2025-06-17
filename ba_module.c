#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "ba.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SessionHu");
MODULE_DESCRIPTION("Prints Bad Apple with Linux Kernel");

// 定义边界符号行
const char * const BOUNDARY_SEQUENCE = "\033[2J\033[H";
#define BOUNDARY_SEQUENCE_LEN (sizeof(BOUNDARY_SEQUENCE) - 1)

#define FRAMES_PER_SECOND 30
// 计算每帧的毫秒延迟
#define DELAY_MS_PER_FRAME (1000 / FRAMES_PER_SECOND)

static int __init ba_module_init(void)
{
    unsigned int current_offset = 0;
    unsigned int frame_count = 0;
    unsigned long start_time_of_frame; // 记录当前帧开始的时间

    printk(KERN_INFO "ba_module: Loading Bad Apple module...\n");

    if (ba_asc_len == 0) {
        printk(KERN_ERR "ba_module: ba_asc_len is 0. No data to print.\n");
        return -EINVAL;
    }

    if (BOUNDARY_SEQUENCE_LEN == 0) {
        printk(KERN_WARNING "ba_module: BOUNDARY_SEQUENCE_LEN is 0. Cannot sync frames.\n");
        return -EINVAL;
    }

    printk(KERN_INFO "ba_module: Total data length: %u bytes.\n", ba_asc_len);
    printk(KERN_INFO "ba_module: Boundary sequence length: %u bytes.\n", BOUNDARY_SEQUENCE_LEN);
    printk(KERN_INFO "ba_module: Playing at %d frames per second, delay of %u ms per frame.\n",
           FRAMES_PER_SECOND, DELAY_MS_PER_FRAME);

    start_time_of_frame = jiffies; // 记录第一帧的开始时间

    while (current_offset < ba_asc_len) {
        // --- 1. 检查是否是边界序列 ---
        if ((current_offset + BOUNDARY_SEQUENCE_LEN <= ba_asc_len) &&
            (strncmp((char *)&ba_asc[current_offset], BOUNDARY_SEQUENCE, BOUNDARY_SEQUENCE_LEN) == 0)) {

            // 这是一个帧的开始 (或者说前一个帧的结束，新帧的开始)
            // 跳过边界序列
            current_offset += BOUNDARY_SEQUENCE_LEN;

            // 计算当前帧实际花费了多少时间
            unsigned long elapsed_jiffies = jiffies - start_time_of_frame;
            unsigned long target_jiffies_for_frame = msecs_to_jiffies(DELAY_MS_PER_FRAME);

            // 如果当前帧处理得比目标时间快，就睡眠剩余的时间
            if (elapsed_jiffies < target_jiffies_for_frame) {
                unsigned long sleep_jiffies = target_jiffies_for_frame - elapsed_jiffies;
                // 将 jiffies 转换为毫秒进行 msleep
                unsigned int sleep_ms = jiffies_to_msecs(sleep_jiffies);
                if (sleep_ms > 0) {
                    msleep(sleep_ms);
                    // printk(KERN_DEBUG "ba_module: Frame %u slept for %u ms.\n", frame_count, sleep_ms);
                }
            }
            
            // 记录新帧的开始时间
            start_time_of_frame = jiffies;

            frame_count++;
            printk(KERN_DEBUG "ba_module: Starting frame %u (offset %u).\n", frame_count, current_offset);

            // 确保跳过边界符后，还有数据可打印
            if (current_offset >= ba_asc_len) {
                printk(KERN_INFO "ba_module: Reached end of data after skipping boundary.\n");
                break; // 数据已处理完
            }

        } else {
            // --- 2. 打印普通字符行 ---
            // 找到当前行的结束 (换行符)
            unsigned int line_end_offset = current_offset;
            while (line_end_offset < ba_asc_len && ba_asc[line_end_offset] != '\n') {
                line_end_offset++;
            }

            // 计算当前行的长度 (不包括换行符本身)
            size_t line_length = line_end_offset - current_offset;

            // 打印行内容
            if (line_length > 0) {
                printk(KERN_CONT "%.*s\n", (int)line_length, &ba_asc[current_offset]);
            } else {
                // 如果是空行，也打印一个换行
                printk(KERN_CONT "\n");
            }

            // 移动到下一行 (跳过当前行内容和换行符)
            current_offset = line_end_offset + 1; // +1 跳过换行符
            // 确保不超出数组范围
            if (current_offset > ba_asc_len) {
                current_offset = ba_asc_len;
            }
        }
    }

    printk(KERN_INFO "ba_module: Finished playing %u frames. Module loaded successfully.\n", frame_count);
    return 0;
}

static void __exit ba_module_exit(void)
{
    printk(KERN_INFO "ba_module: Unloading Bad Apple module.\n");
}

module_init(ba_module_init);
module_exit(ba_module_exit);
