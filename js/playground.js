// playground.js: 初始化编辑器、基本操作按钮与页面控制等
function importProgram(btnElem) {
    var formData = new FormData();
    formData.append("digest",
          btnElem.parentElement.parentElement.getAttribute('data-snippet-digest'));

    var request = new XMLHttpRequest();
    request.open("POST", globals.pathPrefix + "tools/fetch-snippet-contents.php");
    request.onload = function (oEvent) {
        let response = JSON.parse(request.response);
        if (response.retCode == 0) {
            editor.replaceSelection(response.data);
            dismissRepoPanel();
        }
    };

    request.send(formData);
}

function assemblyShareLink(digest) {
    return window.location.origin + window.location.pathname + '?snippet=' + digest;
}

function copyProgramLink(btnElem) {
    let digest = btnElem.parentElement.parentElement.getAttribute('data-snippet-digest');

    try {
        let link = assemblyShareLink(digest);

        navigator.clipboard.writeText(link);
        const tooltip = bootstrap.Tooltip.getInstance(btnElem);
        tooltip.setContent({ '.tooltip-inner': '已复制！' });
        window.setTimeout(() => {
            tooltip.setContent({ '.tooltip-inner': '复制分享链接到剪贴板' });
            tooltip.hide();
        }, 1000);
    }
    catch (error) {
        console.error(error.message);
    }
}

function showPromptModal(message) {
    const msgElem = document.getElementById("promptModalMsg");
    msgElem.textContent = message;
    const promptModal = new bootstrap.Modal('#promptModal',
            { dropback: true, focus: true, keyboard: true });
    promptModal.show();
}

function deleteProgram(btnElem) {
    let username = window.localStorage.getItem("username");
    let password = window.localStorage.getItem("password");
    if (username === null || password === null) {
        console.log("不应该发生的事情发生了！");
        return;
    }

    let title = btnElem.parentElement.parentElement.getAttribute('data-snippet-title');
    if (!window.confirm(`程序被删除后不可恢复，请谨慎操作！\n请再次确认是否删除 '${title}' 程序？`)) {
        return;
    }

    var formData = new FormData();
    formData.append("digest",
          btnElem.parentElement.parentElement.getAttribute('data-snippet-digest'));
    formData.append("username", username);
    formData.append("password", password);

    var request = new XMLHttpRequest();
    request.open("POST", globals.pathPrefix + "tools/delete-snippet.php");
    request.onload = function (oEvent) {
        let response = JSON.parse(request.response);
        if (response.retCode == 0) {
            let digest = response.extraMsg;

            let eleProgramList = document.getElementById('programListPanel');
            while (true) {
                let eleLi = eleProgramList.querySelector(`li.snippet-${digest}`);
                if (eleLi) {
                    eleLi.remove();
                }
                else
                    break;
            }
        }
        else {
            showPromptModal(`删除错误：${response.retMsg}`);
        }
    };

    request.send(formData);
}

const groupBy = (array, key) => {
        return array.reduce((result, currentValue) => {
        const groupKey = currentValue[key];
        if (!result[groupKey]) {
            result[groupKey] = [];
        }
        result[groupKey].push(currentValue);
        return result;
    }, {});
};

function enableTooltips(sectionElem) {
    const sectionCollapseList = sectionElem.querySelectorAll(".program-section");
    sectionCollapseList.forEach(function(myCollapsible) {
        const deleteBtns = myCollapsible.querySelector('[data-bs-title="删除此程序"]');
        const username = getUsername();
        deleteBtns.forEach(function(btnElem) {
            if (username == null) {
                btnElem.setAttribute('disabled', 'disabled');
            }
            else {
                let snippet_user = btnElem.getAttribute('data-snippet-username');
                if (username != '老师' && username != snippet_user) {
                    btnElem.setAttribute('disabled', 'disabled');
                }
                else {
                    btnElem.removeAttribute('disabled');
                }
            }
        });

        myCollapsible.addEventListener('shown.bs.collapse', function(e) {
            const tooltipTriggerList =
                    myCollapsible.querySelectorAll('[data-bs-toggle="tooltip"]');
            const tooltipList = [...tooltipTriggerList].map(tooltipTriggerEl =>
                    new bootstrap.Tooltip(tooltipTriggerEl));

        });
    });
}

