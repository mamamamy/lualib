#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <math.h>
#include <ctype.h>

#define TOKEN_ERROR "invalid character '%c', require %s"
#define TOKEN_ERROR_ASCII "invalid character %d(ascii), require %s"

// #define TOKEN_ERROR_DEBUG "invalid character '%c', require %s, at "__FILE__":%d"
// #define TOKEN_ERROR_DEBUG_ASCII "invalid character %d(ascii), require %s, at "__FILE__":%d"

// #define json_parse_error(l, c, s) luaL_error(L, isprint(c) ? TOKEN_ERROR_DEBUG : TOKEN_ERROR_DEBUG_ASCII, c, s, __LINE__);
#define json_parse_error(l, c, s) luaL_error(L, isprint(c) ? TOKEN_ERROR : TOKEN_ERROR_ASCII, c, s);

static const char *skip_whitespace(lua_State *L, const char *p) {
  for (;*p;) {
    switch (*p) {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
      ++p;
      break;
    default:
      return p;
    }
  }
  return p;
}

static const char *parse_object(lua_State *L, const char *p);
static const char *parse_array(lua_State *L, const char *p);
static const char *parse_string(lua_State *L, const char *p);
static const char *parse_number(lua_State *L, const char *p);
static const char *parse_true(lua_State *L, const char *p);
static const char *parse_false(lua_State *L, const char *p);
static const char *parse_null(lua_State *L, const char *p);
static const char *parse_value(lua_State *L, const char *p);

static const char *parse_object(lua_State *L, const char *p) {
  ++p; // '{'
  lua_newtable(L);
  p = skip_whitespace(L, p);
  if (*p == '}') {
    ++p; // '}'
    return p;
  }
  for (;*p;) {
    if (*p != '"') {
      json_parse_error(L, *p, "'\"'");
    }
    p = parse_string(L, p);
    p = skip_whitespace(L, p);
    if (*p != ':') {
      json_parse_error(L, *p, "':'");
    }
    ++p; // ':'
    p = skip_whitespace(L, p);
    p = parse_value(L, p);
    p = skip_whitespace(L, p);
    lua_settable(L, -3);
    if (*p != ',') {
      break;
    }
    ++p; // ','
    p = skip_whitespace(L, p);
  }
  if (*p != '}') {
    json_parse_error(L, *p, "'}' or ','");
  }
  ++p; // '}'
  return p;
}

static const char *parse_array(lua_State *L, const char *p) {
  ++p; // '['
  lua_newtable(L);
  p = skip_whitespace(L, p);
  if (*p == ']') {
    ++p; // ']'
    return p;
  }
  size_t idx = 1;
  for (;*p;) {
    p = parse_value(L, p);
    p = skip_whitespace(L, p);
    lua_seti(L, -2, idx++);
    if (*p != ',') {
      break;
    }
    ++p; // ','
    p = skip_whitespace(L, p);
  }
  if (*p != ']') {
    json_parse_error(L, *p, "']' or ','");
  }
  ++p; // ']'
  return p;
}

