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
	//rtc = gpio_request(FAN_OUT, "FAN");			// GPIO 핀은 allocation 되어야 함
	//assert(rtc == 0, "FAN pin request fail");
	//rtc = gpio_direction_input(FAN_OUT);			// GPIO가 입력으로 사용될 경우
	//assert(rtc == 0, "FAN pin setting fail");
	//rtc = gpio_direction_output(FAN_OUT, 0);		// GPIO가 출력으로 사용될 경우, 0이나 1의 값을 세팅해주어야 함
	//assert(rtc == 0, "FAN pin setting fail");
	//gpio_get_value(FAN_OUT);						// 값 읽기
	//gpio_set_value(FAN_OUT, 1);					// 값 세팅
	//int gpio_to_irq(unsigned int gpio);			// 몇몇 GPIO 컨트롤러는 GPIO 입력 값이 바뀌면 인터럽트를 걸어주는데, 이 IRQ 번호를 얻고 싶으면 이 함수를 사용한다.
	//gpio_free(FAN_OUT);							// GPIO 핀 allocation 해제

	int rtc;

	rtc = gpio_request(FAN_OUT, "FAN");				// GPIO 핀은 allocation 되어야 함
	if (rtc != 0) {
		printk(KERN_ALERT "FAN pin request fail\n");
		return -1;
	}
	rtc = gpio_direction_input(FAN_OUT);			// GPIO가 입력으로 사용될 경우
	if (rtc != 0) {
		printk(KERN_ALERT "FAN pin request fail\n");
		return -1;
	}
	rtc = gpio_direction_output(FAN_OUT, 0);		// GPIO가 출력으로 사용될 경우, 0이나 1의 값을 세팅해주어야 함
	if (rtc != 0) {
		printk(KERN_ALERT "FAN pin request fail\n");
		return -1;
	}

	//rtc = gpio_to_irq(FAN_OUT);					// 몇몇 GPIO 컨트롤러는 GPIO 입력 값이 바뀌면 인터럽트를 걸어주는데, 이 IRQ 번호를 얻고 싶으면 이 함수를 사용한다.
	
	//gpio_get_value(FAN_OUT);						// 값 읽기
	gpio_set_value(FAN_OUT, 1);						// 값 세팅
	return 0;
}

static int iom_fan_open(struct inode *inode, struct file *file) {
	if (fan_port_usage) {						// 정상 종료 시 0을 반환
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