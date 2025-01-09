<?php

class HttpUtils {
    public static function httpsGetAndOutput($url, $auth = true) {
        if ($auth) {
            $url .= JBT_API_ACCESS_TOKEN;
        }

        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_TIMEOUT, 30);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, false);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
        curl_setopt($ch, CURLOPT_HEADER, false);
        curl_exec($ch);
        $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);

        if ($http_code != '200') {
            my_log("HttpUtils::httpsRequest: error when getting response from backend server: $url returned $http_code.");
        }
    }

    public static function httpsRequest($url, $data = null, $header = null) {
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
        curl_setopt($ch, CURLOPT_TIMEOUT, 30);
        if (!empty($data)) {
            curl_setopt($ch, CURLOPT_POST, true);
            curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
        }
        if (!empty($header)) {
            curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
        }

        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);

        $res = curl_exec($ch);
        if ($res === false) {
            my_log("HttpUtils::httpsRequest: Fatal error: request to API Server Error.");
            $res = '{"error": "Fatal", "retCode": -1, "retMsg": "Fatal error: request to API Server Error."}';
        }
        else {
            $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            if ($http_code != '200') {
                my_log("HttpUtils::httpsRequest: error when getting response from backend server: $url returned $http_code.");
            }
        }

        curl_close($ch);
        return $res;
    }

    public static function httpsRequestJson($url, $data, $method = 'POST') {
        $dataString = json_encode($data);

        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
        if ($method == 'POST') {
            curl_setopt($ch, CURLOPT_POST, true);
        }
        else {
            curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $method);
        }
        curl_setopt($ch, CURLOPT_TIMEOUT, 30);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $dataString);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPHEADER, array(
            'Content-Type: application/json',
            'Content-Length: ' . strlen($dataString))
        );

        $res = curl_exec($ch);
        if ($res === false) {
            my_log("HttpUtils::httpsRequestJson: Fatal error: request to API Server Error.");
            $res = '{"retCode": -1, "retMsg": "Fatal error: request to API Server Error: "}';
        }
        else {
            $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            if ($http_code != '200') {
                my_log("HttpUtils::httpsRequestJson: error when getting response from backend server: $url returned $http_code.");
            }
        }

        curl_close($ch);
        return $res;
    }

    public static function responseImage($pathname, $contentType = '') {
        if (empty($contentType)) {
            $contentType = mime_content_type($pathname);
        }

        header('Content-type: ' . $contentType);
        readfile($pathname);
    }

    public static function acceptFromHttp($http_accept_language) {
        if (isset($_SERVER['HTTP_ACCEPT_LANGUAGE'])) {
            // break up string into pieces(languages and q factors)
            preg_match_all('/([a-z]{1,8}(-[a-z]{1,8})?)\s*(;\s*q\s*=\s*(1|0\.[0-9]+))?/i', $_SERVER['HTTP_ACCEPT_LANGUAGE'], $lang_parse);

            if (count($lang_parse[1])) {
                // create a list like "en" => 0.8
                $langs = array_combine($lang_parse[1], $lang_parse[4]);

                // set default to 1 for any without q factor
                foreach ($langs as $lang => $val) {
                    if ($val === '') $langs[$lang] = 1;
                }

                // sort list based on value
                arsort($langs, SORT_NUMERIC);
            }
        }

        // look through sorted list and use first one that matches our languages
        foreach ($langs as $lang => $val) {
            return $lang;
        }
    }

    public static function httpsGetMedia($url, $temp_dir, $temp_file_prefix, $desired_type = '/^image/') {
        $ch = curl_init($url);
        curl_setopt($ch, CURLOPT_TIMEOUT, 100);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);

        $start_time = microtime(true);
        $raw = curl_exec($ch);
        $end_time = microtime(true);
        if (($end_time - $start_time) > 1.0) {
            my_log("HttpUtils::httpsGetMedia takes " . ($end_time - $start_time) . " seconds ($url)");
        }

        if ($raw === false) {
            my_log("HttpUtils::httpsGetMedia: Fatal error when pulling media from backend server ($url).");
        }
        else {
            $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            if ($http_code != '200') {
                my_log("HttpUtils::httpsGetMedia: error when pulling media from backend server ($url): $http_code.");
                $raw = false;
            }

            $content_type = curl_getinfo($ch, CURLINFO_CONTENT_TYPE);
            if (!preg_match ($desired_type, $content_type)) {
                my_log("HttpUtils::httpsGetMedia: Server error: not desired content type ($url): $raw.");
                $raw = false;
            }
        }
        curl_close($ch);

        if ($raw !== false && $temp_file_prefix) {
            $tmp_pathname = tempnam($temp_dir, $temp_file_prefix);
            $tmp = fopen($tmp_pathname, 'w');
            fwrite($tmp, $raw);
            fclose($tmp);
            return $tmp_pathname;
        }

        return false;
    }

    public static function httpsPostMedia($url, $media_file, $ctn_type = 'image/jpeg') {
        if (!file_exists($media_file)) {
            return false;
        }

        $ch = curl_init($url);
        curl_setopt($ch, CURLOPT_TIMEOUT, 100);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, file_get_contents($media_file));
        curl_setopt($ch, CURLOPT_HTTPHEADER, array (
            "Content-Type: $ctn_type",
            'Content-Length: ' . filesize($media_file))
        );

        $start_time = microtime(true);
        $res = curl_exec($ch);
        $end_time = microtime(true);
        if (($end_time - $start_time) > 1.0) {
            my_log("HttpUtils::httpsPostMedia takes " . ($end_time - $start_time) . " seconds ($url)");
        }

        if ($res === false) {
            my_log("HttpUtils::httpsPostMedia: Fatal error when posting media to backend server ($url).");
            $res = '{"retCode": -1, "retMsg": "Fatal error: request to API Server Error"}';
        }
        else {
            $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            if ($http_code != '200') {
                my_log("HttpUtils::httpsPostMedia: error when posting media to backend server ($url): $http_code");
            }
        }

        curl_close($ch);
        return $res;
    }

    public static function httpsGitLabSnippetNew($server, $access_token,
            $username, $section, $title, $description, $snippet, $digest) {
        $url = 'https://' . $server . '/api/v4/snippets';

        $title = "$username-$section-$title";
        $array = array(
                'title' => "$title",
                'description' => $description,
                'visibility' => 'public',
                'files' => array (
                    array (
                        'content' => $snippet,
                        'file_path' => "$digest.ec2",
                        ),
                    ),
                );

        $contents = json_encode($array);

        $ch = curl_init($url);
        curl_setopt($ch, CURLOPT_TIMEOUT, 100);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $contents);
        curl_setopt($ch, CURLOPT_HTTPHEADER, array (
            "PRIVATE-TOKEN: $access_token",
            "Content-Type: application/json")
        );

        $start_time = microtime(true);
        $res = curl_exec($ch);
        $end_time = microtime(true);
        if (($end_time - $start_time) > 3.0) {
            my_log("HttpUtils::httpsGitLabSnippetNew takes " . ($end_time - $start_time) . " seconds ($url)");
        }

        if ($res === false) {
            my_log("HttpUtils::httpsGitLabSnippetNew: Fatal error when posting snippet to the GitLab server ($url).");
        }
        else {
            $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            if ($http_code != '201' && $http_code != '200') {
                my_log("HttpUtils::httpsGitLabSnippetNew: error when creating new snippet on backend server ($url): $http_code");
                $res = false;
            }
            else {
                $res = json_decode($res, true);
                $res['_digest'] = $digest;
            }
        }

        curl_close($ch);
        return $res;
    }

    public static function httpsGitLabSnippetDelete($server, $access_token,
            $gitlabId) {
        $url = "https://$server/api/v4/snippets/$gitlabId";
        $ch = curl_init($url);
        curl_setopt($ch, CURLOPT_TIMEOUT, 100);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_setopt($ch, CURLOPT_HTTPHEADER, array (
            "PRIVATE-TOKEN: $access_token")
        );

        $start_time = microtime(true);
        $res = curl_exec($ch);
        $end_time = microtime(true);
        if (($end_time - $start_time) > 3.0) {
            my_log("HttpUtils::httpsGitLabSnippetNew takes " . ($end_time - $start_time) . " seconds ($url)");
        }

        if ($res === false) {
            my_log("HttpUtils::httpsGitLabSnippetNew: Fatal error when deleting snippet to the GitLab server ($url).");
        }
        else {
            $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            if ($http_code != '204' && $http_code != '200') {
                my_log("HttpUtils::httpsGitLabSnippetNew: error when deleting snippet on backend server ($url): $http_code");
                $res = false;
            }
            else {
                $res = true;
            }
        }

        curl_close($ch);
        return $res;
    }

    public static function httpsGitLabSnippetContents($server, $access_token, $snippet_id) {
        $url = 'https://' .  $server . "/api/v4/snippets/$snippet_id/raw";

        $ch = curl_init($url);
        curl_setopt($ch, CURLOPT_TIMEOUT, 100);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPHEADER, array("PRIVATE-TOKEN: $access_token"));

        $start_time = microtime(true);
        $res = curl_exec($ch);
        $end_time = microtime(true);
        if (($end_time - $start_time) > 3.0) {
            my_log("HttpUtils::httpsGitLabSnippetNew takes " . ($end_time - $start_time) . " seconds ($url)");
        }

        if ($res === false) {
            my_log("HttpUtils::httpsGitLabSnippetNew: Fatal error when fetching snippet contents from the GitLab server ($url).");
        }
        else {
            $http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
            if ($http_code != '200') {
                my_log("HttpUtils::httpsGitLabSnippetNew: error when fetching snippet contents from backend server ($url): $http_code");
                $res = false;
            }
        }

        curl_close($ch);
        return $res;
    }
}

