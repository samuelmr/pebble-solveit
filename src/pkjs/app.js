var Clay = require('pebble-clay');
var clayConfig = require('./config');
var customClay = require('./custom-clay');
new Clay(clayConfig, customClay);
