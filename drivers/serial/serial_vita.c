#include <dm.h>
#include <debug_uart.h>
#include <errno.h>
#include <malloc.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/types.h>

#define VITA_UART_STATUS 0x28
#define VITA_UART_READ_CNT 0x68
#define VITA_UART_WRITE_FIFO 0x70
#define VITA_UART_READ_FIFO 0x78

#define VITA_UART_STATUS_TXREADY (1 << 8)
#define VITA_UART_STATUS_DEVREADY (1 << 9)

#define VITA_UART_READ_CNT_MASK (0x3F)

#define VITA_UART_MAX_PORTS 7

struct vita_serial_priv {
	void __iomem *base;
};

static int vita_serial_pending(struct udevice *dev, bool input)
{
	struct vita_serial_priv *priv = dev_get_priv(dev);

	return readl(priv->base + VITA_UART_READ_CNT) & VITA_UART_READ_CNT_MASK;
}

static int vita_serial_setbrg(struct udevice *dev, int baudrate)
{
	return 0;
}

static int vita_serial_putc(struct udevice *dev, const char ch)
{
	struct vita_serial_priv *priv = dev_get_priv(dev);

    while(!(readl(priv->base + VITA_UART_STATUS) & VITA_UART_STATUS_TXREADY))
        {}; // Do something about this later. Probably a NOP alternative.

	writel(ch, priv->base + VITA_UART_WRITE_FIFO);

	return 0;
}

static int vita_serial_getc(struct udevice *dev)
{
	struct vita_serial_priv *priv = dev_get_priv(dev);
	u32 val;

	val = readl(priv->base + VITA_UART_READ_CNT);
	if ((val & VITA_UART_READ_CNT_MASK) > 0) {
        val = readl(priv->base + VITA_UART_READ_FIFO);
    }
    else {
        val = -EAGAIN;
    }
	return val;
}

static int vita_serial_probe(struct udevice *dev)
{
	struct vita_serial_priv *priv = dev_get_priv(dev);

	/* get address */
	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	/* initialize serial */
	return 0; // Should already be initialized (bcm6345_serial_init(priv->base, priv->uartclk, CONFIG_BAUDRATE);)
}

static const struct dm_serial_ops vita_serial_ops = {
	.putc = vita_serial_putc,
	.pending = vita_serial_pending,
	.getc = vita_serial_getc,
	.setbrg = vita_serial_setbrg,
};

static const struct udevice_id vita_serial_ids[] = {
	{ .compatible = "sony,vita-uart" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(vita_serial) = {
	.name = "vita-uart",
	.id = UCLASS_SERIAL,
	.of_match = vita_serial_ids,
	.probe = vita_serial_probe,
	.priv_auto	= sizeof(struct vita_serial_priv),
	.ops = &vita_serial_ops,
};