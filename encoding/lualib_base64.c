#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <openssl/evp.h>

#define ENCODING_BASE64_METATABLE "encoding.base64"

#define ENCODE_STD "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#define ENCODE_URL "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
#define PADDING_CHAR '='

struct encoding {
  char encode_map[64];
  char decode_map[256];
};

static void encoding_init(struct encoding *enc, const char *encoder) {
  for (int i = 0; i < 64; ++i) {
    enc->encode_map[i] = encoder[i];
  }
  for (int i = 0; i < 64; ++i) {
    enc->decode_map[(unsigned char)encoder[i]] = i;
  }
}

static int l_encoding_base64_encode(lua_State *L) {
  struct encoding *enc = lua_touserdata(L, 1);
  size_t len;
  const char *data = lua_tolstring(L, 2, &len);
  luaL_Buffer buf;
  luaL_buffinitsize(L, &buf, (len + 2) / 3 * 4);
  size_t n = (len / 3) * 3;
  size_t di = 0;
  for (; di < n; di += 3) {
    luaL_addchar(&buf, enc->encode_map[(unsigned char)data[di+0]>>2]);
    luaL_addchar(&buf, enc->encode_map[(((unsigned char)data[di+0]&0x3)<<4) | (unsigned char)data[di+1]>>4]);
    luaL_addchar(&buf, enc->encode_map[(((unsigned char)data[di+1]&0xF)<<2) | (unsigned char)data[di+2]>>6]);
    luaL_addchar(&buf, enc->encode_map[(unsigned char)data[di+2]&0x3F]);
  }
  size_t more = len - di;
  if (more == 1) {
    luaL_addchar(&buf, enc->encode_map[(unsigned char)data[di+0]>>2]);
    luaL_addchar(&buf, enc->encode_map[((unsigned char)data[di+0]&0x3)<<4]);
    luaL_addchar(&buf, PADDING_CHAR);
    luaL_addchar(&buf, PADDING_CHAR);
  } else if (more == 2) {
    luaL_addchar(&buf, enc->encode_map[(unsigned char)data[di+0]>>2]);
    luaL_addchar(&buf, enc->encode_map[(((unsigned char)data[di+0]&0x3)<<4) | (unsigned char)data[di+1]>>4]);
    luaL_addchar(&buf, enc->encode_map[((unsigned char)data[di+1]&0xF)<<2]);
    luaL_addchar(&buf, PADDING_CHAR);
  }
  luaL_pushresult(&buf);
  return 1;
}

static int l_encoding_base64_decode(lua_State *L) {
  struct encoding *enc = lua_touserdata(L, 1);
  size_t len;
  const char *data = lua_tolstring(L, 2, &len);
  if (len & 3) {
    luaL_error(L, "data length is not a multiple of four");
  }
  luaL_Buffer buf;
  luaL_buffinitsize(L, &buf,  len / 4 * 3);
  size_t di = 0;
  size_t n = data[len-1] == PADDING_CHAR ? len-4 : len;
  char c1, c2, c3, c4;
  for (; di < n; di += 4) {
    c1 = enc->decode_map[(unsigned char)data[di+0]];
    c2 = enc->decode_map[(unsigned char)data[di+1]];
    c3 = enc->decode_map[(unsigned char)data[di+2]];
    c4 = enc->decode_map[(unsigned char)data[di+3]];
    luaL_addchar(&buf, (unsigned char)(c1<<2) | (unsigned char)(c2>>4));
    luaL_addchar(&buf, (unsigned char)(c2<<4) | (unsigned char)(c3>>2));
    luaL_addchar(&buf, (unsigned char)(c3<<6) | (unsigned char)(c4>>0));
  }
  if (len != n) {
    if (data[di+2] == PADDING_CHAR) {
      c1 = enc->decode_map[(unsigned char)data[di+0]];
      c2 = enc->decode_map[(unsigned char)data[di+1]];
      luaL_addchar(&buf, (unsigned char)(c1<<2) | (unsigned char)(c2>>4));
    } else {
      c1 = enc->decode_map[(unsigned char)data[di+0]];
      c2 = enc->decode_map[(unsigned char)data[di+1]];
      c3 = enc->decode_map[(unsigned char)data[di+2]];
      luaL_addchar(&buf, (unsigned char)(c1<<2) | (unsigned char)(c2>>4));
      luaL_addchar(&buf, (unsigned char)(c2<<4) | (unsigned char)(c3>>2));
    }
  }
  luaL_pushresult(&buf);
  return 1;
}

static int l_encoding_base64_new(lua_State *L) {
  size_t len;
  const char *encoder = lua_tolstring(L, 1, &len);
  if (len != 64) {
    luaL_error(L, "encoding alphabet is not 64-bytes long");
  }
  struct encoding *enc = lua_newuserdata(L, sizeof(*enc));
  encoding_init(enc, encoder);
  luaL_getmetatable(L, ENCODING_BASE64_METATABLE);
  lua_setmetatable(L, -2);
  return 1;
}

static const luaL_Reg encoding_base64_methods[] = {
  {"Encode", l_encoding_base64_encode},
  {"Decode", l_encoding_base64_decode},
  {NULL, NULL}
};

static const luaL_Reg encoding_base64_functions[] = {
  {"New", l_encoding_base64_new},
  {NULL, NULL}
};

static void create_encoding_base64_metatable(lua_State *L) {
  luaL_newmetatable(L, ENCODING_BASE64_METATABLE);
  luaL_setfuncs(L, encoding_base64_methods, 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
}

static void add_const(lua_State *L) {
  struct encoding *std_encoding = lua_newuserdata(L, sizeof(struct encoding));
  encoding_init(std_encoding, ENCODE_STD);
  luaL_getmetatable(L, ENCODING_BASE64_METATABLE);
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "StdEncoding");

  struct encoding *url_encoding = lua_newuserdata(L, sizeof(struct encoding));
  encoding_init(url_encoding, ENCODE_URL);
  luaL_getmetatable(L, ENCODING_BASE64_METATABLE);
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "URLEncoding");
}

int luaopen_encoding_base64(lua_State *L) {
  create_encoding_base64_metatable(L);
  luaL_newlib(L, encoding_base64_functions);
  add_const(L);
  return 1;
}