#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>   /* Necessary because we use sysfs */
#include <linux/device.h>

#define sysfs_dir  "tomas"
#define sysfs_file "rtc"

//Converts value received from rtc
//BCD to decimal
static uint8_t FixValue(uint8_t val)
{
  uint8_t temp = 0;
  uint8_t dec = 0;
  temp = val;
  dec = (((temp & 0xF0) >> 4) *10) + (temp & 0x0F);
  return dec;
}

//Method to get RTC
static void GetRTC(uint8_t *h, uint8_t *m, uint8_t *s, uint8_t *d, uint8_t *mon, uint8_t *y)
{
  uint8_t temp = 0;
  //time
  outb_p (0x00, 0x70);  //Select index 0x00 on Port 0x70
  *s = inb_p (0x71);    //Get value
  *s = FixValue(*s);    //Convert from BCD to decimal
  outb_p (0x02, 0x70);
  *m = inb_p (0x71);
  *m = FixValue(*m);
  outb_p (0x04, 0x70);
  *h = inb_p (0x71);
  *h = FixValue(*h);
  //date
  outb_p (0x07, 0x70);
  *d = inb_p (0x71);
  *d = FixValue(*d);
  outb_p (0x08, 0x70);
  *mon = inb_p (0x71);
  *mon = FixValue(*mon);
  outb_p (0x09, 0x70);
  *y = inb_p (0x71);
  *y = FixValue(*y);
}

static ssize_t
sysfs_show(struct device *dev,
           struct device_attribute *attr,
           char *buffer)
{
  //ToDo
    uint8_t h, m, s, d, mon, y;
    printk(KERN_INFO "sysfile_read (/sys/kernel/%s/%s) called\n", sysfs_dir, sysfs_file);
    GetRTC(&h, &m, &s, &d, &mon, &y);
    return sprintf(buffer, "time: %2u:%2u:%2u\ndate: %2u-%2u-%2u\n", h, m, s, d, mon, y);
}

static DEVICE_ATTR(rtc, S_IRUGO, sysfs_show, NULL);

static struct attribute *attrs[] = {
    &dev_attr_rtc.attr,
    NULL   /* need to NULL terminate the list of attributes */
};

static struct kobject *rtc_obj = NULL;

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static int rtc_init(void){
  int result = 0;

  //First we must create our kobject, which is a directory in /sys/...
  rtc_obj = kobject_create_and_add(sysfs_dir, kernel_kobj);
  if (rtc_obj == NULL)
  {
      printk (KERN_INFO "%s module failed to load: kobject_create_and_add failed\n", sysfs_file);
      return -ENOMEM;
  }

  /* now we create all files in the created subdirectory */
  result = sysfs_create_group(rtc_obj, &attr_group);
  if (result != 0)
  {
      /* creating files failed, thus we must remove the created directory! */
      printk (KERN_INFO "%s module failed to load: sysfs_create_group failed with result %d\n", sysfs_file, result);
      kobject_put(rtc_obj);
      return -ENOMEM;
  }

  printk(KERN_INFO "/sys/kernel/%s/%s created\n", sysfs_dir, sysfs_file);
  return result;
}

static void rtc_exit(void){
  kobject_put(rtc_obj);
  printk (KERN_INFO "/sys/kernel/%s/%s removed\n", sysfs_dir, sysfs_file);
}

module_init(rtc_init);
module_exit(rtc_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Tomas Aukstikalnis");
MODULE_DESCRIPTION("sysfs RTC");
