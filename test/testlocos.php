#!/usr/bin/php
<?php

include('functions.php');

$socket = startrailcontrol();

// list locos
socket_write($socket, "lla\n");
$data = socket_read($socket, 128);
$numLocos = substr_count($data, "\n") - 1;
if ($numLocos != 0) {
	stoprailcontrol($socket, "Number of locos at start is not 0");
}

stoprailcontrol($socket);

?>
