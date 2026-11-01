#!/usr/bin/env python3
"""
mock_server.py — учебный сервер сигнализации для PoC-рации.

Что он умеет:
  * принимает несколько клиентов по TCP;
  * JOIN <канал> <имя>   — заходит в канал, отвечает ACK JOINED и список людей;
  * PTT_START            — даёт право говорить первому обратившемуся,
                           остальным отвечает ERROR BUSY;
  * PTT_STOP             — освобождает эфир;
  * LEAVE                — выход из канала;
  * всем участникам канала рассылаются SPEAKER <имя> / SPEAKER (пусто).

Протокол совпадает с тем, что ожидает C++-клиент (RadioState).
Запуск:  python3 mock_server.py [порт]     (по умолчанию 5000)
"""
import asyncio

PORT = 5000


class Client:
    def __init__(self, writer):
        self.writer = writer
        self.name = ""
        self.channel = ""
        self.alive = True


class Channel:
    def __init__(self):
        self.members = []      # список Client
        self.speaker = None    # Client, который сейчас "говорит"

    def broadcast(self, text, exclude=None):
        for c in self.members:
            if c is not exclude:
                send(c, text)


CHANNELS = {}   # имя канала -> Channel
CLIENTS = set()


def send(client, text):
    if client.alive:
        client.writer.write((text + "\n").encode("utf-8"))


async def handle(reader, writer):
    client = Client(writer)
    CLIENTS.add(client)
    buf = b""
    try:
        while True:
            data = await reader.read(4096)
            if not data:
                break
            buf += data
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                await process(client, line.decode("utf-8").strip())
    finally:
        cleanup(client)
        writer.close()


async def process(client, line):
    parts = line.split(maxsplit=2)
    if not parts:
        return
    cmd = parts[0].upper()

    if cmd == "JOIN" and len(parts) >= 3:
        ch_name, name = parts[1], parts[2]
        ch = CHANNELS.setdefault(ch_name, Channel())
        client.name, client.channel = name, ch_name
        ch.members.append(client)
        send(client, "ACK JOINED")
        others = ", ".join(c.name for c in ch.members)
        send(client, f"USERS {others}")
        print(f"[join] {name} -> канал {ch_name}")

    elif cmd == "PTT_START":
        ch = CHANNELS.get(client.channel)
        if ch is None:
            send(client, "ERROR NOT_IN_CHANNEL")
            return
        if ch.speaker is not None:
            send(client, f"ERROR BUSY {ch.speaker.name}")
            return
        ch.speaker = client
        send(client, "ACK PTT_GRANTED")
        ch.broadcast(f"SPEAKER {client.name}", exclude=client)
        send(client, f"SPEAKER {client.name}")
        print(f"[ptt ] {client.name} говорит в {client.channel}")

    elif cmd == "PTT_STOP":
        release_speaker(client)

    elif cmd == "LEAVE":
        release_speaker(client)
        leave_channel(client)
        send(client, "ACK LEFT")

    elif cmd == "PING":
        send(client, "PONG")


def release_speaker(client):
    ch = CHANNELS.get(client.channel)
    if ch and ch.speaker is client:
        ch.speaker = None
        ch.broadcast("SPEAKER")
        print(f"[ptt ] {client.name} закончил говорить")


def leave_channel(client):
    ch = CHANNELS.get(client.channel)
    if ch and client in ch.members:
        ch.members.remove(client)
        names = ", ".join(c.name for c in ch.members)
        ch.broadcast(f"USERS {names}")
    client.channel = ""


def cleanup(client):
    CLIENTS.discard(client)
    client.alive = False
    release_speaker(client)
    leave_channel(client)
    print(f"[leave] клиент отключился ({client.name or '?'})")


async def main():
    server = await asyncio.start_server(handle, "0.0.0.0", PORT)
    print(f"mock signaling server слушает порт {PORT}")
    async with server:
        await server.serve_forever()


if __name__ == "__main__":
    import sys
    if len(sys.argv) > 1:
        PORT = int(sys.argv[1])
    asyncio.run(main())
