/* Achro-i.MX6Q External Sensor GPIO Control
FILE : hc-sr04_driver.c
AUTH : gmlee@huins.com */

#include <linux/module.h>
#include <linux/cdev.h>			// cdev_init(), cdev_add()
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>		// MAJOR(), MKDEV()
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

#define IR_DATA IMX_GPIO_NR(2, 1)

static int remote_control_major = 0;
static int remote_control_minor = 0;
static int result;
static int res;
static dev_t remote_control_dev;
static struct cdev remote_control_cdev;
struct timeval after;		// struct timeval {
							//		long tv_sec;		// s ����
							//		long tv_usec;		// us ����
							// };
struct timeval before;
u32 irq = -1;

u32 data;
int index = 0;

static int remote_control_open(struct inode *inode, struct file *filp);
static int remote_control_release(struct inode *inode, struct file *filp);
static int remote_control_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);

struct file_operations remote_control_fops = {
	.open = remote_control_open,
	.release = remote_control_release,
	.read = remote_control_read
};

static irqreturn_t remote_control_interrupt(int irq, void *dev_id, struct pt_regs *regs);
static int remote_control_register_cdev(void);
void output_sonicburst(void);
int gpio_init(void);


static int remote_control_open(struct inode *inode, struct file *filp) {
	printk(KERN_ALERT "< Device has been opened >\n");
	return 0;
}
static int remote_control_release(struct inode *inode, struct file *filp) {
	printk(KERN_ALERT "< Device has been closed > \n");
	return 0;
}
static int remote_control_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {				
	printk(KERN_ALERT" Data : %lu\n", (unsigned long)target);

	return 0;
}


