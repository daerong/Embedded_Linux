#include "../include/fpga_driver.h"

static int step_motor_port_usage = 0;
static unsigned char *iom_fpga_step_motor_addr;	 // 가상 주소를 저장할 변수

static ssize_t iom_step_motor_write(struct file *file, const char *buf, size_t count, loff_t *f_pos);
//static ssize_t iom_step_motor_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
static int iom_step_motor_open(struct inode *inode, struct file *file);
static int iom_step_motor_release(struct inode *inode, struct file *file);

// write() 만 구현
struct file_operations iom_step_motor_fops = {
	.owner = THIS_MODULE,
	.open = iom_step_motor_open,
	.write = iom_step_motor_write,
	//.read = iom_step_motor_read,
	.release = iom_step_motor_release
};

static int iom_step_motor_open(struct inode *inode, struct file *file) {
	if (step_motor_port_usage) {				// 정상 종료 시 0을 반환
		return -EBUSY;
	}

	step_motor_port_usage = 1;
	return 0;
}

static int iom_step_motor_release(struct inode *inode, struct file *file) {
	step_motor_port_usage = 0;
	return 0;
}

static ssize_t iom_step_motor_write(struct file *file, const char *buf, size_t count, loff_t *f_pos) {
	unsigned char value[3];
	unsigned short _s_value = 0;

	if (count != 3) {							// 정상 종료 시 0을 반환
		return -EFAULT;
	}

	if (copy_from_user(&value, buf, count)) {	// 정상 종료 시 0을 반환
		return -EFAULT;
	}

	_s_value = value[0] & 0xF;
	outw(_s_value, (unsigned int)iom_fpga_step_motor_addr);
	_s_value = value[1] & 0xF;
	outw(_s_value, (unsigned int)iom_fpga_step_motor_addr + 2);
	_s_value = value[2] & 0xFF;
	outw(_s_value, (unsigned int)iom_fpga_step_motor_addr + 4);
	
	return count;
}

// __init : <linux/init.h>, this function is called for OS initialization only
int __init iom_step_motor_init(void) {
	int result = register_chrdev(IOM_STEP_MOTOR_MAJOR, IOM_STEP_MOTOR_NAME, &iom_step_motor_fops);

	if (result < 0) {
		printk(KERN_WARNING "Can't get any major number\n");
		return result;
	}

	// 물리 주소를 가상 주소에 mapping한다.
	iom_fpga_step_motor_addr = ioremap(IOM_STEP_MOTOR_ADDRESS, 0x6); // 사용할 메모리 크기
	printk("init module %s, major number: %d\n", IOM_STEP_MOTOR_NAME, IOM_STEP_MOTOR_MAJOR);
	
	return 0;
}

void __exit iom_step_motor_exit(void) {
	iounmap(iom_fpga_step_motor_addr);
	unregister_chrdev(IOM_STEP_MOTOR_MAJOR, IOM_STEP_MOTOR_NAME);
}

module_init(iom_step_motor_init);
module_exit(iom_step_motor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your name");
MODULE_DESCRIPTION("FPGA STEP MOTOR driver");