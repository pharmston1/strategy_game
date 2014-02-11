<?php
function merge_sort(&$arr){ mergesort($arr, 0, count($arr) - 1); }
function mergesort(&$arr, $lo, $hi){
	if($hi <= $lo) return;
	$mid = (int)(($hi + $lo) / 2);
	mergesort($arr, $lo, $mid);
	mergesort($arr, $mid + 1, $hi);
	merge($arr, $lo, $mid, $hi);
}
function merge(&$arr, $lo, $mid, $hi){
	$aux = $arr;
	$i = $lo;
	$j = $mid + 1;
	for($k = $lo; $k <= $hi; $k++){
		if($i > $mid)				{$arr[$k] = $aux[$j++];}
		else if($j > $hi)			{$arr[$k] = $aux[$i++];}
		else if($aux[$j] < $aux[$i]){$arr[$k] = $aux[$j++];}
		else						{$arr[$k] = $aux[$i++];}	 
	}
}
?>
