# Testy pro IJC

## Důležité info

- Byl jsem línej, když jsem to dělal, test vytvoří a smaže několik souborů v adresáři, kde je spuštěný - soubory "stdout", "stderr", "infile" a možná další
- testy jsem zkoušel pouze na linuxu, Mac by měl fungovat, windows možná ne. Při testování jsou vstupy a výstupy přesměrovány do souborů a později přesměrovány zpět na terminál, tohle ale nemusí fungovat na windows. Tohle definuje macro OUT_REDIRECT
- Na začátku lze vypnout testy pro tail nebo hashovací tabulku pomocí TEST_HTAB a TEST_TAIL
- Testy nejsou kompletní, budu přidávat další, a uvítám pomoc :)
- Ačkoliv jsem se snažil zachytit signály jako je segfault, ne vždy se to povede, pokud testy nedoběhnou a není vypsaný, že nějaký test selhal, koukněte se do souboru stderr, měl by obsahovat zbytek výstupu. Tohle se stává hlavně pokud [Address sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) ukončí program.


- Testy potřebují knihovnu [cmocka](https://cmocka.org/) - na mac/linux by měla být v repozitářích, na windowsu vám věřím, že to nějak zvládnete :)

## použití

Testy jde použít např klonováním repozitáře do adresáře vedle úkolu a symlinknutím souboru test.c
např.:
`ln -s ../IJC_02_tests/test.c .`
a přidáním cíle do makefile