function refreshSnippetsByUsername(username) {
    var formData = new FormData();
    formData.append("username", username);

    var request = new XMLHttpRequest();
    request.open("POST", globals.pathPrefix + "tools/fetch-snippets-by-username.php");
    request.onload = function (oEvent) {
        let response = JSON.parse(request.response);
        if (response.retCode == 0) {
            let snippets = groupBy(response.data, "section");

            let sectionList = null;
            if (username == '老师') {
                sectionList = document.getElementById('teachersSnippets');
            }
            else {
                sectionList = document.getElementById('mySnippets');
            }
            sectionList.replaceChildren();

            if (Object.keys(snippets).length == 0)
                return;

            let templateSection = document.getElementById('snippetSectionTemplate');
            let sectionContent = templateSection.content;
            let templateSnippet = document.getElementById('mySnippetTemplate');
            let snippetContent = templateSnippet.content;

            for (const section in snippets) {
                let newSectionNode = sectionContent.cloneNode(true);

                let eleBtn = newSectionNode.querySelector('button');
                eleBtn.setAttribute('data-bs-target', `#${username}-${section}-collapse`);
                eleBtn.textContent = section;
                let eleDiv = newSectionNode.querySelector('div');
                eleDiv.setAttribute('id', `${username}-${section}-collapse`);

                let eleUl = newSectionNode.querySelector('ul');
                for (const snippet of snippets[section]) {
                    let newSnippetNode = snippetContent.cloneNode(true);
                    let eleLi = newSnippetNode.querySelector('li');
                    eleLi.classList.add(`snippet-${snippet.digest}`);
                    eleLi.setAttribute('data-snippet-title', snippet.title);
                    eleLi.setAttribute('data-snippet-digest', snippet.digest);

                    let eleA = newSnippetNode.querySelector('a');
                    eleA.setAttribute('href', `?snippet=${snippet.digest}`);
                    if (snippet.description == null || snippet.description.length == 0) {
                        eleA.setAttribute('data-bs-title', snippet.title);
                    }
                    else {
                        eleA.setAttribute('data-bs-title', `${snippet.title}<br/><small>${snippet.description}</small>`);
                    }
                    eleA.textContent = snippet.title;

                    eleUl.appendChild(newSnippetNode);
                }

                sectionList.appendChild(newSectionNode);
            }
            enableTooltips(sectionList);
        }
    };

    request.send(formData);
}

function refreshSelectedSnippets() {
    var request = new XMLHttpRequest();
    request.open("GET", globals.pathPrefix + "tools/fetch-selected-snippets.php");
    request.onload = function (oEvent) {
        let response = JSON.parse(request.response);
        if (response.retCode == 0) {
            let snippets = groupBy(response.data, "name");
        }
    };

    request.send();
}

