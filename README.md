# deepstream-test1-app_rtsp
基于官方源码deepstream-test1修改，调用rtsp摄像头，并推理显示结果。

依赖：
GStreamer
Deepstream

编译：
```shell
mkdir build
cd build
cmake ..
make
```

运行：
（注：需要先将dstest1_pgie_config.txt拷贝到build下，运行时需要加载。）
```shell
./deepstream_test1_app_demo_rtsp_
```


