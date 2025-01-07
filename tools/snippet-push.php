<?php

require_once("utils.php");
require_once("Result.php");
require_once("HttpUtils.php");

if (!(include "../config/database.php")) {
    $result = new Result (100, "Bad installation.");
    goto error;
}

$username = $_POST['username'];
$password = $_POST['password'];
if ($_SERVER["REQUEST_METHOD"] != "POST" || empty($_POST["username"])
        || empty($_POST["password"])) {
    $result = new Result (100, 'Bad request.');
    goto error;
}

$rows = $db->query("SELECT id, passwd FROM users WHERE name = '$username'");
$record = $db->fetch_array($rows);
$userId = $record["id"];
$hashed = $record["passwd"];
if (!password_verify($password, $hashed)) {
    $result = new Result (100, "Bad password or no such user.");
    goto error;
}

$section = $_POST['section'];
$title = $_POST['title'];
$snippet = $_POST['snippet'];
if (empty($section) || empty($title) || empty($snippet)) {
    $result = new Result (100, "Bad contents.");
    goto error;
}

$res = HttpUtils::httpsGitLabSnippetNew($db->gitlab_host(),
        $db->gitlab_token(), $username, $section, $title, $snippet);

if (!is_array($res)) {
    $result = new Result (100, "Failed to push snipeet to GitLab server.");
    goto error;
}

$digest = $res['_digest'];
$gitlabId = $res['id'];

$action = $db->query("INSERT INTO snippets (digest, userId, section, title, gitlabId)
        VALUES ('$digest', $userId, '$section', '$title', $gitlabId)");

if (!$action) {
    $result = new Result (100, "Failed to insert new record for the snippet.");
    goto error;
}

$result = new Result(0, 'Success');
$result->extrMsg = $digest;
$result->data = $gitlabId;

error:
header('content-type:application/json;charset=utf8');
echo json_encode($result);

exit (0);
?>
