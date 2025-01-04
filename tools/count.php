<?php

$today = date("Y-m-d");
$nrUsers = $db->query("SELECT COUNT(*) FROM users");
$nrSnippets = $db->query("SELECT COUNT(*) FROM snippets");

?>
