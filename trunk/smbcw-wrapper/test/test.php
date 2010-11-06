<?php

function get_access($mode)
{
	$res = ($mode & 040000) ? "d" : "-";

	$mode &= 0777;
	for ($i = 2; $i >= 0; $i--)
	{
		$mask = (7 << ($i * 3));
		$part = ($mode & $mask) >> ($i * 3);
		$res .= ($part & 4) ? "r" : "-";
		$res .= ($part & 2) ? "w" : "-";
		$res .= ($part & 1) ? "x" : "-";
	}

	return $res;
}


$url = "smb://smb_user:smb_user@localhost/smb_test/foo.test";

if (is_readable($url))
{
	$fd = fopen($url, "r");
	if ($fd) {
		while (!feof($fd)) {
			$str = fread($fd,512);
			echo($str);
		}
		fclose($fd);
	}
} else {
	echo("Url $url is not readable.\n");
}

//List the given directory
$url = "smb://smb_user:smb_user@localhost/smb_test/";
$fd = opendir($url);
if ($fd)
{
	while (false !== ($file = readdir($fd)))
	{
		//Get the stat
		$stat = stat($url.$file);
		
		echo(get_access($stat['mode'])." $file\n");
	}
}

echo("UID: ".getmyuid()." ".$stat['uid']."\n");
echo("GID: ".getmygid()." ".$stat['gid']."\n");

echo("> Done.\n");

