<?php
	/*
	 * 
	 * Name:		register.php
	 * Parameters:	HTTP GET nickname; HTTP POST password
	 * Return:		Echoes userID on success
	 * Summary:		Used for automatic registration of new users.
	 * 
	 */
	 
	/*	Connects to database.	*/
	require "dbinfo.php";
	
	/*	Receives POST nickname and hashed password (hash again). Default nickname if none given.	*/
	if($_REQUEST["nickname"] != NULL){
		$nickname	= mysql_real_escape_string($_REQUEST["nickname"]);
	}
	else{
		$num		= trim( file_get_contents("./num.txt") );
		$nickname	= "Player" . $num;
		file_put_contents("./num.txt", intval($num) + 1);
	}
	$password		= md5(mysql_real_escape_string($_REQUEST["password"]));
	
	/*	Gives a unique userID and checks against table of userIDs; if present, generates new ID.	*/
	do{
		$userID		= uniqid();
		$results 	= mysql_query("SELECT userID FROM strat150_auth WHERE userID = '$userID'") or die("Failed: ".mysql_error());
	}while(mysql_num_rows($results));
	
	/*	Adds info to table.	*/
	$sess_token		= uniqid();
	$expiration		= strtotime("+1 day");
	mysql_query("INSERT INTO strat150_auth (userID, nickname, password, session_token, token_expires)
	VALUES ('$userID', '$nickname', '$password', '$sess_token', $expiration)") or die("Failed: ".mysql_error());
	
	echo "userID $userID";
?>
