module.exports = function(minified) {
  var clayConfig = this;
  
  var add, subtract, multiply, divide, square, root, mode, setMode, resetMode;

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    add = clayConfig.getItemByMessageKey('ADD');
    subtract = clayConfig.getItemByMessageKey('SUBTRACT');
    multiply = clayConfig.getItemByMessageKey('MULTIPLY');
    divide = clayConfig.getItemByMessageKey('DIVIDE');
    square = clayConfig.getItemByMessageKey('SQUARE');
    root = clayConfig.getItemByMessageKey('ROOT');
    mode = clayConfig.getItemByMessageKey('MODE');
    mode.on('change', setMode);
    add.on('change', resetMode);
    subtract.on('change', resetMode);
    multiply.on('change', resetMode);
    divide.on('change', resetMode);
    square.on('change', resetMode);
    root.on('change', resetMode);
  });

  setMode = function() {
    var modeValue = this.get();
    console.log('Setting preset mode to ' + mode);
    switch(modeValue) {
      case 'beginner':
        add.set(5);
        subtract.set(3);
        multiply.set(3);
        divide.set(1);
        square.set(0);
        root.set(0);
        break;
      case 'normal':
        add.set(3);
        subtract.set(3);
        multiply.set(5);
        divide.set(1);
        square.set(5);
        root.set(1);
        break;
      case 'hardcore':
        add.set(0);
        subtract.set(0);
        multiply.set(1);
        divide.set(3);
        square.set(5);
        root.set(5);
        break;
      default:
    }
    // restore mode (overridden by onchange)
    this.set(modeValue);
  };

  resetMode = function() {
    mode.set('custom');
  };

};