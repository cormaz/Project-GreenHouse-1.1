<?php
$servername = "localhost";
$username = "user";
$password = "pass";
$dbname = "my_db";

$id = $_GET["name"];
$temp = $_GET["parameter1"];
$umid = $_GET["parameter2"];


echo "name ". $_GET['name']. "<br />";
echo "parameter1 ". $_GET['parameter1']. "<br />";
echo "parameter2 ". $_GET['parameter2']. "<br />";


// Create connection
$conn = mysqli_connect($servername, $username, $password, $dbname);
// Check connection
if (!$conn) {
    die("Connection failed: " . mysqli_connect_error());
}

$sql = "INSERT INTO greenhouse (name, parameter1, parameter2)
VALUES ('".$name."', '".$parameter1."', '".$parameter2."')";

if (mysqli_query($conn, $sql)) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . mysqli_error($conn);
}

mysqli_close($conn);
?>
