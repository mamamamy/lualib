#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <openssl/evp.h>

#define CRYPTO_SHA256_METATABLE "crypto.sha256"

static int l_crypto_sha256_new(lua_State *L) {
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (ctx == NULL) {
    luaL_error(L, "failed to allocate EVP_MD_CTX");
    return 0;
  }
  EVP_DigestInit(ctx, EVP_sha256());
  lua_pushlightuserdata(L, ctx);
  luaL_getmetatable(L, CRYPTO_SHA256_METATABLE);
  lua_setmetatable(L, -2);
  return 1;
}

static int l_crypto_sha256_write(lua_State *L) {
  EVP_MD_CTX *ctx = lua_touserdata(L, 1);
  size_t len;
  const char *msg = luaL_checklstring(L, 2, &len);
  if (EVP_DigestUpdate(ctx, msg, len) != 1) {
    luaL_error(L, "failed to update digest");
    return 0;
  }
  return 0;
}

static int l_crypto_sha256_reset(lua_State *L) {
  EVP_MD_CTX *ctx = lua_touserdata(L, 1);
  if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
    luaL_error(L, "failed to reset digest");
    return 0;
  }
  return 0;
}

static int l_crypto_sha256_sum(lua_State *L) {
  EVP_MD_CTX *ctx = lua_touserdata(L, 1);
  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hash_len;
  if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
    luaL_error(L, "failed to finalize digest");
    return 0;
  }
  lua_pushlstring(L, (const char *)hash, hash_len);
  return 1;
}

static int l_crypto_sha256_gc(lua_State *L) {
  EVP_MD_CTX *ctx = lua_touserdata(L, 1);
  EVP_MD_CTX_free(ctx);
  return 0;
}

static const luaL_Reg crypto_sha256_methods[] = {
  {"Write", l_crypto_sha256_write},
  {"Reset", l_crypto_sha256_reset},
  {"Sum", l_crypto_sha256_sum},
  {"__gc", l_crypto_sha256_gc},
  {NULL, NULL}
};

static const luaL_Reg crypto_sha256_functions[] = {
  {"New", l_crypto_sha256_new},
  {NULL, NULL}
};

static void create_crypto_sha256_metatable(lua_State *L) {
  luaL_newmetatable(L, CRYPTO_SHA256_METATABLE);
  luaL_setfuncs(L, crypto_sha256_methods, 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
}

static void add_const(lua_State *L) {
  lua_pushinteger(L, 32);
  lua_setfield(L, -2, "Size");
}

int luaopen_crypto_sha256(lua_State *L) {
  create_crypto_sha256_metatable(L);
  luaL_newlib(L, crypto_sha256_functions);
  add_const(L);
  return 1;
}