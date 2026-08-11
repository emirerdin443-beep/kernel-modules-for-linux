```c
/*
 * hwmgr.c
 *
 * Hardware Manager
 * ----------------
 * A generic, safe hardware monitoring/management kernel module.
 *
 * Features:
 *   - CPU topology information
 *   - RAM statistics
 *   - Kernel uptime
 *   - Runtime monitoring thread
 *   - /proc/hwmgr interface
 *   - /sys/kernel/hwmgr interface
 *   - Configurable monitoring interval
 *   - Configurable logging
 *   - Runtime statistics
 *
 * This module intentionally does NOT directly manipulate arbitrary
 * physical hardware registers. Hardware control is device-specific
 * and doing that generically would be a spectacular way to crash
 * someone's machine.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/jiffies.h>
#include <linux/mm.h>
#include <linux/sysinfo.h>
#include <linux/cpu.h>
#include <linux/ktime.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/version.h>

#define HWMGR_NAME              "hwmgr"
#define HWMGR_VERSION           "1.0.0"

#define HWMGR_DEFAULT_INTERVAL  5000
#define HWMGR_MIN_INTERVAL      100
#define HWMGR_MAX_INTERVAL      60000

#define HWMGR_LOG_SILENT        0
#define HWMGR_LOG_ERROR         1
#define HWMGR_LOG_INFO          2
#define HWMGR_LOG_DEBUG         3

static struct task_struct *hwmgr_thread;
static struct kobject *hwmgr_kobj;
static struct proc_dir_entry *hwmgr_proc;

static DEFINE_MUTEX(hwmgr_lock);

static unsigned int monitor_interval = HWMGR_DEFAULT_INTERVAL;
static unsigned int log_level = HWMGR_LOG_INFO;
static bool monitoring_enabled = true;

static unsigned long monitor_cycles;
static unsigned long monitor_errors;
static u64 module_start_time;

/* ------------------------------------------------------------ */
/* Logging                                                       */
/* ------------------------------------------------------------ */

#define hwmgr_error(fmt, ...) \
	pr_err("hwmgr: " fmt, ##__VA_ARGS__)

#define hwmgr_info(fmt, ...) \
	pr_info("hwmgr: " fmt, ##__VA_ARGS__)

#define hwmgr_debug(fmt, ...) \
	do { \
		if (READ_ONCE(log_level) >= HWMGR_LOG_DEBUG) \
			pr_info("hwmgr: " fmt, ##__VA_ARGS__); \
	} while (0)

/* ------------------------------------------------------------ */
/* Hardware information                                         */
/* ------------------------------------------------------------ */

static void hwmgr_collect_memory(unsigned long *total_kb,
				  unsigned long *free_kb,
				  unsigned long *available_kb)
{
	struct sysinfo info;
	unsigned long factor;

	si_meminfo(&info);

	factor = info.mem_unit / 1024;

	*total_kb = info.totalram * factor;
	*free_kb = info.freeram * factor;

#ifdef CONFIG_MMU
	/*
	 * available memory is not directly provided by si_meminfo().
	 * global_node_page_state() gives us a useful approximation.
	 */
	*available_kb =
		global_node_page_state(NR_FREE_PAGES) * (PAGE_SIZE / 1024);
#else
	*available_kb = *free_kb;
#endif
}

static unsigned int hwmgr_online_cpus(void)
{
	return num_online_cpus();
}

static unsigned int hwmgr_possible_cpus(void)
{
	return num_possible_cpus();
}

/* ------------------------------------------------------------ */
/* /proc output                                                  */
/* ------------------------------------------------------------ */

static int hwmgr_proc_show(struct seq_file *m, void *v)
{
	unsigned long total_kb;
	unsigned long free_kb;
	unsigned long available_kb;
	unsigned long uptime_seconds;
	u64 now;

	hwmgr_collect_memory(&total_kb, &free_kb, &available_kb);

	now = ktime_get_boottime_seconds();

	uptime_seconds = (unsigned long)now;

	mutex_lock(&hwmgr_lock);

	seq_puts(m, "========================================\n");
	seq_puts(m, "        Hardware Manager\n");
	seq_puts(m, "========================================\n\n");

	seq_printf(m, "Version             : %s\n", HWMGR_VERSION);

	seq_puts(m, "\n[CPU]\n");
	seq_printf(m, "Possible CPUs       : %u\n",
		   hwmgr_possible_cpus());
	seq_printf(m, "Online CPUs         : %u\n",
		   hwmgr_online_cpus());

	seq_puts(m, "\n[MEMORY]\n");
	seq_printf(m, "Total RAM           : %lu KB\n",
		   total_kb);
	seq_printf(m, "Free RAM            : %lu KB\n",
		   free_kb);
	seq_printf(m, "Free pages estimate : %lu KB\n",
		   available_kb);

	seq_puts(m, "\n[SYSTEM]\n");
	seq_printf(m, "Kernel uptime       : %lu seconds\n",
		   uptime_seconds);

	seq_puts(m, "\n[MONITOR]\n");
	seq_printf(m, "Enabled             : %s\n",
		   monitoring_enabled ? "yes" : "no");
	seq_printf(m, "Interval            : %u ms\n",
		   monitor_interval);
	seq_printf(m, "Log level           : %u\n",
		   log_level);
	seq_printf(m, "Monitor cycles      : %lu\n",
		   monitor_cycles);
	seq_printf(m, "Monitor errors      : %lu\n",
		   monitor_errors);

	seq_puts(m, "\n========================================\n");

	mutex_unlock(&hwmgr_lock);

	return 0;
}

static int hwmgr_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwmgr_proc_show, NULL);
}

static const struct proc_ops hwmgr_proc_ops = {
	.proc_open    = hwmgr_proc_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

/* ------------------------------------------------------------ */
/* Sysfs: monitoring                                             */
/* ------------------------------------------------------------ */

static ssize_t monitoring_show(struct kobject *kobj,
			       struct kobj_attribute *attr,
			       char *buf)
{
	return sysfs_emit(buf, "%u\n",
			  READ_ONCE(monitoring_enabled));
}

static ssize_t monitoring_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf,
				size_t count)
{
	bool value;

	if (kstrtobool(buf, &value))
		return -EINVAL;

	WRITE_ONCE(monitoring_enabled, value);

	hwmgr_info("monitoring %s\n",
		   value ? "enabled" : "disabled");

	return count;
}

static struct kobj_attribute monitoring_attr =
	__ATTR_RW(monitoring);

/* ------------------------------------------------------------ */
/* Sysfs: interval                                               */
/* ------------------------------------------------------------ */

static ssize_t interval_show(struct kobject *kobj,
			     struct kobj_attribute *attr,
			     char *buf)
{
	return sysfs_emit(buf, "%u\n",
			  READ_ONCE(monitor_interval));
}

static ssize_t interval_store(struct kobject *kobj,
			      struct kobj_attribute *attr,
			      const char *buf,
			      size_t count)
{
	unsigned int value;

	if (kstrtouint(buf, 10, &value))
		return -EINVAL;

	if (value < HWMGR_MIN_INTERVAL ||
	    value > HWMGR_MAX_INTERVAL)
		return -ERANGE;

	WRITE_ONCE(monitor_interval, value);

	hwmgr_info("monitor interval changed to %u ms\n",
		   value);

	return count;
}

static struct kobj_attribute interval_attr =
	__ATTR_RW(interval);

/* ------------------------------------------------------------ */
/* Sysfs: log level                                              */
/* ------------------------------------------------------------ */

static ssize_t log_level_show(struct kobject *kobj,
			      struct kobj_attribute *attr,
			      char *buf)
{
	return sysfs_emit(buf, "%u\n",
			  READ_ONCE(log_level));
}

static ssize_t log_level_store(struct kobject *kobj,
			       struct kobj_attribute *attr,
			       const char *buf,
			       size_t count)
{
	unsigned int value;

	if (kstrtouint(buf, 10, &value))
		return -EINVAL;

	if (value > HWMGR_LOG_DEBUG)
		return -ERANGE;

	WRITE_ONCE(log_level, value);

	return count;
}

static struct kobj_attribute log_level_attr =
	__ATTR_RW(log_level);

/* ------------------------------------------------------------ */
/* Sysfs: statistics                                             */
/* ------------------------------------------------------------ */

static ssize_t statistics_show(struct kobject *kobj,
			       struct kobj_attribute *attr,
			       char *buf)
{
	return sysfs_emit(buf,
			  "cycles=%lu errors=%lu\n",
			  READ_ONCE(monitor_cycles),
			  READ_ONCE(monitor_errors));
}

static struct kobj_attribute statistics_attr =
	__ATTR_RO(statistics);

/* ------------------------------------------------------------ */
/* Sysfs: CPU information                                        */
/* ------------------------------------------------------------ */

static ssize_t cpu_info_show(struct kobject *kobj,
			     struct kobj_attribute *attr,
			     char *buf)
{
	return sysfs_emit(buf,
			  "possible=%u online=%u\n",
			  hwmgr_possible_cpus(),
			  hwmgr_online_cpus());
}

static struct kobj_attribute cpu_info_attr =
	__ATTR_RO(cpu_info);

/* ------------------------------------------------------------ */
/* Sysfs: memory information                                     */
/* ------------------------------------------------------------ */

static ssize_t memory_info_show(struct kobject *kobj,
				struct kobj_attribute *attr,
				char *buf)
{
	unsigned long total_kb;
	unsigned long free_kb;
	unsigned long available_kb;

	hwmgr_collect_memory(&total_kb,
			     &free_kb,
			     &available_kb);

	return sysfs_emit(buf,
			  "total_kb=%lu free_kb=%lu available_kb=%lu\n",
			  total_kb,
			  free_kb,
			  available_kb);
}

static struct kobj_attribute memory_info_attr =
	__ATTR_RO(memory_info);

/* ------------------------------------------------------------ */
/* Sysfs attribute group                                         */
/* ------------------------------------------------------------ */

static struct attribute *hwmgr_attrs[] = {
	&monitoring_attr.attr,
	&interval_attr.attr,
	&log_level_attr.attr,
	&statistics_attr.attr,
	&cpu_info_attr.attr,
	&memory_info_attr.attr,
	NULL
};

static const struct attribute_group hwmgr_attr_group = {
	.attrs = hwmgr_attrs,
};

/* ------------------------------------------------------------ */
/* Monitoring thread                                             */
/* ------------------------------------------------------------ */

static int hwmgr_monitor_thread(void *data)
{
	while (!kthread_should_stop()) {
		unsigned int interval;

		if (READ_ONCE(monitoring_enabled)) {
			unsigned int online = hwmgr_online_cpus();
			unsigned long total_kb;
			unsigned long free_kb;
			unsigned long available_kb;

			hwmgr_collect_memory(&total_kb,
					     &free_kb,
					     &available_kb);

			monitor_cycles++;

			hwmgr_debug(
				"CPU online=%u RAM total=%luKB free=%luKB\n",
				online,
				total_kb,
				free_kb
			);
		}

		interval = READ_ONCE(monitor_interval);

		if (msleep_interruptible(interval))
			break;
	}

	return 0;
}

/* ------------------------------------------------------------ */
/* Initialization                                                */
/* ------------------------------------------------------------ */

static int __init hwmgr_init(void)
{
	int ret;

	module_start_time = ktime_get_boottime_seconds();

	hwmgr_info("initializing Hardware Manager %s\n",
		   HWMGR_VERSION);

	/* /proc/hwmgr */
	hwmgr_proc = proc_create(HWMGR_NAME,
				 0444,
				 NULL,
				 &hwmgr_proc_ops);

	if (!hwmgr_proc) {
		hwmgr_error("failed to create /proc/%s\n",
			    HWMGR_NAME);
		return -ENOMEM;
	}

	/* /sys/kernel/hwmgr */
	hwmgr_kobj = kobject_create_and_add(HWMGR_NAME, kernel_kobj);

	if (!hwmgr_kobj) {
		hwmgr_error("failed to create sysfs object\n");
		proc_remove(hwmgr_proc);
		return -ENOMEM;
	}

	ret = sysfs_create_group(hwmgr_kobj,
				 &hwmgr_attr_group);

	if (ret) {
		hwmgr_error("failed to create sysfs group: %d\n",
			    ret);

		kobject_put(hwmgr_kobj);
		proc_remove(hwmgr_proc);

		return ret;
	}

	/* Start monitoring thread */
	hwmgr_thread = kthread_run(hwmgr_monitor_thread,
				   NULL,
				   "hwmgr-monitor");

	if (IS_ERR(hwmgr_thread)) {
		ret = PTR_ERR(hwmgr_thread);
		hwmgr_thread = NULL;

		hwmgr_error("failed to start monitor thread: %d\n",
			    ret);

		sysfs_remove_group(hwmgr_kobj,
				   &hwmgr_attr_group);
		kobject_put(hwmgr_kobj);
		proc_remove(hwmgr_proc);

		return ret;
	}

	hwmgr_info("Hardware Manager initialized\n");
	hwmgr_info("proc interface: /proc/%s\n",
		   HWMGR_NAME);
	hwmgr_info("sysfs interface: /sys/kernel/%s/\n",
		   HWMGR_NAME);

	return 0;
}

/* ------------------------------------------------------------ */
/* Cleanup                                                       */
/* ------------------------------------------------------------ */

static void __exit hwmgr_exit(void)
{
	if (hwmgr_thread)
		kthread_stop(hwmgr_thread);

	if (hwmgr_kobj) {
		sysfs_remove_group(hwmgr_kobj,
				   &hwmgr_attr_group);
		kobject_put(hwmgr_kobj);
	}

	if (hwmgr_proc)
		proc_remove(hwmgr_proc);

	hwmgr_info("Hardware Manager unloaded\n");
}

module_init(hwmgr_init);
module_exit(hwmgr_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emir");
MODULE_DESCRIPTION(
	"Generic hardware monitoring and management kernel module"
);
MODULE_VERSION(HWMGR_VERSION);
```
