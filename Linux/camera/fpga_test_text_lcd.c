/* FPGA Push Switch Test Application
File : fpga_test_push.c
Auth : largest@huins.com */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#include "camera.h"
#include "fpga_dot_font.h"

#define MOTOR_ATTRIBUTE_ERROR_RANGE 4
#define FPGA_STEP_MOTOR_DEVICE "/dev/fpga_step_motor"
#define BUZZER_DEVICE "/dev/fpga_buzzer"
#define MAX_DIGIT 4
#define FND_DEVICE "/dev/fpga_fnd"
#define LED_DEVICE "/dev/fpga_led"
#define FPGA_DOT_DEVICE "/dev/fpga_dot"
#define SONIC_DEVICE "/dev/sonic_name"
#define GPIO

#define MAX_BUFF 32
#define LINE_BUFF 16
#define FPGA_TEXT_LCD_DEVICE "/dev/fpga_text_lcd"

#define I2C_DEVICE "/dev/i2c-1"
#define AM2321_ADDR 0x5C


unsigned int j = 0;
static uint16_t
_calc_crc16(const uint8_t *buf, size_t len)
{
        uint16_t crc = 0xFFFF;

        while (len--)
        {
                crc ^= (uint16_t)*buf++;
                for (j = 0; j < 8; j++)
                {
                        if (crc & 0x0001)
                        {
                                crc >>= 1;
                                crc ^= 0xA001;
                        }
                        else
                        {
                                crc >>= 1;
                        }
                }
        }

        return crc;
}

static uint16_t
_combine_bytes(uint8_t msb, uint8_t lsb)
{
        return ((uint16_t)msb << 8) | (uint16_t)lsb;
}

int am2321(float *out_temperature, float *out_humidity)
{
        int fd;
        uint8_t data[8];

        fd = open(I2C_DEVICE, O_RDWR);
        if (fd < 0)
                return 1;

        if (ioctl(fd, I2C_SLAVE, AM2321_ADDR) < 0)
                return 2;

        /* wake AM2320 up, goes to sleep to not warm up and
   * affect the humidity sensor 
   */
        write(fd, NULL, 0);
        usleep(1000); /* at least 0.8ms, at most 3ms */

        /* write at addr 0x03, start reg = 0x00, num regs = 0x04 */
        data[0] = 0x03;
        data[1] = 0x00;
        data[2] = 0x04;
        if (write(fd, data, 3) < 0)
                return 3;

        /* wait for AM2320 */
        usleep(1600); /* Wait atleast 1.5ms */

        /*
   * Read out 8 bytes of data
   * Byte 0: Should be Modbus function code 0x03
   * Byte 1: Should be number of registers to read (0x04)
   * Byte 2: Humidity msb
   * Byte 3: Humidity lsb
   * Byte 4: Temperature msb
   * Byte 5: Temperature lsb
   * Byte 6: CRC lsb byte
   * Byte 7: CRC msb byte
   */
        if (read(fd, data, 8) < 0)
                return 4;

        close(fd);

        //printf("[0x%02x 0x%02x  0x%02x 0x%02x  0x%02x 0x%02x  0x%02x 0x%02x]\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7] );

        /* Check data[0] and data[1] */
        if (data[0] != 0x03 || data[1] != 0x04)
                return 9;

        /* Check CRC */
        uint16_t crcdata = _calc_crc16(data, 6);
        uint16_t crcread = _combine_bytes(data[7], data[6]);
        if (crcdata != crcread)
                return 10;

        uint16_t temp16 = _combine_bytes(data[4], data[5]);
        uint16_t humi16 = _combine_bytes(data[2], data[3]);
        //printf("temp=%u 0x%04x  hum=%u 0x%04x\n", temp16, temp16, humi16, humi16);

        /* Temperature resolution is 16Bit, 
   * temperature highest bit (Bit15) is equal to 1 indicates a
   * negative temperature, the temperature highest bit (Bit15)
   * is equal to 0 indicates a positive temperature; 
   * temperature in addition to the most significant bit (Bit14 ~ Bit0)
   *  indicates the temperature sensor string value.
   * Temperature sensor value is a string of 10 times the
   * actual temperature value.
   */
        if (temp16 & 0x8000)
                temp16 = -(temp16 & 0x7FFF);

        *out_temperature = (float)temp16 / 10.0;
        *out_humidity = (float)humi16 / 10.0;

        return 0;
}

