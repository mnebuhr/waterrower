# Dokumentation

Waterrower System (16.06.2020)
![waterrower system](/waterrower-meets-python/waterrower-meets-shotgun.png)

## Firmware broadcast functions
Arduino Code auf dem Node MCU
![firmware](/waterrower-meets-python/firmware.png)

Die folgenden Values werden übertragen

- ticks
- speed
- maxspeed
- avgspeed

## Konfiguration in Node-Red

![node-red](/waterrower-meets-python/node-red.PNG)

```
import json
import responder
import asyncio
from asyncio_mqtt import Client
from starlette.websockets import WebSocketDisconnect

api = responder.API(static_dir="static")

waterrower_speed = 0.0

@api.route("/")
async def greet_world(req, resp):
    resp.html = api.template('hello.jinja')


async def handle_message(client, data):
    async with client.filtered_messages("/nebuhrhood/waterrower/speed") as messages:
    #async with client.unfiltered_messages() as messages:
        async for message in messages:
            waterrower_speed = float(message.payload)
            data["waterrower_speed"] = waterrower_speed

@api.route('/ws', websocket=True)
async def websocket(ws):

    loop = asyncio.get_event_loop()
    await ws.accept()
    connected = True
    counter = 0
    async with Client("nebuhrhood") as client:
        await client.subscribe("/nebuhrhood/waterrower/#")
        data = {}
        data["waterrower_speed"] = 12
        task = loop.create_task(handle_message(client, data))
        while connected:
            try:
                name = await ws.receive_text()
                
                await ws.send_text(f"{data['waterrower_speed']}")
                counter = counter + 1
            except WebSocketDisconnect:
                connected = False
                task.cancel()
    await ws.close()

if __name__ == '__main__':
    api.serve(address="0.0.0.0")
    # asyncio.run(main())
```
