import discord
import asyncio
import os
from dotenv import load_dotenv

# .envファイルを読み込む
load_dotenv()

# Discord Bot設定
TOKEN = os.getenv("DISCORD_TOKEN")

intents = discord.Intents.default()
intents.message_content = True

client = discord.Client(intents=intents)

# XIAO ESP32-S3設定
ESP32_IP = "192.168.0.156"  # 実際のIPアドレスに変更
ESP32_PORT = 5000

# 同時通信を防ぐ
tcp_lock = asyncio.Lock()


# ESP32へデータを送信する関数
async def send_to_esp32(message):

    async with tcp_lock:
        try:
            reader, writer = await asyncio.wait_for(
                asyncio.open_connection(ESP32_IP, ESP32_PORT),
                timeout=5
            )

            # データ送信
            writer.write((message + "\n").encode("utf-8"))
            await writer.drain()

            # ESP32から返信を受信
            response = await asyncio.wait_for(
                reader.readline(),
                timeout=5
            )

            writer.close()
            await writer.wait_closed()

            return response.decode("utf-8").strip()

        except Exception as e:
            print("TCP Error:", e)
            return None


# 起動時
@client.event
async def on_ready():

    print("Discordログインしました")
    print("Bot:", client.user)


# メッセージ受信時
@client.event
async def on_message(message):

    if message.author.bot:
        return

    print("Discord:", repr(message.content))

    # あけて
    if message.content == "あけて":

        response = await send_to_esp32("OPEN")

        if response:
            await message.channel.send(
                f"{response}"
            )
        else:
            await message.channel.send(
                "ESP32との通信に失敗しました"
            )

    # しめて
    elif message.content == "しめて":

        response = await send_to_esp32("CLOSE")

        if response:
            await message.channel.send(
                f"ESP32からの返信: {response}"
            )
        else:
            await message.channel.send(
                "ESP32との通信に失敗しました"
            )


# Bot起動
if not TOKEN:
    raise RuntimeError("DISCORD_TOKEN が設定されていません")

client.run(TOKEN)