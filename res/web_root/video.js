// var camDataPath = "./cat.svg"
var camData = "./cat.svg"

function start() {
    connecteClient();
}

function connecteClient() {
	// 打开一个 web socket
    var ws = new WebSocket("ws://" + window.location.host); // ws://localhost:8000/websocket

    // 连接建立后的回调函数
    ws.onopen = function () {
        console.log("WebSocket 连接成功");
    };

    // 接收到服务器消息后的回调函数
    ws.onmessage = function (evt) {
        var received_msg = evt.data;
        blobToDataURI(received_msg, function (result) {
            camData = result;
            document.getElementById("Camera").src = result;
        })
    };

    // 连接关闭后的回调函数
    ws.onclose = function () {
        // 关闭 websocket
        alert("连接已关闭..." + "ws://" + window.location.host);
    }; 
}

function blobToFile(blob, fileName, type) {  //blob转file
    let files = new window.File([blob], fileName, {type: type})
    return files
}

function blobToDataURI(blob, callback) {
    var reader = new FileReader();
    reader.readAsDataURL(blob);
    reader.onload = function (e) {
        callback(e.target.result.replace("data:application/octet-stream", "data:image/bmp"));
    }
}