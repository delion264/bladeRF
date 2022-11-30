/* Save to a file, e.g. boilerplate.c, and then compile:
 * $ gcc boilerplate.c -o libbladeRF_example_boilerplate -lbladeRF
 */
#include "boilerplate.h"

/* The RX and TX channels are configured independently for these parameters */

int configure_channel(struct bladerf *dev, struct channel_config *c)
{
    int status;
    status = bladerf_set_frequency(dev, c->channel, c->frequency);
    if (status != 0)
    {
        fprintf(stderr, "Failed to set frequency = %u: %s\n", c->frequency,
                bladerf_strerror(status));
        return status;
    }
    status = bladerf_set_sample_rate(dev, c->channel, c->samplerate, NULL);
    if (status != 0)
    {
        fprintf(stderr, "Failed to set samplerate = %u: %s\n", c->samplerate,
                bladerf_strerror(status));
        return status;
    }
    status = bladerf_set_bandwidth(dev, c->channel, c->bandwidth, NULL);
    if (status != 0)
    {
        fprintf(stderr, "Failed to set bandwidth = %u: %s\n", c->bandwidth,
                bladerf_strerror(status));
        return status;
    }
    status = bladerf_set_gain(dev, c->channel, c->gain);
    if (status != 0)
    {
        fprintf(stderr, "Failed to set gain: %s\n", bladerf_strerror(status));
        return status;
    }
    return status;
}

static int sync_config_rx(struct bladerf *dev, struct stream_config *s)
{
    int status;
    /* These items configure the underlying asynch stream used by the sync
     * interface. The "buffer" here refers to those used internally by worker
     * threads, not the user's sample buffers.
     */

    /* Configure both the device's x1 RX and TX channels for use with the
     * synchronous
     * interface. SC16 Q11 samples *without* metadata are used. */
    status = bladerf_sync_config(dev, BLADERF_RX_X1, s->format,
                                 s->num_buffers, s->buffer_size, s->num_transfers,
                                 s->timeout_ms);
    if (status != 0)
    {
        fprintf(stderr, "Failed to configure RX sync interface: %s\n",
                bladerf_strerror(status));
        return status;
    }
}

static int sync_config_tx(struct bladerf *dev, struct stream_config *s)
{
    int status;
    /* These items configure the underlying asynch stream used by the sync
     * interface. The "buffer" here refers to those used internally by worker
     * threads, not the user's sample buffers.
     */

    /* Configure both the device's x1 RX and TX channels for use with the
     * synchronous
     * interface. SC16 Q11 samples *without* metadata are used. */

    status = bladerf_sync_config(dev, BLADERF_TX_X1, s->format,
                                 s->num_buffers, s->buffer_size, s->num_transfers,
                                 s->timeout_ms);
    if (status != 0)
    {
        fprintf(stderr, "Failed to configure TX sync interface: %s\n",
                bladerf_strerror(status));
    }
    return status;
}

int sync_rx(struct bladerf *dev, struct stream_config *s)
{
    int status, ret;
    bool done = false;
    bool flag_meta = (s->format = BLADERF_FORMAT_SC16_Q11) ? false : true;
    /* "User" samples buffers and their associated sizes, in units of samples.
     * Recall that one sample = two int16_t values. */
    int16_t *rx_samples = NULL;
    const unsigned int samples_len = 10000; /* May be any (reasonable) size */
    /* Allocate a buffer to store received samples in */
    rx_samples = malloc(samples_len * 2 * 1 * sizeof(int16_t));
    if (rx_samples == NULL)
    {
        perror("malloc");
        return BLADERF_ERR_MEM;
    }

    /* Initialize synch interface on RX and TX */
    status = sync_config_rx(dev, s); /* Calls bladerf_sync_config() */
    if (status != 0)
    {
        goto out;
    }
    status = bladerf_enable_module(dev, BLADERF_RX, true);
    if (status != 0)
    {
        fprintf(stderr, "Failed to enable RX: %s\n", bladerf_strerror(status));
        goto out;
    }
    while (status == 0 && !done)
    {
        /* Receive samples */
        status = bladerf_sync_rx(dev, rx_samples, samples_len, NULL, 5000);
        if (status != 0)
        {
            fprintf(stderr, "Failed to RX samples: %s\n",
                    bladerf_strerror(status));
        }
    }
out:
    ret = status;

    /* Disable RX, shutting down our underlying RX stream */
    status = bladerf_enable_module(dev, BLADERF_RX, false);
    if (status != 0)
    {
        fprintf(stderr, "Failed to disable RX: %s\n", bladerf_strerror(status));
    }
    free(rx_samples);
    return ret;
}

int sync_tx(struct bladerf *dev, struct stream_config *s)
{
    int status, ret;
    // bool done = false;
    bool have_tx_data = false;
    bool flag_meta = (s->format = BLADERF_FORMAT_SC16_Q11) ? false : true;

    /* "User" samples buffers and their associated sizes, in units of samples.
     * Recall that one sample = two int16_t values. */
    int16_t *tx_samples = NULL;
    const unsigned int samples_len = 10000; /* May be any (reasonable) size */

    /* Allocate a buffer to prepare transmit data in */
    tx_samples = malloc(samples_len * 2 * 1 * sizeof(int16_t));
    if (tx_samples == NULL)
    {
        perror("malloc");
        return BLADERF_ERR_MEM;
    }

    /* Initialize synch interface on TX */
    status = sync_config_tx(dev, s); /* Calls bladerf_sync_config() */
    if (status != 0)
    {
        goto out;
    }

    status = bladerf_enable_module(dev, BLADERF_TX, true);
    if (status != 0)
    {
        fprintf(stderr, "Failed to enable TX: %s\n", bladerf_strerror(status));
        goto out;
    }

    /* Process these samples, and potentially produce a response
     * to transmit */
    /* done = do_work(rx_samples, samples_len, &have_tx_data, tx_samples,
                    samples_len); */
    if (have_tx_data)
    {
        /* Transmit */
        status = bladerf_sync_tx(dev, tx_samples, samples_len, NULL, 5000);
        if (status != 0)
        {
            fprintf(stderr, "Failed to TX samples: %s\n",
                    bladerf_strerror(status));
        }
    }
    if (status == 0)
    {
        /* Wait a few seconds for any remaining TX samples to finish
         * reaching the RF front-end */
        usleep(2000000);
    }

out:
    ret = status;

    /* Disable TX, shutting down our underlying TX stream */
    status = bladerf_enable_module(dev, BLADERF_TX, false);
    if (status != 0)
    {
        fprintf(stderr, "Failed to disable TX: %s\n", bladerf_strerror(status));
    }
    /* Free up our resources */
    free(tx_samples);
    return ret;
}