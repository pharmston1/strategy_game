<?php
	/* statWrite.php
	 * Copyright 2012 Aaron <Aaron@FIREBALL-GTX>
	 */
	require "dbinfo.php";
	$info		= explode("\n", file_get_contents("./stats/stats.txt"));
	$time		= trim($info[0]);
	$length		= count($info);
	 
	/*	Load file starting at line 1 to length of file into stats DB.	*/
	while(++$i != $length){
		$keyLoc	= explode(" ", $info[$i]); 
		$key	= trim($keyLoc[0]);
		
		if(strpos($info[$i], "_") !== FALSE) {$string = str_replace("_", " ", $info[$i])};
		$strLoc	= explode(" ", $string);
		$unit	= trim(strLoc[0]);
		$stat	= trim(strLoc[1]);
		$value	= trim(strLoc[2]);
		mysql_query("INSERT INTO strat150_stats (key_name, tick, value, unit, stat)
		VALUES ('$key', '$time', '$value', '$unit', $stat)") or die(mysql_error());
	}
?>