function refreshLatestSnippets() {
    var request = new XMLHttpRequest();
    request.open("GET", globals.pathPrefix + "tools/fetch-latest-snippets.php");
    request.onload = function (oEvent) {
        let response = JSON.parse(request.response);
        if (response.retCode == 0) {
            let snippets = groupBy(response.data, "name");

            let sectionList = document.getElementById('latestSnippets');
            sectionList.replaceChildren();

            if (Object.keys(snippets).length == 0)
                return;

            let templateSection = document.getElementById('snippetSectionTemplate');
            let sectionContent = templateSection.content;
            let templateSnippet = document.getElementById('mySnippetTemplate');
            let snippetContent = templateSnippet.content;

            for (const username in snippets) {
                let newSectionNode = sectionContent.cloneNode(true);

                let eleBtn = newSectionNode.querySelector('button');
                eleBtn.setAttribute('data-bs-target', `#${username}-LATEST-collapse`);
                eleBtn.textContent = username;
                let eleDiv = newSectionNode.querySelector('div');
                eleDiv.setAttribute('id', `${username}-LATEST-collapse`);

                let eleUl = newSectionNode.querySelector('ul');
                for (const snippet of snippets[username]) {
                    let newSnippetNode = snippetContent.cloneNode(true);
                    let eleLi = newSnippetNode.querySelector('li');
                    eleLi.classList.add(`snippet-${snippet.digest}`);
                    eleLi.setAttribute('data-snippet-title', snippet.title);
                    eleLi.setAttribute('data-snippet-digest', snippet.digest);

                    let eleA = newSnippetNode.querySelector('a');
                    eleA.setAttribute('href', `?snippet=${snippet.digest}`);
                    if (snippet.description == null || snippet.description.length == 0) {
                        eleA.setAttribute('data-bs-title', snippet.title);
                    }
                    else {
                        eleA.setAttribute('data-bs-title', `${snippet.title}<br/><small>${snippet.description}</small>`);
                    }
                    eleA.textContent = snippet.title;

                    eleUl.appendChild(newSnippetNode);
                }

                sectionList.appendChild(newSectionNode);
            }

            enableTooltips(sectionList);
        }
    };

    request.send();
}

function getUsername() {
    let username = window.localStorage.getItem("username");
    let password = window.localStorage.getItem("password");

    if (username !== null && password !== null) {
        return username;
    }

    return null;
}

function showRepository(btnElem) {
    document.getElementById('programListPanel').style.setProperty('left', '0px');
}

function refreshRepository(username) {
    refreshSnippetsByUsername('老师');
    // refreshSelectedSnippets();
    refreshLatestSnippets();

    if (username == null)
        username = getUsername();
    if (username !== null && username != '老师') {
        refreshSnippetsByUsername(username);
    }
}

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
        console.log("没有用户信息。");
        return null;
    }
    else {
        let elemName = document.getElementById("shareModalStudentName");
        elemName.setAttribute("value", username);
        let elemPass = document.getElementById("shareModalParentName");
        elemPass.setAttribute("value", password);

        let elemWrap = document.getElementById("shareModalNewUserFields");
        elemWrap.style.display = "none";
        return username;
    }
}

window.onload = function() {
    username = updateUserFields();
    refreshRepository(username);

    const searchParams = new URLSearchParams(window.location.search);
    const snippetDigest = searchParams.get('snippet');
    if (snippetDigest && snippetDigest.length == 32) {
        var formData = new FormData();
        formData.append("digest", snippetDigest);

        var request = new XMLHttpRequest();
        request.open("POST", globals.pathPrefix + "tools/fetch-snippet-contents.php");
        request.onload = function (oEvent) {
            let response = JSON.parse(request.response);
            if (response.retCode == 0) {
                editor.setValue(response.data);
                if (response.extraMsg.length > 0)
                    showPromptModal(response.extraMsg);
            }
        };

        request.send(formData);
    }
}

function dismissRepoPanel() {
    document.getElementById('programListPanel').style.setProperty('left', '-400px');
}

function dismissShareModal() {
    window.setTimeout(() => {
            const closeBtn = document.getElementById("shareModalClose");
            closeBtn.click();
        }, 3000);
}

