<?php
	/*	Connects to database.	*/
	require "dbinfo.php";
	
	/*	Checks authentication.	*/
	require "authentication.php";
	
	/*	Gets current tick so that the most recent digest can be returned.	*/
	$info		= explode(" ", file_get_contents("~/current_tick.txt"));
	$tick		= trim($info[0]);
	
	/*	Checks to see if digest is fine. Encodes digest and hashes it.	*/
	$digest		= trim( file_get_contents("~/digest/$userID-$tick.json") );
	if(json_decode($digest) == NULL) {die("Failed: Digest isn't properly formatted");}
	$digest		= base64_encode($digest);
	$checksum	= md5($digest);
	
	echo "$checksum\n$digest";
?>
