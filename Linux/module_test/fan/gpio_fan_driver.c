#include "../include/fpga_driver.h"

static int fan_port_usage = 0;

static ssize_t iom_fan_write(struct file *file, const char *buf, size_t count, loff_t *f_pos);
//static ssize_t iom_fan_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static int iom_fan_open(struct inode *inode, struct file *file);
static int iom_fan_release(struct inode *inode, struct file *file);

struct file_operations iom_fan_fops = {
	.owner = THIS_MODULE,
	.open = iom_fan_open,
	.write = iom_fan_write,
	/*.read = iom_fan_read,*/
	.release = iom_fan_release
};

int gpio_init(void)
{
	//int rtc;
	//rtc = gpio_request(FAN_OUT, "FAN");			// GPIO ���� allocation �Ǿ�� ��
	//assert(rtc == 0, "FAN pin request fail");
	//rtc = gpio_direction_input(FAN_OUT);			// GPIO�� �Է����� ���� ���
	//assert(rtc == 0, "FAN pin setting fail");
	//rtc = gpio_direction_output(FAN_OUT, 0);		// GPIO�� ������� ���� ���, 0�̳� 1�� ���� �������־�� ��
	//assert(rtc == 0, "FAN pin setting fail");
	//gpio_get_value(FAN_OUT);						// �� �б�
	//gpio_set_value(FAN_OUT, 1);					// �� ����
	//int gpio_to_irq(unsigned int gpio);			// ��� GPIO ��Ʈ�ѷ��� GPIO �Է� ���� �ٲ�� ���ͷ�Ʈ�� �ɾ��ִµ�, �� IRQ ��ȣ�� ��� ������ �� �Լ��� ����Ѵ�.
	//gpio_free(FAN_OUT);							// GPIO �� allocation ����

	int rtc;

	rtc = gpio_request(FAN_OUT, "FAN");				// GPIO ���� allocation �Ǿ�� ��
	if (rtc != 0) {
		printk(KERN_ALERT "FAN pin request fail\n");
		return -1;
	}
	rtc = gpio_direction_input(FAN_OUT);			// GPIO�� �Է����� ���� ���
	if (rtc != 0) {
		printk(KERN_ALERT "FAN pin request fail\n");
		return -1;
	}
	rtc = gpio_direction_output(FAN_OUT, 0);		// GPIO�� ������� ���� ���, 0�̳� 1�� ���� �������־�� ��
	if (rtc != 0) {
		printk(KERN_ALERT "FAN pin request fail\n");
		return -1;
	}

	//rtc = gpio_to_irq(FAN_OUT);					// ��� GPIO ��Ʈ�ѷ��� GPIO �Է� ���� �ٲ�� ���ͷ�Ʈ�� �ɾ��ִµ�, �� IRQ ��ȣ�� ��� ������ �� �Լ��� ����Ѵ�.
	
	//gpio_get_value(FAN_OUT);						// �� �б�
	gpio_set_value(FAN_OUT, 1);						// �� ����
	return 0;
}

static int iom_fan_open(struct inode *inode, struct file *file) {
	if (fan_port_usage) {						// ���� ���� �� 0�� ��ȯ
		return -EBUSY;
	}

	fan_port_usage = 1;
	return 0;
}

static int iom_fan_release(struct inode *inode, struct file *file) {
	fan_port_usage = 0;
	return 0;
}
//
//static ssize_t iom_fan_read(struct file *file, char *buf, size_t count, loff_t *f_pos) {
//	
//	if (buff == 0x0) {
//		gpio_set_value(FAN_OUT, 0);
//	}
//	else {
//		gpio_set_value(FAN_OUT, 1);
//	mdelay(1);
//
//	return 1;
//}

static ssize_t iom_fan_write(struct file *file, const char *buf, size_t count, loff_t *f_pos) {
	if (buff == 0x0) {
		gpio_set_value(FAN_OUT, 0);
	}
	else {
		gpio_set_value(FAN_OUT, 1);
		mdelay(1);

		return 1;
}

// __init : <linux/init.h>, this function is called for OS initialization only
int __init iom_fan_init(void) {
	int result;

	printk(KERN_ALERT "< Fan module is up > \n");
	if ((result = us_register_cdev()) < 0)
	{
		printk(KERN_ALERT "< Fan register fail > \n");
		return result;
	}
	res = gpio_init();

	if (res < 0) return -1;
	
	return 0;
}

void __exit iom_fan_exit(void) {
	printk(KERN_ALERT "< Fan module is down > \n");
	gpio_free(FAN_OUT);
}

module_init(iom_fan_init);
module_exit(iom_fan_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your name");
MODULE_DESCRIPTION("FPGA BUZZER driver");