static const char *parse_string(lua_State *L, const char *p) {
  ++p; // '"'
  if (*p == '"') {
    ++p; // '"'
    lua_pushlstring(L, "", 0);
    return p;
  }
  luaL_Buffer buf;
  luaL_buffinit(L, &buf);
  for (;*p;) {
    if (*p == '\\') { // escape
      ++p; // '\'
      switch (*p) {
      case '"':
      case '\\':
      case '/':
        luaL_addchar(&buf, *p);
        break;
      case 'b':
        luaL_addchar(&buf, '\b');
        break;
      case 'f':
        luaL_addchar(&buf, '\f');
        break;
      case 'n':
        luaL_addchar(&buf, '\n');
        break;
      case 'r':
        luaL_addchar(&buf, '\r');
        break;
      case 't':
        luaL_addchar(&buf, '\t');
        break;
      case 'u':
        break;
      default:
        json_parse_error(L, *p, "'\"' or '\' or '/' or 'b' or 'f' or 'n' or 'r' or 't' or 'u' hex hex hex hex");
      }
      if (*p != 'u') {
        ++p; // escape
      } else {
        ++p; // 'u'
        int code = 0;
        for (int i = 0; i < 4; ++i) {
          char c = p[i];
          char v = 0;
          if (c >= '0' && c <= '9') {
            v = c - '0';
          } else if (c >= 'A' && c <= 'F') {
            v = c - 'A' + 10;
          } else if (c >= 'a' && c <= 'f') {
            v = c - 'a' + 10;
          } else {
            json_parse_error(L, c, "'0'-'9' or 'a'-'f' or 'A'-'F'");
          }
          code <<= 4;
          code |= v;
        }
        luaL_addchar(&buf, (code>>12) | 0xE0);
        luaL_addchar(&buf, ((code>>6) & 0x3F) | 0x80);
        luaL_addchar(&buf, (code&0x3F) | 0x80);
        p += 4;
      }
    } else if ((unsigned char)*p >= 0x20) {
      if (((unsigned char)p[0]>>7) == 0) {
        luaL_addchar(&buf, *p++);
      } else if (((unsigned char)p[0]>>5) == 0x6 &&
                 ((unsigned char)p[1]>>6) == 0x2) {
        luaL_addchar(&buf, *p++);
        luaL_addchar(&buf, *p++);
      } else if (((unsigned char)p[0]>>4) == 0xE &&
                 ((unsigned char)p[1]>>6) == 0x2 &&
                 ((unsigned char)p[2]>>6) == 0x2) {
        luaL_addchar(&buf, *p++);
        luaL_addchar(&buf, *p++);
        luaL_addchar(&buf, *p++);
      } else if (((unsigned char)p[0]>>4) == 0x1E &&
                 ((unsigned char)p[1]>>6) == 0x2 &&
                 ((unsigned char)p[2]>>6) == 0x2 &&
                 ((unsigned char)p[3]>>6) == 0x2) {
        luaL_addchar(&buf, *p++);
        luaL_addchar(&buf, *p++);
        luaL_addchar(&buf, *p++);
        luaL_addchar(&buf, *p++);
      } else {
        json_parse_error(L, *p, "U+0020~U+10FFFF");
      }
    } else {
      json_parse_error(L, *p, "U+0020~U+10FFFF");
    }
    if (*p == '"') {
      break;
    }
  }
  if (*p != '"') {
    json_parse_error(L, *p, "'\"'");
  }
  ++p; // '"'
  luaL_pushresult(&buf);
  return p;
}

static const char *parse_number_e(lua_State *L, const char *p, double *mul) {
  ++p; // 'e' or 'E'
  int signmul = 1;
  if (*p == '-') {
    ++p; // '-'
    signmul = -1;
  } else if (*p == '+') {
    ++p; // '+'
  } else if (*p <= '0' || *p >= '9') {
    json_parse_error(L, *p, "'-' or '+' or '0'-'9'");
  }
  long idxnum = 0;
  for (;*p == '0';);
  for (;*p >= '0' && *p <= '9';) {
    idxnum *= 10;
    idxnum += *p++ - '0';
  }
  idxnum *= signmul;
  *mul = pow(10, idxnum);
  return p;
}

static const char *parse_number_decimal(lua_State *L, const char *p, double *decimal) {
  double val = 0;
  ++p; // '.'
  if (*p >= '0' && *p <= '9') {
    double mul = 1;
    for (;*p >= '0' && *p <= '9';) {
      mul *= 0.1;
      val *= 10;
      val += *p++ - '0';
    }
    val *= mul;
  } else {
    json_parse_error(L, *p, "'0'-'9'");
  }
  *decimal = val;
  return p;
}

static const char *parse_number(lua_State *L, const char *p) {
  int signmul = 1;
  long integer_value = 0;
  double number_value = 0;
  if (*p == '-') {
    signmul = -1;
    ++p; // '-'
  }
  if (*p == '0') {
    ++p; // '0'
    if (*p == '.') {
      double decimal;
      p = parse_number_decimal(L, p, &decimal);
      number_value += decimal;
      if (*p == 'e' || *p == 'E') {
        double idxmul;
        p = parse_number_e(L, p, &idxmul);
        number_value *= idxmul;
      }
      lua_pushnumber(L, number_value * signmul);
    } else if (*p == 'e' || *p == 'E') {
      ++p; // 'e' or 'E'
      if (*p == '-' || *p == '+') {
        ++p; // '-' or '+'
      }
      for (;*p >= '0' && *p <= '9';) {
        ++p;
      }
      lua_pushnumber(L, 0);
    } else {
      lua_pushinteger(L, 0);
    }
  } else { // 1-9
    for (;*p >= '0' && *p <= '9';) {
      integer_value *= 10;
      integer_value += *p++ - '0';
    }
    if (*p == '.') {
      number_value = integer_value;
      double decimal;
      p = parse_number_decimal(L, p, &decimal);
      number_value += decimal;
      if (*p == 'e' || *p == 'E') {
        double idxmul;
        p = parse_number_e(L, p, &idxmul);
        number_value *= idxmul;
      }
      lua_pushnumber(L, number_value * signmul);
    } else if (*p == 'e' || *p == 'E') {
      number_value = integer_value;
      double idxmul;
      p = parse_number_e(L, p, &idxmul);
      number_value *= idxmul;
      lua_pushnumber(L, number_value * signmul);
    } else {
      lua_pushinteger(L, integer_value * signmul);
    }
  }
  return p;
}

