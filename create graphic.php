<?php // content="text/plain; charset=utf-8"
 
define('__ROOT__', dirname(dirname(__FILE__))); 
require_once ('../jpgraph.php');
require_once ('../jpgraph_line.php');
require_once ('../jpgraph_error.php');
 
$parameter1 = array();
$parameter2 = array();
$time_axis = array();
$i = 0;
 
$con=mysqli_connect("localhost","user","pass","my_db");
// Check connection
if (mysqli_connect_errno()) {
  echo "Failed to connect to MySQL: " . mysqli_connect_error();
}
 
$result = mysqli_query($con,"SELECT your_parameter FROM your_raw");
 
while($row = mysqli_fetch_array($result)) {
$parameter1[$i] =  $row["parameter1"];
$parameter2[$i] = $row["parameter2"];
$time_axis[$i] = $row["date"];
    $i++;
}
     
mysqli_close($con);
 
// Setup the graph
$graph = new Graph(300,250);
$graph->SetScale("textlin");

$theme_class=new UniversalTheme;

$graph->SetTheme($theme_class);
$graph->img->SetAntiAliasing(false);
$graph->title->Set('Title');
$graph->SetBox(false);

$graph->img->SetAntiAliasing();

$graph->yaxis->HideZeroLabel();
$graph->yaxis->HideLine(false);
$graph->yaxis->HideTicks(false,false);

$graph->xgrid->Show();
$graph->xgrid->SetLineStyle("solid");
$graph->xaxis->SetTickLabels($time_axis);
$graph->xgrid->SetColor('#E3E3E3');
$graph->xaxis->SetLabelAngle(90);
$graph->legend->SetPos(0.5,0.08,'center','top');

// Create the first line
$p1 = new LinePlot($parameter1);
$graph->Add($p1);
$p1->SetColor("#6495ED");
$p1->SetLegend('your_parameter1');
$graph->yscale->SetGrace(0);

// Create the second line
$p2 = new LinePlot($parameter2);
$graph->Add($p2);
$p2->SetColor("#B22222");
$p2->SetLegend('your_parameter2');

$graph->yscale->SetGrace(0);

$graph->legend->SetFrameWeight(1);

// Output line
$graph->Stroke();

?>