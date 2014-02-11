<?php
	/*
	 * 
	 * Name:		createDB.php
	 * Parameters:	No parameters
	 * Return:		No return
	 * Summary:		Creates a database which will contain the
	 * 				authentication and statistics table that will be
	 * 				used for the game and by the other script files.
	 * 
	 */
	 
	/*	Opens configuration file and searches for necessary properties.	*/
	$file = fopen("./config/server.properties", "r") or die("Failed: Unable to open PROPERTIES file");
	while($line = fgets($file)){
		/*	Checks to see if current line contains certain key properties. Removes whitespace.	*/
		if(strpos($line, "db_name") !== FALSE)			{$db		= str_replace(array(" ", "\t", "db_name="), "", trim($line));}
		else if(strpos($line, "db_server") !== FALSE)	{$server 	= str_replace(array(" ", "\t", "db_server="), "", trim($line));}
		else if(strpos($line, "db_username") !== FALSE)	{$username	= str_replace(array(" ", "\t", "db_username="), "", trim($line));}
		else if(strpos($line,"db_password") !== FALSE)	{$password	= str_replace(array(" ", "\t", "db_password="), "", trim($line));}
	}
	fclose($file);
	
	/*	Checks to see if information is missing.	*/
	if($db == NULL || $server == NULL || $username == NULL || $password == NULL) {die("Failed: Configuration information is missing");}
	
	/*	Connects to server. Creates DB and authentication/statistics tables.	*/
	$con = mysql_connect($server, $username, $password) or die("Failed: ".mysql_error());
	mysql_query("CREATE DATABASE IF NOT EXISTS $db", $con) or die("Failed: ".mysql_error());
	mysql_select_db("db", $con) or die("Failed: ".mysql_error());
	mysql_query("CREATE TABLE IF NOT EXISTS strat150_auth (userID varchar(16), nickname varchar(16), password varchar(32),
		session_token varchar(32), token_expires bigint(20) unsigned)", $con) or die("Failed: ".mysql_error());
	mysql_query("CREATE TABLE IF NOT EXISTS strat150_stats (key_name varchar(32), tick varchar(16), value varchar(32),
		unit varchar(32), stat varchar(32))", $con) or die("Failed: ".mysql_error());
?>
