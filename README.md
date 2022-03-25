# Testy pro IJC

## Důležité info

- Byl jsem línej, když jsem to dělal, test vytvoří a smaže několik souborů v adresáři, kde je spuštěný - soubory "stdout", "stderr", "infile" a možná další
- testy jsem zkoušel pouze na linuxu, Mac by měl fungovat, windows možná ne. Při testování jsou vstupy a výstupy přesměrovány do souborů a později přesměrovány zpět na terminál, tohle ale nemusí fungovat na windows. Tohle definuje macro OUT_REDIRECT
- Na začátku lze vypnout testy pro tail nebo hashovací tabulku pomocí TEST_HTAB a TEST_TAIL
- Testy nejsou kompletní, budu přidávat další, a uvítám pomoc :)

## použití

Testy jde použít např klonováním repozitáře do adresáře vedle úkolu a symlinknutím souboru test.c
např.:
`ln -s ../IJC_02_tests/test.c .`
a přidáním cíle do makefile
