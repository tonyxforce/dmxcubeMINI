<?php


$currentVer = $_GET["fw"];
$model = $_GET["model"];

if (is_dir("./files/" . $model)) {
	$files = scandir("./files/" . $model);
	if(count($files)<=2){
		echo "no files";
	}else{
		array_shift($files);
		array_shift($files);
		print_r(json_encode($files));
	};
} else {
	echo "bad model";
}
