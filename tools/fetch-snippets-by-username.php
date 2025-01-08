<?php

require_once("utils.php");
require_once("Result.php");
require_once("HttpUtils.php");

if (!(include "../config/database.php")) {
    $result = new Result (100, "Bad installation.");
    goto error;
}

if ($_SERVER["REQUEST_METHOD"] != "POST" || empty($_POST["username"])) {
    $result = new Result (100, 'Bad request.');
    goto error;
}

$username = $_POST['username'];
$db_result = $db->query("SELECT id FROM users WHERE name = '$username'");
if (!$db_result) {
    $result = new Result(100, $db->last_error());
    goto error;
}

if ($db->num_rows($db_result) == 0) {
    $result = new Result(100, "No such user.");
    goto error;
}

$row = $db->fetch_one($db_result);
$userId = $row['id'];
$db->free_result($db_result);

$db_result = $db->query(
  "SELECT digest, section, title, gitlabId FROM snippets WHERE userId = $userId");
if (!$db_result) {
    $result = new Result(100, $db->last_error());
    goto error;
}

$rows = $db->fetch_all($db_result);
$db->free_result($db_result);

$result = new Result(0, 'Success');
$result->data = $rows;

error:
header('content-type:application/json;charset=utf8');
echo json_encode($result);

exit (0);
?>
