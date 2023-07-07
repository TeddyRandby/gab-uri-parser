#include <gab/gab.h>
#include <uriparser/Uri.h>

void gab_lib_parse(gab_engine *gab, gab_vm *vm, u8 argc, gab_value argv[argc]) {
  if (argc != 1 && GAB_VAL_IS_STRING(argv[0]))
    return;

  gab_obj_string *uri = GAB_VAL_TO_STRING(argv[0]);

  s_i8 r_keys[2] = {
      s_i8_cstr("path"),
      s_i8_cstr("query"),
  };
  gab_value r_values[2] = {GAB_VAL_NIL(), GAB_VAL_NIL()};

  const char *errorPos = NULL;
  UriUriA parsed_uri;

  const char *start = (char *)uri->data;
  const char *after_end = start + uri->len;

  if (uriParseSingleUriExA(&parsed_uri, start, after_end, &errorPos) ==
      URI_SUCCESS) {
    UriPathSegmentA *path = parsed_uri.pathHead;

    if (path) {
      u64 path_count = 1;
      while (path->next) {
        path = path->next;
        path_count++;
      }

      path = parsed_uri.pathHead;

      gab_value values[path_count];

      u64 index = 0;
      while (index < path_count) {
        values[index] = GAB_VAL_OBJ(gab_obj_string_create(
            gab, s_i8_create((i8 *)path->text.first,
                             path->text.afterLast - path->text.first)));

        path = path->next;
        index++;
      }

      r_values[0] = gab_tuple(gab, vm, path_count, values);
    }
  }

  UriQueryListA *query = NULL;
  int item_count = 0;

  if (uriDissectQueryMallocA(&query, &item_count, parsed_uri.query.first,
                             parsed_uri.query.afterLast) == URI_SUCCESS) {
    UriQueryListA *walker = query;
    gab_value values[item_count];
    s_i8 keys[item_count];
    u64 index = 0;

    while (index < item_count) {
      s_i8 key = s_i8_cstr(query->key);
      gab_obj_string *value =
          gab_obj_string_create(gab, s_i8_cstr(query->value));

      keys[index] = key;
      values[index] = GAB_VAL_OBJ(value);

      walker = walker->next;
      index++;
    }

    r_values[1] = gab_record(gab, vm, item_count, keys, values);

    uriFreeQueryListA(query);
  };

  uriFreeUriMembersA(&parsed_uri);

  gab_push(vm, LEN_CARRAY(r_values), r_values);
}

gab_value gab_mod(gab_engine *gab, gab_vm *vm) {
  gab_value names[] = {
      GAB_STRING("to_uri"),
  };

  gab_value types[] = {
      gab_type(gab, kGAB_STRING),
  };

  gab_value values[] = {
      GAB_BUILTIN(parse),
  };

  for (i32 i = 0; i < LEN_CARRAY(values); i++) {
    gab_specialize(gab, vm, names[i], types[i], values[i]);
    gab_val_dref(vm, values[i]);
  }

  return GAB_VAL_NIL();
}
