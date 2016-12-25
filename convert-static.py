#!/usr/bin/env python3

# convert-static.py
#
# This program reads the static html and js files and converts them into
# minified strings defined in a C header file.
#
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# <xythobuz@xythobuz.de> & <ghost-ghost@web.de> wrote this file.  As long as
# you retain this notice you can do whatever you want with this stuff. If we
# meet some day, and you think this stuff is worth it, you can buy us a beer
# in return.                                   Thomas Buck & Christian Högerle
# ----------------------------------------------------------------------------

def fileToString(filename, js = False):
    f = open(filename, "r")
    sf = ""
    for s in f.readlines():
        st = s
        if js == True:
            ss = s.split("//", 1)
            if ss[0][-1:] == ':':
                st = ss[0] + '//' + ss[1]
            else:
                st = ss[0]
        sf += st
    return sf

def minify(text, js = False):
    text = text.replace("\r", "")
    text = text.replace("\t", "")
    text = text.replace("  ", " ")
    if js == True:
        text = text.replace("\\", "\\\\")
        text = text.replace("\n", "\\n")
    else:
        text = text.replace("\n", " ")
    text = text.replace("  ", " ")
    text = text.replace("  ", " ")
    text = text.replace("  ", " ")
    text = text.replace("  ", " ")
    text = text.replace("  ", " ")
    text = text.replace("> <", "><")
    if js == True:
        text = text.replace("\\n ", "\\n")
    if (text[-1:] == ' '):
        text = text[:-1]
    return text

def getAsDefine(name, text):
    return "#define " + name + " \"" + text.replace("\"", "\\\"") + "\""

template = fileToString("template.html")
template = minify(template)
templates = template.split(" /* %% INSERT_CLIENT_LIST_HERE %% */ ")

print(getAsDefine("HTML_BEGIN", templates[0]))
print(getAsDefine("HTML_END", templates[1]))

js = fileToString("client-script.js", True)
js = minify(js, True)
print(getAsDefine("JS_FILE", js))

