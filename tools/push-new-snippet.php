<?php

require_once("utils.php");
require_once("Result.php");
require_once("HttpUtils.php");

if (!(include "../config/database.php")) {
    $result = new Result(100, "Bad installation.");
    goto error;
}

if ($_SERVER["REQUEST_METHOD"] != "POST" || empty($_POST["username"])
        || empty($_POST["password"])) {
    $result = new Result(100, '缺少必要请求参数。');
    goto error;
}

$username = $db->escape_string($_POST['username']);
$password = $_POST['password'];
$db_result = $db->query("SELECT id, passwd FROM users WHERE name = '$username'");
if (!$db_result) {
    $result = new Result(100, '数据库错误：' . $db->last_error());
    goto error;
}

if ($db->num_rows($db_result) == 0) {
    $hashed_passwd = $db->escape_string(password_hash(
                $password, PASSWORD_BCRYPT, ["cost"=>10]));
    $db_result = $db->query("INSERT INTO users (name, passwd, lastSignIn)
            VALUES ('$username', '$hashed_passwd', NOW())");
    if (!$db_result) {
        $result = new Result(100, "数据库错误：". $db->last_error());
        goto error;
    }
}
else {
    $row = $db->fetch_one($db_result);
    $userId = $row["id"];
    $hashed = $row["passwd"];
    $db->free_result($db_result);

    if (!password_verify($password, $hashed)) {
        $result = new Result(100, '密码错误。');
        goto error;
    }
}

if (empty($_POST['section']) || empty($_POST['title'])
        || empty($_POST['snippet'])) {
    $result = new Result(100, '类名、标题或者程序为空。');
    goto error;
}

$section = $_POST['section'];
$title = $_POST['title'];
$description = empty($_POST['description']) ? "" : $_POST['description'];
$snippet = $_POST['snippet'];
$res = HttpUtils::httpsGitLabSnippetNew($db->gitlab_host(),
        $db->gitlab_token(), $username, $section, $title, $description, $snippet);

if (!is_array($res)) {
    $result = new Result (100, "将程序发布到 GitLab 服务器时出现错误。");
    goto error;
}

$digest = $res['_digest'];
$gitlabId = $res['id'];

$section = $db->escape_string($section);
$title = $db->escape_string($title);
$db_result =
    $db->query("INSERT INTO snippets (digest, userId, section, title, gitlabId)
        VALUES ('$digest', $userId, '$section', '$title', $gitlabId)");

if (!$db_result) {
    $result = new Result(100,
            '数据库错误：' . $db->last_error());
    goto error;
}

$result = new Result(0, '成功');
$result->extrMsg = $digest;
$result->data = $gitlabId;

error:
header('content-type:application/json;charset=utf8');
echo json_encode($result);

exit (0);
?>
