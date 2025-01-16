<?php

require_once("utils.php");
require_once("Result.php");
require_once("HttpUtils.php");

if (!(include "../config/database.php")) {
    $result = new Result(100, '初始化数据库失败；请检查安装。');
    goto error;
}

if ($_SERVER["REQUEST_METHOD"] != "POST" || empty($_POST["digest"])) {
    $result = new Result(100, '缺少必要的请求参数。');
    goto error;
}

$digest = $db->escape_string($_POST["digest"]);
$db_result = $db->query( "SELECT
        users.name AS username, snippets.title, snippets.section, snippets.description, snippets.gitlabId, snippets.createdAt, snippets.updatedAt
        FROM users, snippets
        WHERE users.id = snippets.userId AND snippets.digest = '$digest'");
if (!$db_result) {
    $result = new Result(100, '数据库错误：' . $db->last_error());
    goto error;
}

if ($db->num_rows($db_result) == 0) {
    $db->free_result($db_result);
    $result = new Result(100, '不存在指定的程序。');
    goto error;
}

$row = $db->fetch_one($db_result);
$db->free_result($db_result);

$result = new Result(0, '成功');
$result->extraMsg = $digest;
$result->data = $row;

error:
header('content-type:application/json;charset=utf8');
echo json_encode($result);

exit (0);
?>
