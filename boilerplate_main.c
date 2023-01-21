#include "boilerplate.h"

/*
 * Compiled with command gcc -Wall -Werror -o libbladeRF_boilerplate boilerplate_main.c boilerplate.c -lbladeRF
 *
 * Usage:
 * libbladeRF_example_boilerplate [serial #]
 *
 * If a serial number is supplied, the program will attempt to open the
 * device with the provided serial number.
 *
 * Otherwise, the first available device will be used.
 */
int main(int argc, char *argv[])
{
    int status, sync;
    struct channel_config ch_config;
    struct bladerf *dev = NULL;
    struct bladerf_devinfo dev_info;

    /* Initialize the information used to identify the desired device
     * to all wildcard (i.e., "any device") values */
    bladerf_init_devinfo(&dev_info);
    /* Request a device with the provided serial number.
     * Invalid strings should simply fail to match a device. */
    if (argc >= 2)
    {
        strncpy(dev_info.serial, argv[1], sizeof(dev_info.serial) - 1);
    }
    status = bladerf_open_with_devinfo(&dev, &dev_info);
    if (status != 0)
    {
        fprintf(stderr, "Unable to open device: %s\n",
                bladerf_strerror(status));
        return 1;
    }

    /* Set up RX channel parameters */
    ch_config.channel = BLADERF_CHANNEL_RX(0);
    ch_config.frequency = 910000000;
    ch_config.bandwidth = 2000000;
    ch_config.samplerate = 300000;
    ch_config.gain = 39;
    status = configure_channel(dev, &ch_config);
    if (status != 0)
    {
        fprintf(stderr, "Failed to configure RX channel. Exiting.\n");
        goto out;
    }

    /* Set up TX channel parameters */
    ch_config.channel = BLADERF_CHANNEL_TX(0);
    ch_config.frequency = 918000000;
    ch_config.bandwidth = 1500000;
    ch_config.samplerate = 250000;
    ch_config.gain = -14;
    status = configure_channel(dev, &ch_config);
    if (status != 0)
    {
        fprintf(stderr, "Failed to configure TX channel. Exiting.\n");
        goto out;
    }

    /* Set up stream parameters.
     * It is important to remember that TX buffers will not be submitted to
     * the hardware until `buffer_size` samples are provided via the
     * bladerf_sync_tx call. Similarly, samples will not be available to
     * RX via bladerf_sync_rx() until a block of `buffer_size` samples has been
     * received.
     */
    const struct stream_config st_config = {.num_buffers = 16,
                                            .buffer_size = 8192,
                                            .num_transfers = 8,
                                            .timeout_ms = 3500,
                                            .samples_len = 10000};
    /* Application code goes here.
     *
     * Don't forget to call bladerf_enable_module() before attempting to
     * transmit or receive samples!
     * bladerf_enable_module() is called in sync_rx()
     */
    sync = sync_rx(dev, &st_config);
    printf("%d\n", sync);

out:
    bladerf_close(dev);
    return status;
}