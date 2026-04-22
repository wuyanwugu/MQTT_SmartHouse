## 在板子和Windows上配置MQTT

### 1.1 安装APP

安装这2个APP：

![image-20220224163837813](pic/13_apps.png)

### 1.2 启动服务器

在PC上启动MQTT Broker：使用DOS命令行，进入mosquitto-2.0.14-install-windows-x64的安装目录

* 修改配置文件mosquitto.conf，如下修改：

  ```shell
  # listener port-number [ip address/host name/unix socket path]
  listener 1883

  allow_anonymous true
  ```
* 启动MQTT Broker：

```shell
cd  "c:\Program Files\mosquitto"
.\mosquitto.exe -c mosquitto.conf -v
```

![image-20220224164017462](pic/14_start_broker.png)

在下面的实验中，无论是使用MQTTX还是使用mosquitto_pub/mosquitto_sub，都要保持mosquitto.exe在运行。

### 1.3 使用MQTTX

#### 1.3.1 建立连接

运行MQTTX后，如下图操作：

![image-20220224164238214](pic/15_create_connect.png)

![image-20220224164453390](pic/16_create_connect2.png)

#### 1.3.2 订阅主题

建立连接后，如下图操作：

![image-20220224164541550](pic/17_subcribe_1.png)

![image-20220224164643934](pic/18_subcribe_2.png)

#### 1.3.3 发布主题

如下操作：

![image-20220224165011262](pic/19_publish_mqttx.png)

### 1.4开发板连接服务器

·修改main.c中的IP和端口（端口固定1883，IP为创建服务器的电脑IP地址）

![1776842451470](image/02_MQTT框架与体验/1776842451470.png)

开发板连接串口可看到串口打印成功信息
