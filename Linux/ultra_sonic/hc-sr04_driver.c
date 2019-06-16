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
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

#define HCSR04_TRIGGER IMX_GPIO_NR(2, 2)
#define HCSR04_ECHO IMX_GPIO_NR(2, 3)

int inner_distace = 0;
static int us_major = 0;
static int us_minor = 0;
static int result;
static int res;
static dev_t us_dev;
static struct cdev us_cdev;
struct timeval after;		// struct timeval {
							//		long tv_sec;		// s 단위
							//		long tv_usec;		// us 단위
							// };
struct timeval before;
u32 irq = -1;

static int us_open(struct inode *inode, struct file *filp);
static int us_release(struct inode *inode, struct file *filp);
static int us_read(struct file *filp, int *buf, size_t count, loff_t *f_pos);

struct file_operations us_fops = {
	.open = us_open,
	.release = us_release,
	.read = us_read
};

static irqreturn_t ultrasonics_echo_interrupt(int irq, void *dev_id, struct pt_regs *regs);
static int us_register_cdev(void);
void output_sonicburst(void);
int gpio_init(void);


static int us_open(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "< Device has been opened >\n");
	return 0;
}
static int us_release(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "< Device has been closed > \n");
	return 0;
}
static int us_read(struct file *filp, const int *buf, size_t count, loff_t *f_pos){
	output_sonicburst();
	
	if (copy_to_user(buf, &inner_distace, 1)) {
		return -EFAULT;
	}

	mdelay(1);
	return 0;
}

