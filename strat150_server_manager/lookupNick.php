<?php
	/*
	 * 
	 * Name:		lookupNick.php
	 * Parameters:	HTTP GET ids
	 * Return:		Echo list of userIDs and nicknames found
	 * Summary:		Looks up the nickname for the userIDs
	 * 				requested by the client.
	 * 
	 */
	 
	/*	Connects to database.	*/
	require "dbinfo.php";
	
	/*	Gets list of IDs being requested by the client.	*/
	if(php_sapi_name() === 'cli'){
		$idList	= explode(",", mysql_real_escape_string($argv[1]));
	}else{
		$idList	= explode(",", mysql_real_escape_string($_REQUEST["ids"]));
	}
	
	/*	Looks up nickname in database.	*/
	foreach($idList as $id){
		$ID			= str_replace(",", "", $id);
		$results	= mysql_query("SELECT nickname FROM strat150_auth WHERE userID = '$ID'") or die("Failed: ".mysql_error());
		if(mysql_num_rows($results)){
			$row	= mysql_fetch_array($results);
			$nick	= base64_encode($row["nickname"]);
			echo "$ID\n$nick\n";
		}
	}
?>
