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
"  <input class='button3' type='button' name='ACT1' value='ACT1' onclick='act1()'>",
"  &nbsp;",
"  <input class='button2' type='button' name='FWD' value='FWD' onclick='fwd()'>",
"  &nbsp;",
"  <input class='button3' type='button' name='ACT2' value='ACT2' onclick='act2()'><br>",
"</form>",
"<form method='get'>",
"  <input class='button2' type='button' name='LEFT' value='LEFT' onclick='left()'>",
"  &nbsp;",
"  <input class='button2' type='button' name='STOP' value='STOP' onclick='stop()'>",
"  &nbsp;",
"  <input class='button2' type='button' name='RIGHT' value='RIGHT' onclick='right()'><br>",
"</form>",
"<form method='get'>",
"  <input class='button3' type='button' name='SPIN_L' value='SPIN_L' onclick='spin_l()'>",
"  &nbsp;",
"  <input class='button2' type='button' name='BACK' value='BACK' onclick='back()'>",
"  &nbsp;",
"  <input class='button3' type='button' name='SPIN_R' value='SPIN_R' onclick='spin_r()'><br>",
"</form>",
"<br>",

"<script>",
//reload
"function doReloadNoCache(){",
"    window.location.reload(true);",
"}",

//send Motor Foward
"function fwd(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=FWD&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor STOP
"function stop(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=STOP&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor BACK
"function back(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=BACK&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send ACT1
"function act1(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=ACT1&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor LEFT
"function left(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=LEFT&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor SPINLEFT
"function spin_l(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=SPIN_L&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send ACT2
"function act2(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=ACT2&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor RIGHT
"function right(){",
"    var xhr = new XMLHttpRequest();",
"    xhr.open('get', '?MOTOR=RIGHT&');",
"    xhr.timeout = 1000 ;",
"    xhr.send();",
"}",
//send Motor SPINRIGHT
"function spin_r(){",
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
