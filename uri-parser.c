#include <gab/core.h>
#include <gab/gab.h>
#include <gab/object.h>
#include <gab/value.h>
#include <uriparser/Uri.h>
#include <uriparser/UriBase.h>

gab_value gab_lib_parse(gab_engine *gab, gab_value *argv, u8 argc) {
  if (argc != 1 && GAB_VAL_IS_STRING(argv[0])) {
    return GAB_VAL_NULL();
  }

  gab_obj_string *uri = GAB_VAL_TO_STRING(argv[0]);

  gab_value r_keys[2] = {
      GAB_VAL_OBJ(gab_obj_string_create(gab, s_i8_cstr("path"))),
      GAB_VAL_OBJ(gab_obj_string_create(gab, s_i8_cstr("query"))),
  };

  gab_obj_shape *r_shape = gab_obj_shape_create(gab, r_keys, 2, 1);
  gab_value r_values[2] = {GAB_VAL_NULL(), GAB_VAL_NULL()};

  const char *errorPos = NULL;
  UriUriA parsed_uri;

  const char *start = (char *)uri->data;
  const char *after_end = start + uri->size;

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

      gab_obj_shape *p_shape = gab_obj_shape_create_array(gab, path_count);
      gab_obj_object *p =
          gab_obj_object_create(gab, p_shape, values, path_count, 1);

      r_values[0] = GAB_VAL_OBJ(p);
    }

  }

  UriQueryListA *query = NULL;
  int item_count = 0;

  if (uriDissectQueryMallocA(&query, &item_count, parsed_uri.query.first,
                             parsed_uri.query.afterLast) == URI_SUCCESS) {
    UriQueryListA *walker = query;
    gab_value values[item_count], keys[item_count];
    u64 index = 0;

    while (index < item_count) {
      gab_obj_string *key = gab_obj_string_create(gab, s_i8_cstr(query->key));
      gab_obj_string *value =
          gab_obj_string_create(gab, s_i8_cstr(query->value));

      keys[index] = GAB_VAL_OBJ(key);
      values[index] = GAB_VAL_OBJ(value);

      walker = walker->next;
      index++;
    }

    gab_obj_shape *q_shape = gab_obj_shape_create(gab, keys, item_count, 1);
    gab_obj_object *q =
        gab_obj_object_create(gab, q_shape, values, item_count, 1);

    r_values[1] = GAB_VAL_OBJ(q);

    uriFreeQueryListA(query);
  };

  uriFreeUriMembersA(&parsed_uri);

  gab_value r =
      GAB_VAL_OBJ(gab_obj_object_create(gab, r_shape, r_values, 2, 1));

  gab_dref(gab, r);

  return r;
}

gab_value gab_mod(gab_engine *gab) {
  s_i8 keys[] = {
      s_i8_cstr("parse"),
  };

  gab_value values[] = {
      GAB_VAL_OBJ(gab_obj_builtin_create(gab, gab_lib_parse, "parse", 1)),
  };

  return gab_bundle(gab, 1, keys, values);
}
