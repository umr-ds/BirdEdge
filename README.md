# Bird@Edge Core

Bird@Edge Core is the software running on NVIDIA Jetson Nano devices for birdsong recognition in soundscape recordings.

## Bird Song Recognition at the Edge

Bird@Edge is an Edge AI system for recognizing bird species in audio recordings to support real-time biodiversity monitoring. Bird@Edge is based on embedded edge devices operating in a distributed system to enable efficient, continuous evaluation of soundscapes recorded in forests. If you are interested in our research, [read our paper](https://jonashoechst.de/assets/papers/hoechst2022birdedge.pdf) or watch the talk recorded for [Springer NETYS 2022 conference](https://www.youtube.com/watch?v=reAm4HSyQl8). 

Separate repositories exist for the code running on [Bird@Edge Mic](https://github.com/umr-ds/BirdEdge-Mic) and for the operating system image [Bird@Edge OS](https://github.com/umr-ds/BirdEdge-OS) created for running on the Bird@Edge Stations.

## Requirements
### Hardware
- [Jetson Nano Developer Kit](https://developer.nvidia.com/embedded/jetson-nano-developer-kit)
- MicroSD Card
- Micro-USB Power Supply (5V⎓2A)

#### Software 
- [Jetson Nano Developer Kit SD Card Image](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#write)
- [DeepStream SDK 6.0](https://developer.nvidia.com/deepstream-getting-started)

## Installation 

Run the following command to compile the application:```make -j4```


## Usage

- Download the [inference model](https://hessenbox.uni-marburg.de/getlink/fi2ctdeqV9y3d16gt4h85W/bird%40edge) and place it in the ```./model``` directory.
- Set the environment variables:
  * ```LD_LIBRARY_PATH=/opt/nvidia/deepstream/deepstream-6.0/lib/gst-plugins/```
  * ```LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libgomp.so.1```
- Run the following command to start the pipeline:
  * ```./birdedge -c configs/ds_audio_config.txt --gst-debug=1```
- The output format is as follows:
  * ```{"frame_num": %d, "timestamp": %ld, "label": %s, "source_id": %d, "confidence": %f}```

## Scientific Usage & Citation

If you are using Bird@Edge in academia, we'd appreciate if you cited our [scientific research paper](https://jonashoechst.de/assets/papers/hoechst2022birdedge.pdf). Please cite as "Höchst & Bellafkir et al."

> J. Höchst, H. Bellafkir, P. Lampe, M. Vogelbacher, M. Mühling, D. Schneider, K. Lindner, S. Rösner, D. G. Schabo, N. Farwig, and B. Freisleben, "Bird@Edge: Bird Species Recognition at the Edge," in *International Conference on Networked Systems (NETYS)*, 2022. DOI: [10.1007/978-3-031-17436-0_6](https://dx.doi.org/10.1007/978-3-031-17436-0_6)

```bibtex
@inproceedings{hoechst2022birdedge,
  title = {{Bird@Edge: Bird Species Recognition at the Edge}},
  author = {H{\"o}chst, Jonas and Bellafkir, Hicham and Lampe, Patrick and Vogelbacher, Markus and M{\"u}hling, Markus and Schneider, Daniel and Lindner, Kim and R{\"o}sner, Sascha and Schabo, Dana G. and Farwig, Nina and Freisleben, Bernd},
  booktitle = {International Conference on Networked Systems (NETYS)},
  year = {2022},
  month = may,
  organization = {Springer},
  keywords = {Bird Species Recognition, Edge Computing, Passive Acoustic Monitoring, Biodiversity},
  doi = {10.1007/978-3-031-17436-0_6},
}
```