static irqreturn_t remote_control_interrupt(int irq, void *dev_id, struct pt_regs *regs) {
	if (gpio_get_value(IR_DATA)) {			// int gpio_get_value(unsigned int gpio); : ��� ��� GPIO ���� ���� �о�´�.
		do_gettimeofday(&after);
		if (after.tv_usec - before.tv_usec < 10) {
			data = data | (0x00000001 << index++);
		}
		else if (after.tv_usec - before.tv_usec > 1000) {
			data = 0x00000000;
			index = 0;
		}
		else {
			data = data & (0xFFFFFFFF - (0x00000001 << index++));
		}
		memset(&before, 0, sizeof(struct timeval));
		memset(&after, 0, sizeof(struct timeval));
	}
	else {
		do_gettimeofday(&before);			// do_gettimeofday(struct timeval *tv) : �� / ����ũ���� ������ ����ð��� ����
	}
	return IRQ_HANDLED;		// �ùٸ� ��� IRQ_HANDLED�� ���� ���� ��� IRQ_NONE�� ����
}
static int remote_control_register_cdev(void) {
	int error;
	if (remote_control_major) {
		remote_control_dev = MKDEV(remote_control_major, remote_control_minor);		// MKDEV(int major, int minor); : �ֹ�ȣ�� major �� �ι�ȣ�� minor ���� �̿��Ͽ� dev_t ���� ��´�.
		error = register_chrdev_region(remote_control_dev, 1, "remote_control");				// int register_chrdev_region(dev_t from, unsigned count, const char *name)
																					// : from�� major �ֹ�ȣ�� minor �ι�ȣ�� �������� count ��ŭ major, minor��ȣ�� �Ҵ�
	}
	else {
		error = alloc_chrdev_region(&remote_control_dev, remote_control_minor, 1, "remote_control");	// int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
																							// : baseminor �ι�ȣ�� �������� count ��ŭ minor��ȣ�� �Ҵ�, major ��ȣ�� �������� ����� ã�Ƽ� ����, ȣ���� �ʿ��� ù��° �Ű����� dev_t *dev�� �� �� �ִ�.
		remote_control_major = MAJOR(remote_control_dev);									// MAJOR(dev_t dev); : dev ���� �ֹ�ȣ ���� ��´�.
	}
	if (error < 0) {
		printk(KERN_WARNING "remote_control: can't get major %d\n", remote_control_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", remote_control_major);
	cdev_init(&remote_control_cdev, &remote_control_fops);			// void cdev_init (	struct cdev * cdev, const struct file_operations * fops); : struct cdev ����ü�� �ʱ�ȭ �����ִ� �Լ�
																	// static struct cdev dasom_cdev = {
	remote_control_cdev.owner = THIS_MODULE;						//	 .owner = THIS_MODULE,
	remote_control_cdev.ops = &remote_control_fops;					//	 .ops = &dasom_fops,
																	// };
	error = cdev_add(&remote_control_cdev, remote_control_dev, 1);	//	int cdev_add(struct cdev * p, dev_t dev, unsigned count); : major, minor ���� �̿��� struct cdev�� dev_t dev�� ����
	if (error)
	{
		printk(KERN_NOTICE "remote_control Register Error %d\n", error);
	}
	return 0;
}
int gpio_init(void) {
	int rtc;
	rtc = gpio_request(IR_DATA, "DATA");					// int gpio_request(unsigned int gpio, const char *label); : GPIO �� ���� ���ǰ� �ִ� �� Ȯ��, ��� �����ϴٸ� lock�� �ɾ� �����Ѵ�. (������ -EBUSY, ������ 0)
	if (rtc != 0) {
		printk(KERN_ALERT "<Data Pin Request Fail>\n");
		goto fail;
	}
	rtc = gpio_direction_input(IR_DATA);					// int gpio_direction_input(unsigned int gpio); : GPIO ���� �Է� �뵵�� ����
	if (rtc != 0) {
		printk(KERN_ALERT "<Echo Pin Setting Fail>\n");
		goto fail;
	}
	rtc = gpio_to_irq(IR_DATA);								// int gpio_to_irq(unsigned int gpio); : gpio�� �ش��ϴ� interrupt �ּҸ� �������ִ� �Լ�
	if (rtc < 0) {
		printk(KERN_ALERT "<irq Pin GPIO Request Fail>\n");
		goto fail;
	}
	else {
		irq = rtc;
	}
	rtc = request_irq(irq, (void*)remote_control_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED, "us", NULL);
	// int request_irq(unsigned int irq, irqreturn_t (*handler)(int, void*, struct pt_regs*), unsigned long frags, const char* device, void* dev_id); : ���ͷ�Ʈ ���� �Լ� ���
	if (rtc) {
		printk(KERN_ALERT "<irq Register Fail>\n");
		goto fail;
	}
	printk(KERN_INFO "HC-SR04 Enable\n");
	return 0;
fail:
	return -1;
}


static int remote_control_init(void) {
	printk(KERN_ALERT "< IR Module is up > \n");
	if ((result = remote_control_register_cdev()) < 0) {						// Ŀ�� �����
		printk(KERN_ALERT "< IR Register Fail > \n");
		return result;
	}
	res = gpio_init();		// GPIO, ISR ���� �� ����
	if (res < 0)
		return -1;
	return 0;
}
static void remote_control_exit(void) {
	printk(KERN_ALERT "< Ultrasonic Module is down > \n");
	free_irq(irq, NULL);			// void free_irq(unsigned int irq, void* dev_id); : ���ͷ�Ʈ ���� �Լ��� ����
	gpio_free(IR_DATA);		// gpio_request()�� ������ GPIO ���� lock�� ����
	cdev_del(&remote_control_cdev);				// cdev_add()�� ������ cdev ����ü�� ����
	unregister_chrdev_region(remote_control_dev, 1);		// alloc_chrdev_region()�� �Ҵ��� major, minor ��ȣ�� ����
}

MODULE_AUTHOR("gunmin, lee <gmlee@huins.com>");
MODULE_DESCRIPTION("HUINS ext sensor Device Driver");
MODULE_LICENSE("GPL");
module_init(remote_control_init);
module_exit(remote_control_exit);