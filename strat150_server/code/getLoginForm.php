<?php
	if(trim($_REQUEST["name"])){ $name = $_REQUEST["name"]; }
	if(trim($_REQUEST["pass"])){ $pass = md5(md5(md5($_REQUEST["pass"]))); }
	if($name && $pass){
		session_start();
		$_SESSION["username"] = $name;
		$_SESSION["password"] = $pass;
		$filename = "info.txt";
		
		$file = fopen($filename, "w");
		fwrite($file, $name." ".$pass." ".session_id());
		fclose($file);
		
		$file = fopen($filename, "r");
		$filesize = filesize($filename);
		$txt = fread($file, $filesize);
		fclose($file);
		
		echo $txt;
		exit();
	}
?>
<html>
<body>
	<center>
	<form id="login" action="<?php $_PHP_SELF ?>?action=LOGIN" method="POST">
	<fieldset>
		<legend>Project Pending</legend>
	<label for="name">Username:</label>
	<input type="text" name="name" /><br>
	<label for="pass">Password:</label>
	<input type="password" name="pass" /><br>
	<input type="submit" value="Login"/>
	</fieldset>
	</form>
	</center>
</body>
</html>
