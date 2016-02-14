(function(){
    var self = {
        canvas: null,
        ctx: null,
        config: {
            deltaZoom: 1.5,
            minZoom: 0.001,
            maxZoom: 30,
            radius: 20,
            lineInterval: 1.2,
            fontSize: 20,
            params: {
                "All": {c: "#FFA223", h: 1/3},
                "ReadingInput": {c: "#5BA9EA", h: 1},
                "WritingOutput": {c: "#5BA9EA", h: 1},

                "PreparingInput": {c: "#51ECAE", h: 2/3},
                "CalcR2Matrix": {c: "#1ED78C", h: 1},
                "SortMatrix": {c: "#00BB70", h: 1},
                "CalcR2Indexes": {c: "#007B49", h: 1},

                "PlanBuilding": {c: "#FF8A58", h: 1},
                "RMerging": {c: "#FF4C00", h: 2/3},
                "QHandling": {c: "#B53600", h: 1/3},

                "CrossThreading": {c: "#074375", h: 1},
                "Threading": {c: "#0966B4", h: 1},

                "Unknown": {c: "#F00", h: 1}
            }
        },
        viewPort: {
            x: 0,
            y: 0,
            zoom: 1,
            xBounds: {min: 0, max: 0},
            yBounds: {min: 0, max: 0}
        },
        mouse: {
            x: 0,
            y: 0
        },
        area: {
            width: 0,
            height: 0
        },
        primitivies: [

        ],
        
        tmp: {},
        data: {},

        cached: !!window.tests,
        tests: window.tests || [],
        cache: window.cache || {},
        
        };


    function startMovingViewPort(x, y) {
        self.tmp.viewPortMoving = {x: x, y: y, sx: self.viewPort.x, sy: self.viewPort.y};
    }

    function updateViewPortBounds() {
        var zoomedWidth, zoomedHeight;

        zoomedWidth = self.area.width * self.viewPort.zoom;
        zoomedHeight = self.area.height * self.viewPort.zoom;

        self.viewPort.xBounds = zoomedWidth > self.canvas.width
            ? {min: 0, max: zoomedWidth - self.canvas.width}
            : {min: zoomedWidth - self.canvas.width, max: 0};

        self.viewPort.yBounds = zoomedHeight > self.canvas.height
            ? {min: 0, max: zoomedHeight - self.canvas.height}
            : {min: zoomedHeight - self.canvas.height, max: 0};
    }

    function updateStat(filename, worktime, threadsCount, threadsSyncTime)
    {
        document.querySelector("#stat .id").innerText = filename;
        document.querySelector("#stat .work-time").innerText = worktime;
        document.querySelector("#stat .threads-count").innerText = threadsCount;
        document.querySelector("#stat .threads-sync-time").innerText = threadsSyncTime;
    }

    function updateCache() {
        document.querySelector("#cache").innerHTML =
            "window.tests = " + JSON.stringify(self.tests) +";\n" +
            "window.cache = " + JSON.stringify(self.cache) +";\n";
    }

    function checkViewPortBounds() {
        var checkBound = function(value, bounds) {
                if(value < bounds.min) {
                    return bounds.min;
                }
                if(value > bounds.max) {
                    return bounds.max;
                }
                return value;
            };

        self.viewPort.x = checkBound(self.viewPort.x, self.viewPort.xBounds);
        self.viewPort.y = checkBound(self.viewPort.y, self.viewPort.yBounds);
    }

    function updateMousePosition(x, y) {
        self.mouse.x = self.viewPort.x + x / self.viewPort.zoom;
        self.mouse.y = self.viewPort.y + y / self.viewPort.zoom;
    }

    function updateMovingViewPort(x, y) {
        var vpm;

        if(!self.tmp.viewPortMoving)
            return;

        vpm = self.tmp.viewPortMoving;

        self.viewPort.x = vpm.sx - (x - vpm.x) / self.viewPort.zoom;
        self.viewPort.y = vpm.sy - (y - vpm.y) / self.viewPort.zoom;

        checkViewPortBounds();
        render();
    }

    function stopMovingViewPort(x, y) {
        updateMovingViewPort(x, y);
        delete self.tmp.viewPortMoving;
    }

    function updateZoom(x, y, riseUp) {
        var tmpZoom = self.viewPort.zoom * (riseUp ? self.config.deltaZoom : 1 / self.config.deltaZoom),
            moving = !!self.tmp.viewPortMoving;

        if(tmpZoom < self.config.minZoom || tmpZoom > self.config.maxZoom)
            return;

        if(moving)
            stopMovingViewPort(x, y);

        self.viewPort.x += (1 / self.viewPort.zoom - 1 / tmpZoom) * x;
        self.viewPort.y += (1 / self.viewPort.zoom - 1 / tmpZoom) * y;
        self.viewPort.zoom = tmpZoom;

        updateViewPortBounds();

        self.viewPort.xBounds.min /= self.viewPort.zoom;
        self.viewPort.xBounds.max /= self.viewPort.zoom;
        self.viewPort.yBounds.min /= self.viewPort.zoom;
        self.viewPort.yBounds.max /= self.viewPort.zoom;

        checkViewPortBounds();

        if(moving)
            startMovingViewPort(x, y);

        render();
    }

    function prepareControls() {
        self.canvas = document.getElementById("myCanvas");
        self.ctx = self.canvas.getContext("2d");

        self.canvas.addEventListener('mousewheel',function(event){
            updateZoom(event.x, event.y, event.wheelDelta > 0);
            event.preventDefault();
        });
        self.canvas.addEventListener('mousedown',function(event){
            startMovingViewPort(event.x, event.y);
            event.preventDefault();
        });
        self.canvas.addEventListener('mousemove',function(event){
            self.tmp.mouseTest = {x : event.x, y : event.y};
            render();
            updateMovingViewPort(event.x, event.y);
            updateMousePosition(event.x, event.y);
            event.preventDefault();
        });
        self.canvas.addEventListener('mouseup',function(event){
            stopMovingViewPort(event.x, event.y);
            event.preventDefault();
        });
        window.addEventListener('keyup', function(event){
            prepareData(event.keyCode - 49);
        });
    }

    function prepareData(index) {
        var parseFile = function(data) {
            var i = 0,
                parsedLine,
                koefX = 0.0001,
                koefY = 100,
                threadId,
                type,
                params,
                startTime,
                endTime,
                maxThreadId = 0,
                maxTime = 0,
                threadsSyncTime = 0,
                transformX = function (value) {
                    return value * koefX;
                },
                transformY = function (value) {
                    return value * koefY;
                },
                lines = data.split(/\r?\n/);

            self.primitivies.splice(0, self.primitivies.length);

            for(i=1; i<lines.length; ++i) {
                parsedLine = lines[i].split(/\s+/);

                threadId = parseInt(parsedLine[0]);
                type = parsedLine[1];
                startTime = parseInt(parsedLine[2]);
                endTime = parseInt(parsedLine[3]);
                params = self.config.params[type] || self.config.params["Unknown"];

                if (threadId > maxThreadId) {
                    maxThreadId = threadId;
                }

                if (endTime > maxTime) {
                    maxTime = endTime;
                }

                if (type == "CrossThreading") {
                    threadsSyncTime += endTime - startTime;
                }

                if (params.h == 0) {
                    continue;
                }

                self.primitivies.push({
                    type: "rectangle",
                    fillColor: params.c,
                    x: transformX(startTime),
                    y: transformY(threadId, params.draw_func),
                    width: transformX(endTime - startTime),
                    height: params.h * koefY
                });
            }
            
            self.area.width = transformX(maxTime);
            self.area.height = transformY(maxThreadId);

            updateStat(self.tests[self.data.currentIndex], maxTime, maxThreadId, threadsSyncTime);
            updateViewPortBounds();
            render();
        };

        if(index < 0 || index >= self.tests.length){
            return;
        }

        self.data.currentIndex = index;

        if(self.cache[self.data.currentIndex] != undefined) {
            parseFile(self.cache[self.data.currentIndex]);
        } else if (!self.cached) {
            $.ajax(self.tests[self.data.currentIndex]).done(function(data){
                self.cache[self.data.currentIndex] = data;
                updateCache();
                parseFile(data);
            });
        }
    }

    function init() {
        prepareControls();
        prepareData(0);
        resize();
    }

    function resize() {
        self.canvas.width = window.innerWidth;
        self.canvas.height = window.innerHeight;
        self.canvas.style.background = self.config.backgroundColor;
        render();
    }

    function render() {
        self.ctx.clearRect(0, 0, self.canvas.width, self.canvas.height);

        self.ctx.save();

        self.ctx.setTransform(self.viewPort.zoom, 0, 0, self.viewPort.zoom,
            -self.viewPort.x * self.viewPort.zoom, -self.viewPort.y * self.viewPort.zoom);

        _.each(self.primitivies, function(primitive){
            switch (primitive.type) {
                case "rectangle":
                    self.ctx.fillStyle = primitive.fillColor;
                    self.ctx.fillRect(primitive.x, primitive.y, primitive.width, primitive.height);
                    break;
            }
        });

        self.ctx.restore();
    }

    window.onload = function() {
        if (self.cached) {
            init();
            return;
        }
        
        $.ajax("enumerate_tests.json").done(function(data){
            self.tests = data;
            updateCache();
            init();
        });
    }
    window.onresize = function() { resize(); }
}).call(this);
