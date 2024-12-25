(function (mod) {
    if (typeof exports == "object" && typeof module == "object") // CommonJS
        mod(require("../../lib/codemirror"));
    else if (typeof define == "function" && define.amd) // AMD
        define(["../../lib/codemirror"], mod);
    else // Plain browser env
        mod(CodeMirror);
})(function (CodeMirror) {
    "use strict";

    CodeMirror.defineMode("ec2", function () {
        function words(str) {
            var obj = {}, words = str.split(" ");
            for (var i = 0; i < words.length; ++i) obj[words[i]] = true;
            return obj;
        }
        var keywords = words(
            "算始 算终 定义 函始 函终 若始 若终 若又 若否 " +
            "当始 当终 跳出 继续 返回 未定义 真 假 或 且 非 空 " +
            "导入 声明"
        );// 关键词高亮表
        var atoms = words(
            "输出 输入 终止 整数 浮点数 字符 字符串 字符序列 分割 键序列 值序列 转储 解析 长度 类型 执行 拷贝"
        );//内置函数高亮表
        var op = words(" + - * / // = := % == != < > <= >= & | ^ ~ << >> ; : [ ] { }  ( ) ,");

        function tokenBase(stream, state) {
            var ch = stream.next();
            if (ch == "#") {
                stream.skipToEnd();
                return "comment";
            }
            if (ch == '"' || ch == "'") {
                state.tokenize = tokenString(ch);
                return state.tokenize(stream, state);
            }

            if (ch == '.' || ch == ',' || ch == ';' || ch == ':' || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == '(' || ch == ')') {
                return "operator";
            }
            if (/\d/.test(ch)) {
                stream.eatWhile(/[\w\.]/);
                return "number";
            }

            // if (isOperatorChar.test(ch)) {
            //     stream.eatWhile(isOperatorChar);
            //     return "operator";
            // }
            // 支持中文字符
            stream.eatWhile(/\p{L}\w*/u);
            var cur = stream.current();
            if (keywords.propertyIsEnumerable(cur)) return "keyword";
            if (atoms.propertyIsEnumerable(cur)) return "atom";
            if (op.propertyIsEnumerable(cur)) return "operator";

            return "variable";
        }

        function tokenString(quote) {
            return function (stream, state) {
                var escaped = false, next, end = false;
                while ((next = stream.next()) != null) {
                    if (next == quote && !escaped) { end = true; break; }
                    escaped = !escaped && next == "\\";
                }
                if (end || !escaped) state.tokenize = null;
                return "string";
            };
        }


        // Interface

        return {
            startState: function () {
                return { tokenize: null };
            },

            token: function (stream, state) {
                if (stream.eatSpace()) return null;
                var style = (state.tokenize || tokenBase)(stream, state);
                if (style == "comment" || style == "meta") return style;
                return style;
            },
            indent: function (state, textAfter) {
                // 每 四个空格或一个tab缩进一个层级
                return CodeMirror.Pass;

            },

            // 支持方括号补全
            autoCloseBrackets: "()[]{}''\"\""
        };
    });
    CodeMirror.defineMIME("text/x-ec2", "ec2");



    // 提示函数
    CodeMirror.registerHelper("hint", "ec2keyword", function (editor, options) {
        var hintword = [
            "算始  算终", "函始  函终", "当始 当终", "若始  若终", "若又", "若否",
            "定义", "返回", "跳出", "继续",
            "或", "且", "非", "真", "假",
            "未定义", "空", "导入", "声明"
        ];
        var cur = editor.getCursor();

        return {
            list: hintword,
            from: CodeMirror.Pos(cur.line, cur.ch),
            to: CodeMirror.Pos(cur.line, cur.ch)
        };
    });
    CodeMirror.registerHelper("hint", "ec2symbol", function (editor, options) {
        var cur = editor.getCursor();
        var hintword = [
            "\"\"", "()", "[]", "{}", ",", ":", "\\",
            ">", "<", ">=", "<=", "==", "!=",
            "&", "|", "^", "~", "<<", ">>"
        ];
        return {
            list: hintword,
            from: CodeMirror.Pos(cur.line, cur.ch),
            to: CodeMirror.Pos(cur.line, cur.ch)
        };
    });

    CodeMirror.registerHelper("hint", "ec2func_core", function (editor, options) {
        var cur = editor.getCursor();
        var hintword = [
            "输入()", "输出()", "执行()", "终止()",
            "整数()", "浮点数()", "字符()", "字符串()",
            "字符序列()", "分割()",
            "键序列()", "值序列()", "转储()", "解析()",
            "长度()", "类型()", "拷贝()"
        ];
        return {
            list: hintword,
            from: CodeMirror.Pos(cur.line, cur.ch),
            to: CodeMirror.Pos(cur.line, cur.ch)
        };
    });



});