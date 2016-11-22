#include <linux/kernel.h>
#include <linux/linkage.h>
asmlinkage long	sys_simple_add(int number1, int number2, int *result)
{
	printk(KERN_ALERT "Number 1: %d\n", number1);
	printk(KERN_ALERT "Number 2: %d\n", number2);

	*result = number1+number2;

	printk(KERN_ALERT "Result: %d\n", *result);

	return 0;
}
