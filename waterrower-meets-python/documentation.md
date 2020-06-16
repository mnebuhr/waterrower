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

## Programmierung PyServer
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
## Programmierung PyServer
 
```
<!DOCTYPE html>
<html>

<head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    <script type="application/javascript">

        var background = new Image();
        var layer = [
            {
                image : new Image(),
                x : 0,
                y : 0,
                factor : 0.5
            }, 
            {
                image : new Image(),
                x : 0,
                y : 0,
                factor : 1,
            }, 
            {
                image : new Image(),
                x : 0,
                y : 0,
                factor : 2.0,
            }, 
            {
                image : new Image(),
                x : 0,
                y : 0,
                factor : 4.0
            } 
        ] 

        layer[0].image.src = '/static/image/2.png';
        layer[1].image.src = '/static/image/3.png';
        layer[2].image.src = '/static/image/4.png';
        layer[3].image.src = '/static/image/5.png';

        var x = 0;
        var dx = 0;
        var speed = 0;
        const width = 576*2;
        const height = 423*2;
        var speed_elem = null;

        const mysocket = new WebSocket("ws://192.168.178.62:5042/ws");
        mysocket.onopen = function (event) {
            mysocket.send("get_speed");
        };

        mysocket.onmessage = function(event) {
            //data = JSON.parse(event.data);
            speed = parseFloat(event.data);
            dx = -2 * speed;
            //console.log(event.data);
        };

        mysocket.onclose = function(event) {
            console.log("Socket closed");
        }
        
        background.src = '/static/image/1.png';
        background.onload = function() {
            console.log("image loaded")
        }
        
        function updateData() {
            if (mysocket.readyState != WebSocket.OPEN) {
                console.error("webSocket is not open: " + mysocket.readyState);
            } else {
                mysocket.send("get_speed");
            }
        }

        function play() {
            setInterval(draw, 30);
            setInterval(updateData, 500);
            speed_elem = document.getElementById("wr_speed");

        }

        function drawLayer(ctx, idx) {
            l = layer[idx]
            ctx.drawImage(l.image, l.x+width, 0,width, height);
            ctx.drawImage(l.image, l.x, 0,width, height);
            l.x += (dx * l.factor);
            if (l.x < -width) {
                l.x += width;
            }            
        }
        function draw() {
            console.log(speed);
            speed_elem.textContent = speed;
            var canvas = document.getElementById("canvas");
            if (canvas.getContext) {
                var ctx = canvas.getContext("2d");
                ctx.drawImage(background,0,0,width, height);
                drawLayer(ctx, 0);
                drawLayer(ctx, 1);
                drawLayer(ctx, 2);
                drawLayer(ctx, 3);
            }
        }
    </script>
</head>

<body onload="play();" style="background-color:black;vertical-align:center;">
    <canvas id="canvas" width="1152" height="846"></canvas>
    <div id="wr_speed" style="font-size:3em; color:white; font-family: Roboto, Helvetica, Arial">SPEED</div>
</body>

</html>   
```
