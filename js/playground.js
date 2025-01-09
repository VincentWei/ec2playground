// playground.js: 初始化编辑器、基本操作按钮与页面控制等
function saveUserInfo(username, password) {
    let username_saved = window.localStorage.getItem("username");
    let password_saved = window.localStorage.getItem("password");

    if (username_saved === null || password_saved === null) {
        window.localStorage.setItem("username", username);
        window.localStorage.setItem("password", password);
    }
}

function updateUserFields() {
    let username = window.localStorage.getItem("username");
    let password = window.localStorage.getItem("password");

    if (username === null || password === null) {
        console.log("not signed in.");
    }
    else {
        let elemName = document.getElementById("shareModalStudentName");
        elemName.setAttribute("value", username);
        let elemPass = document.getElementById("shareModalParentName");
        elemPass.setAttribute("value", password);

        let elemWrap = document.getElementById("shareModalNewUserFields");
        elemWrap.style.display = "none";
    }
}

window.onload = function() {
    updateUserFields();
}

function dismissShareModal() {
    window.setTimeout(() => {
            const closeBtn = document.getElementById("shareModalClose");
            closeBtn.click();
        }, 3000);
}

function shareProgram() {
    const formElem = document.getElementById("shareModalForm");
    var formData = new FormData(formElem);
    formData.append("snippet", code);

    var request = new XMLHttpRequest();
    request.open("POST", "/ec2/tools/push-new-snippet.php");
    request.onload = function (oEvent) {
        const errElem = document.getElementById("shareModalErrorMsg");
        let response = JSON.parse(request.response);
        console.log(response);
        if (request.status == 200 && response.retCode == 0) {
            const nameElem = document.getElementById("shareModalStudentName");
            let username = nameElem.getAttribute("value");
            const passElem = document.getElementById("shareModalParentName");
            let password = passElem.getAttribute("value");
            saveUserInfo(username, password);
            updateUserFields();
            errElem.textContent = "已成功分享！";
        }
        else {
            errElem.textContent = response.retMsg;
        }

        errElem.style.display = "block";
    };

    request.send(formData);
}

function tryToShareCode() {
    let code = editor.getValue().trim();
    if (code.length < 10) {
        const msgElem = document.getElementById("promptModalMsg");
        msgElem.textContent = "至少写一个完整程序才能分享哦。";
        const promptModal = new bootstrap.Modal('#promptModal',
                { dropback: true, focus: true, keyboard: true });
    }
    else {
        const errElem = document.getElementById("shareModalErrorMsg");
        errElem.style.display = "none";

        const shareModal = new bootstrap.Modal('#shareModal',
                { dropback: true, focus: true, keyboard: true });
    }
}

window.onerror = function (message) {
    var errorMessage = `页面崩溃: ${message}\n需要刷新`;
    wasmIOprinterr(errorMessage);
    return true; // 阻止默认的错误处理
};

function runCode() {
    // 清除上一次的运行结果
    if (wasmLock == 1)
        return;
    document.getElementById('output').innerHTML = new Date().toLocaleString() + "\n--------------------------------\n";
    // 运行 WebAssembly 代码
    Module._wasmIOrunCode();
    // 结束。输出当前时间，年月日，时分秒
}

function clearCode() {
    // 弹出一个选择框，进行操作确认
    var result = confirm("确定要清空代码吗？");
    if (result) {
        // 清除代码编辑器
        editor.setValue("");
        // 清除运行结果
        document.getElementById('output').innerHTML = "";
    }
}

function downloadCode() {
    var code = editor.getValue();
    var blob = new Blob([code], { type: "text/plain;charset=utf-8" });
    var url = URL.createObjectURL(blob);
    var link = document.createElement('a');
    link.href = url;
    link.download = "code.ec2"; // 设置下载文件的名称
    link.click();
    setTimeout(function () {
        URL.revokeObjectURL(url); // 释放 URL 对象
    }, 0);
}

