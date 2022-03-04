# Bird@Edge
## Bird Song Recognition at the Edge
### Requirements
#### Hardware
- [Jetson Nano Developer Kit](https://developer.nvidia.com/embedded/jetson-nano-developer-kit)
- MicroSD Card
- Micro-USB Power Supply (5V⎓2A)

##### Software 
- [Jetson Nano Developer Kit SD Card Image](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#write)
- [DeepStream SDK 6.0](https://developer.nvidia.com/deepstream-getting-started)

### Installation 
Run the following command to compile the application:
  * ```make -j4```


### Usage

- Make sure that ```alsa-device``` in the configuration file ```configs/ds_audio_config.txt```
is correctly set to the connected USB Microphone. The card and device number can be found e.g with ```arecord -l```
- Download the [inference model](https://pc12439.mathematik.uni-marburg.de/nextcloud/s/jfANCLCJR9jNQ8k) and place it in the ```./model``` directory.
- Set the environment variables:
  * ```LD_LIBRARY_PATH=/opt/nvidia/deepstream/deepstream-6.0/lib/gst-plugins/```
  * ```LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libgomp.so.1```
- Run the following command to start the pipeline:
  * ```./birdedge -c configs/ds_audio_config.txt --gst-debug=1```
- The output format is as follows:
  * ```{"frame_num": %d, "timestamp": %ld, "label": %s, "source_id": %d, "confidence": %f}```
