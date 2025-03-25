<?php
	include_once "assets/class.php";
	
	// Define the new 'sample' object: 
	$sample = new sample();
	$sample->__init__($conn, $server["table"]);
    
    // Elicit the total number of data records (samples) for classes (labels) in the given database table.
    $count = $sample->count_samples();

    // Create a CSV file for each data record (sample) in the given database table.
    if(isset($_POST["data"]) && $_POST["data"] != ""){
		$sample->create_sample_files($_POST["data"]);
	}
	
    // Download all generated CSV sample files in the ZIP file format.
    if(isset($_GET["download"])){
		$sample->download_samples("data.zip");
	}	
?>
<!DOCTYPE html>
<html>
<head>
<title>Food Irradiation Data Logger</title>

<!--link to index.css-->
<link rel="stylesheet" type="text/css" href="assets/index.css"></link>

<!--link to favicon-->
<link rel="icon" type="image/png" sizes="36x36" href="assets/icon.png">

<!-- link to FontAwesome-->
<link rel="stylesheet" href="https://use.fontawesome.com/releases/v6.1.1/css/all.css">
 
<!-- link to font -->
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Oswald&display=swap" rel="stylesheet">

</head>
<body>
<?php ini_set('display_errors',1);?> 
<h1><i class="fa-solid fa-circle-radiation"></i> Food Irradiation Data Logger</h1>

<div class="container">
<section>
<h2>Created Samples:</h2>
<table>
  <tr>
    <th>Samples</th>
    <th>Download</th>
  </tr>
  <tr>
	<td>Download all samples.</td>
	<td><a href="?download"><button><i class="fa-solid fa-cloud-arrow-down"></i></button></a></td>
  </tr>
<?php
  foreach(glob("data/*.csv") as $sample){
	  echo '
        <tr>
           <td>'.explode("/", $sample)[1].'</td>
        </tr>		   
	  ';
  }
?>
</table>
</section>
<section>
<div>
<h2><i class="fa-solid fa-database"></i> Database Status:</h2>
<p>Total Samples: <span><?php echo $count["total"]; ?></span></p>
<p>Regulated: <span><?php echo $count["regulated"]; ?></span></p>
<p>Unsafe: <span><?php echo $count["unsafe"]; ?></span></p>
<p>Hazardous: <span><?php echo $count["hazardous"]; ?></span></p>
<form method="post">
<fieldset>
<legend>Data Type</legend>
<br>
<label><input type="radio" name="data" value="training" /><span class="mark"></span> Training</label>
<label><input type="radio" name="data" value="testing" /><span class="mark"></span> Testing</label>
<br><br>
</fieldset>
<br>
<button type="submit"><i class="fa-solid fa-folder-plus"></i> Create Samples</button>
</form>
</div>
</section>
</div>
</body>
</html>