wasmLock = 0;


// wasm IO： 输出、输入、弹窗、拍手、说出、选择框、获取代码
function wasmIOprint(str) {// C: printf(fmt);
    document.getElementById('output').innerHTML += str + '\n';
}
function wasmIOprinterr(str) {
    document.getElementById('output').innerHTML += '<p style="color:red">' + str + "</p>";
}
function wasmIOinput(str) { // C: const char* s;
    return prompt(str);
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

        window.speechSynthesis.speak(utterance);
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