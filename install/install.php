<?php
$_POST = json_decode(file_get_contents("php://input"),true);
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    if (!empty($_POST["req"])) {
        if (($_POST["req"] == "poop") && (!empty($_POST["dbhost"])) && (!empty($_POST["dbuser"])) && (!empty($_POST["dbname"]))) {
            ini_set('display_errors', 0);
            ini_set('log_errors', 0);
            $dbhost = strtolower($_POST['dbhost']);
            $dbuser = $_POST["dbuser"];
            $dbpass = $_POST["dbpass"];
            $dbname = $_POST["dbname"];
            $con = mysqli_connect($dbhost, $dbuser, $dbpass, $dbname);
            if (mysqli_connect_errno()) {
                echo mysqli_connect_errno();
                exit;
            } else {
                mysqli_close($con);
                echo 0;

                $change = file_get_contents("globals.js");
                $change = str_replace("POOP1", $_POST['pathprefix'], $change);
                file_put_contents("../config/globals.js", $change);

                $change = file_get_contents("database.php");
                $change = str_replace("poop1", $dbhost, $change);
                $change = str_replace("poop2", $dbuser, $change);
                $change = str_replace("poop3", $dbpass, $change);
                $change = str_replace("poop4", $dbname, $change);
                $change = str_replace("poop5", $_POST['gitlabhost'], $change);
                $change = str_replace("poop6", $_POST['gitlabuser'], $change);
                $change = str_replace("poop7", $_POST['gitlabtoken'], $change);
                file_put_contents("../config/database.php", $change);
                include "../config/database.php";
                $filename = ($_POST["option"]) ? 'play-upgrade.sql':'play.sql';
                $templine = '';
                $lines = file($filename);
                foreach ($lines as $line) {
                    if (substr($line, 0, 2) == '--' || $line == '')
                        continue;
                    $templine .= $line;
                    if (substr(trim($line), -1, 1) == ';') {
                        $db->query($templine);
                        $templine = '';
                    }
                }

                $pass_hash = $db->escape_string(password_hash(
                            $_POST['teacherpasswd'], PASSWORD_BCRYPT, ["cost"=>10]));
                $action = $db->query("INSERT INTO users (name, passwd, lastSignIn)
                    VALUES ('老师', '$pass_hash', NOW())");

                $db->close_connection();
            };
        };
    }
}
?>
