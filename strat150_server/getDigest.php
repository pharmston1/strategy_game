<?php
	/*
	 * 
	 * Name:		getDigest.php
	 * Parameters:	HTTP GET userID; HTTP POST auth
	 * Return:		MD5 checksum, new line, and digest is echoed.
	 * Summary:		Gets the digest for the user that corresponds to
	 * 				the current tick in the game.
	 * 
	 */
	 
	/*	Connects to database.	*/
	require "dbinfo.php";
	
	/*	Checks authentication.	*/
	require "authenticate.php";
	
	/*	Gets current tick so that the most recent digest can be returned.	*/
	$info		= explode(" ", file_get_contents("./bin/tickfile.txt"));
	$tick		= trim($info[0]);
	
	/*	Checks to see if digest is fine. Encodes digest and hashes it.	*/
	$digest		= file_get_contents("./digest/$userID-$tick.json");
	if(json_decode($digest) == NULL) {die("Failed: Digest at ./digest/$userID-$tick.json isn't properly formatted");}
//	$digest		= base64_encode($digest);
//	$checksum	= md5($digest);
	
	echo $digest;
//	echo "$checksum\n$digest";
?>
