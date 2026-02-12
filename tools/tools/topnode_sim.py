#!/usr/bin/env python3
"""TPMesh TopNode simulator + UDP bridge over serial AT.

Main features:
- Simulate Top node registration/heartbeat ACK over serial AT module.
- Bridge DDC uplink (rule=0x00 no-compress tunnel) to UDP on BMS network.
- Inject local UDP (e.g. YABE Who-Is) into mesh as no-compress tunnel frames.
"""

from __future__ import annotations

import argparse
import ipaddress
import socket
import struct
import sys
import time
from dataclasses import dataclass
from typing import Dict, Optional, Set

import serial

TPMESH_MTU = 200
MESH_ADDR_BROADCAST = 0x0000
SCHC_RULE_NO_COMPRESS = 0x00
SCHC_RULE_REGISTER = 0x10
ETH_TYPE_IPV4 = 0x0800
IP_PROTO_UDP = 17


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


def mac_text(mac: bytes) -> str:
    return ":".join(f"{b:02X}" for b in mac)


def ipv4_text_to_lwip_u32(text: str) -> int:
    # Align with firmware's reg_frame uint32 memory layout.
    return int.from_bytes(ipaddress.IPv4Address(text).packed, "little")


def lwip_u32_to_ipv4_text(value: int) -> str:
    raw = int(value & 0xFFFFFFFF).to_bytes(4, "little")
    return str(ipaddress.IPv4Address(raw))


def ip_checksum(hdr: bytes) -> int:
    if len(hdr) % 2:
        hdr += b"\x00"
    s = 0
    for i in range(0, len(hdr), 2):
        s += (hdr[i] << 8) | hdr[i + 1]
        s = (s & 0xFFFF) + (s >> 16)
    return (~s) & 0xFFFF


def is_broadcast_ip(ip_bytes: bytes) -> bool:
    return ip_bytes == b"\xFF\xFF\xFF\xFF" or ip_bytes[-1] == 0xFF


@dataclass
class NnmiFrame:
    src_mesh: int
    payload: bytes


@dataclass
class RegisteredNode:
    mesh_id: int
    mac: bytes
    ip_lwip_u32: int


@dataclass
class ReassemblySession:
    expected_seq: int
    buffer: bytearray
    last_update: float


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


