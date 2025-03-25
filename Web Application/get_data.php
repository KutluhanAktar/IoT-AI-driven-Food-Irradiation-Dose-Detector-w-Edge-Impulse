<?php

include_once "assets/class.php";

// Define the new 'food' object:
$food = new _main();
$food->__init__($conn, $server["table"]); 

// Obtain the transferred information from the Beetle ESP32-C3.
// Then, insert the received information into the given database table.
if(isset($_GET["weight"]) && isset($_GET["F1"]) && isset($_GET["F2"]) && isset($_GET["F3"]) && isset($_GET["F4"]) && isset($_GET["F5"]) && isset($_GET["F6"]) && isset($_GET["F7"]) && isset($_GET["F8"]) && isset($_GET["CPM"]) && isset($_GET["nSv"]) && isset($_GET["uSv"]) && isset($_GET["class"])){
	if($food->insert_new_data($_GET["weight"], $_GET["F1"], $_GET["F2"], $_GET["F3"], $_GET["F4"], $_GET["F5"], $_GET["F6"], $_GET["F7"], $_GET["F8"], $_GET["CPM"], $_GET["nSv"], $_GET["uSv"], $_GET["class"])){
		echo("Data received and saved successfully!");
	}else{
		echo("Database error!");
	}
}else{
	echo("Waiting Data...");
}

// If requested, create a new database table.
if(isset($_GET["create_table"]) && $_GET["create_table"] == "OK") $food->database_create_table();

?>