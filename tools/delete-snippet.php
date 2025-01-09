<?php

require_once("utils.php");
require_once("Result.php");
require_once("HttpUtils.php");

if (!(include "../config/database.php")) {
    $result = new Result(100, "初始化数据库失败；请检查安装。");
    goto error;
}

if ($_SERVER["REQUEST_METHOD"] != "POST" || empty($_POST["username"])
        || empty($_POST["password"]) || empty($_POST["digest"])) {
    $result = new Result(100, '缺少必要的请求参数。');
    goto error;
}

$username = $db->escape_string($_POST['username']);
$password = $_POST['password'];
$digest = $_POST['digest'];
$db_result = $db->query("SELECT id, passwd FROM users WHERE name = '$username'");
if (!$db_result) {
    $result = new Result(100, '数据库错误：' . $db->last_error());
    goto error;
}

if ($db->num_rows($db_result) == 0) {
    $result = new Result(100, '没有指定的用户。');
    goto error;
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

$db_result = $db->query("SELECT gitlabId FROM snippets WHERE digest = '$digest'");
if (!$db_result) {
    $result = new Result(100, '数据库错误：' . $db->last_error());
    goto error;
}

if ($db->num_rows($db_result) == 0) {
    $db->free_result($db_result);
    $result = new Result(100, '没有指定的程序。');
    goto error;
}
else {
    $row = $db->fetch_one($db_result);
    $gitlabId = $row["gitlabId"];
    $db->free_result($db_result);
}

$res = HttpUtils::httpsGitLabSnippetDelete($db->gitlab_host(), $db->gitlab_token(),
        $gitlabId);

if (!is_array($res)) {
    $result = new Result (100, "将程序从 GitLab 服务器删除时出现错误。");
    goto error;
}

$db_result =
    $db->query("DELETE FROM snippets WHERE digest='$digest'");
if (!$db_result) {
    $result = new Result(100, '数据库错误：' . $db->last_error());
    goto error;
}

if ($db->affected_rows() == 0) {
    $result = new Result(100, '未能删除指定的程序。');
    goto error;
}

$result = new Result(0, '成功');
$result->extraMsg = $digest;

error:
header('content-type:application/json;charset=utf8');
echo json_encode($result);

exit (0);
?>

