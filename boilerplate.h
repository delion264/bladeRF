#ifndef BOILERPLATE_H
#define BOILERPLATE_H

#include <libbladeRF.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct channel_config
{
    bladerf_channel channel;
    unsigned int frequency;
    unsigned int bandwidth;
    unsigned int samplerate;
    int gain;
};

struct stream_config
{
    unsigned int num_buffers;
    unsigned int buffer_size; /* Must be a multiple of 1024 */
    unsigned int num_transfers;
    unsigned int timeout_ms;
    unsigned int samples_len;
    bladerf_format format;
    bool schedule;
};

int configure_channel(struct bladerf *dev, struct channel_config *c);
int sync_config_rx(struct bladerf *dev, const struct stream_config *s); // formerly static int sync_config_rx()
int sync_config_tx(struct bladerf *dev, const struct stream_config *s); // formerly static int sync_config_tx()
int sync_rx(struct bladerf *dev, const struct stream_config *s);
int sync_tx(struct bladerf *dev, const struct stream_config *s);

#endif