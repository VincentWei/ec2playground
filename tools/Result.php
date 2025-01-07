<?php

class Result {
    public $retCode = 0;
    public $retMsg = 'Success.';
    public $extraMsg = '';
    public $data = NULL;

    function __construct ($retCode, $retMsg) {
        $this->retCode = $retCode;
        $this->retMsg = $retMsg;
    }

    function __get ($name) {
        return $this->$name;
    }

    function __set ($name, $value) {
        $this->$name = $value;
    }
}

?>