static const char *parse_true(lua_State *L, const char *p) {
  if (*p++ != 't' ||
      *p++ != 'r' ||
      *p++ != 'u' ||
      *p++ != 'e') {
    json_parse_error(L, *(p-1), "true");
  }
  lua_pushboolean(L, 1);
  return p;
}

static const char *parse_false(lua_State *L, const char *p) {
  if (*p++ != 'f' ||
      *p++ != 'a' ||
      *p++ != 'l' ||
      *p++ != 's' ||
      *p++ != 'e') {
    json_parse_error(L, *(p-1), "false");
  }
  lua_pushboolean(L, 0);
  return p;
}

static const char *parse_null(lua_State *L, const char *p) {
  if (*p++ != 'n' ||
      *p++ != 'u' ||
      *p++ != 'l' ||
      *p++ != 'l') {
    json_parse_error(L, *(p-1), "null");
  }
  lua_pushnil(L);
  return p;
}

static const char *parse_value(lua_State *L, const char *p) {
  switch (*p) {
  case '{':
    return parse_object(L, p);
  case '[':
    return parse_array(L, p);
  case '"':
    return parse_string(L, p);
  case '-':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return parse_number(L, p);
  case 't':
    return parse_true(L, p);
  case 'f':
    return parse_false(L, p);
  case 'n':
    return parse_null(L, p);
  default:
    json_parse_error(L, *p, "'{' or '[' or '\"' or '-' or '0'-'9' or 't' or 'f' or 'n'");
  }
  return p;
}

static void stringify_table(lua_State *L, luaL_Buffer *buf, int idx);
static void stringify_string(lua_State *L, luaL_Buffer *buf, int idx);
static void stringify_value(lua_State *L, luaL_Buffer *buf, int idx);

static void stringify_table(lua_State *L, luaL_Buffer *buf, int idx) {
  idx = lua_absindex(L, idx);
  lua_Unsigned len = lua_rawlen(L, idx);
  if (len > 0) {
    luaL_addchar(buf, '[');
    for (lua_Unsigned i = 0; i < len; ++i) {
      if (i > 0) {
        luaL_addchar(buf, ',');
      }
      lua_rawgeti(L, idx, i+1);
      stringify_value(L, buf, -1);
      lua_pop(L, 1);
    }
    luaL_addchar(buf, ']');
  } else {
    luaL_addchar(buf, '{');
    lua_pushnil(L);
    int begin = 1;
    while (lua_next(L, idx)) {
      if (begin == 1) {
        begin = 0;
      } else {
        luaL_addchar(buf, ',');
      }
      int type = lua_type(L, -2);
      switch (type) {
      case LUA_TNIL:
        luaL_addstring(buf, "\"null\"");
        break;
      case LUA_TBOOLEAN:
        if (lua_toboolean(L, -2)) {
          luaL_addstring(buf, "\"true\"");
        } else {
          luaL_addstring(buf, "\"false\"");
        }
        break;
      case LUA_TNUMBER:
        luaL_addchar(buf, '"');
        lua_pushvalue(L, -2);
        luaL_addvalue(buf);
        luaL_addchar(buf, '"');
        break;
      case LUA_TSTRING:
        stringify_string(L, buf, -2);
        break;
      case LUA_TTABLE:
      default:
        goto label_skip;
      }
      luaL_addchar(buf, ':');
      stringify_value(L, buf, -1);
label_skip:
      lua_pop(L, 1);
    }
    luaL_addchar(buf, '}');
  }
}