char buff[50];
int main(void)
{
        int fd;

        int dev;
        int dev_1;
        int dev_dot;
        int dev_sonic;
        int dev_lcd;
        int dev_gpio;
        
        int str_size;
        int chk_size;
        unsigned char string[32];

        int motor_action;
        int motor_direction;
        int motor_speed;

        int fnd_speed;

        unsigned char state = 0;
        unsigned char retval;
        unsigned char buzzer_data;
        int data_sonic=0;
        int gpio;

        int dev_2;
        unsigned char retval_1;

        unsigned char motor_state[3];

        unsigned char data_1[4];

        float temp, humi;

        unsigned int ys_count = 0;
        /**************opencv************/
        Mat M(5, 5, CV_8UC3, Scalar(0, 0, 255));
		cout << "M = " << M << endl;

        //add
        //char v4l_device[100] =  "/dev/video0";
        /**************opencv************/
        int fd_v4l;
        int fd_fb;
        struct fb_var_screeninfo fvs;
        user_fb my_fb = {1024, 640, 32};
        int check;
        U32 pixel;
        unsigned short pixel_short;
        unsigned int *pfbdata;
        int offset;
        int rows;
        int cols;
        unsigned int r, g, b, a;
        int i = 0;
        int clear_pixel;
        int posx1, posx2, posy1, posy2;
        int repx, repy;

        int ret = am2321(&temp, &humi);
        if (ret)
        {
                printf("Err=%d\n", ret);
                return ret;
        }

        memset(motor_state, 0, sizeof(motor_state));

        motor_state[0] = (unsigned char)1;
        motor_state[1] = (unsigned char)1;
        motor_state[2] = (unsigned char)10;

        int dev_3;
        unsigned char led = 1;
        dev_3 = open(LED_DEVICE, O_RDWR);
        dev_2 = open(FND_DEVICE, O_RDWR);
        dev_1 = open(BUZZER_DEVICE, O_RDWR);
        dev = open(FPGA_STEP_MOTOR_DEVICE, O_WRONLY);
        fd = open("/dev/fpga_push_switch", O_RDWR);
        dev_dot = open(FPGA_DOT_DEVICE, O_WRONLY);
        fd_fb = open("/dev/fb0", O_RDWR);
        dev_sonic = open(SONIC_DEVICE, O_RDWR); 
        
        dev_gpio = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);
        
        if (dev_dot<0) 
        {
                printf("Device open error : %s\n",FPGA_DOT_DEVICE);
                exit(1);
        }

        if (fd_fb < 0)
        {
                printf("Unable to open %s\n", "/dev/fb0");
        }
        else
        {
                printf("Success to open %s\n", "/dev/fb0");
        }
        if ((check = ioctl(fd_fb, FBIOGET_VSCREENINFO, &fvs)) < 0)
        {
                perror("Get Information Error - VSCREENINFO!");
                exit(1);
        }
        if (dev_3 < 0)
        {
                printf("Device open error : %s\n", LED_DEVICE);
                exit(1);
        }
        if (dev_1 < 0)
        {
                printf("dev_1 open failed \n");
                return -1;
        }

        if (fd < 0)
        {
                printf("fd open failed \n");
                return -1;
        }
        if (dev < 0)
        {
                printf("dev open failed \n");
                return -1;
        }
        printf("/dev/fpga_push_switch opend ....\n");

        if (dev_2 < 0)
        {
                printf("Device open error : %s\n", FND_DEVICE);
                exit(1);
        }
        for (i = 0; i < 4; i++)
        {
                data_1[i] = i;
                printf("%d", data_1[i]);
        }
        fvs.xres = my_fb.xres;
        fvs.yres = my_fb.yres;
        fvs.bits_per_pixel = my_fb.bpps;
        if (check = ioctl(fd_fb, FBIOPUT_VSCREENINFO, &fvs))
        {
                perror("PUT Information Error - VSCREENINFO!");
                exit(1);
        }
        if (fvs.bits_per_pixel != 32)
        {
                perror("Unsupport Mode. 32Bpp Only.");
                exit(1);
        }
        if (lseek(fd_fb, 0, SEEK_SET) < 0)
        { // Set Pixel Map
                perror("LSeek Error.");
                exit(1);
        }

        //framebuffer clear
        clear_pixel = makepixel(0, 0, 0);
        posx1 = 0;
        posx2 = 1024;
        posy1 = 0;
        posy2 = 640;

        for (repy = posy1; repy < posy2; repy++)
        {
                offset = repy * fvs.xres * (32 / 8) + posx1 * (32 / 8);
                if (lseek(fd_fb, offset, SEEK_SET) < 0)
                {
                        perror("LSeek Error!");
                        exit(1);
                }
                for (repx = posx1; repx <= posx2; repx++)
                        write(fd_fb, &pixel, (32 / 8));
        }

        CvCapture *capture;
        Mat frame;
        Mat rgb_frame;

        capture = cvCaptureFromCAM(-1);
        if (capture)
                printf("Success to open webcam\n");

        pfbdata = (unsigned int *)mmap(0, fvs.xres * fvs.yres * (32 / 8), PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
        if ((unsigned)pfbdata == (unsigned)-1)
        {
                perror("Error Mapping!\n");
        }

        while (1)
        {

                usleep(400000);
                memset(buff, '\0', 50);
                read(fd, buff, sizeof(buff));

                for (i = 0; i < 9; i++)
                {
                        printf("[%d]", buff[i]);
                }
                printf("\n");
                
                
                retval = write(dev_3, &led, 1);
                led = led*2;
                if(led == 16)led=1;
                
                if (buff[0] == 1)
                {
                        printf("Start!!\n");
                        while (buff[8]==0)
                        {
                                
                                usleep(400000);
                                memset(buff, '\0', 50);
                                read(fd, buff, sizeof(buff));
                                read(dev_sonic, &data_sonic, 1);
                                read(dev_gpio, &gpio, 1);
                                printf("%d",gpio);
                                

                                

                                for (i = 0; i < 9; i++)
                                {
                                        printf("[%d]", buff[i]);
                                }
                                printf("\n");
                                ret = am2321(&temp, &humi);
                                printf("Temperature %.1f [C]\n", temp);
                                printf("Humidity    %.1f [%%]\n", humi);
                                usleep(100000);

                                
                               





                                if (buff[0] == 1)
                                {

                                        
                                        
                                }

                                if (buff[1] == 1)
                                {
                                        
                                        led = 8;
                                        retval = write(dev_3, &led, 1);
                                        led = 0;
                                        retval = read(dev_3, &led, 1);
                                }

                                if (buff[2] == 1)
                                {
                                }
                                        
                                        
                                if (buff[3] == 1)
                                        motor_state[1] = 0;
                                /********* 온습도센서에 따른 if문 ********/ 
                                if (humi > 80)
                                {

                                        state++;
                                        buzzer_data = 1;
                                        retval = write(dev_1, &buzzer_data, 1);
                                        if (retval < 0)
                                        {
                                                printf("Write Error!\n");
                                                return -1;
                                        }
                                }
                                else
                                {
                                        //state=1;
                                        buzzer_data = 0;
                                        retval = write(dev_1, &buzzer_data, 1);
                                        if (retval < 0)
                                        {
                                                printf("Write Error!\n");
                                                return -1;
                                        }
                                }
                                /********* 초음파 센서에 따른 if문 ********/ 

                                if(data_sonic > 5 && data_sonic < 7)
                                {
                                        motor_state[0] = 1; //motor start
                                        motor_state[1] = 1;
                                        motor_state[2] = 3;
                                        write(dev, motor_state, 3);
                                        str_size=sizeof(fpga_number[6]);
                                        write(dev_dot,fpga_number[6],str_size);
                                        
                                }
                                else if(data_sonic > 7 && data_sonic < 15)
                                {
                                        motor_state[0] = 1; //motor start
                                        motor_state[1] = 1;
                                        motor_state[2] = 10;
                                        write(dev, motor_state, 3);
                                        str_size=sizeof(fpga_number[4]);
                                        write(dev_dot,fpga_number[4],str_size);
                                        
                                }

                                else if(data_sonic > 15 && data_sonic < 20)
                                {
                                        motor_state[0] = 1; //motor start
                                        motor_state[1] = 1;
                                        motor_state[2] = 60;
                                        write(dev, motor_state, 3);
                                        str_size=sizeof(fpga_number[3]);
                                        write(dev_dot,fpga_number[3],str_size);
                                        
                                }
                                else if(data_sonic > 20 && data_sonic <25)
                                {
                                        motor_state[0] = 1; //motor start
                                        motor_state[1] = 1;
                                        motor_state[2] = 120;
                                        write(dev, motor_state, 3);
                                        str_size=sizeof(fpga_number[2]);
                                        write(dev_dot,fpga_number[2],str_size);
                                        
                                }
                                else if(data_sonic > 25)
                                {
                                        motor_state[0] = 0; //motor Stop
                                        motor_state[1] = 1;
                                        motor_state[2] = 120;
                                        write(dev, motor_state, 3);
          
                                        str_size=sizeof(fpga_number[0]);
                                        write(dev_dot,fpga_number[0],str_size);
                                        
                                }


                          
                                if (buff[5] == 1)
                                {
                                        write(dev, motor_state, 3);
                                }

                                if (buff[6] == 1)
                                {
                                        while (ys_count < 30)
                                        {
                                                frame = cvQueryFrame(capture);
                                                if (frame.empty())
                                                {
                                                        printf("webcam data frame error\n");
                                                }

                                                //memory mapping method -> fast, not use system call 'lseek'..

                                                for (i = 0; i < frame.rows; i++)
                                                {
                                                        offset = i * fvs.xres;
                                                        for (j = 0; j < frame.cols; j++)
                                                        {
                                                                r = frame.at<Vec3b>(i, j)[0];
                                                                g = frame.at<Vec3b>(i, j)[1];
                                                                b = frame.at<Vec3b>(i, j)[2];
                                                                pixel = makepixel(r, g, b);
                                                                //				printf("pixel=%d, r=%d, g=%d, b=%d\n",pixel,r,g,b);
                                                                *(pfbdata + offset + j) = pixel;
                                                        }
                                                }
                                                usleep(1);
                                                printf(" Ys count = %d\n", ys_count);
                                                ys_count++;
                                        }
                                        ys_count = 0;
                                }

                                
                                char temp_set[32];
                                                                
         
                                

                                sprintf(temp_set, "%.1f", humi);
                                retval_1 = write(dev_2, &temp_set, 4);
                                if (retval_1 < 0)
                                {
                                        printf("Read Error!\n");
                                        return -1;
                                }

                                retval_1 = read(dev_2, &temp_set, 4);
                                if (retval_1 < 0)
                                {
                                        printf("Read Error!\n");
                                        return -1;
                                }


                        }
                        printf("Stop bye~bye~\n");
                        break;

                }
        }

        close(dev);
        close(fd);
        close(dev_1);
        close(dev_2);
        close(dev_3);
        close(fd_fb);
        close(dev_dot);
        printf("/dev/fpga_push_switch...closed....\n");
        return 0;
}

