<?php

require_once("utils.php");
require_once("Result.php");

if (!(include "../config/database.php")) {
    $result = new Result(100, '初始化数据库失败；请检查安装。');
    goto error;
}

$db_result = $db->query("SELECT users.name AS username, snippets.digest, snippets.section, snippets.title, snippets.description, snippets.gitlabId
        FROM users, snippets
        WHERE users.id = snippets.userId
        ORDER BY snippets.rankValue DESC
        LIMIT 10");
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