static const char hexmap[] = {
  '0', '1', '2', '3',
  '4', '5', '6', '7',
  '8', '9', 'A', 'B',
  'C', 'D', 'E', 'F',
};

static void stringify_string(lua_State *L, luaL_Buffer *buf, int idx) {
  idx = lua_absindex(L, idx);
  size_t len;
  const char *s = lua_tolstring(L, idx, &len);
  luaL_addchar(buf, '"');
  for (size_t i = 0; i < len;) {
    if ((unsigned char)s[i+0] < 0x20) {
      unsigned char c = s[i+0];
      if (c == '\b') {
        luaL_addstring(buf, "\\b");
      } else if (c == '\f') {
        luaL_addstring(buf, "\\f");
      } else if (c == '\n') {
        luaL_addstring(buf, "\\n");
      } else if (c == '\r') {
        luaL_addstring(buf, "\\r");
      } else if (c == '\t') {
        luaL_addstring(buf, "\\t");
      } else {
        luaL_addstring(buf, "\\u00");
        luaL_addchar(buf, hexmap[c>>4]);
        luaL_addchar(buf, hexmap[c&0xF]);
      }
      ++i;
    } else if (((unsigned char)s[i+0]>>7) == 0) {
      unsigned char c = s[i+0];
      switch (c) {
      case '"':
      case '\\':
      case '/':
        luaL_addchar(buf, '\\');
        break;
      default:
        break;
      }
      luaL_addchar(buf, s[i++]);
    } else if (((unsigned char)s[i+0]>>5) == 0x6 &&
               ((unsigned char)s[i+1]>>6) == 0x2) {
      luaL_addchar(buf, s[i++]);
      luaL_addchar(buf, s[i++]);
    } else if (((unsigned char)s[i+0]>>4) == 0xE &&
               ((unsigned char)s[i+1]>>6) == 0x2 &&
               ((unsigned char)s[i+2]>>6) == 0x2) {
      luaL_addchar(buf, s[i++]);
      luaL_addchar(buf, s[i++]);
      luaL_addchar(buf, s[i++]);
    } else if (((unsigned char)s[i+0]>>4) == 0x1E &&
               ((unsigned char)s[i+1]>>6) == 0x2 &&
               ((unsigned char)s[i+2]>>6) == 0x2 &&
               ((unsigned char)s[i+3]>>6) == 0x2) {
      luaL_addchar(buf, s[i++]);
      luaL_addchar(buf, s[i++]);
      luaL_addchar(buf, s[i++]);
      luaL_addchar(buf, s[i++]);
    }
  }
  luaL_addchar(buf, '"');
}

static void stringify_value(lua_State *L, luaL_Buffer *buf, int idx) {
  idx = lua_absindex(L, idx);
  int type = lua_type(L, idx);
  switch (type) {
  case LUA_TNIL:
    luaL_addstring(buf, "null");
    break;
  case LUA_TBOOLEAN:
    if (lua_toboolean(L, idx)) {
      luaL_addstring(buf, "true");
    } else {
      luaL_addstring(buf, "false");
    }
    break;
  case LUA_TNUMBER:
    lua_pushvalue(L, idx);
    luaL_addvalue(buf);
    break;
  case LUA_TSTRING:
    stringify_string(L, buf, idx);
    break;
  case LUA_TTABLE:
    stringify_table(L, buf, idx);
    break;
  }
}

static int l_encoding_json_stringify(lua_State *L) {
  luaL_Buffer buf;
  luaL_buffinit(L, &buf);
  stringify_value(L, &buf, 1);
  luaL_pushresult(&buf);
  return 1;
}

static int l_encoding_json_parse(lua_State *L) {
  size_t len;
  const char *data = lua_tolstring(L, 1, &len);
  data = skip_whitespace(L, data);
  data = parse_value(L, data);
  data = skip_whitespace(L, data);
  if (*data) {
    json_parse_error(L, *data, "'\\0'");
  }
  return 1;
}

static const luaL_Reg encoding_json_functions[] = {
  {"Stringify", l_encoding_json_stringify},
  {"Parse", l_encoding_json_parse},
  {NULL, NULL}
};

int luaopen_encoding_json(lua_State *L) {
  luaL_newlib(L, encoding_json_functions);
  return 1;
}