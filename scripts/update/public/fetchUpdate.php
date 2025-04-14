<?php


$currentVer = $_GET["fw"];


$specificVer = "";

if (array_key_exists("version", $_GET))
	$specificVer = $_GET["version"];


if ($currentVer == "") {
	$currenVer = "v0.0.0";
}
$model = $_GET["model"];

if (is_dir("./files/" . $model)) {
	$files = scandir("./files/" . $model);
	if (count($files) <= 2) {
		echo "no files";
	} else {
		array_shift($files);
		array_shift($files);
		if ($specificVer != "") {
			if (in_array($specificVer . ".bin", $files)) {
				sendFile($specificVer, $model);
			} else {
				echo "0";
			};
			return;
		};

		$highest = "v0.0.0";
		/*version_compare() returns
		-1 if the first version is lower than the second,
		0 if they are equal, and
		1 if the second is lower. */
		foreach ($files as $file) {
			$file = explode(".bin", $file)[0];
			if (version_compare($highest, $file) == -1) {
				$highest = $file;
			}
		};
		if (version_compare($currentVer, $highest) < 0) {
			sendFile($highest, $model);
		} else {
			echo "0";
		}
	};
} else {
	echo "bad model";
}

function sendFile($name, $_model)
{
	header('Content-Disposition: attachment; filename="DMXCube mini W_' . $name . '.bin"');
	$filename = "./files/" . $_model . "/" . $name . ".bin";
	header('Content-Type: application/octet-stream');
	header("Content-Length: " . filesize($filename));
	readfile($filename);
}
