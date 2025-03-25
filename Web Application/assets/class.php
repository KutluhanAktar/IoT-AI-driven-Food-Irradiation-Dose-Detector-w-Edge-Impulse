<?php

// Define the _main class and its functions:
class _main {
	public $conn, $table;
	
	public function __init__($conn, $table){
		$this->conn = $conn;
		$this->table = $table;
	}
	
    // Database -> Insert Data:
	public function insert_new_data($d1, $d2, $d3, $d4, $d5, $d6, $d7, $d8, $d9, $d10, $d11, $d12, $c){
		$sql = "INSERT INTO `$this->table`(`weight`, `f1`, `f2`, `f3`, `f4`, `f5`, `f6`, `f7`, `f8`, `cpm`, `nsv`, `usv`, `class`) VALUES ('$d1', '$d2', '$d3', '$d4', '$d5', '$d6', '$d7', '$d8', '$d9', '$d10', '$d11', '$d12', '$c')";
		if(mysqli_query($this->conn, $sql)){ return true; }else { return false; }
	}

	// Database -> Create Table
	public function database_create_table(){
		// Create a new database table.
		$sql_create = "CREATE TABLE `$this->table`(		
							id int AUTO_INCREMENT PRIMARY KEY NOT NULL,
							weight varchar(255) NOT NULL,
							f1 varchar(255) NOT NULL,
							f2 varchar(255) NOT NULL,
							f3 varchar(255) NOT NULL,
							f4 varchar(255) NOT NULL,
							f5 varchar(255) NOT NULL,
							f6 varchar(255) NOT NULL,
							f7 varchar(255) NOT NULL,
							f8 varchar(255) NOT NULL,
							cpm varchar(255) NOT NULL,
							nsv varchar(255) NOT NULL,
							usv varchar(255) NOT NULL,
							`class` varchar(255) NOT NULL
					   );";
		if(mysqli_query($this->conn, $sql_create)) echo("<br><br>Database Table Created Successfully!");
	}
}

// Define the sample class and its functions:
class sample Extends _main {
	// Define the irradiation dose class (label) names.
	public $class_names = ["Regulated", "Unsafe", "Hazardous"];
	
	// Count the registered data records (samples) in the given database table. 
	public function count_samples(){
		$count = [
			"total" => mysqli_num_rows(mysqli_query($this->conn, "SELECT * FROM `$this->table`")),
			"regulated" => mysqli_num_rows(mysqli_query($this->conn, "SELECT * FROM `$this->table` WHERE class='0'")),
			"unsafe" => mysqli_num_rows(mysqli_query($this->conn, "SELECT * FROM `$this->table` WHERE class='1'")),
			"hazardous" => mysqli_num_rows(mysqli_query($this->conn, "SELECT * FROM `$this->table` WHERE class='2'")),
		];
		return $count;
	}
	
	// Create a CSV file for each data record (sample) identified with the sample number.
	public function create_sample_files($type){
		// Obtain the registered data records (samples) from the given database table.
		$sql = "SELECT * FROM `$this->table`";
		$result = mysqli_query($this->conn, $sql);
		$check = mysqli_num_rows($result);
		if($check > 0){
			while($row = mysqli_fetch_assoc($result)){
				// Scale (normalize) data items to define appropriately formatted inputs (samples).
				$scaled = [
					"weight" => $row["weight"] / 10,
					"f1" => $row["f1"] / 100,
					"f2" => $row["f2"] / 100,
					"f3" => $row["f3"] / 100,
					"f4" => $row["f4"] / 100,
					"f5" => $row["f5"] / 100,
					"f6" => $row["f6"] / 100,
					"f7" => $row["f7"] / 100,
					"f8" => $row["f8"] / 100,
					"cpm" => $row["cpm"] / 100,
					"nsv" => $row["nsv"] / 100,
					"usv" => $row["usv"]
				];
				// Add the header as the first row.
				$processed_data = [
					['weight','f1','f2','f3','f4','f5','f6','f7','f8','cpm','nsv','usv'],
					[$scaled["weight"],$scaled["f1"],$scaled["f2"],$scaled["f3"],$scaled["f4"],$scaled["f5"],$scaled["f6"],$scaled["f7"],$scaled["f8"],$scaled["cpm"],$scaled["nsv"],$scaled["usv"]]
				];
				$filename = "data/".$this->class_names[$row["class"]].".".$type.".sample_".$row["id"].".csv";
				$f = fopen($filename, "w");
				foreach($processed_data as $r){
					fputcsv($f, $r);
				}
				fclose($f);
			}
		}
	}
	
	// Download all generated CSV sample files in the ZIP file format.
	public function download_samples($zipname){
		if(count(scandir("data")) > 2){
			$zip = new ZipArchive;
			$zip->open($zipname, ZipArchive::CREATE);
			foreach(glob("data/*.csv") as $sample){
				$zip->addFile($sample);
			}
			$zip->close();

			header('Content-Type: application/zip');
			header("Content-Disposition: attachment; filename='$zipname'");
			header('Content-Length: ' . filesize($zipname));
			header("Location: $zipname");
		}else{
			header("Location: .");
			exit();
		}
	}
}

// Define database and server settings:
$server = array(
	"name" => "localhost",
	"username" => "root",
	"password" => "bot",
	"database" => "foodirradiation",
	"table" => "entries"

);

$conn = mysqli_connect($server["name"], $server["username"], $server["password"], $server["database"]);

?>