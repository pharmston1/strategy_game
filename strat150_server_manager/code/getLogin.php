<?php
	if($_GET["name"]){
		$name = $_GET["name"];
	}
	if($_GET["pass"]){
		$pass = md5( $_GET["pass"] );
	}
	if($pass){
		$filename = "info.txt";
		
		$file = fopen($filename, "w");
		fwrite($file, $name." ".$pass);
		fclose($file);
		
		$file = fopen($filename, "r");
		$filesize = filesize($filename);
		$txt = fread($file, $filesize);
		fclose($file);
		
		echo $txt;
	}
	exit();
?>
