# sonic test
This gui is used to test the `pcm` returned by the Xiaozhi device through `udp` and convert it to the time domain/frequency domain. It can save the sound of the window length and is used to determine the noise frequency distribution and test the accuracy of sound wave transmission ascii.

Firmware testing needs to open `USE_AUDIO_DEBUGGER` and set `AUDIO_DEBUG_UDP_SERVER` to the local address.
Sonic `demod` can be used to output the sonic test through `sonic_wifi_config.html` or uploaded to `PinMe`’s [小智 Sonic Configuration Network] (https://iqf7jnhi.pinit.eth.limo)

# Sonic decoding test record

> `✓` means that the decoding can be successful when I2S DIN receives the original PCM signal, `△` means that noise reduction or additional operations are required to achieve stable decoding, `X` means that the effect after noise reduction is not good (it may be partially decoded but very unstable).
> Some ADCs require more detailed noise reduction adjustments during the I2C configuration stage. Since the equipment is not universal, we can only test according to the config provided in the boards.
| Equipment | ADC | MIC | Effect | Remarks |
| ----| ----| ---| ---| ----|
| bread-compact | INMP441 | integrated MEMEMIC | ✓ |
| atk-dnesp32s3-box | ES8311 | | ✓ |
| magiclick-2p5 | ES8311 | | ✓ |
| lichuang-dev | ES7210 | | △ | You need to turn off INPUT_REFERENCE during testing
| kevin-box-2 | ES7210 | | △ | INPUT_REFERENCE needs to be turned off during testing
| m5stack-core-s3 | ES7210 | | △ | INPUT_REFERENCE needs to be turned off during testing
| xmini-c3 | ES8311 | | △ | Noise reduction required
| atoms3r-echo-base | ES8311 | | △ | Noise reduction required
| atk-dnesp32s3-box0 | ES8311 | |
| movecall-moji-esp32s3 | ES8311 | |