function shareProgram() {
    let code = editor.getValue().trim();
    const formElem = document.getElementById("shareModalForm");
    var formData = new FormData(formElem);
    formData.append("snippet", code);

    const submitElem = document.getElementById("shareModalSubmit");
    submitElem.setAttribute("disabled", "disabled");

    var request = new XMLHttpRequest();

    const promptElem = document.getElementById("shareModalPromptUpdate");
    if (promptElem.style.display == 'none') {
        request.open("POST", globals.pathPrefix + "tools/push-new-snippet.php");
    }
    else {
        const searchParams = new URLSearchParams(window.location.search);
        const snippetDigest = searchParams.get('snippet');
        formData.append("digest", snippetDigest);

        request.open("POST", globals.pathPrefix + "tools/update-existing-snippet.php");
    }
    request.onload = function (oEvent) {
        const errElem = document.getElementById("shareModalErrorMsg");
        let response = JSON.parse(request.response);
        if (request.status == 200 && response.retCode == 0) {
            const nameElem = document.getElementById("shareModalStudentName");
            const passElem = document.getElementById("shareModalParentName");
            saveUserInfo(nameElem.value, passElem.value);
            updateUserFields();
            if (response.extraMsg == 'updated') {
                errElem.innerHTML = `已成功更新！`;
            }
            else {
                errElem.innerHTML = `已成功分享！点击 <a href="${assemblyShareLink(response.extraMsg)}">链接</a> 访问。`;
            }
            refreshRepository(nameElem.value);
        }
        else {
            errElem.textContent = response.retMsg;
            const submitElem = document.getElementById("shareModalSubmit");
            submitElem.removeAttribute("disabled");
        }

        errElem.style.display = "block";
    };

    request.send(formData);
}

function showShareModal(snippetHeaders) {
    var elem = document.getElementById("shareModalPromptUpdate");
    if (snippetHeaders) {
        elem.style.display = "block";

        elem = document.getElementById("shareModalProgramSection");
        elem.value = snippetHeaders.section;
        elem = document.getElementById("shareModalProgramTitle");
        elem.value = snippetHeaders.title;
        elem = document.getElementById("shareModalProgramDesc");
        elem.value = snippetHeaders.description;
    }
    else {
        elem.style.display = "none";

        elem = document.getElementById("shareModalProgramSection");
        elem.value = '';
        elem = document.getElementById("shareModalProgramTitle");
        elem.value = '';
        elem = document.getElementById("shareModalProgramDesc");
        elem.value = '';
    }

    const errElem = document.getElementById("shareModalErrorMsg");
    errElem.style.display = "none";
    const submitElem = document.getElementById("shareModalSubmit");
    submitElem.removeAttribute("disabled");

    const shareModal = new bootstrap.Modal('#shareModal',
            { dropback: true, focus: true, keyboard: true });
    shareModal.show();
}

function tryToShareCode() {
    let code = editor.getValue().trim();
    if (code.length < 10) {
        showPromptModal("至少写一个完整程序才能分享哦。");
    }
    else {
        const searchParams = new URLSearchParams(window.location.search);
        const snippetDigest = searchParams.get('snippet');
        if (snippetDigest && snippetDigest.length == 32) {
            var formData = new FormData();
            formData.append("digest", snippetDigest);

            var request = new XMLHttpRequest();
            request.open("POST", globals.pathPrefix + "tools/fetch-snippet-headers.php");
            request.onload = function (oEvent) {
                let response = JSON.parse(request.response);
                let username = window.localStorage.getItem("username");
                if (response.retCode == 0 && response.data.username == username) {
                    showShareModal(response.data);
                }
                else {
                    showShareModal(null);
                }
            };

            request.send(formData);
        }
        else {
            showShareModal(null);
        }
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
    var code = editor.getValue();
    if (code.length < 10) {
        showPromptModal("编辑区空空如也，清除啥呢？");
        return;
    }

    var result = confirm("确定要清空代码吗？");
    if (result) {
        editor.setValue("");
        document.getElementById('output').innerHTML = "";
    }
}

function downloadCode() {
    var code = editor.getValue();
    if (code.length < 10) {
        showPromptModal("至少写点东西再保存嘛。");
        return;
    }

    var blob = new Blob([code], { type: "text/plain;charset=utf-8" });
    var url = URL.createObjectURL(blob);
    var link = document.createElement('a');
    link.href = url;
    link.download = "my-executable-coding-code.ec2"; // 设置下载文件的名称
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
