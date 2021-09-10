// Vue.component('settings', {
//     template: '<li>This is a todo</li>'
//   })
  
var app = new Vue({
    el: '#app',
    delimiters: ['[[', ']]'],
    data() {
        return {    devicename: "",
                    timerCount: 0,
                    uptime: 0,
                    firmware: "Unknown",
                    opentime: 5000,
                    closetime: 5000,
                    buttons_switched: false,
                    speed: 80,
                    status: "",
                    pwm: 0,
                    dis: 1,
                    dir: 0,
                    cw: -1,
                    ccw: -1,
                    log: "",
                    // connection: null,
        }
    },
    computed: {
        // a computed getter
        isMoving: function () {
          // `this` points to the vm instance
          // return this.dis == 0 && this.pwm > 0;
          return this.status == "UNDEFINED";
        },
        isOpened: function () {
            // `this` points to the vm instance
            return this.status == "OPENED";
        },
        isClosed: function () {
            // `this` points to the vm instance
            return this.status == "CLOSED";
        },
    },
    created () {

        var self = this;

        // console.log("Starting connection to WebSocket Server")
        // this.connection = new WebSocket("ws://127.0.0.1:5000")

        // this.connection.onmessage = function(event) {
        //     console.log(event);
        // }

        // this.connection.onopen = function(event) {
        //     console.log(event)
        //     console.log("Successfully connected to the echo websocket server...")
        // }

        this.getData();
        this.getStatus();

        // Too slow doing this :(
        // setInterval(function () {
        //     self.getStatus();
        //   }.bind(this), 2000); 
    },
    watch: {

        timerCount: {
            handler(value) {

                if (value > 0) {
                    setTimeout(() => {
                        this.timerCount-=1000;
                    }, 1000);
                }

            },
            immediate: true // This ensures the watcher is triggered upon creation
        }
    },
    methods: {
        getData: function () {   
            var self = this;
            $.ajax({
                url: '/settings',
                method: 'GET',
                success: function (obj) {
                    console.log("Data");
                    console.log(obj);
    
                    self.devicename = obj["devicename"];
                    self.firmware = obj["firmware"];
                    self.opentime = obj["opentime"];
                    self.closetime = obj["closetime"];
                    self.buttons_switched = obj["buttons_switched"];
                    self.speed = obj["speed"];
                },
                error: function (error) {
                    console.log(JSON.stringify(error));
                }
            });
        },
        getStatus: function () {   
            var self = this 
            $.ajax({
                url: '/status',
                method: 'POST',
                success: function (obj) {
                    console.log("Data");
                    console.log(obj);
    
                    self.uptime = obj["uptime"] / 1000;
                    self.status = obj["status"];
                    self.pwm = obj["pwm"];
                    self.dis = obj["dis"];
                    self.dir = obj["dir"];
                    self.cw = obj["cw"];
                    self.ccw = obj["ccw"];
                },
                error: function (error) {
                    console.log(error);
                }
            });
        },
        getLog: function () {   
            var self = this 
            $.ajax({
                url: '/log',
                method: 'GET',
                success: function (obj) {
                    console.log("Data");
                    console.log(obj);
    
                    self.log = obj;
                },
                error: function (error) {
                    console.log(error);
                }
            });
        },
        onSave: function () {   

        },
        onOpen: function () {

            var self = this;

            this.status = "UNDEFINED";
            this.timerCount = self.opentime + 1000;

            $.ajax({
                url: '/open',
                method: 'POST',
                success: function (obj) {

                    window.setTimeout(function(){
                        // code to run after opentime
                        // self.getStatus();
                        self.status = "OPENED";
                        self.getLog();

                    }, self.opentime + 1000);

                },
                error: function (error) {
                    console.log(JSON.stringify(error));
                }
            });
    
        },
        onClose: function () {

            var self = this;

            this.status = "UNDEFINED";
            this.timerCount = self.closetime + 1000;

            $.ajax({
                url: '/close',
                method: 'POST',
                success: function (obj) {
                    window.setTimeout(function(){
                        // code to run after opentime
                        // self.getStatus();
                        self.status = "CLOSED";
                        self.getLog();

                    }, self.closetime + 1000);
                },
                error: function (error) {
                    console.log(JSON.stringify(error));
                }
            });
    
        },
    }
})