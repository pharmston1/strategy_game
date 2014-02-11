<?php
if($_SERVER["REQUEST_METHOD"] == "POST"){
	if($_POST["password"] == NULL){
		echo "<script>alert('Necessary information is missing.')</script>";
	}
	else{$_POST["password"] = md5($_POST["password"]);}
	if(isset($_POST["register"])){
		require "registration.php";
		exit;
	}
	else{
		require "login.php";
		exit;
	}
}
?>
