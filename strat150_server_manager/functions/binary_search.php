<?php
require "quicksort.php";
function binary_search($arr, $key){
	$lo = 0;
	$hi = count($arr) - 1;
	quicksort($arr);
	while(TRUE){
		if($hi < $lo)				{return -1;}
		$mid = (int)(($hi + $lo) / 2);
		if($arr[$mid] > $key)		{$hi = $mid - 1;}
		else if($arr[$mid] < $key)	{$lo = $mid + 1;}
		else						{return $mid;}
	}
}
?>
