#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
DDC MQTT AT 透传工具 - PC 端

功能：
1. 注册 mDNS 服务，让 DDC 发现 Broker
2. 连接 MQTT Broker
3. 发送 AT 指令，接收响应

依赖安装：
    pip install zeroconf paho-mqtt

使用方法：
    python pc_mqtt_tool.py --broker 192.168.10.2 --port 1883

作者: DDC Team
"""

import argparse
import socket
import sys
import threading
import time
from typing import Optional

try:
    from zeroconf import ServiceInfo, Zeroconf
    import paho.mqtt.client as mqtt
except ImportError:
    print("请先安装依赖: pip install zeroconf paho-mqtt")
    sys.exit(1)


class DDCMqttTool:
    """DDC MQTT AT 透传工具"""

    # mDNS 服务配置
    MDNS_SERVICE_TYPE = "_ddc-mqtt._tcp.local."
    MDNS_SERVICE_NAME = "DDC-Config-Service._ddc-mqtt._tcp.local."

    # MQTT Topic
    TOPIC_STATUS = "ddc/status"
    TOPIC_CMD_TEMPLATE = "ddc/{device_id}/at/cmd"
    TOPIC_RESP_TEMPLATE = "ddc/{device_id}/at/resp"

    def __init__(self, broker_ip: str, broker_port: int = 1883):
        self.broker_ip = broker_ip
        self.broker_port = broker_port
        
        # mDNS
        self.zeroconf: Optional[Zeroconf] = None
        self.service_info: Optional[ServiceInfo] = None
        
        # MQTT
        self.mqtt_client: Optional[mqtt.Client] = None
        self.connected = False
        
        # 设备列表
        self.devices: dict = {}
        self.current_device: Optional[str] = None
        
        # 响应等待
        self.response_event = threading.Event()
        self.last_response: str = ""

    def start(self):
        """启动服务"""
        print(f"\n{'='*60}")
        print(f"DDC MQTT AT 透传工具")
        print(f"{'='*60}")
        print(f"Broker: {self.broker_ip}:{self.broker_port}")
        print(f"{'='*60}\n")

        # 1. 注册 mDNS 服务
        self._register_mdns()

        # 2. 连接 MQTT Broker
        self._connect_mqtt()

        # 3. 进入命令行交互
        self._command_loop()

    def stop(self):
        """停止服务"""
        print("\n正在停止服务...")
        
        # 注销 mDNS
        if self.service_info and self.zeroconf:
            self.zeroconf.unregister_service(self.service_info)
            self.zeroconf.close()
            print("mDNS 服务已注销")

        # 断开 MQTT
        if self.mqtt_client:
            self.mqtt_client.disconnect()
            self.mqtt_client.loop_stop()
            print("MQTT 已断开")

    def _register_mdns(self):
        """注册 mDNS 服务"""
        print("正在注册 mDNS 服务...")

        # 获取本机 IP
        local_ip = self._get_local_ip()
        print(f"本机 IP: {local_ip}")

        # 创建服务信息
        # TXT 记录包含 Broker 信息
        properties = {
            b"broker_ip": self.broker_ip.encode(),
            b"broker_port": str(self.broker_port).encode(),
            b"version": b"1.0",
        }

        self.service_info = ServiceInfo(
            type_=self.MDNS_SERVICE_TYPE,
            name=self.MDNS_SERVICE_NAME,
            addresses=[socket.inet_aton(local_ip)],
            port=self.broker_port,
            properties=properties,
            server=f"ddc-config.local.",
        )

        # 注册服务
        self.zeroconf = Zeroconf()
        self.zeroconf.register_service(self.service_info)

        print(f"mDNS 服务已注册: {self.MDNS_SERVICE_TYPE}")
        print(f"  - 名称: {self.MDNS_SERVICE_NAME}")
        print(f"  - Broker: {self.broker_ip}:{self.broker_port}")

    def _connect_mqtt(self):
        """连接 MQTT Broker"""
        print(f"\n正在连接 MQTT Broker ({self.broker_ip}:{self.broker_port})...")

        self.mqtt_client = mqtt.Client(client_id="PC_DDC_Tool", protocol=mqtt.MQTTv311)

        # 设置回调
        self.mqtt_client.on_connect = self._on_mqtt_connect
        self.mqtt_client.on_message = self._on_mqtt_message
        self.mqtt_client.on_disconnect = self._on_mqtt_disconnect

        try:
            self.mqtt_client.connect(self.broker_ip, self.broker_port, keepalive=60)
            self.mqtt_client.loop_start()

            # 等待连接
            for _ in range(50):  # 最多等待 5 秒
                if self.connected:
                    break
                time.sleep(0.1)

            if not self.connected:
                print("MQTT 连接超时")
                return False

        except Exception as e:
            print(f"MQTT 连接失败: {e}")
            return False

        return True

    def _on_mqtt_connect(self, client, userdata, flags, rc):
        """MQTT 连接回调"""
        if rc == 0:
            print("MQTT 连接成功!")
            self.connected = True

            # 订阅设备状态
            client.subscribe(self.TOPIC_STATUS)
            print(f"已订阅: {self.TOPIC_STATUS}")

            # 订阅所有设备的响应 (使用通配符)
            client.subscribe("ddc/+/at/resp")
            print("已订阅: ddc/+/at/resp")
        else:
            print(f"MQTT 连接失败, rc={rc}")

    def _on_mqtt_message(self, client, userdata, msg):
        """MQTT 消息回调"""
        topic = msg.topic
        payload = msg.payload.decode("utf-8", errors="replace")

        if topic == self.TOPIC_STATUS:
            # 设备上线消息
            print(f"\n[设备状态] {payload}")
            # 解析设备 ID
            try:
                import json
                data = json.loads(payload)
                device_id = data.get("device", "unknown")
                status = data.get("status", "unknown")
                if status == "online":
                    self.devices[device_id] = {"status": "online", "last_seen": time.time()}
                    print(f"  设备 {device_id} 已上线")
                    if self.current_device is None:
                        self.current_device = device_id
                        print(f"  已自动选择设备: {device_id}")
            except:
                pass

        elif "/at/resp" in topic:
            # AT 响应
            print(f"\n[AT 响应] {payload}", end="")
            self.last_response = payload
            self.response_event.set()

    def _on_mqtt_disconnect(self, client, userdata, rc):
        """MQTT 断开回调"""
        print(f"\nMQTT 连接已断开, rc={rc}")
        self.connected = False

    def _command_loop(self):
        """命令行交互循环"""
        print("\n" + "="*60)
        print("命令行模式 (输入 'help' 查看帮助, 'quit' 退出)")
        print("="*60 + "\n")

        while True:
            try:
                # 显示当前设备
                device_hint = f"[{self.current_device}]" if self.current_device else "[无设备]"
                cmd = input(f"{device_hint}> ").strip()

                if not cmd:
                    continue

                if cmd.lower() == "quit" or cmd.lower() == "exit":
                    break

                if cmd.lower() == "help":
                    self._show_help()
                    continue

                if cmd.lower() == "list":
                    self._list_devices()
                    continue

                if cmd.lower().startswith("select "):
                    device_id = cmd[7:].strip()
                    self._select_device(device_id)
                    continue

                if cmd.lower() == "status":
                    self._show_status()
                    continue

                # 发送 AT 命令
                self._send_at_command(cmd)

            except KeyboardInterrupt:
                print("\n")
                break
            except EOFError:
                break

        self.stop()

    def _show_help(self):
        """显示帮助"""
        print("""