static irqreturn_t ultrasonics_echo_interrupt(int irq, void *dev_id, struct pt_regs *regs){
	if (gpio_get_value(HCSR04_ECHO)){		// int gpio_get_value(unsigned int gpio); : 출력 모드 GPIO 핀의 값을 읽어온다.
		do_gettimeofday(&before);			// do_gettimeofday(struct timeval *tv) : 초 / 마이크로초 단위의 절대시간을 얻어옴
	}
	else{
		do_gettimeofday(&after);
		printk(KERN_ALERT" Distance : %.0ld [cm] \n ", (after.tv_usec - before.tv_usec) / 58);		// 단위변환, us/58 = Centimeter
		inner_distace = (after.tv_usec - before.tv_usec) / 58;
		memset(&before, 0, sizeof(struct timeval));
		memset(&after, 0, sizeof(struct timeval));
	}
	return IRQ_HANDLED;		// 올바른 경우 IRQ_HANDLED를 관련 없는 경우 IRQ_NONE를 리턴
}
static int us_register_cdev(void){
	int error;
	if (us_major) {
		us_dev = MKDEV(us_major, us_minor);		// MKDEV(int major, int minor); : 주번호인 major 와 부번호인 minor 값을 이용하여 dev_t 값을 얻는다.
		error = register_chrdev_region(us_dev, 1, "us");	// int register_chrdev_region(dev_t from, unsigned count, const char *name)
															// : from의 major 주번호와 minor 부번호를 기준으로 count 만큼 major, minor번호를 할당
	}
	else {
		error = alloc_chrdev_region(&us_dev, us_minor, 1, "us");	// int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
																	// : baseminor 부번호를 기준으로 count 만큼 minor번호를 할당, major 번호는 동적으로 빈곳을 찾아서 리턴, 호출한 쪽에서 첫번째 매개변수 dev_t *dev로 알 수 있다.
		us_major = MAJOR(us_dev);				// MAJOR(dev_t dev); : dev 에서 주번호 값을 얻는다.
	}
	if (error < 0) {
		printk(KERN_WARNING "us: can't get major %d\n", us_major);
		return result;
	}
	printk(KERN_ALERT "major number = %d\n", us_major);
	cdev_init(&us_cdev, &us_fops);		// void cdev_init (	struct cdev * cdev, const struct file_operations * fops); : struct cdev 구조체를 초기화 시켜주는 함수
										// static struct cdev dasom_cdev = {
	us_cdev.owner = THIS_MODULE;		//	 .owner = THIS_MODULE,
	us_cdev.ops = &us_fops;				//	 .ops = &dasom_fops,
										// };
	error = cdev_add(&us_cdev, us_dev, 1);	//	int cdev_add(struct cdev * p, dev_t dev, unsigned count); : major, minor 값을 이용해 struct cdev와 dev_t dev를 연결
	if (error)
	{
		printk(KERN_NOTICE "us Register Error %d\n", error);
	}
	return 0;
}
void output_sonicburst(void){
	gpio_set_value(HCSR04_TRIGGER, 1);			// // void gpio_set_value(unsigned int gpio, int value); : 출력 모드 GPIO 핀에 값을 쓴다.
	udelay(10);
	gpio_set_value(HCSR04_TRIGGER, 0);
}
int gpio_init(void){
	int rtc;
	rtc = gpio_request(HCSR04_TRIGGER, "TRIGGER");				// int gpio_request(unsigned int gpio, const char *label); : GPIO 핀 현재 사용되고 있는 지 확인, 사용 가능하다면 lock을 걸어 점유한다. (있으면 -EBUSY, 없으면 0)
	if (rtc != 0) {
		printk(KERN_ALERT "<Trigger Pin Request Fail>\n");
		goto fail;
	}
	rtc = gpio_request(HCSR04_ECHO, "ECHO");
	if (rtc != 0) {
		printk(KERN_ALERT "<Echo Pin Request Fail>\n");
		goto fail;
	}
	rtc = gpio_direction_output(HCSR04_TRIGGER, 0);				// int gpio_direction_output(unsigned int gpio, int value); : GPIO 핀을 출력 용도로 지정, 초기 출력을 세팅해야함
	if (rtc != 0) {
		printk(KERN_ALERT "<Trigget pin Setting Fail>\n");
		goto fail;
	}
	rtc = gpio_direction_input(HCSR04_ECHO);					// int gpio_direction_input(unsigned int gpio); : GPIO 핀을 입력 용도로 지정
	if (rtc != 0) {
		printk(KERN_ALERT "<Echo Pin Setting Fail>\n");
		goto fail;
	}
	rtc = gpio_to_irq(HCSR04_ECHO);								// int gpio_to_irq(unsigned int gpio); : gpio에 해당하는 interrupt 주소를 리턴해주는 함수
	if (rtc < 0) {
		printk(KERN_ALERT "<irq Pin GPIO Request Fail>\n");
		goto fail;
	}
	else {
		irq = rtc;
	}
	rtc = request_irq(irq, (void*)ultrasonics_echo_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED, "us", NULL);
	// int request_irq(unsigned int irq, irqreturn_t (*handler)(int, void*, struct pt_regs*), unsigned long frags, const char* device, void* dev_id); : 인터럽트 서비스 함수 등록
	if (rtc) {
		printk(KERN_ALERT "<irq Register Fail>\n");
		goto fail;
	}
	printk(KERN_INFO "HC-SR04 Enable\n");
	gpio_set_value(HCSR04_TRIGGER, 0);				// void gpio_set_value(unsigned int gpio, int value); : 출력 모드 GPIO 핀에 값을 쓴다.
	return 0;
fail:
	return -1;
}


static int us_init(void) {
	printk(KERN_ALERT "< Ultrasonic Module is up > \n");
	if ((result = us_register_cdev()) < 0){						// 커널 연결부
		printk(KERN_ALERT "< Ultrasonic Register Fail > \n");
		return result;
	}
	res = gpio_init();		// GPIO, ISR 연결 및 세팅
	if (res < 0)
		return -1;
	return 0;
}
static void us_exit(void) {
	printk(KERN_ALERT "< Ultrasonic Module is down > \n");
	free_irq(irq, NULL);			// void free_irq(unsigned int irq, void* dev_id); : 인터럽트 서비스 함수를 해제
	gpio_free(HCSR04_TRIGGER);		// gpio_request()로 점유한 GPIO 핀의 lock을 해제
	gpio_free(HCSR04_ECHO);
	cdev_del(&us_cdev);				// cdev_add()로 연결한 cdev 구조체를 지움
	unregister_chrdev_region(us_dev, 1);		// alloc_chrdev_region()로 할당한 major, minor 번호를 해제
}

MODULE_AUTHOR("gunmin, lee <gmlee@huins.com>");
MODULE_DESCRIPTION("HUINS ext sensor Device Driver");
MODULE_LICENSE("GPL");
module_init(us_init);
module_exit(us_exit);