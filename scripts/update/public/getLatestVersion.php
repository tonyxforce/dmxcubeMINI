<?php


$currentVer = $_GET["fw"];
$model = $_GET["model"];
$channel = $_GET["ch"];

if($channel == ""){
	$channel = "prod";
};

if (is_dir("./files/" . $model . "/" . $channel)) {
	$files = scandir("./files/" . $model . "/" . $channel);
	if (count($files) <= 2) {
		echo "no files";
	} else {
		array_shift($files);
		array_shift($files);
		/*version_compare() returns
		-1 if the first version is lower than the second,
		0 if they are equal, and
		1 if the second is lower. */
		$highest = "v0.0.0";
		foreach ($files as $file) {
			$file = explode(".bin", $file)[0];
			if (version_compare($highest, $file) == -1) {
				$highest = $file;
			}
		};
		echo $highest;
	};
} else {
	echo "bad model";
}
