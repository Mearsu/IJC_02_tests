# Testy pro IJC

## Důležité info

- na testech stále pracuju, takže nezapomeňte občas na `git pull`, pokud máte pocit, že je někde v testech chyba/nejdou přeložit, stačí otevřít github issue nebo mi napsat na discordu
- Pokud chybová hláška nestačí, koukněte se do toho souboru na řádek, kde testy selhávají, někde je, jinde dopíšu, co přesně se testuje a očekává

- Byl jsem línej, když jsem to dělal, test vytvoří a smaže několik souborů v adresáři, kde je spuštěný - soubory "stdout", "stderr", "infile" a možná další
- testy jsem zkoušel pouze na linuxu, Mac by měl fungovat, windows možná ne. Při testování jsou vstupy a výstupy přesměrovány do souborů a později přesměrovány zpět na terminál, tohle ale nemusí fungovat na windows. Tohle definuje macro OUT_REDIRECT
- Na začátku lze vypnout testy pro tail nebo hashovací tabulku pomocí TEST_HTAB a TEST_TAIL
- Testy nejsou kompletní, budu přidávat další, a uvítám pomoc :)
- Ačkoliv jsem se snažil zachytit signály jako je segfault, ne vždy se to povede, pokud testy nedoběhnou a není vypsaný, že nějaký test selhal, koukněte se do souboru stderr, měl by obsahovat zbytek výstupu. Tohle se stává hlavně pokud [Address sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) ukončí program.


- Testy potřebují knihovnu [cmocka](https://cmocka.org/) - na mac/linux by měla být v repozitářích, na windowsu vám věřím, že to nějak zvládnete :)

## použití

Stačí spustit `curl https://raw.githubusercontent.com/Mearsu/IJC_02_tests/master/setup.sh | sh` ve složce, s projektem, script stáhne repozitář do `../IJC_02_tests`, linkne potřebné soubory a přidá nedokončený cíl do Makefile, nebo můžete manuálně:

ja to používám tak, že mám adresáře `IJC/02` a `IJC/02_tests` a v `02` linknutý všechny .c soubory z `02_tests` pomocí `ln -s ../02_tests/*.c  .` a v makefile v adresáři `02` 
```
test: $(HTAB_OBJS) test.o
  $(CC) $(LDFLAGS) -lcmocka $^ -o $@
```
`HTAB_OBJS` obsahuje všechny .o soubory pro hash table

test.o nesmí být linkován s tail.o ani jiným souborem, který obsahuje funkci main.
test.c přidává tail.c pomocí #include aby mohl změnit jméno funkce main v testech viz. tail.c:4-6

