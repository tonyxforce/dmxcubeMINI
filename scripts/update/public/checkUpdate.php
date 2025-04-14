<?php


$currentVer = $_GET["fw"];
$model = $_GET["model"];

if (is_dir("./files/" . $model)) {
	$files = scandir("./files/" . $model);
	if (count($files) <= 2) {
		echo "no files";
	} else {
		array_shift($files);
		array_shift($files);
		/*version_compare() returns
		-1 if the first version is lower than the second,
		0 if they are equal, and
		1 if the second is lower. */
		$highest = $currentVer;
		foreach ($files as $file) {
			$file = explode(".bin", $file)[0];
			if (version_compare($highest, $file) == -1) {
				$highest = $file;
			}
		};
		echo version_compare($highest, $currentVer);
	};
} else {
	echo "bad model";
}
