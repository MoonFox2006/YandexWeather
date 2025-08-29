#pragma once

static const char TEXT_PLAIN[] = "text/plain";
static const char TEXT_HTML[] = "text/html";
static const char TEXT_CSS[] = "text/css";
static const char TEXT_JS[] = "text/javascript";
static const char TEXT_JSON[] = "application/json";

#define HTML_HEADER(title) "<!DOCTYPE html>\n"\
  "<html>\n"\
  "<head>\n"\
  "<title>" title "</title>\n"\
  "<meta charset=\"utf-8\">\n"\
  "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
#define HTML_STYLE(style) "<style>\n"\
  style\
  "</style>\n"
#define HTML_SCRIPT(script) "<script>\n"\
  script\
  "</script>\n"
#define HTML_BODY "</head>\n"\
  "<body>\n"
#define HTML_BODY_EXT(ext) "</head>\n"\
  "<body " ext ">\n"
#define HTML_FOOTER "</body>\n"\
  "</html>"

#define HTML_TAG(tag, value) "<" tag ">" value "</" tag ">"
#define HTML_LINK(url, text) "<a href=\"" url "\">" text "</a>"

#define JS_TRIM "function trim(field){\n"\
  "field.value=field.value.trim();\n"\
  "}\n"

#define JS_VALIDATE_INT "function validateInt(field,minval,maxval){\n"\
  "let val=parseInt(field.value);\n"\
  "if(isNaN(val)||(val<minval))\n"\
  "field.value=minval;\n"\
  "else if(val>maxval)\n"\
  "field.value=maxval;\n"\
  "}\n"

#define JS_VALIDATE_FLOAT "function validateFloat(field,minval,maxval){\n"\
  "let val=parseFloat(field.value);\n"\
  "if(isNaN(val)||(val<minval))\n"\
  "field.value=minval;\n"\
  "else if(val>maxval)\n"\
  "field.value=maxval;\n"\
  "}\n"
