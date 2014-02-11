<?php
function quicksort(&$arr){ shuffle($arr); qSort($arr, 0, count($arr) - 1); }
function qSort(&$arr, $lo, $hi){
	if($hi <= $lo){return;}
	$j = partition($arr, $lo, $hi);
	qSort($arr, $lo, $j-1);
	qSort($arr, $j+1, $hi);
}
function partition(&$arr, $lo, $hi){
	$i = $lo;
	$j = $hi + 1;
	while(TRUE){
		while($arr[++$i] < $arr[$lo]){ if($i == $hi){break;} }
		while($arr[$lo] < $arr[--$j]){ if($j == $lo){break;} }
		if($i >= $j){break;}
		swap($arr, $i, $j);
	}
	swap($arr, $lo, $j);
	return $j;
}
function swap(&$arr, $x, $y){
	$temp = $arr[$x];
	$arr[$x] = $arr[$y];
	$arr[$y] = $temp;
}
?>
