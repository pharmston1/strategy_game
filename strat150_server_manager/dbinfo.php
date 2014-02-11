<?php
	/* Author: Aaron <Aaron@FIREBALL-GTX>
	 * Change server,username, and password to meet your database structure
	 * returns $con
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
	
	/*	Connects to database using the properties obtained.	*/
	$con = mysql_connect($server, $username, $password) or die("Failed: ".mysql_error());
	mysql_select_db($db, $con) or die("Failed: ".mysql_error());
?>
