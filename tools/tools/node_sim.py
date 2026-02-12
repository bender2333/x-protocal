#!/usr/bin/env python3
"""Simple TPMesh node simulator over serial AT.

Sends AT+SEND frames in a fixed schedule:
- register (once at startup, optional periodic retry)
- heartbeat (periodic)
- BACnet I-Am broadcast payload (periodic)
"""

from __future__ import annotations

import argparse
import ipaddress
import struct
import time
from dataclasses import dataclass
from typing import Optional

import serial

SCHC_RULE_REGISTER = 0x10
SCHC_RULE_NO_COMPRESS = 0x00
SCHC_RULE_BACNET_IP = 0x01
ETH_TYPE_IPV4 = 0x0800
IP_PROTO_UDP = 17
BACNET_PORT = 47808


def crc16_modbus(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF


def parse_mac(text: str) -> bytes:
    parts = text.replace("-", ":").split(":")
    if len(parts) != 6:
        raise ValueError(f"invalid MAC: {text}")
    return bytes(int(p, 16) & 0xFF for p in parts)


def ip_checksum(hdr: bytes) -> int:
    if len(hdr) % 2:
        hdr += b"\x00"
    s = 0
    for i in range(0, len(hdr), 2):
        s += (hdr[i] << 8) | hdr[i + 1]
        s = (s & 0xFFFF) + (s >> 16)
    return (~s) & 0xFFFF


def ipv4_text_to_lwip_u32(text: str) -> int:
    return int.from_bytes(ipaddress.IPv4Address(text).packed, "little")


@dataclass
class NnmiFrame:
    src_mesh: int
    payload: bytes


def parse_nnmi_line(line: str) -> Optional[NnmiFrame]:
    if not line.startswith("+NNMI:"):
        return None
    body = line[6:].strip()
    parts = [p.strip() for p in body.split(",")]
    try:
        if len(parts) >= 5:
            src = int(parts[0], 16)
            declared_len = int(parts[3], 10)
            payload = bytes.fromhex(parts[4])
            if len(payload) != declared_len:
                return None
            return NnmiFrame(src_mesh=src, payload=payload)
        if len(parts) >= 3:
            src = int(parts[0], 16)
            declared_len = int(parts[1], 10)
            payload = bytes.fromhex(parts[2])
            if len(payload) != declared_len:
                return None
            return NnmiFrame(src_mesh=src, payload=payload)
    except (ValueError, IndexError):
        return None
    return None


class NodeSim:
    def __init__(
        self,
        *,
        port: str,
        baud: int,
        node_mesh_id: int,
        top_mesh_id: int,
        node_mac: bytes,
        node_ip: str,
        top_ip: str,
        init_module: bool,
        register_retry_s: float,
        heartbeat_s: float,
        iam_s: float,
        iam_apdu: bytes,
        iam_src_port: int,
    ) -> None:
        self.node_mesh_id = node_mesh_id & 0xFFFF
        self.top_mesh_id = top_mesh_id & 0xFFFF
        self.node_mac = node_mac
        self.node_ip = node_ip
        self.top_ip = top_ip
        self.init_module = init_module
        self.register_retry_s = max(0.0, register_retry_s)
        self.heartbeat_s = max(0.1, heartbeat_s)
        self.iam_s = max(0.1, iam_s)
        self.iam_apdu = iam_apdu
        self.iam_src_port = iam_src_port & 0xFFFF
        self.node_ip_u32_lwip = ipv4_text_to_lwip_u32(node_ip)
        self.last_iam_send_at = 0.0

        self.ser = serial.Serial(port=port, baudrate=baud, timeout=0.2)
        self.ip_id = 1

    @staticmethod
    def _ts() -> str:
        return time.strftime("%H:%M:%S")

    def log(self, msg: str) -> None:
        print(f"[{self._ts()}] {msg}", flush=True)

    def write_line(self, text: str) -> None:
        clean = text.rstrip("\r\n")
        raw = (clean + "\r\n").encode("ascii", errors="ignore")
        self.ser.write(raw)
        self.log(f"SER/TX **{clean}**")

    def read_line(self) -> Optional[str]:
        raw = self.ser.readline()
        if not raw:
            return None
        line = raw.decode("ascii", errors="ignore").strip()
        if line:
            self.log(f"SER/RX **{line}**")
        return line

    def send_cmd_wait_ok(self, cmd: str, timeout_s: float = 1.0) -> bool:
        self.write_line(cmd)
        deadline = time.time() + timeout_s
        while time.time() < deadline:
            line = self.read_line()
            if not line:
                continue
            if line == "OK" or line.endswith(":OK"):
                return True
            if line == "ERROR" or line.startswith("ERROR") or line.endswith(":ERROR"):
                return False
        return False

    def send_at_send(self, dest_mesh: int, payload: bytes, send_type: int = 0) -> None:
        cmd = f"AT+SEND={dest_mesh:04X},{len(payload)},{payload.hex().upper()},{send_type}"
        self.write_line(cmd)

    def build_register_or_hb(self, frame_type: int) -> bytes:
        header = struct.pack(
            "<B6sIH", frame_type, self.node_mac, self.node_ip_u32_lwip, self.node_mesh_id
        )
        crc = crc16_modbus(header)
        reg = header + struct.pack("<H", crc)
        return bytes((0x00, 0x80, SCHC_RULE_REGISTER)) + reg

    def _build_ipv4_udp(self, payload: bytes, dst_ip: str, dst_port: int) -> bytes:
        src_ip_b = ipaddress.IPv4Address(self.node_ip).packed
        dst_ip_b = ipaddress.IPv4Address(dst_ip).packed

        udp_len = 8 + len(payload)
        udp_hdr = struct.pack("!HHHH", self.iam_src_port, dst_port & 0xFFFF, udp_len, 0)

        total_len = 20 + udp_len
        ident = self.ip_id & 0xFFFF
        self.ip_id = (self.ip_id + 1) & 0xFFFF

        ip_wo = struct.pack(
            "!BBHHHBBH4s4s",
            0x45,
            0,
            total_len,
            ident,
            0,
            0xFF,
            IP_PROTO_UDP,
            0,
            src_ip_b,
            dst_ip_b,
        )
        cksum = ip_checksum(ip_wo)
        ip_hdr = struct.pack(
            "!BBHHHBBH4s4s",
            0x45,
            0,
            total_len,
            ident,
            0,
            0xFF,
            IP_PROTO_UDP,
            cksum,
            src_ip_b,
            dst_ip_b,
        )

        eth = b"\xFF\xFF\xFF\xFF\xFF\xFF" + self.node_mac + struct.pack("!H", ETH_TYPE_IPV4)
        return eth + ip_hdr + udp_hdr + payload

    def build_iam_tunnel(self) -> bytes:
        eth = self._build_ipv4_udp(
            payload=self.iam_apdu,
            dst_ip="192.168.10.255",
            dst_port=BACNET_PORT,
        )
        src_mac = eth[6:12]
        dst_mac = eth[0:6]
        rest = eth[12:]
        return bytes((0x80, 0x80, SCHC_RULE_NO_COMPRESS)) + src_mac + dst_mac + rest

    def send_register(self) -> None:
        payload = self.build_register_or_hb(0x01)
        self.log(f"SEND register mesh=0x{self.node_mesh_id:04X} -> top=0x{self.top_mesh_id:04X}")
        self.send_at_send(self.top_mesh_id, payload, 0)

    # def send_reboot(self) -> None:
    #     payload = self.build_register_or_hb(0x02)
    #     self.log("SEND reboot")
    #     self.send_at_send(self.top_mesh_id, payload, 0)

    def send_heartbeat(self) -> None:
        payload = self.build_register_or_hb(0x03)
        self.log("SEND heartbeat")
        self.send_at_send(self.top_mesh_id, payload, 0)

    def send_iam(self) -> None:
        payload = self.build_iam_tunnel()
        self.log("SEND I-Am")
        self.send_at_send(self.top_mesh_id, payload, 0)
        self.last_iam_send_at = time.time()

    @staticmethod
    def is_whois_tunnel(payload: bytes) -> bool:
        if len(payload) < 3:
            return False
        if (payload[1] & 0x7F) != 0 or (payload[1] & 0x80) == 0:
            return False

        rule = payload[2]
        app = b""

        if rule == SCHC_RULE_BACNET_IP:
            # BACNET_IP compressed: [l2][frag][rule][src_mac(6)][BACnet payload...]
            if len(payload) < 3 + 6 + 4:
                return False
            app = payload[3 + 6 :]
        elif rule == SCHC_RULE_NO_COMPRESS:
            # no-compress tunnel: [l2][frag][rule][src_mac][dst_mac][eth...]
            if len(payload) < 3 + 14 + 20 + 8:
                return False
            eth = payload[3:]
            eth_type = (eth[12] << 8) | eth[13]
            if eth_type != ETH_TYPE_IPV4:
                return False

            ip_pkt = eth[14:]
            ihl = (ip_pkt[0] & 0x0F) * 4
            if (ip_pkt[0] >> 4) != 4 or ihl < 20 or len(ip_pkt) < ihl + 8:
                return False
            if ip_pkt[9] != IP_PROTO_UDP:
                return False

            udp = ip_pkt[ihl:]
            dport = (udp[2] << 8) | udp[3]
            udp_len = (udp[4] << 8) | udp[5]
            if dport != BACNET_PORT or udp_len < 10 or len(udp) < udp_len:
                return False
            app = udp[8:udp_len]
        else:
            return False

        # BACnet/IP Who-Is: BVLC type=0x81 function=0x0B, APDU contains 0x10 0x08.
        return len(app) >= 4 and app[0] == 0x81 and app[1] == 0x0B and b"\x10\x08" in app

    def init_as_node(self) -> None:
        if not self.init_module:
            return
        init_cmds = [
            "AT",
            f"AT+ADDR={self.node_mesh_id:04X}",
            "AT+CELL=254",
            "AT+LP=3",
        ]
        for cmd in init_cmds:
            ok = self.send_cmd_wait_ok(cmd, timeout_s=1.5)
            if not ok:
                self.log(f"WARN: init cmd may have failed: {cmd}")

        # REBOOT may reset UART quickly; command can return no final OK.
        self.write_line("AT+REBOOT")
        time.sleep(3.0)

    def run(self) -> None:
        self.log(
            f"NodeSim start mesh=0x{self.node_mesh_id:04X} top=0x{self.top_mesh_id:04X} "
            f"ip={self.node_ip} top_ip={self.top_ip}"
        )
        self.init_as_node()

        self.send_register()
        now = time.time()
        next_register = now + self.register_retry_s if self.register_retry_s > 0 else float("inf")
        next_hb = now + self.heartbeat_s

        while True:
            line = self.read_line()
            if line:
                nnmi = parse_nnmi_line(line)
                if nnmi is not None and self.is_whois_tunnel(nnmi.payload):
                    self.log(f"WHO-IS trigger from mesh src=0x{nnmi.src_mesh:04X}")
                    if (time.time() - self.last_iam_send_at) >= self.iam_s:
                        self.send_iam()
            now = time.time()
            if now >= next_register:
                self.send_register()
                next_register = now + self.register_retry_s
            if now >= next_hb:
                self.send_heartbeat()
                next_hb = now + self.heartbeat_s
            time.sleep(5)


def main() -> int:
    parser = argparse.ArgumentParser(description="TPMesh node simulator (serial AT)")
    parser.add_argument("--port", default="COM36", help="Serial port")
    parser.add_argument("--baud", type=int, default=460800, help="Serial baudrate")
    parser.add_argument("--node-mesh-id", default="0003", help="Node mesh ID (hex)")
    parser.add_argument("--top-mesh-id", default="FFFE", help="Top mesh ID (hex)")
    parser.add_argument("--node-mac", default="00:6B:A0:00:00:10", help="Node MAC")
    parser.add_argument("--node-ip", default="192.168.10.11", help="Node IP")
    parser.add_argument("--top-ip", default="192.168.10.10", help="Top IP (for display)")
    parser.add_argument("--no-init", action="store_true", help="Skip AT/ADDR/CELL/LP init")
    parser.add_argument("--register-retry", type=float, default=0.0, help="Periodic register retry seconds (0=off)")
    parser.add_argument("--heartbeat", type=float, default=30.0, help="Heartbeat period seconds")
    parser.add_argument(
        "--iam",
        type=float,
        default=0.2,
        help="I-Am trigger cooldown seconds after Who-Is",
    )
    parser.add_argument("--iam-src-port", type=int, default=BACNET_PORT, help="I-Am UDP source port")
    parser.add_argument(
        "--iam-apdu-hex",
        default="810B00180120FFFF00FF1000C4020200112205C49103217F",
        help="I-Am UDP payload in hex",
    )
    args = parser.parse_args()

    try:
        node_mesh_id = int(args.node_mesh_id, 16)
        top_mesh_id = int(args.top_mesh_id, 16)
        node_mac = parse_mac(args.node_mac)
        iam_apdu = bytes.fromhex(args.iam_apdu_hex)
    except ValueError as e:
        print(f"arg error: {e}")
        return 2

    try:
        sim = NodeSim(
            port=args.port,
            baud=args.baud,
            node_mesh_id=node_mesh_id,
            top_mesh_id=top_mesh_id,
            node_mac=node_mac,
            node_ip=args.node_ip,
            top_ip=args.top_ip,
            init_module=not args.no_init,
            register_retry_s=args.register_retry,
            heartbeat_s=args.heartbeat,
            iam_s=args.iam,
            iam_apdu=iam_apdu,
            iam_src_port=args.iam_src_port,
        )
        sim.run()
    except serial.SerialException as e:
        print(f"serial error: {e}")
        return 1
    except KeyboardInterrupt:
        print("\nStopped.")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
