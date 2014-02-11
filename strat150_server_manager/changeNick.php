<?php
	/*
	 * 
	 * Name:		changeNick.php
	 * Parameters:	HTTP GET userID; HTTP POST auth
	 * Return:		Acknowledge success of the changed nickname
	 * Summary:		Allows the user to change their game nickname.
	 * 
	 */
	 
	/*	Connects to database.	*/
	require "dbinfo.php";
	
	/*	Checks authentication.	*/
	require "authenticate.php";
	
	/*	Receives nickname and updates the DB. Updates expiration time for token.	*/
	$nickname	= mysql_real_escape_string($_REQUEST["nickname"]);
	$expiration	= strtotime("+1 day");
	mysql_query("UPDATE strat150_auth SET nickname = '$nickname', token_expires = '$$expiration' WHERE userID = '$userID'") or die("Failed: ".mysql_error());
	
	echo "Success";
?>
