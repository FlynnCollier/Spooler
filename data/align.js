var degrees = document.getElementById('degrees');

function setup() {
  var leftButton = document.getElementById('left-button');
  var rightButton = document.getElementById('right-button'); 
  holdIt(leftButton, function() { jog(-degrees.value) }, 100);
  holdIt(rightButton, function() { jog(degrees.value) }, 100);
}

function holdIt(btn, action, speed) {
  var timeout;
  var repeat = function() {
    action();
    timeout = setTimeout(repeat, speed);
  }
  
  btn.onmousedown = btn.mousedown = repeat;
  btn.onmouseup = btn.mouseup = btn.mouseout = function() {
    clearTimeout(timeout);
  };
}

function jog(degrees) {
  console.debug("jog:" + degrees)
  var xhttp = new XMLHttpRequest();
  xhttp.open("POST", "/jog?degrees=" + degrees, false);
  xhttp.send(degrees);
}