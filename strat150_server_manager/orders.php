<?php	require "check.php";	?>
<html>
<head>
	<title>Test for Registration</title>
	<meta http-equiv="content-type" content="text/html;charset=utf-8" />
	<meta name="generator" content="Geany 1.22" />
</head>

<body>
<center>
	<form action = "<?php $_PHP_SELF ?>" method = "POST">
	Username: <input type = "text"		name = "nickname"/><br>
	Password: <input type = "password"	name = "password"/><br>
	<input type = "submit" name = "login" value = "Log-in"/> 
	<input type = "submit" name = "register" value = "Register"/>
	</form>
</center>	
</body>

</html>
