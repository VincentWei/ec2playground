// wasmLock = 0;
var WasmMutex = {
    'runLock': 0, // 运行锁
    'speekLock': 0, // 语音合成锁
};

// wasm IO： 输出、输入、弹窗、拍手、说出、选择框、获取代码
function wasmIOprint(str) {// C: printf(fmt);
    document.getElementById('output').innerHTML += str + '\n';
}
function wasmIOprinterr(str) {
    document.getElementById('output').innerHTML += '<p style="color:red">' + str + "</p>";
}
function wasmIOinput(str) { // C: const char* s;
    var s = prompt(str);
    // 如果按下取消，打断wasm
    if (s == null) {
        Module._wasmIObreakCode();
    }
    return s;
}
function wasmIOalert(str) {
    alert(str);
}
function wasmIOclapshow() {
    document.getElementById("clapModal").style.display = "block";
}
function wasmIOclapclose() {
    document.getElementById("clapModal").style.display = "none";
}
function wasmIOspeek(str) {
    if ('speechSynthesis' in window) {
        const utterance = new SpeechSynthesisUtterance(str);

        // 设置语言为简体中文
        utterance.lang = 'zh-CN';

        // 可选配置
        utterance.pitch = 1; // 设置音高（1 是默认值）
        utterance.rate = 1; // 设置语速（1 是默认值）
        utterance.volume = 1; // 设置音量（1 是最大值）
        utterance.onend = function () {
            WasmMutex.speekLock = 0; // 合成完成后释放语音合成锁
        };

        WasmMutex.speekLock = 1; // 语音合成锁
        window.speechSynthesis.speak(utterance);
        // 在 C 代码中执行异步等待，直到合成锁被释放
    } else {
        wasmIOprint('[浏览器不支持语音合成]\n' + str);
    }
}
function wasmIOconfirm(str) {
    return confirm(str) ? 1 : 0;
}
function wasmIOgetCode() {
    return editor.getValue();
}

// 初始化wasm
var Module = {
    preRun: [],
    postRun: [],
    print: wasmIOprint,    // Redirect stdout
    printErr: wasmIOprinterr, // Redirect stderr
    // canvas: (function () { return document.getElementById('canvas'); })(),
    noInitialRun: true,
    onRuntimeInitialized: function () {
        // Call the main function of the WebAssembly module

    }
};
var WasmMod = Module;

var spt = document.createElement('script');
spt.src = "./js/ec2_wasm.js";
document.body.appendChild(spt);