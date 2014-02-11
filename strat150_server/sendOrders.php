<?php
	/*
	 * 
	 * Name:		sendOrders.php
	 * Parameters:	HTTP GET userID; HTTP POST auth, orders, checksum
	 * Return:		Acknowledge receipt of orders if successful.
	 * Summary:		Receives game orders from user and places them in
	 * 				the "orders" folders for the Simulator.
	 * 
	 */
	
	/*	Connects to database.	*/
	require "dbinfo.php";
	
	/*	Checks authentication.	*/
	require "authenticate.php";
	
	/*	Receives encoded orders and compares to checksum. Decodes orders.	*/
	$orders		= mysql_real_escape_string($_REQUEST["orders"]);
	$checksum	= mysql_real_escape_string($_REQUEST["checksum"]);
	if($checksum != md5($orders)) {die("Failed: Orders were not sent correctly");}
	$orders		= base64_decode($orders);
	
	/*	Writes the orders into temp folder. Copies file to actual "orders" folder.	*/
	$stamp		= time();
	$tempPath	= "./orders_temp/$userID-$stamp.json";
	file_put_contents($tempPath, $orders);
	copy($tempPath, "./orders/$userID-$stamp.json");
	
	/*	Updates expiration time for token.	*/
	$expiration	= strtotime("+1 day");
	mysql_query("UPDATE strat150_auth SET token_expires = '$expiration' WHERE userID = '$userID'") or die("Failed: ".mysql_error());
	
	echo "Received"; // in $tempPath";
?>
