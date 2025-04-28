<?php


$currentVer = $_GET["fw"];
$model = $_GET["model"];
$channel = $_GET["ch"];

if($channel == ""){
	$channel = "prod";
};

if (is_dir("./files/" . $model . "/" . $channel)) {
	$files = scandir("./files/" . $model . "/" . $channel);
	if(count($files)<=2){
		echo "no files";
	}else{
		array_shift($files); // remove .. and .
		array_shift($files);
		print_r(json_encode($files));
	};
} else {
	echo "bad model";
}
