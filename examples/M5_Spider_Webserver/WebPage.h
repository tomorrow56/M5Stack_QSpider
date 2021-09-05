/**************************************************************
   WebPage html file
   2020.05.06 modified by @tomorrow56
 **************************************************************/

#ifndef WebPage_h
#define WebPage_h

const String HTTP_Page[] = {
// Display the HTML web page
"<!DOCTYPE html><html><head>",
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">",
"<title>M5Stack Web controller</title>",
//"<link rel=\"icon\" href=\"data:,\">",
// CSS to style the on/off buttons
// Feel free to change the background-color and font-size attributes to fit your preferences
"<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}",
"  #ledPosR{color: red;}",
"  #ledPosG{color: green;}",
"  #ledPosB{color: blue;}",
"  input.button2{margin:8px;width:80px;height:60px;color:#ffffff;background-color:#2f4f4f;border-color:#fdf5e6 #fdf5e6 #ffdab9 #ffdab9;}"
"  input.button3{margin:8px;width:80px;height:60px;color:#ffffff;background-color:#4f2f2f;border-color:#fdf5e6 #fdf5e6 #ffdab9 #ffdab9;}"
"  input.slid{width: 300px;}",
"</style>",
// Web Page
"</head>",
"<body>",
"<h1>M5Stack Q-Spider controller</h1>",
"<h2>Motion Control</h2>",
//Motor Control button
"<form method='get'>",
"  <input class='button3' type='button' name='MOTOR_L' value='ACT1' onclick='motorL()'>",
"  &nbsp;",
"  <input class='button2' type='button' name='MOTOR_F' value='FWD' onclick='motorF()'>",
"  &nbsp;",
"  <input class='button3' type='button' name='MOTOR_R' value='ACT2' onclick='motorR()'><br>",
"</form>",
"<form method='get'>",
"  <input class='button2' type='button' name='MOTOR_TL' value='LEFT' onclick='motorTL()'>",
"  &nbsp;",
"  <input class='button2' type='button' name='STOP' value='STOP' onclick='motorS()'>",
"  &nbsp;",
"  <input class='button2' type='button' name='MOTOR_TR' value='RIGHT' onclick='motorTR()'><br>",
"</form>",
"<form method='get'>",
"  <input class='button3' type='button' name='MOTOR_SL' value='SPIN_L' onclick='motorSL()'>",
"  &nbsp;",
"  <input class='button2' type='button' name='MOTOR_B' value='BACK' onclick='motorB()'>",
"  &nbsp;",
"  <input class='button3' type='button' name='MOTOR_SR' value='SPIN_R' onclick='motorSR()'><br>",
"</form>",
"<br>",

"<script>",
//reload
"function doReloadNoCache(){",
"    window.location.reload(true);",
"}",

//send Motor Foward
"function motorF(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=FWD&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor STOP
"function motorS(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=STOP&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor BACK
"function motorB(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=BACK&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor LEFT
"function motorL(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=LEFT&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor TURN LEFT
"function motorTL(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=TURN_L&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor SPINLEFT
"function motorSL(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=SPIN_L&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor RIGHT
"function motorR(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=RIGHT&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor TURN RIGHT
"function motorTR(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=TURN_R&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor SPINRIGHT
"function motorSR(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=SPIN_R&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",

"</script>",
"</body>",
"</html>"
};
#endif
