<?php
$username = "root";
$password = "";
$hostname = "localhost"; 

//connection to the database
$dbhandle = mysql_connect($hostname, $username, $password) 
 or die("Unable to connect to MySQL");
//echo "Connected to MySQL<br>";

//select a database to work with
$selected = mysql_select_db("serra",$dbhandle) 
  or die("Could not select examples");
  
$datada = $_GET["datada"];
$dataa = $_GET["dataa"];

//execute the SQL query and return records
$result = mysql_query("SELECT hum,temp,date FROM record WHERE date >= '".$datada."' and date <='".$dataa."'");


$records=array();
//fetch tha data from the database 
while ($row = mysql_fetch_array($result)) {
	$tmp = array("hum"=>$row{'hum'}, "temp"=>$row{'temp'}, "date"=>$row{'date'});
	array_push($records,$tmp);
	//echo "HUM:".$row{'hum'}." TEMP:".$row{'temp'}."Date: ". //display the results
	$row{'date'}."<br>";
}

echo json_encode($records);

//close the connection
mysql_close($dbhandle);
?>
