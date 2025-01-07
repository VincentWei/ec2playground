<?php

/* NOTE: Change this to 0 if you do not want to log info. */
define ('ENABLE_LOG', 1);

function my_log($info) {
    if (ENABLE_LOG == 0)
        return;

    error_log(date(DATE_ATOM, time()) . ": $info\n", 3, '/var/tmp/' . $_SERVER['SERVER_NAME'] . '.log');
}

?>
