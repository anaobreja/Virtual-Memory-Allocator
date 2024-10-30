# Obreja Ana-Maria

- **Grupa:** 311CAb
- **An universitar:** 2022-2023

---

# Virtual Memory Allocator - Tema 1

## Functionalitate

Programul simulează un alocator de memorie format dintr-o arenă generică, care 
limitează modificările într-un interval specific.

### Arena și blocurile de memorie

- Arena este compusă din blocuri de memorie continuă, cu spațiu între ele, 
astfel încât să fie considerate blocuri separate.
- În funcție de dimensiunea memoriei alocate și poziționarea între blocurile 
existente, se disting următoarele cazuri:

  - **Alocare între două blocuri cu spațiu disponibil:** Memoria devine un nou 
    bloc și este adăugată în lista de blocuri.
  - **Alocare adiacentă unui bloc succesor:** Zona de memorie devine un 
    minibloc și este plasată în lista de miniblocuri a blocului succesor. Se 
    ajustează dimensiunea blocului.
  - **Alocare adiacentă unui bloc predecesor:** Zona de memorie este adăugată 
    la finalul listei de miniblocuri a blocului predecesor, iar dimensiunea 
    acestuia se modifică.
  - **Alocare între două blocuri existente:** Zona devine ultimul minibloc al 
    predecesorului, iar miniblocurile succesorului sunt adăugate la lista 
    predecesorului. Blocul succesor este șters din arenă.

### Dealocarea memoriei

Pentru a elibera o zonă de memorie, am definit următoarele cazuri:

  - **Dealocarea unui minibloc unic:** Blocul care conține lista de miniblocuri 
    este eliminat.
  - **Dealocarea unui minibloc la începutul sau sfârșitul listei:** Dimensiunea 
    și adresa de început a blocului se modifică.
  - **Dealocarea unui minibloc din interiorul listei:** Miniblocul este eliminat,
    formându-se două liste: una rămâne în blocul inițial, iar cealaltă devine un 
    bloc nou. Dimensiunile și adresele fiecărui bloc și listă se actualizează.

### Scrierea și citirea informației

- Într-un minibloc se poate scrie informație cu lungime maximă dimensiunea 
  acestuia.
- Dacă informația depășește miniblocul, continuă în miniblocurile următoare, 
  respectiv blocul următor (dacă dimensiunea depășește lungimea blocului).
- Citirea folosește același principiu, având grijă să respecte limitele 
  dimensiunii blocului și miniblocului.

### Comanda "PMAP"

Comanda "PMAP" afișează informații generale despre memorie: dimensiunea totală,
numărul de blocuri, miniblocuri și caracteristicile fiecăruia. Pentru aceasta, 
programul parcurge lista de blocuri și miniblocuri.

### Bonus: Permisiuni

- Permisiunile miniblocurilor includ scriere și citire ("RW-") și, opțional, 
  permisiunea de executare.
- Operațiile de citire/scriere/executare sunt condiționate de permisiuni. Dacă 
  un minibloc dintr-un bloc nu are permisiunea necesară, niciun minibloc din 
  acel bloc nu poate primi comanda respectivă.

Programul respectă principiile alocării dinamice; toate resursele sunt eliberate 
la final.

### Observații

Din motive de coding style, comentariile sunt generalizate la nivel de funcție 
pentru a acoperi esențialul și explica raționamentele.
