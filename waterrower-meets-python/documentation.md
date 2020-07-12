# Dokumentation

Waterrower System (16.06.2020)
![waterrower system](/waterrower-meets-python/waterrower-meets-three.png)

## Videos: 
https://drive.google.com/file/d/11Y4omaRP-BRkYAcbQQviKOUNP_x2pU_6/view?usp=sharing
https://drive.google.com/file/d/1YEi5VG4MpYDuGJVBlnGvpaoNKF_MHK11/view?usp=sharing

## Firmware broadcast functions
Arduino Code auf dem Node MCU
![firmware](/waterrower-meets-python/firmware.png)

Die folgenden Values werden übertragen

- ticks
- speed
- maxspeed
- avgspeed

## Halterung mit 3D-Druck
![Halterung](/waterrower-meets-python/halterung.PNG)

![3D Object - STL](/waterrower-meets-python/Handyhalter.stl)

## Konfiguration in Node-Red

![node-red](/waterrower-meets-python/node-red.PNG)

## Node-Red Display
https://waterrower.nebuhrhood.org/#!/0?socketid=Sc35ifwYYR3e4bGFAAMr

## Programmierung PyServer
Tutorial: https://responder.kennethreitz.org/en/latest/

```python
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
## Programmierung HTML mit Canvas und JavaScript

Thanx to https://craftpix.net/freebies/free-swamp-game-tileset-pixel-art/ for the graphics.
Zunächst die Animation in 2D, bzw. 2 1/2 D mit fertigen Grafiken. Die nächste Herausforderung ist, die Animation in 3D darzustellen.  
 
```html
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
## Programmierung HTML mit THREE und JavaScript 
```html
<!DOCTYPE html>
<html>

<head>
    <meta charset="utf-8">
    <title>My first three.js app</title>
    <style>
        body {
            margin: 0;
        }

        canvas {
            display: block;
        }
    </style>
</head>

<body>
    <script src="js/three.js"></script>
    <script>
        var boxes = [];
        var maxDepth = 5.5;
        var numberOfBoxes = 100;
        var boxSize = 0.5;
        var speedFactor = 1.0;
        var globalSpeed = 0.1;
        var maxDeviationX = 3;
        var maxDeviationY = 2;
        var originZ = 18;

        // the max. positiv position on the z axis,
        // bevor the box gets reset.
        var maxDepth = 5.5; 
        

        function createBox() {

            var geometry = new THREE.BoxGeometry(boxSize, boxSize, boxSize);
            var edges = new THREE.EdgesGeometry( geometry );
            var line = new THREE.LineSegments( edges, new THREE.LineBasicMaterial( { color: 0xffffff } ) );

            return {
                geometry: geometry,
                edges: edges,
                line: line,
                depth: 0.0,
                speedFactor: 1.0,
                move: function(delta) {
                    var dz = delta * this.speedFactor;
                    this.depth += dz;
                    line.geometry.translate(0,0,dz);
                },
                checkDepth: function(d) {
                    if (this.depth > d) {
                        this.move(-1 * (d+18));
                    };
                }
            }
            
        };

        /**
        * Creates a box with randomized position.
        */
        function createRandomizedBox() {
            var box = createBox();
            var randX = (Math.random() * (maxDeviationX*2)) - maxDeviationX;
            var randY = (Math.random() * (maxDeviationY*2)) - maxDeviationY;
            var randZ = (Math.random() * (originZ + maxDepth)) - originZ;
            //var randZ = (Math.random() * (23)) - 18;
            box.speedFactor = Math.random() + 0.5;
            box.line.geometry.translate(randX, randY, 0);
            box.move(randZ);
            return box;
        }

        
        var scene = new THREE.Scene();
        var camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 0.1, 1000 );
        var lines = [];


        var renderer = new THREE.WebGLRenderer();
        renderer.setSize( window.innerWidth, window.innerHeight );
        document.body.appendChild( renderer.domElement );

        // Create som random boxes and store them in an array.
        for (i=0; i<numberOfBoxes; i++) {
            boxes.push(createRandomizedBox());
        };

        // Add the boxes to the scene
        for (box of boxes) {
            scene.add( box.line );
        };
        
        camera.position.z = 5;

        var animate = function () {
            //box.line.geometry.translate(0,0,0.1);
            for (box of boxes) {
                box.move(globalSpeed);
                box.checkDepth(maxDepth);
            };
            requestAnimationFrame( animate );
            renderer.render( scene, camera );
        };

        animate();
    </script>
</body>

</html>
```