function editorShowHint(cm, func) {
    var hint = CodeMirror.showHint(cm, func, {
        completeSingle: false // 当只有一个匹配项时不自动完成
    });

    if (hint) {
        var cursorCoords = cm.cursorCoords();
        var hintElement = document.querySelector('.CodeMirror-hints');
        hintElement.style.left = cursorCoords.left + 'px';
        hintElement.style.top = (cursorCoords.bottom + 5) + 'px'; // 5px is a margin from the cursor
    }
}

// 初始化 CodeMirror 编辑器
var editor = CodeMirror.fromTextArea(document.getElementById('codeEditor'), {
    lineNumbers: true,
    mode: "ec2",
    theme: "solarized light",
    matchBrackets: true,
    indentUnit: 4, // 每个缩进级别使用 4 个空格
    smartIndent: true, // 启用智能缩进
    lineWrapping: true,
});

// 绑定鼠标点击事件
{
    var funcListPtr = 0;
    var funcListSize = 3;
    function doHint(cm, event) {
        // 判断是否已经弹出提示框
        if (!cm.state.completionActive) {
            funcListPtr = 0;
        }
        switch (funcListPtr) {
            case 0:
                editorShowHint(cm, CodeMirror.hint.ec2keyword);
                break;
            case 1:
                editorShowHint(cm, CodeMirror.hint.ec2symbol);
                break
            case 2:
                editorShowHint(cm, CodeMirror.hint.ec2func_core);
                break;
            case 3:
                editorShowHint(cm, CodeMirror.hint.ec2func_math);
                break;
        }
    }
    editor.on('mousedown', function (cm, event) {
        doHint(cm, event);
        funcListPtr = funcListPtr + 1;
        if (funcListPtr > funcListSize) funcListPtr = 0;
    });

    // 绑定鼠标双击
    // editor.on('dblclick', function (cm, event) {
    //     event.preventDefault();
    //     editorShowHint(cm, CodeMirror.hint.ec2func);
    // });

    // 绑定键盘事件，退格并减少缩进
    editor.on('keydown', function (cm, event) {
        if (event.keyCode === 8) { // 检查是否按下了退格键
            var cursor = cm.getCursor();
            var line = cm.getLine(cursor.line);
            var cursorCh = cursor.ch;

            // 获取光标前的字符
            var beforeCursor = line.substring(0, cursorCh);
            var match = beforeCursor.match(/^\s*$/); // 匹配光标前是否都是空格

            if (match && beforeCursor.length != 0 && beforeCursor.length % 4 === 0) { // 如果匹配成功且长度是4的倍数
                cm.execCommand('indentLess'); // 减少缩进
                event.preventDefault(); // 阻止默认的退格行为
            }
        }
        // 如果按下 tab
        if (event.keyCode === 9) {
            event.preventDefault();
            if (!cm.state.completionActive) {
                doHint(cm, event);
                funcListPtr = funcListPtr + 1;
                if (funcListPtr > funcListSize) funcListPtr = 0;
            } else {
                // 在光标处放置4个空格
                var spaces = '    ';
                cm.replaceSelection(spaces);
                // 移动光标到下一个位置
            }

        }
        // 如果按下左键，并且已经弹出提示框
        if (event.keyCode === 37 && cm.state.completionActive) {
            event.preventDefault();
            funcListPtr = funcListPtr - 1;
            if (funcListPtr < 0) funcListPtr = funcListSize;
            funcListPtr = funcListPtr - 1;
            if (funcListPtr < 0) funcListPtr = funcListSize;
            doHint(cm, event);
            funcListPtr = funcListPtr + 1;
            if (funcListPtr > funcListSize) funcListPtr = 0;

        }
        if (event.keyCode === 39 && cm.state.completionActive) {
            event.preventDefault();
            doHint(cm, event);
            funcListPtr = funcListPtr + 1;
            if (funcListPtr > funcListSize) funcListPtr = 0;
        }
    });
}
