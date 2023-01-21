#include <libbladeRF.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_FREQUENCY 800e6
#define MAX_FREQUENCY 900e6
#define BANDWIDTH 5e6
#define SAMPLERATE 12e6

int main(int argc, char *argv[])
{
    int status;
    struct bladerf *dev;
    struct bladerf_range range = {.max = MAX_FREQUENCY,
                                  .min = MIN_FREQUENCY,
                                  .step = BANDWIDTH,
                                  .scale = 1};
    bladerf_frequency freq; // frequency of the device
    float frequency;        // counter variable
    bladerf_channel ch = BLADERF_RX_X1;
    int32_t *preamble = NULL;
    int32_t *symbol = NULL;

    const struct bladerf_range *range_ptr = &range;

    printf("Segfault after assigning range_ptr to range\n");

    status = bladerf_open(&dev, NULL);
    if (status != 0)
    {
        printf("Failed to open device: %s\n", bladerf_strerror(status));
        return 1;
    }

    status = bladerf_set_sample_rate(dev, BLADERF_MODULE_RX, SAMPLERATE, NULL);
    if (status != 0)
    {
        printf("Failed to set sample rate: %s\n", bladerf_strerror(status));
        goto out;
    }

    status = bladerf_set_bandwidth(dev, BLADERF_MODULE_RX, BANDWIDTH, NULL);
    if (status != 0)
    {
        printf("Failed to set bandwidth: %s\n", bladerf_strerror(status));
        goto out;
    }

    status = bladerf_get_frequency_range(dev, BLADERF_MODULE_RX, &range_ptr);
    if (status != 0)
    {
        printf("Failed to get frequency range: %s\n", bladerf_strerror(status));
        goto out;
    }

    printf("Scanning frequency band from %g to %g MHz...\n", MIN_FREQUENCY / 1e6, MAX_FREQUENCY / 1e6);

    for (frequency = MIN_FREQUENCY; frequency <= MAX_FREQUENCY; frequency += BANDWIDTH)
    {
        status = bladerf_set_frequency(dev, BLADERF_MODULE_RX, (uint32_t)frequency);
        if (status != 0)
        {
            printf("Failed to set frequency: %s\n", bladerf_strerror(status));
            goto out;
        }

        status = bladerf_get_frequency(dev, BLADERF_MODULE_RX, &freq);
        if (status != 0)
        {
            printf("Failed to get frequency: %s\n", bladerf_strerror(status));
            goto out;
        }

        if (freq > 0)
        {
            printf("Transmitting frequency found: %g MHz\n", (float)freq / 1e6);
            // printf("%d\n", bladerf_get_rfic_rssi(dev, ch, preamble, symbol));
        }
    }

out:
    bladerf_close(dev);
    return 0;
}