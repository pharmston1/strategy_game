<?php
	/*	Checks to see if user has valid auth token and it hasn't expired.	*/
	$userID		= mysql_real_escape_string($_REQUEST["userID"]);
	$auth		= mysql_real_escape_string($_REQUEST["auth"]);
	$results	= mysql_query("SELECT * FROM strat150_auth WHERE userID = '$userID'") or die("Failed: ".mysql_error());
	$row		= mysql_fetch_array($results);	
	if($auth != $row["session_token"]) {die("Failed: Authentication token does not match");}
	if(time() > $row["token_expires"]) {die("Failed: Authentication token has expired");}
?>
