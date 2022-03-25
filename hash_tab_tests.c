void htab_init_test(void **state) {
  UNUSED(state);
  htab_t *tab = htab_init(10);
  for (int i = 0; i < 10; i++) {
    assert_int_equal(tab->arr[i], NULL);
  }
  assert_int_equal(htab_bucket_count(tab), 10);
  assert_int_equal(tab->arr_size, 10);
  htab_free(tab);
}
