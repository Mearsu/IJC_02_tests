git clone https://github.com/Mearsu/IJC_02_tests ../IJC_02_tests
ln -s ../IJC_02_tests/*.c  .
ln -s ../IJC_02_tests/test_mallocs.sh  .
echo '

####################################################
# Doplněno z IJC_02_tests setup scriptu
####################################################
test: test.o
	$(CC) $(LDFLAGS) -lcmocka $^ -o $@' >> Makefile
echo "Zkontrolujte makefile, byl doplněn cíl na konci souboru"
