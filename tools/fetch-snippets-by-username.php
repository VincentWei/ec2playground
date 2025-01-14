<?php

require_once("utils.php");
require_once("Result.php");
require_once("HttpUtils.php");

if (!(include "../config/database.php")) {
    $result = new Result(100, '初始化数据库失败；请检查安装。');
    goto error;
}

if ($_SERVER["REQUEST_METHOD"] != "POST" || empty($_POST["username"])) {
    $result = new Result(100, '缺乏必要的请求参数。');
    goto error;
}

$username = $_POST['username'];
$db_result = $db->query("SELECT id FROM users WHERE name = '$username'");
if (!$db_result) {
    $result = new Result(100, '数据库错误：' . $db->last_error());
    goto error;
}

if ($db->num_rows($db_result) == 0) {
    $result = new Result(100, '不存在的用户。');
    goto error;
}

$row = $db->fetch_one($db_result);
$userId = $row['id'];
$db->free_result($db_result);

$db_result = $db->query("SELECT users.name AS username, snippets.digest, snippets.section, snippets.title, snippets.description, snippets.gitlabId
        FROM users, snippets
        WHERE users.id = snippets.userId AND snippets.userId = $userId");
if (!$db_result) {
    $result = new Result(100, '数据库错误：' . $db->last_error());
    goto error;
}

$rows = $db->fetch_all($db_result);
$db->free_result($db_result);

$result = new Result(0, '成功');
$result->data = $rows;

error:
header('content-type:application/json;charset=utf8');
echo json_encode($result);

exit (0);
?>
