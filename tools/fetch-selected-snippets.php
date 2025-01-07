<?php

require_once("utils.php");
require_once("Result.php");

if (!(include "../config/database.php")) {
    $result = new Result (100, "Bad installation.");
    goto error;
}

$db_result = $db->query("SELECT snippets.id, snippets.userId, snippets.section, snippets.title, snippets.gitlabId FROM snippets, selectedSnippets WHERE selectedSnippets.snippetId = snippets.id");
if (!$db_result) {
    $result = new Result(100, $db->last_error());
    goto error;
}

$rows = $db->fetch_all($db_result);
$db->free_result($db_result);

$result = new Result(0, 'Success');
$result->extrMsg = $userId;
$result->data = $rows;

error:
header('content-type:application/json;charset=utf8');
echo json_encode($result);

exit (0);
?>
