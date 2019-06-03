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
							//		long tv_sec;		// s 단위
							//		long tv_usec;		// us 단위
							// };
struct timeval before;
u32 irq = -1;

static int remote_control_open(struct inode *inode, struct file *filp);
static int remote_control_release(struct inode *inode, struct file *filp);
static int remote_control_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);

struct file_operations remote_control_fops = {
	.open = remote_control_open,
	.release = remote_control_release,
	.read = remote_control_read
};

static irqreturn_t ultrasonics_echo_interrupt(int irq, void *dev_id, struct pt_regs *regs);
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
	int target = gpio_get_value(IR_DATA);		// int gpio_get_value(unsigned int gpio); : 출력 모드 GPIO 핀의 값을 읽어온다.
	unsigned char *p = (char *)&target;

	for (int i = 0; i < 4; i++) {
		buf[i] = *(p + i);
		printk(KERN_ALERT"%c", *(p + i));

	}
	
	printk(KERN_ALERT" Data : %d\n ", target);

	return 0;
}
static int remote_control_register_cdev(void) {
	int error;
	if (remote_control_major) {
		remote_control_dev = MKDEV(remote_control_major, remote_control_minor);		// MKDEV(int major, int minor); : 주번호인 major 와 부번호인 minor 값을 이용하여 dev_t 값을 얻는다.
		error = register_chrdev_region(remote_control_dev, 1, "remote_control");				// int register_chrdev_region(dev_t from, unsigned count, const char *name)
																					// : from의 major 주번호와 minor 부번호를 기준으로 count 만큼 major, minor번호를 할당
	}
	else {
		error = alloc_chrdev_region(&remote_control_dev, remote_control_minor, 1, "remote_control");	// int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
																							// : baseminor 부번호를 기준으로 count 만큼 minor번호를 할당, major 번호는 동적으로 빈곳을 찾아서 리턴, 호출한 쪽에서 첫번째 매개변수 dev_t *dev로 알 수 있다.
		remote_control_major = MAJOR(remote_control_dev);									// MAJOR(dev_t dev); : dev 에서 주번호 값을 얻는다.
	}
	if (error < 0) {
		printk(KERN_WARNING "remote_control: can't get major %d\n", remote_control_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", remote_control_major);
	cdev_init(&remote_control_cdev, &remote_control_fops);			// void cdev_init (	struct cdev * cdev, const struct file_operations * fops); : struct cdev 구조체를 초기화 시켜주는 함수
																	// static struct cdev dasom_cdev = {
	remote_control_cdev.owner = THIS_MODULE;						//	 .owner = THIS_MODULE,
	remote_control_cdev.ops = &remote_control_fops;					//	 .ops = &dasom_fops,
																	// };
	error = cdev_add(&remote_control_cdev, remote_control_dev, 1);	//	int cdev_add(struct cdev * p, dev_t dev, unsigned count); : major, minor 값을 이용해 struct cdev와 dev_t dev를 연결
	if (error)
	{
		printk(KERN_NOTICE "remote_control Register Error %d\n", error);
	}
	return 0;
}
int gpio_init(void) {
	int rtc;
	rtc = gpio_request(IR_DATA, "DATA");					// int gpio_request(unsigned int gpio, const char *label); : GPIO 핀 현재 사용되고 있는 지 확인, 사용 가능하다면 lock을 걸어 점유한다. (있으면 -EBUSY, 없으면 0)
	if (rtc != 0) {
		printk(KERN_ALERT "<Data Pin Request Fail>\n");
		goto fail;
	}
	rtc = gpio_direction_input(IR_DATA);					// int gpio_direction_input(unsigned int gpio); : GPIO 핀을 입력 용도로 지정
	if (rtc != 0) {
		printk(KERN_ALERT "<Echo Pin Setting Fail>\n");
		goto fail;
	}
	return 0;
fail:
	return -1;
}


static int remote_control_init(void) {
	printk(KERN_ALERT "< IR Module is up > \n");
	if ((result = remote_control_register_cdev()) < 0) {						// 커널 연결부
		printk(KERN_ALERT "< IR Register Fail > \n");
		return result;
	}
	res = gpio_init();		// GPIO, ISR 연결 및 세팅
	if (res < 0)
		return -1;
	return 0;
}
static void remote_control_exit(void) {
	printk(KERN_ALERT "< Ultrasonic Module is down > \n");
	free_irq(irq, NULL);			// void free_irq(unsigned int irq, void* dev_id); : 인터럽트 서비스 함수를 해제
	gpio_free(IR_DATA);		// gpio_request()로 점유한 GPIO 핀의 lock을 해제
	cdev_del(&remote_control_cdev);				// cdev_add()로 연결한 cdev 구조체를 지움
	unregister_chrdev_region(remote_control_dev, 1);		// alloc_chrdev_region()로 할당한 major, minor 번호를 해제
}

MODULE_AUTHOR("gunmin, lee <gmlee@huins.com>");
MODULE_DESCRIPTION("HUINS ext sensor Device Driver");
MODULE_LICENSE("GPL");
module_init(remote_control_init);
module_exit(remote_control_exit);