🍎 Fruital Combat
Zabawna, dynamiczna gra akcji/survival napisana w C++ z pełnym wykorzystaniem najnowszej wersji biblioteki SFML 3.0.2!

🎥 LINK DO FILMU Z GAMEPLAYU -> Obejrzyj na YouTube

Architektura ECS (Entity Component System)
Najważniejszym założeniem i celem technicznym projektu było napisanie gry w oparciu o nowoczesną architekturę ECS. Jest to wzorzec projektowy, który odchodzi od klasycznego i często problematycznego dziedziczenia obiektowego na rzecz kompozycji, opierając się na trzech głównych filarach. Byt (Entity) to jedynie unikalny identyfikator reprezentujący konkretny obiekt w świecie gry, taki jak truskawka czy zmutowany brokuł. Komponenty (Component) to "czyste" pakiety danych przypisywane do Bytów, przechowujące informacje o ich stanie (np. pozycja, prędkość, zdrowie), ale pozbawione jakiejkolwiek logiki. Cała logika gry znajduje się w Systemach (System), które operują na wszystkich Bytach posiadających określony zestaw Komponentów. System ruchu przetwarza na przykład wszystkie obiekty mające pozycję i prędkość, aktualizując ich współrzędne. Takie podejście zapewnia ogromną elastyczność przy dodawaniu nowych mechanik w locie oraz doskonałą optymalizację dzięki strukturze przyjaznej dla pamięci podręcznej procesora.

Zarys fabularny
W sercu Wielkiej Dżungli od tysiącleci panował pokój. Jednak gdy Mroczny Nawóz skaził Ziemię, zwykłe warzywa zyskały świadomość i chcą zdominować cały ogród. Jako gracz musisz wybrać swojego owocowego bohatera i oczyścić grządki z niebezpiecznych mutacji.

Cele gry i postępy
Rozgrywka dzieli się na cele krótko i długoterminowe. Podstawowym, krótkoterminowym zadaniem w trakcie każdego podejścia jest przetrwanie jak najdłużej i odpieranie kolejnych fal nacierających warzyw. Długoterminowa meta-progresja opiera się natomiast na odkrywaniu nowych owoców oraz odblokowywaniu kolejnych wpisów w genetycznym bestiariuszu. Dzięki temu gracz ma podgląd na to, jakie potwory sam wyewoluował i zdołał pokonać.

Sklepy i Ekonomia
Gra oferuje rozbudowany system ekonomii podzielony na dwa etapy. Pierwszym z nich jest sklep w lobby, oparty na meta-progresji. Płaci się w nim Złotym Nektarem, który ma określoną szansę na wypadnięcie z pokonanych bossów oraz elitarnych warzyw. Za tę walutę kupujemy ulepszenia globalne, odblokowujemy nowe postacie oraz nabywamy modyfikatory ziemi, czyli Nawozy. Nawozy kupuje się przed startem poziomu, a ich działanie kończy się wraz z jego ukończeniem.

Drugim elementem jest sklep w trakcie gry (in-run progression). Co falę rozgrywka zatrzymuje się, a gracz może wydać zwykły nektar zebrany z pokonanych przeciwników. Oferta obejmuje trzy losowe ulepszenia, z możliwością przelosowania puli (reroll). Można tu zwiększyć prędkość, punkty zdrowia czy regenerację many. Dodatkowo istnieje szansa na trafienie rzadkich modyfikatorów, takich jak zatrucia, dodatkowe strzały, zamrożenie czy wampiryzm, a także niezwykle rzadkich ulepszeń unikalnych, dedykowanych konkretnej postaci.

System Walki i Umiejętności
Istotnym aspektem przetrwania jest zarządzanie Nawodnieniem, czyli systemem many reprezentowanym przez żółty pasek. Mana odnawia się pasywnie w bardzo wolnym tempie, jednak można ją szybko zregenerować podnosząc Krople rosy upuszczane przez pokonane warzywa. Wymusza to na graczu ciągły ruch i dynamiczne pozycjonowanie na planszy.

Każda z grywalnych postaci charakteryzuje się unikalnymi statystykami i zestawem umiejętności. Poza podstawowym strzelaniem, owoce dysponują zdolnościami pochłaniającymi większe ilości many, takimi jak błyskawiczny dash u truskawki, oraz potężnym, ładowanym atakiem ostatecznym (ult). Odpowiednie łączenie tych zdolności z ulepszeniami ze sklepu jest kluczem do przetrwania na późniejszych falach.

Technologie
Cały projekt został stworzony z wykorzystaniem języka C++ w standardzie 17. Wykorzystano nowoczesną bibliotekę graficzną SFML w wersji 3.0.2, a głównym środowiskiem programistycznym użytym do pracy nad grą był Visual Studio 2022.