可用命令:
  help              - 显示此帮助
  list              - 列出已发现的设备
  select <device>   - 选择要通信的设备
  status            - 显示当前状态
  quit/exit         - 退出程序
  
  <AT命令>          - 直接输入 AT 命令发送到选定设备
                      例如: AT+GMR
                           AT+CWMODE?
""")

    def _list_devices(self):
        """列出设备"""
        if not self.devices:
            print("  未发现任何设备")
            return

        print("  已发现的设备:")
        for device_id, info in self.devices.items():
            marker = " *" if device_id == self.current_device else "  "
            print(f"  {marker} {device_id} - {info['status']}")

    def _select_device(self, device_id: str):
        """选择设备"""
        if device_id in self.devices:
            self.current_device = device_id
            print(f"  已选择设备: {device_id}")
        else:
            print(f"  设备 {device_id} 未找到")
            print(f"  可用设备: {list(self.devices.keys())}")

    def _show_status(self):
        """显示状态"""
        print(f"""
  状态信息:
    MQTT 连接: {'已连接' if self.connected else '未连接'}
    Broker: {self.broker_ip}:{self.broker_port}
    当前设备: {self.current_device or '无'}
    已发现设备: {len(self.devices)}
""")

    def _send_at_command(self, cmd: str):
        """发送 AT 命令"""
        if not self.connected:
            print("  错误: MQTT 未连接")
            return

        if not self.current_device:
            print("  错误: 未选择设备，请使用 'select <device>' 选择设备")
            return

        # 构造 Topic
        topic = self.TOPIC_CMD_TEMPLATE.format(device_id=self.current_device)

        # 确保命令以 \r\n 结尾
        if not cmd.endswith("\r\n"):
            if cmd.endswith("\r") or cmd.endswith("\n"):
                cmd = cmd.rstrip() + "\r\n"
            else:
                cmd = cmd + "\r\n"

        # 发送
        print(f"  发送: {repr(cmd)} -> {topic}")
        self.response_event.clear()
        self.mqtt_client.publish(topic, cmd.encode())

        # 等待响应
        print("  等待响应...", end="", flush=True)
        if self.response_event.wait(timeout=5.0):
            # 响应已在回调中打印
            pass
        else:
            print("\n  响应超时")

    def _get_local_ip(self) -> str:
        """获取本机 IP 地址"""
        try:
            # 创建 UDP socket 来获取本机 IP
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect((self.broker_ip, 80))
            ip = s.getsockname()[0]
            s.close()
            return ip
        except:
            return "127.0.0.1"


def main():
    parser = argparse.ArgumentParser(
        description="DDC MQTT AT 透传工具",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  %(prog)s --broker 192.168.10.2
  %(prog)s --broker 192.168.10.2 --port 1883
        """,
    )

    parser.add_argument(
        "--broker", "-b",
        type=str,
        default="127.0.0.1",
        help="MQTT Broker IP 地址 (默认: 127.0.0.1)",
    )

    parser.add_argument(
        "--port", "-p",
        type=int,
        default=1883,
        help="MQTT Broker 端口 (默认: 1883)",
    )

    args = parser.parse_args()

    tool = DDCMqttTool(broker_ip=args.broker, broker_port=args.port)

    try:
        tool.start()
    except KeyboardInterrupt:
        pass
    finally:
        tool.stop()


if __name__ == "__main__":
    main()
