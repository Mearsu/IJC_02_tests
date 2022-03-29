git clone https://github.com/Mearsu/IJC_02_tests ../IJC_02_tests
ln -s ../IJC_02_tests/*.c  .
echo '

####################################################
# Doplněno z IJC_02_tests setup scriptu
####################################################
test: HTAB_OBJS_TADY test.o 
	$(CC) $(LDFLAGS) -lcmocka $^ -o $@' >> Makefile
echo "Upravte ještě makefile, je nutno doplnit .o soubory pro test na konci makefile"
