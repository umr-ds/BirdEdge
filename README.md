### Requirements
#### Hardware
- Jetson Nano
- MicroSD Card
- Micro-USB Power Supply ( 5V⎓2A)
- USB Microphone 
##### Software 
- [Jetson Nano Developer Kit SD Card Image](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#write)
- [DeepStream SDK 6.0](https://developer.nvidia.com/deepstream-getting-started)

### Installation 
Run the following command to compile the application:
  * <code>make -j4</code>


### Usage

- Make sure that <code>alsa-device</code> in the configuration file <code>configs/ds_audio_config.txt</code> 
is correctly set to the connected USB Microphone. The card and device number can be found e.g with <code> arecord -l
  </code> 
- Download the [inference model](https://pc12439.mathematik.uni-marburg.de/nextcloud/s/JPLiB2Jp8CJqxgC) and place it in the <code>./model</code> directory.
- Run the following command to start the pipeline:
  * <code>sudo LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libgomp.so.1 birdedge -c configs/ds_audio_config.txt</code> 