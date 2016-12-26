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

import binascii

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
        text = text.replace("\\n\\n", "\\n")
    if (text[-1:] == ' '):
        text = text[:-1]
    if (text[-2:] == '\\n'):
        text = text[:-2]
    if (text[0] == ' '):
        text = text[1:]
    if (text[0] == '\\') and (text[1] == 'n'):
        text = text[2:]
    return text

def getBinaryFile(filename, id):
    f = open(filename, "rb")
    s = "const static unsigned char " + id + "[] PROGMEM = {\n";
    i = 0
    c = 0
    while True:
        d = f.read(1)
        if not d:
            break
        if i == 0:
            s += "    "
        s += "0x" + binascii.hexlify(d).decode("utf-8") + ", "
        i += 1
        c += 1
        if i >= 8:
            i = 0
            s += "\n"
    if i == 0:
        s = s[:-3]
    else:
        s = s[:-2]
    s += "\n"
    s += "};"
    s = "const static unsigned int " + id + "Size = " + str(c) + ";\n" + "const static char faviconMimeType[] PROGMEM = \"image/x-icon\";\n" + s;
    return s

def getAsDefine(name, text):
    return "#define " + name + " \"" + text.replace("\"", "\\\"") + "\""

print("Preparing static.h output file...")
f = open("static.h", "w")
f.write("// !!DO NOT EDIT, AUTO-GENERATED FILE!!\n")
f.write("// Use convert-static.py to recreate this.\n")
f.write("\n")
f.write("#ifndef __STATIC_H__\n")
f.write("#define __STATIC_H__\n")
f.write("\n")

print("Processing template.html...")
template = fileToString("template.html")
template = minify(template)
templates = template.split(" /* %% INSERT_CLIENT_LIST_HERE %% */ ")

f.write(getAsDefine("HTML_BEGIN", templates[0]) + "\n")
f.write(getAsDefine("HTML_END", templates[1]) + "\n")

print("Processing client-script.js...")
js = fileToString("client-script.js", True)
js = minify(js, True)
f.write(getAsDefine("JS_FILE", js) + "\n")

print("Processing client-style.css...")
css = fileToString("client-style.css", True)
css = minify(css)
f.write(getAsDefine("CSS_FILE", css) + "\n")
f.write("\n")

print("Processing favicon.ico...")
f.write(getBinaryFile("favicon.ico", "favicon") + "\n")

print("Done!")
f.write("\n")
f.write("#endif // __STATIC_H__\n")
f.write("\n")
f.close()

