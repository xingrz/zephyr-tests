#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define SW0_NODE DT_ALIAS(sw0)

K_SEM_DEFINE(sem_press, 0, 1);

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback button_cb_data;

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
	k_sem_give(&sem_press);
}

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&button) || !gpio_is_ready_dt(&led)) {
		printk("Error: GPIO device is not ready\n");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n", ret, button.port->name,
		       button.pin);
		return ret;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n", ret, led.port->name, led.pin);
		return ret;
	}

	while (1) {
		printk("Press the button on %s to toggle the LED on %s\n", button.port->name,
		       led.port->name);

		gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
		k_sem_take(&sem_press, K_FOREVER);

		gpio_pin_interrupt_configure_dt(&button, GPIO_INT_DISABLE);
		gpio_pin_toggle_dt(&led);

		k_msleep(1000);
	}
}
