<?php

require_once("utils.php");
require_once("Result.php");
require_once("HttpUtils.php");

if (!(include "../config/database.php")) {
    $result = new Result(100, '初始化数据库失败；请检查安装。');
    goto error;
}

if ($_SERVER["REQUEST_METHOD"] != "POST" || empty($_POST["username"])
        || empty($_POST["password"])) {
    $result = new Result(100, '缺少必要的请求参数。');
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
    $username = trim($username);
    if (strlen($username) < 2) {
        $result = new Result(100, '用户名太短；需要至少两个字符。');
        goto error;
    }
    if (strlen($password) < 8) {
        $result = new Result(100, '密码太短；需要至少八个字符。');
        goto error;
    }

    $hashed_passwd = $db->escape_string(password_hash(
                $password, PASSWORD_BCRYPT, ["cost"=>10]));
    $db_result = $db->query("INSERT INTO users (name, passwd, lastSignIn)
            VALUES ('$username', '$hashed_passwd', NOW())");
    if (!$db_result) {
        $result = new Result(100, '数据库错误：'. $db->last_error());
        goto error;
    }
    $userId = $db->insert_id();
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
    $result = new Result(100, '程序分类、标题或者代码为空。');
    goto error;
}

$section = $_POST['section'];
$title = $_POST['title'];
$description = empty($_POST['description']) ? "" : $_POST['description'];
$snippet = $_POST['snippet'];
$digest = hash_hmac('md5', "$username-$section-$title-$description-$snippet",
        $db->gitlab_host());

$db_result =
    $db->query("SELECT id FROM snippets WHERE digest='$digest'");
if (!$db_result) {
    $result = new Result(100, '数据库错误：' . $db->last_error());
    goto error;
}

if ($db->num_rows($db_result) > 0) {
    $db->free_result($db_result);
    $result = new Result(100, '您分享过一模一样的程序。');
    goto error;
}
else {
    $db->free_result($db_result);
}

$res = HttpUtils::httpsGitLabSnippetNew($db->gitlab_host(), $db->gitlab_token(),
        $username, $section, $title, $description, $snippet, $digest);

if (!is_array($res)) {
    $result = new Result (100, "将程序发布到 GitLab 服务器时出现错误。");
    goto error;
}

$digest = $res['_digest'];
$gitlabId = $res['id'];

$section = $db->escape_string($section);
$title = $db->escape_string($title);
$description = $db->escape_string($description);
$db_result =
    $db->query("INSERT INTO snippets (digest, userId, section, title, description, gitlabId)
        VALUES ('$digest', $userId, '$section', '$title', '$description', $gitlabId)");

if (!$db_result) {
    $result = new Result(100,
            '数据库错误：' . $db->last_error());
    goto error;
}

$result = new Result(0, '成功');
$result->extraMsg = $digest;
$result->data = true;

error:
header('content-type:application/json;charset=utf8');
echo json_encode($result);

exit (0);
?>

