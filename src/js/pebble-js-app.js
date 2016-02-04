var initialized = false;
var messageQueue = [];
// var NEVER= 0;
var RARELY = 1;
var REGULARLY = 3;
var OFTEN = 5;

var config = {
  "shake": 1,
  "labels": 1,
  "add": REGULARLY,
  "subtract": REGULARLY,
  "multiply": OFTEN,
  "divide": RARELY,
  "square": OFTEN,
  "root": RARELY
};

Pebble.addEventListener("ready",
  function(e) {
    var storedConf = localStorage.getItem("config");
    if (storedConf && (storedConf.substr(0, 1) == "{")) {
      config = JSON.parse(storedConf);
    }
    console.log("JavaScript app ready and running!");
    initialized = true;
    sendConfig(config);
  }
);

Pebble.addEventListener("showConfiguration",
  function() {
    var uri = "https://samuelmr.github.io/pebble-solveit/configure.html?conf="+
    encodeURIComponent(JSON.stringify(config));
    console.log("Configuration url: " + uri);
    Pebble.openURL(uri);
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    var webConf = decodeURIComponent(e.response);
    if (webConf && (webConf.substr(0, 1) == "{")) {
      config = JSON.parse(webConf);
      console.log("Webview window returned: " + JSON.stringify(config));
      sendConfig(config);
      localStorage.setItem("config", JSON.stringify(config));
    }
    else {
      console.warn("Invalid webview config: " + webConf);
    }
  }
);

function sendConfig(config) {
  messageQueue.push(config);
  sendNextMessage();
}

function sendNextMessage() {
  if (messageQueue.length > 0) {
    Pebble.sendAppMessage(messageQueue[0], appMessageAck, appMessageNack);
    console.log("Sent message to Pebble! " + JSON.stringify(messageQueue[0]));
  }
}

function appMessageAck(e) {
  console.log("Message accepted by Pebble!");
  messageQueue.shift();
  sendNextMessage();
}

function appMessageNack(e) {
  console.log("Message rejected by Pebble! " + e.error);
}

