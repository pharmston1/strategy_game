<?php
	/*
	 * 
	 * Name:		login.php
	 * Parameters:	HTTP GET userID; HTTP POST password
	 * Return:		Echoes authenication token on success
	 * Summary:		Allows the user to log into the game.
	 * 
	 */
	 
	/*	Connects to database.	*/
	require "dbinfo.php";
	
	/*	Receives userID and hashed password.	*/
	$userID		= mysql_real_escape_string($_REQUEST["userID"]);
	$password	= md5(mysql_real_escape_string($_REQUEST["password"]));
	
	/*	Checks nicknames and password to see if any match. If match, allow access; if no match, throw error.	*/
	$results	= mysql_query("SELECT * FROM strat150_auth WHERE userID = '$userID' AND password = '$password'") or die("Failed: ".mysql_error());
	if(!mysql_num_rows($results)){die("Failed: Could not login using $userID");}
	
	/*	Updates session token and expiration time.	*/
	$sess_token	= uniqid();
	$expiration	= strtotime("+1 day");
	mysql_query("UPDATE strat150_auth SET session_token = '$sess_token', token_expires = '$expiration' WHERE userID = '$userID'") or die("Failed: ".mysql_error());
	
	echo "auth $sess_token";
?>
