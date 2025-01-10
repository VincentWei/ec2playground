<?php

require_once("utils.php");
require_once("Result.php");
require_once("HttpUtils.php");

if (!(include "../config/database.php")) {
    $result = new Result(100, "Bad installation.");
    goto error;
}

if ($_SERVER["REQUEST_METHOD"] != "POST" || empty($_POST["digest"])) {
    $result = new Result(100, 'Bad request.');
    goto error;
}

$digest = $db->escape_string($_POST["digest"]);
$db_result = $db->query("SELECT id, gitlabId FROM snippets WHERE digest = '$digest'");
if (!$db_result) {
    $result = new Result(100, $db->last_error());
    goto error;
}

if ($db->num_rows($db_result) == 0) {
    $result = new Result(100, "No such snippet.");
    goto error;
}

$row = $db->fetch_one($db_result);
$db->free_result($db_result);

$res = HttpUtils::httpsGitLabSnippetContents($db->gitlab_host(),
        $db->gitlab_token(), $row['gitlabId']);
if (!$res) {
    $result = new Result (100, "Failed to fetch snippet contenst from GitLab server.");
    goto error;
}

$result = new Result(0, 'Success');
$result->extraMsg = $row['gitlabId'];
$result->data = $res;

error:
header('content-type:application/json;charset=utf8');
echo json_encode($result);

exit (0);
?>
