dev = bladeRF_MIMO;

dev.rx.frequency = 2.4e6;
dev.rx.samplerate = 10e6;
dev.rx.bandwidth = 5e6;
dev.rx.lna = 'MAX';
dev.rx.vga1 = 30;
dev.rx.vga2 = 5;

dev.rx.config.num_buffers = 64;
dev.rx.config.buffer_size = 16384;
dev.rx.config.num_transfers = 16;
dev.rx.config.timeout_ms = 5000;

samples = zeros(num_samples, 1);
fft_data = zeros(num_samples, 1);
history = zeros(num_samples, 1);

run = 1;
dev.rx.start();

while run == 1
    [samples, timestamp_out, actual_count, overrun] = dev.receive(0.250*dev.rx.samplerate);
end

dev.rx.stop();
clear dev;