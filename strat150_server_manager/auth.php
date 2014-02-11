<?php
	if(htmlspecialchars(trim($_REQUEST["name"])))
		{ $name = htmlspecialchars(trim($_REQUEST["name"])); }
	if(htmlspecialchars(trim($_REQUEST["pass"])))
		{ $pass = md5(md5(md5(htmlspecialchars(trim($_REQUEST["pass"]))))); }
	if($name && $pass){ //later, check to see if username + pass in database
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
		
		exit();
	}
?>