class TopNodeSim:
    def __init__(
        self,
        *,
        port: str,
        baud: int,
        top_mesh_id: int,
        top_mac: bytes,
        top_ip_u32: int,
        init_module: bool,
        udp_bridge: bool,
        bms_bind_ip: str,
        bms_bind_port: int,
        udp_allow_src_ips: Set[str],
        udp_to_mesh_dst_ip: str,
        udp_to_mesh_dst_port: int,
        mesh_broadcast_id: int,
    ) -> None:
        self.port = port
        self.baud = baud
        self.top_mesh_id = top_mesh_id & 0xFFFF
        self.top_mac = top_mac
        self.top_ip_u32 = top_ip_u32 & 0xFFFFFFFF
        self.init_module = init_module
        self.mesh_broadcast_id = mesh_broadcast_id & 0xFFFF

        self.ser = serial.Serial(port=self.port, baudrate=self.baud, timeout=0.2)

        self.udp_bridge = udp_bridge
        self.bms_bind_ip = bms_bind_ip
        self.bms_bind_port = bms_bind_port
        self.udp_allow_src_ips = set(udp_allow_src_ips)
        self.udp_to_mesh_dst_ip = udp_to_mesh_dst_ip
        self.udp_to_mesh_dst_port = udp_to_mesh_dst_port

        self.udp_sock: Optional[socket.socket] = None
        if self.udp_bridge:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            s.bind((self.bms_bind_ip, self.bms_bind_port))
            s.setblocking(False)
            self.udp_sock = s

        self.nodes_by_mesh: Dict[int, RegisteredNode] = {}
        self.nodes_by_ip: Dict[int, RegisteredNode] = {}
        self.reassembly: Dict[int, ReassemblySession] = {}
        self.ip_id = 1

    def log(self, msg: str) -> None:
        print(msg, flush=True)

    @staticmethod
    def _ts() -> str:
        return time.strftime("%H:%M:%S")

    def _log_io(self, direction: str, raw: bytes, text: str = "") -> None:
        if text:
            self.log(f"[{self._ts()}] SER/{direction} **{text}**")
            return

        decoded = raw.decode("ascii", errors="ignore").strip()
        if decoded:
            self.log(f"[{self._ts()}] SER/{direction} **{decoded}**")

    def write_line(self, text: str) -> None:
        raw = (text + "\r\n").encode("ascii", errors="ignore")
        self.ser.write(raw)
        self._log_io("TX", raw, text)

    def read_line(self) -> Optional[str]:
        raw = self.ser.readline()
        if not raw:
            return None
        decoded = raw.decode("ascii", errors="ignore")
        line = decoded.strip()
        self._log_io("RX", raw, line)
        return line

    @staticmethod
    def is_ok_line(line: str) -> bool:
        return line == "OK" or line.endswith(":OK")

    @staticmethod
    def is_error_line(line: str) -> bool:
        return line == "ERROR" or line.endswith(":ERROR")

    def send_at_send(self, dest_mesh: int, payload: bytes, type=0) -> None:
        cmd = f"AT+SEND={dest_mesh:04X},{len(payload)},{payload.hex().upper()},{type}"
        self.write_line(cmd)

    def fragment_and_send(self, dest_mesh: int, tunnel: bytes, type =0) -> None:
        seq = 0
        offset = 0
        total = len(tunnel)

        while offset < total:
            if seq == 0:
                chunk = bytearray(tunnel[:TPMESH_MTU])
                offset = len(chunk)
            else:
                remain = total - offset
                payload_len = min(remain, TPMESH_MTU - 3)
                chunk = bytearray(tunnel[:3] + tunnel[offset : offset + payload_len])
                offset += payload_len

            is_last = offset >= total
            if seq > 0x7F:
                self.log("ERR: too many fragments")
                return
            chunk[1] = (0x80 if is_last else 0x00) | (seq & 0x7F)
            self.send_at_send(dest_mesh, bytes(chunk),type)
            seq += 1

    def send_register_ack(self, dest_mesh: int, ack_type: int) -> None:
        header = struct.pack(
            "<B6sIH", ack_type, self.top_mac, self.top_ip_u32, self.top_mesh_id
        )
        crc = crc16_modbus(header)
        frame = header + struct.pack("<H", crc)
        tunnel = bytes((0x00, 0x80, SCHC_RULE_REGISTER)) + frame
        self.send_at_send(dest_mesh, tunnel)

    def _decode_register_frame(self, src_mesh: int, payload: bytes) -> None:
        if len(payload) < 18:
            self.log(f"REGISTER short from 0x{src_mesh:04X} len={len(payload)}")
            return

        l2_hdr, frag_hdr, rule = payload[0], payload[1], payload[2]
        _ = l2_hdr
        if rule != SCHC_RULE_REGISTER or (frag_hdr & 0x80) == 0 or (frag_hdr & 0x7F) != 0:
            self.log(f"REGISTER bad tunnel hdr src=0x{src_mesh:04X} frag=0x{frag_hdr:02X}")
            return

        reg = payload[3:18]
        frame_type, mac, ip_u32, mesh_id, checksum = struct.unpack("<B6sIHH", reg)
        calc = crc16_modbus(reg[:-2])
        if calc != checksum:
            self.log(
                f"REGISTER CRC bad src=0x{src_mesh:04X} recv=0x{checksum:04X} calc=0x{calc:04X}"
            )
            return

        ip_text = lwip_u32_to_ipv4_text(ip_u32)
        node = RegisteredNode(mesh_id=mesh_id, mac=mac, ip_lwip_u32=ip_u32)
        self.nodes_by_mesh[src_mesh] = node
        self.nodes_by_ip[ip_u32] = node

        if frame_type == 0x01:
            self.log(
                f"REGISTER from src=0x{src_mesh:04X} mesh=0x{mesh_id:04X} mac={mac_text(mac)} ip={ip_text}"
            )
            self.send_register_ack(src_mesh, ack_type=0x02)
        elif frame_type == 0x03:
            self.log(
                f"HEARTBEAT from src=0x{src_mesh:04X} mesh=0x{mesh_id:04X} mac={mac_text(mac)} ip={ip_text}"
            )
            self.send_register_ack(src_mesh, ack_type=0x04)
        else:
            self.log(f"REGISTER unknown frame_type=0x{frame_type:02X} src=0x{src_mesh:04X}")

    def _reassemble(self, src_mesh: int, payload: bytes) -> Optional[bytes]:
        if len(payload) < 3:
            return None

        frag_hdr = payload[1]
        seq = frag_hdr & 0x7F
        is_last = (frag_hdr & 0x80) != 0

        now = time.time()
        # Session timeout cleanup.
        old = [k for k, v in self.reassembly.items() if now - v.last_update > 5.0]
        for k in old:
            del self.reassembly[k]

        if seq == 0:
            self.reassembly[src_mesh] = ReassemblySession(
                expected_seq=1, buffer=bytearray(payload), last_update=now
            )
            if is_last:
                data = bytes(self.reassembly[src_mesh].buffer)
                del self.reassembly[src_mesh]
                return data
            return None

        sess = self.reassembly.get(src_mesh)
        if sess is None or sess.expected_seq != seq:
            self.reassembly.pop(src_mesh, None)
            return None

        sess.buffer.extend(payload[3:])
        sess.expected_seq += 1
        sess.last_update = now

        if is_last:
            data = bytes(sess.buffer)
            del self.reassembly[src_mesh]
            return data
        return None

    def _forward_mesh_eth_to_udp(self, src_mesh: int, tunnel_full: bytes) -> None:
        if self.udp_sock is None:
            return

        if len(tunnel_full) < 3 + 14:
            return

        l2_hdr = tunnel_full[0]
        _ = l2_hdr
        rule = tunnel_full[2]
        if rule != SCHC_RULE_NO_COMPRESS:
            return

        eth_like = tunnel_full[3:]
        src_mac = eth_like[0:6]
        dst_mac = eth_like[6:12]
        eth_type = (eth_like[12] << 8) | eth_like[13]
        if eth_type != ETH_TYPE_IPV4:
            self.log(
                f"MESH->UDP skip non-IPv4 src=0x{src_mesh:04X} eth=0x{eth_type:04X}"
            )
            return

        if len(eth_like) < 14 + 20:
            return
        ip_pkt = eth_like[14:]
        version = (ip_pkt[0] >> 4) & 0x0F
        ihl = (ip_pkt[0] & 0x0F) * 4
        if version != 4 or ihl < 20 or len(ip_pkt) < ihl + 8:
            return

        proto = ip_pkt[9]
        if proto != IP_PROTO_UDP:
            return

        src_ip = str(ipaddress.IPv4Address(ip_pkt[12:16]))
        dst_ip = str(ipaddress.IPv4Address(ip_pkt[16:20]))
        udp = ip_pkt[ihl:]
        src_port = (udp[0] << 8) | udp[1]
        dst_port = (udp[2] << 8) | udp[3]
        udp_len = (udp[4] << 8) | udp[5]
        if udp_len < 8 or len(udp) < udp_len:
            return
        udp_payload = udp[8:udp_len]

        try:
            self.log(
                f"[{self._ts()}] UDP<-MESH RAW len={len(udp_payload)} hex={udp_payload.hex().upper()}"
            )
            self.udp_sock.sendto(udp_payload, (dst_ip, dst_port))
            self.log(
                f"MESH->UDP src=0x{src_mesh:04X} {src_ip}:{src_port} -> {dst_ip}:{dst_port} "
                f"len={len(udp_payload)} srcmac={mac_text(src_mac)} dstmac={mac_text(dst_mac)}"
            )
        except OSError as e:
            self.log(f"MESH->UDP send error: {e}")

    def _build_ipv4_udp_frame(self, src_ip: str, dst_ip: str, src_port: int, dst_port: int, payload: bytes, dst_mac: bytes) -> bytes:
        src_ip_b = ipaddress.IPv4Address(src_ip).packed
        dst_ip_b = ipaddress.IPv4Address(dst_ip).packed

        udp_len = 8 + len(payload)
        udp_hdr = struct.pack("!HHHH", src_port & 0xFFFF, dst_port & 0xFFFF, udp_len, 0)

        total_len = 20 + udp_len
        ident = self.ip_id & 0xFFFF
        self.ip_id = (self.ip_id + 1) & 0xFFFF

        ip_hdr_wo_cksum = struct.pack(
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
        cksum = ip_checksum(ip_hdr_wo_cksum)
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

        eth_hdr = dst_mac + self.top_mac + struct.pack("!H", ETH_TYPE_IPV4)
        return eth_hdr + ip_hdr + udp_hdr + payload

    def _encap_no_compress(self, eth_frame: bytes, is_broadcast: bool) -> bytes:
        # tunnel payload = [SRC_MAC][DST_MAC][EtherType+Payload]
        src_mac = eth_frame[6:12]
        dst_mac = eth_frame[0:6]
        rest = eth_frame[12:]
        l2_hdr = 0x80 if is_broadcast else 0x00
        return bytes((l2_hdr, 0x80, SCHC_RULE_NO_COMPRESS)) + src_mac + dst_mac + rest

    def poll_udp_rx(self) -> None:
        if self.udp_sock is None:
            return

        while True:
            try:
                data, addr = self.udp_sock.recvfrom(2048)
            except BlockingIOError:
                return
            except OSError as e:
                self.log(f"UDP recv error: {e}")
                return

            if not data:
                continue

            src_ip, src_port = addr[0], addr[1]
            # Inject only whitelisted local source IPs into mesh.
            # This avoids rebroadcast/echo packets from other nodes being re-injected.
            if src_ip not in self.udp_allow_src_ips:
                self.log(
                    f"UDP->MESH skip foreign src {src_ip}:{src_port} "
                    f"(allow only {sorted(self.udp_allow_src_ips)})"
                )
                continue

            dst_ip = self.udp_to_mesh_dst_ip
            dst_port = self.udp_to_mesh_dst_port
            dst_ip_lwip = ipv4_text_to_lwip_u32(dst_ip)
            dst_node = self.nodes_by_ip.get(dst_ip_lwip)

            if dst_node is not None:
                mesh_dest = dst_node.mesh_id
                dst_mac = dst_node.mac
                is_broadcast = False
            else:
                mesh_dest = self.mesh_broadcast_id
                dst_mac = b"\xFF\xFF\xFF\xFF\xFF\xFF"
                is_broadcast = True

            eth = self._build_ipv4_udp_frame(
                src_ip=src_ip,
                dst_ip=dst_ip,
                src_port=src_port,
                dst_port=dst_port,
                payload=data,
                dst_mac=dst_mac,
            )
            tunnel = self._encap_no_compress(eth, is_broadcast=is_broadcast)
            self.fragment_and_send(mesh_dest, tunnel)
            self.log(
                f"[{self._ts()}] UDP->MESH RAW len={len(data)} hex={data.hex().upper()}"
            )
            self.log(
                f"UDP->MESH {src_ip}:{src_port} -> {dst_ip}:{dst_port} len={len(data)} "
                f"dest_mesh=0x{mesh_dest:04X} {'bcast' if is_broadcast else 'ucast'}"
            )

    def handle_nnmi(self, nnmi: NnmiFrame) -> None:
        p = nnmi.payload
        if len(p) < 3:
            self.log(f"NNMI src=0x{nnmi.src_mesh:04X} short payload len={len(p)}")
            return

        rule = p[2]
        if rule == SCHC_RULE_REGISTER:
            self._decode_register_frame(nnmi.src_mesh, p)
            return

        if rule == SCHC_RULE_NO_COMPRESS:
            full = self._reassemble(nnmi.src_mesh, p)
            if full is not None:
                self._forward_mesh_eth_to_udp(nnmi.src_mesh, full)
            return

        self.log(
            f"NNMI src=0x{nnmi.src_mesh:04X} rule=0x{rule:02X} len={len(p)} unsupported"
        )

    def send_cmd_wait_ok(self, cmd: str, timeout_s: float = 2.0) -> bool:
        self.write_line(cmd)
        deadline = time.time() + timeout_s
        while time.time() < deadline:
            line = self.read_line()
            if line is None:
                self.poll_udp_rx()
                continue
            if not line:
                continue

            nnmi = parse_nnmi_line(line)
            if nnmi is not None:
                self.handle_nnmi(nnmi)
                continue

            if self.is_ok_line(line):
                return True
            if self.is_error_line(line):
                return False
        return False

    def init_as_top(self) -> None:
        if not self.init_module:
            return

        cmds = [
            "AT",
            f"AT+ADDR={self.top_mesh_id:04X}",
            "AT+CELL=0",
            "AT+LP=3",
        ]
        for c in cmds:
            ok = self.send_cmd_wait_ok(c, timeout_s=2.5)
            if not ok:
                self.log(f"WARN: init cmd may have failed: {c}")

    def run(self) -> None:
        self.log(
            f"TopNodeSim start: port={self.port} baud={self.baud} "
            f"mesh=0x{self.top_mesh_id:04X} ip={lwip_u32_to_ipv4_text(self.top_ip_u32)}"
        )
        if self.udp_sock is not None:
            self.log(
                f"UDP bridge ON: bind={self.bms_bind_ip}:{self.bms_bind_port}, "
                f"udp->mesh dst={self.udp_to_mesh_dst_ip}:{self.udp_to_mesh_dst_port}"
            )
        else:
            self.log("UDP bridge OFF")

        self.init_as_top()
        self.log("Listening serial +NNMI ... (Ctrl+C to stop)")

        while True:
            self.poll_udp_rx()
            line = self.read_line()
            if line is None:
                continue
            if not line:
                continue

            nnmi = parse_nnmi_line(line)
            if nnmi is not None:
                self.handle_nnmi(nnmi)


def main() -> int:
    parser = argparse.ArgumentParser(description="TPMesh TopNode serial simulator")
    parser.add_argument("--port", default="COM36", help="Serial port, e.g. COM36")
    parser.add_argument("--baud", type=int, default=460800, help="Serial baudrate")
    parser.add_argument("--top-mesh-id", default="FFFE", help="Top mesh ID (hex)")
    parser.add_argument("--top-mac", default="02:00:00:00:FF:FE", help="Top MAC")
    parser.add_argument(
        "--top-ip",
        default=None,
        help="Top IP in register ACK (default: same as --bms-bind-ip)",
    )
    parser.add_argument("--no-init", action="store_true", help="Skip AT/ADDR/CELL/LP init")

    parser.add_argument(
        "--no-udp-bridge",
        action="store_true",
        help="Disable UDP<->Mesh bridge",
    )
    parser.add_argument(
        "--bms-bind-ip",
        default="192.168.10.3",
        help="Local NIC IP to bind UDP listener",
    )
    parser.add_argument(
        "--bms-bind-port", type=int, default=47808, help="UDP bind port"
    )
    parser.add_argument(
        "--udp-allow-src",
        default=None,
        help="Comma-separated source IP allowlist for UDP->MESH injection "
        "(default: --bms-bind-ip)",
    )
    parser.add_argument(
        "--udp-to-mesh-dst-ip",
        default="192.168.10.255",
        help="Destination IPv4 used when injecting UDP to mesh",
    )
    parser.add_argument(
        "--udp-to-mesh-dst-port",
        type=int,
        default=47808,
        help="Destination UDP port used when injecting UDP to mesh",
    )
    parser.add_argument(
        "--mesh-broadcast-id",
        default="0000",
        help="Mesh ID used for broadcast injection (hex)",
    )

    args = parser.parse_args()

    try:
        top_mesh_id = int(args.top_mesh_id, 16)
        top_mac = parse_mac(args.top_mac)
        top_ip_text = args.top_ip if args.top_ip else args.bms_bind_ip
        top_ip_u32 = ipv4_text_to_lwip_u32(top_ip_text)
        mesh_bcast = int(args.mesh_broadcast_id, 16)
        if args.udp_allow_src:
            allow_src = {s.strip() for s in args.udp_allow_src.split(",") if s.strip()}
        else:
            allow_src = {args.bms_bind_ip}
    except ValueError as e:
        print(f"arg error: {e}", file=sys.stderr)
        return 2

    try:
        sim = TopNodeSim(
            port=args.port,
            baud=args.baud,
            top_mesh_id=top_mesh_id,
            top_mac=top_mac,
            top_ip_u32=top_ip_u32,
            init_module=not args.no_init,
            udp_bridge=not args.no_udp_bridge,
            bms_bind_ip=args.bms_bind_ip,
            bms_bind_port=args.bms_bind_port,
            udp_allow_src_ips=allow_src,
            udp_to_mesh_dst_ip=args.udp_to_mesh_dst_ip,
            udp_to_mesh_dst_port=args.udp_to_mesh_dst_port,
            mesh_broadcast_id=mesh_bcast,
        )
        sim.run()
    except serial.SerialException as e:
        print(f"serial error: {e}", file=sys.stderr)
        return 1
    except OSError as e:
        print(f"socket error: {e}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nStopped.")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
